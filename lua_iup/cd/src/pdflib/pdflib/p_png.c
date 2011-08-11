/*---------------------------------------------------------------------------*
 |              PDFlib - A library for generating PDF on the fly             |
 +---------------------------------------------------------------------------+
 | Copyright (c) 1997-2009 Thomas Merz and PDFlib GmbH. All rights reserved. |
 +---------------------------------------------------------------------------+
 |                                                                           |
 |    This software is subject to the PDFlib license. It is NOT in the       |
 |    public domain. Extended versions and commercial licenses are           |
 |    available, please check http://www.pdflib.com.                         |
 |                                                                           |
 *---------------------------------------------------------------------------*/

/* $Id: p_png.c,v 1.2 2009/10/20 18:14:16 scuri Exp $
 *
 * PNG processing for PDFlib
 *
 */

#include "p_intern.h"
#include "p_color.h"
#include "p_image.h"

#if defined(__ia64__) && defined (__linux__)
#define PDF_ALIGN16
#endif

#ifdef HAVE_LIBPNG

static void
pdf_data_source_PNG_init(PDF *p, PDF_data_source *src)
{
    pdf_image   *image = (pdf_image *) src->private_data;

    (void) p;

    image->info.png.cur_line = 0;
    src->buffer_length = image->info.png.rowbytes;
}

#undef min
#define min(a, b) (((a) < (b)) ? (a) : (b))

static pdc_bool
pdf_data_source_PNG_fill(PDF *p, PDF_data_source *src)
{
    pdf_image	*image = (pdf_image *) src->private_data;

    PDC_TRY(p->pdc)
    {
	if (image->info.png.cur_line == image->height)
	{
	    PDC_EXIT_TRY(p->pdc);
	    return pdc_false;
	}

	src->next_byte = image->info.png.raster +
	    image->info.png.cur_line * image->info.png.rowbytes;

	src->bytes_available = src->buffer_length;

	image->info.png.cur_line++;
    }
    PDC_CATCH(p->pdc)
    {
        image->corrupt = pdc_true;
    }

    return !image->corrupt;
}

static void
pdf_data_source_PNG_terminate(PDF *p, PDF_data_source *src)
{
    (void) p;
    (void) src;
}

static void
pdf_png_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
    pdc_file *fp = (pdc_file *) png_ptr->io_ptr;
    char *filename = (char *) pdc_file_name(fp);

    if (!PDC_OK_FREAD(fp, data, length))
    {
        pdc_core *pdc = pdc_file_getpdc(fp);

        pdc_error(pdc, PDF_E_IMAGE_CORRUPT, "PNG", filename, 0, 0);
    }
}

/*
 * We suppress libpng's warning message by supplying
 * our own error and warning handlers
*/
static void
pdf_libpng_warning_handler(png_structp png_ptr, png_const_charp message)
{
    PDF *p = (PDF *)png_ptr->mem_ptr;
    pdc_logg_cond(p->pdc, 5, trc_image, "\tlibpng warning: %s\n", message);
}

/*
 * use the libpng portability aid in an attempt to overcome version differences
 */
#ifndef png_jmpbuf
#define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#endif

static void
pdf_libpng_error_handler(png_structp png_ptr, png_const_charp message)
{
    PDF *p = (PDF *)png_ptr->mem_ptr;

#ifdef PDF_ALIGN16
    jmp_buf jbuf;
#endif

    pdc_logg_cond(p->pdc, 5, trc_image, "\tlibpng error: %s\n", message);

#ifdef PDF_ALIGN16
    memcpy(jbuf, png_jmpbuf(png_ptr), sizeof (jmp_buf));
    longjmp(jbuf, 1);
#else
    longjmp(png_jmpbuf(png_ptr), 1);
#endif
}

static void *
pdf_libpng_malloc(png_structp png_ptr, size_t size)
{
    PDF *p = (PDF *)png_ptr->mem_ptr;

    return pdc_malloc(p->pdc, size, "libpng");
}

static void
pdf_libpng_free(png_structp png_ptr, void *mem)
{
    PDF *p = (PDF *)png_ptr->mem_ptr;

    pdc_free(p->pdc, mem);
}

pdc_bool
pdf_is_PNG_file(PDF *p, pdc_file *fp)
{
    pdc_byte sig[8];

    pdc_logg_cond(p->pdc, 1, trc_image, "\tChecking image type PNG...\n");

    if (!PDC_OK_FREAD(fp, sig, 8) || !png_check_sig(sig, 8)) {
        pdc_fseek(fp, 0L, SEEK_SET);
        return pdc_false;
    }
    return pdc_true;
}

int
pdf_process_PNG_data(
    PDF *p,
    int imageslot)
{
    static const char *fn = "pdf_process_PNG_data";
    pdc_file *save_fp;
    size_t bytecount;
#ifdef PDF_ALIGN16
    jmp_buf jbuf;
#endif

    png_uint_32 width, height, ui;
    png_bytep *row_pointers = NULL, trans;
    png_color_8p sig_bit;
    png_color_16p trans_values;
    int bit_depth, color_type, i, num_trans;
    pdc_scalar dpi_x, dpi_y;
    pdf_image *image;
    volatile int errcode = 0;
    pdf_colorspace cs;
    pdf_colormap * volatile colormap = NULL;
    volatile int slot;

    image = &p->images[imageslot];

    /*
     * We can install our own memory handlers in libpng since
     * our PNG library is specially extended to support this.
     * A custom version of libpng without support for
     * png_create_read_struct_2() is no longer supported.
     */

    image->info.png.png_ptr =
	png_create_read_struct_2(PNG_LIBPNG_VER_STRING, (png_voidp) NULL,
        (png_error_ptr) pdf_libpng_error_handler,
        (png_error_ptr) pdf_libpng_warning_handler,
	p,
        (png_malloc_ptr) pdf_libpng_malloc,
	(png_free_ptr) pdf_libpng_free);

    if (!image->info.png.png_ptr)
    {
        pdc_error(p->pdc, PDC_E_MEM_OUT, fn, 0, 0, 0);
    }

    image->info.png.info_ptr = png_create_info_struct(image->info.png.png_ptr);

    if (image->info.png.info_ptr == NULL)
    {
	png_destroy_read_struct(&image->info.png.png_ptr,
	    (png_infopp) NULL, (png_infopp) NULL);
        pdc_error(p->pdc, PDC_E_MEM_OUT, fn, 0, 0, 0);
    }

    /* due to alignment bug on itanium machines:
    ** use well aligned local jbuf instead of sometimes
    ** bad aligned (allocated) jmp_buf.
    */
#ifdef PDF_ALIGN16
    if (setjmp(jbuf))
#else
    if (setjmp(png_jmpbuf(image->info.png.png_ptr)))
#endif
    {
        errcode = PDF_E_IMAGE_CORRUPT;
        goto PDF_PNG_ERROR;
    }
#ifdef PDF_ALIGN16
    memcpy(png_jmpbuf(image->info.png.png_ptr), jbuf, sizeof (jmp_buf));
#endif

    if (pdf_is_PNG_file(p, image->fp) == pdc_false)
    {
        errcode = PDC_E_IO_BADFORMAT;
        goto PDF_PNG_ERROR;
    }

    /* from file or from memory */
    png_set_read_fn(image->info.png.png_ptr, image->fp,
                    (png_rw_ptr) pdf_png_read_data);

    png_set_sig_bytes(image->info.png.png_ptr, 8);
    png_read_info(image->info.png.png_ptr, image->info.png.info_ptr);
    png_get_IHDR(image->info.png.png_ptr, image->info.png.info_ptr,
	    &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);

    image->width        = (pdc_scalar) width;
    image->height       = (pdc_scalar) height;

    /* reduce 16-bit images to 8 bit since PDF < 1.5 stops at 8 bit */
    if (p->compatibility < PDC_1_5 && bit_depth == 16)
    {
	png_set_strip_16(image->info.png.png_ptr);
	bit_depth = 8;
    }

    image->bpc		= bit_depth;

    /*
     * We currently don't support a real alpha channel but only binary
     * tranparency ("poor man's alpha"). For palette images we do our best and
     * treat alpha values of up to 50% as transparent, and values above 50%
     * as opaque. Gray and RGB images with an associated alpha channel will
     * be pre-multiplied by libpng (against white background).
     */
#define ALPHA_THRESHOLD 128

    switch (color_type)
    {
	case PNG_COLOR_TYPE_GRAY_ALPHA:
	    /* LATER: construct mask from alpha channel */
	    /*
	    png_set_IHDR(image->info.png.png_ptr, image->info.png.info_ptr,
	    	width, height, bit_depth, PNG_COLOR_MASK_ALPHA,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT);
	    */
	    /*
	     * We strip the alpha channel, and let libpng pre-multiply
	     * the opacity values to the image data.
	     */
	    png_set_strip_alpha(image->info.png.png_ptr);
	    /* fall through */

	case PNG_COLOR_TYPE_GRAY:
	    if (png_get_sBIT(image->info.png.png_ptr,
	    		image->info.png.info_ptr, &sig_bit))
            {
		png_set_shift(image->info.png.png_ptr, sig_bit);
	    }

		image->colorspace	= DeviceGray;

	    image->components	= 1;
	    break;

	case PNG_COLOR_TYPE_RGB_ALPHA:
	    /* LATER: construct mask from alpha channel */
	    /*
	     * We strip the alpha channel, and let libpng pre-multiply
	     * the opacity values to the image data.
	     */
	    png_set_strip_alpha(image->info.png.png_ptr);
	    /* fall through */

	case PNG_COLOR_TYPE_RGB:
	    if (image->colorspace == pdc_undef)
		image->colorspace	= DeviceRGB;
	    image->components	= 3;

	    break;

	case PNG_COLOR_TYPE_PALETTE:
        {
            png_colorp pcm;

	    png_get_PLTE(image->info.png.png_ptr, image->info.png.info_ptr,
		&pcm, &cs.val.indexed.palette_size);

	    colormap =
		(pdf_colormap *) pdc_malloc(p->pdc, sizeof(pdf_colormap), fn);

	    /* This seems redundant, but the png_colorp structure may not
	     * be packed on some platforms.
	     */
	    for (i = 0; i < cs.val.indexed.palette_size; i++)
	    {
		(*colormap)[i][0] = (pdc_byte) pcm[i].red;
		(*colormap)[i][1] = (pdc_byte) pcm[i].green;
		(*colormap)[i][2] = (pdc_byte) pcm[i].blue;
	    }

	    image->components = 1;

	    /* This case should arguably be prohibited. However, we allow
	     * it and take the palette indices 0 and 1 as the mask,
	     * disregarding any color values in the palette.
	     */
	    if (image->imagemask) {
		image->colorspace = DeviceGray;
		break;
	    }

		cs.type = Indexed;
		cs.val.indexed.base = DeviceRGB;
		cs.val.indexed.colormap = colormap;
		cs.val.indexed.colormap_id = PDC_BAD_ID;
		slot = pdf_add_colorspace(p, &cs, pdc_false);

		image->colorspace = slot;

        }
	break;
    }
    if (colormap)
       pdc_free(p->pdc, colormap);

    if (image->imagemask)
    {
	if (image->components != 1)
        {
	    errcode = PDF_E_IMAGE_BADMASK;
	    goto PDF_PNG_ERROR;
	}

	if (p->compatibility <= PDC_1_3) {
	    if (image->components != 1 || image->bpc != 1)
            {
		errcode = PDF_E_IMAGE_MASK1BIT13;
		goto PDF_PNG_ERROR;
	    }
	}
        else if (image->bpc > 1)
        {
	    /* images with more than one bit will be written as /SMask,
	     * and don't require an /ImageMask entry.
	     */
	    image->imagemask = pdc_false;
	}
	image->colorspace = DeviceGray;
    }

    /* we invert this flag later */
    if (image->ignoremask)
        image->transparent = pdc_true;

    /* let libpng expand interlaced images */
    (void) png_set_interlace_handling(image->info.png.png_ptr);

    /* read the physical dimensions chunk to find the resolution values */
    dpi_x = (pdc_scalar) png_get_x_pixels_per_meter(image->info.png.png_ptr,
    		image->info.png.info_ptr);
    dpi_y = (pdc_scalar) png_get_y_pixels_per_meter(image->info.png.png_ptr,
    		image->info.png.info_ptr);

    if (dpi_x != 0 && dpi_y != 0)
    {	/* absolute values */
        image->dpi_x = dpi_x * PDC_INCH2METER;
        image->dpi_y = dpi_y * PDC_INCH2METER;

    }
    else
    {				/* aspect ratio */
	image->dpi_y = -png_get_pixel_aspect_ratio(image->info.png.png_ptr,
			    image->info.png.info_ptr);

	if (image->dpi_y == 0)	/* unknown */
	    image->dpi_x = 0;
	else
	    image->dpi_x = -1.0;
    }

    /* read the transparency chunk */
    if (png_get_valid(image->info.png.png_ptr, image->info.png.info_ptr,
    	PNG_INFO_tRNS))
    {
	png_get_tRNS(image->info.png.png_ptr, image->info.png.info_ptr,
	    &trans, &num_trans, &trans_values);
	if (num_trans > 0)
        {
	    if (color_type == PNG_COLOR_TYPE_GRAY)
            {
		image->transparent = !image->transparent;
		/* LATER: scale down 16-bit transparency values ? */
		image->transval[0] = (pdc_byte) trans_values[0].gray;

	    }
            else if (color_type == PNG_COLOR_TYPE_RGB)
            {
		image->transparent = !image->transparent;
		/* LATER: scale down 16-bit transparency values ? */
		image->transval[0] = (pdc_byte) trans_values[0].red;
		image->transval[1] = (pdc_byte) trans_values[0].green;
		image->transval[2] = (pdc_byte) trans_values[0].blue;

	    }
            else if (color_type == PNG_COLOR_TYPE_PALETTE)
            {
		/* we use the first transparent entry in the tRNS palette */
		for (i = 0; i < num_trans; i++)
                {
		    if ((pdc_byte) trans[i] < ALPHA_THRESHOLD)
                    {
			image->transparent = !image->transparent;
			image->transval[0] = (pdc_byte) i;
			break;
		    }
		}
	    }
	}
    }




    png_read_update_info(image->info.png.png_ptr, image->info.png.info_ptr);

    image->info.png.rowbytes =
	png_get_rowbytes(image->info.png.png_ptr, image->info.png.info_ptr);

    /* Check for overflow */
    bytecount = image->info.png.rowbytes * height;
    if (bytecount / height != image->info.png.rowbytes)
    {
	errcode = PDF_E_IMAGE_TOO_LARGE;
	goto PDF_PNG_ERROR;
    }

    image->info.png.raster = (pdc_byte *)
	pdc_calloc(p->pdc,image->info.png.rowbytes * height, fn);

    row_pointers = (png_bytep *)
	pdc_malloc(p->pdc, height * sizeof(png_bytep), fn);

    for (ui = 0; ui < height; ui++)
    {
	row_pointers[ui] =
	    image->info.png.raster + ui * image->info.png.rowbytes;
    }

    /* try the simple way:
    */
    save_fp = image->fp;

    image->src.init		= pdf_data_source_PNG_init;
    image->src.fill		= pdf_data_source_PNG_fill;
    image->src.terminate	= pdf_data_source_PNG_terminate;
    image->src.private_data	= (void *) image;


    image->fp = pdc_fsearch_fopen(p->pdc, image->filename, NULL, NULL,
                                  PDC_FILE_BINARY);
    if (image->fp != (pdc_file *) 0)
	pdc_fclose(image->fp);

    image->fp = save_fp;

    /* Provide a suitable background for images with alpha channel */
    if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA ||
	color_type == PNG_COLOR_TYPE_RGB_ALPHA)
    {
	png_color_16p image_background;

	if (png_get_bKGD(image->info.png.png_ptr, image->info.png.info_ptr,
			&image_background))
	{
	    png_set_background(image->info.png.png_ptr, image_background,
		PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
	}
	else
	{
	    png_color_16 my_white;

	    if (bit_depth == 8)
	    {
		my_white.red	= 0xFF;
		my_white.green	= 0xFF;
		my_white.blue	= 0xFF;
		my_white.gray	= 0xFF;
	    }
	    else
	    {
		my_white.red	= 0xFFFF;
		my_white.green	= 0xFFFF;
		my_white.blue	= 0xFFFF;
		my_white.gray	= 0xFFFF;
	    }

	    png_set_background(image->info.png.png_ptr, &my_white,
		PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);
	}
    }

    /* fetch the actual image data */
    png_read_image(image->info.png.png_ptr, row_pointers);

    image->in_use		= pdc_true;		/* mark slot as used */

    pdf_put_image(p, imageslot, pdc_true, pdc_true);

    pdc_free(p->pdc, image->info.png.raster);
    if (row_pointers != NULL)
        pdc_free(p->pdc, row_pointers);

    png_destroy_read_struct(&image->info.png.png_ptr,
	    &image->info.png.info_ptr, NULL);

    if (!image->corrupt)
        return imageslot;

    PDF_PNG_ERROR:
    {
        const char *stemp = NULL;

        if (errcode)
        {
            png_destroy_read_struct(&image->info.png.png_ptr,
                    &image->info.png.info_ptr, NULL);
            stemp = pdf_get_image_filename(p, image);
        }

        switch (errcode)
        {
            case PDF_E_IMAGE_ICC:
            case PDF_E_IMAGE_ICC2:
            case PDF_E_IMAGE_MASK1BIT13:
            case PDF_E_IMAGE_BADMASK:
		pdc_set_errmsg(p->pdc, errcode, stemp, 0, 0, 0);
		break;

            case PDC_E_IO_BADFORMAT:
		pdc_set_errmsg(p->pdc, errcode, stemp, "PNG", 0, 0);
		break;

            case PDF_E_IMAGE_CORRUPT:
		pdc_set_errmsg(p->pdc, errcode, "PNG", stemp, 0, 0);
		break;

            case PDF_E_IMAGE_TOO_LARGE:
		pdc_set_errmsg(p->pdc, errcode, "PNG", stemp,
		    pdc_errprintf(p->pdc, "%ld", (long) image->width),
		    pdc_errprintf(p->pdc, "%ld", (long) image->height));
		break;

	    case 0: 		/* error code and message already set */
		break;
        }
    }

    return -1;
}

#else	/* !HAVE_LIBPNG */

pdc_bool
pdf_is_PNG_file(PDF *p, pdc_file *fp)
{
    (void) p;
    (void) fp;

    return pdc_false;
}

int
pdf_process_PNG_data(PDF *p, int imageslot)
{
    (void) imageslot;

    pdc_set_errmsg(p->pdc, PDF_E_UNSUPP_IMAGE, "PNG", 0, 0, 0);

    return -1;
}

#endif	/* !HAVE_LIBPNG */
