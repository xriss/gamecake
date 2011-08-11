/* $Id: zprefix.h,v 1.2 2009/10/20 18:12:26 scuri Exp $ */

/*
 * If you *really* need a unique prefix for all types and library functions,
 * compile with -DZ_PREFIX. The "standard" zlib should be compiled without it.
 */
/* PDFlib GmbH: We use "pdf_z_".
** The original list was incomplete, by the way.
*/
#ifndef ZPREFIX_H
#define ZPREFIX_H

#define Z_PREFIX
#ifdef	Z_PREFIX

/* redefine names of all functions for integrating into
 * TET/PLOP/PCOS library, to avoid name clashes if used together with
 * pdflib */
#ifdef PDFLIB_TET_BUILD
#define FLATE_PREFIX(x)		tet_z_##x
#define _FLATE_PREFIX(x)	_tet_z_##x
#else
#ifdef PDFLIB_PLOP_BUILD
#define FLATE_PREFIX(x)		plop_z_##x
#define _FLATE_PREFIX(x)	_plop_z_##x
#else
#ifdef PDFLIB_PCOS_BUILD
#define FLATE_PREFIX(x)		pcos_z_##x
#define _FLATE_PREFIX(x)	_pcos_z_##x
#else
#define FLATE_PREFIX(x)		pdf_z_##x
#define _FLATE_PREFIX(x)	_pdf_z_##x
#endif /* PDFLIB_PCOS_BUILD */
#endif /* PDFLIB_PLOP_BUILD */
#endif /* PDFLIB_TET_BUILD */

#  define longest_match     FLATE_PREFIX(longest_match)
#  define match_init     FLATE_PREFIX(match_init)
#  define longest_match_7fff     FLATE_PREFIX(longest_match_7fff)
#  define cpudetect32     FLATE_PREFIX(cpudetect32)
#  define longest_match_686     FLATE_PREFIX(longest_match_686)
#  define inflate_fast     FLATE_PREFIX(inflate_fast)

#  define _longest_match     _FLATE_PREFIX(longest_match)
#  define _match_init     _FLATE_PREFIX(match_init)
#  define _longest_match_7fff     _FLATE_PREFIX(longest_match_7fff)
#  define _cpudetect32     _FLATE_PREFIX(cpudetect32)
#  define _longest_match_686     _FLATE_PREFIX(longest_match_686)
#  define _inflate_fast     _FLATE_PREFIX(inflate_fast)


#  define inflate_copyright     FLATE_PREFIX(inflate_copyright)
#  define inflate_table         FLATE_PREFIX(inflate_table)
#  define _dist_code          FLATE_PREFIX(_dist_code)
#  define _length_code          FLATE_PREFIX(_length_code)
#  define _tr_align          FLATE_PREFIX(_tr_align)
#  define _tr_flush_block          FLATE_PREFIX(_tr_flush_block)
#  define _tr_init          FLATE_PREFIX(_tr_init)
#  define _tr_stored_block          FLATE_PREFIX(_tr_stored_block)
#  define _tr_tally          FLATE_PREFIX(_tr_tally)
#  define zcalloc          FLATE_PREFIX(zcalloc)
#  define zcfree          FLATE_PREFIX(zcfree)
#  define z_errmsg          FLATE_PREFIX(z_errmsg)
#  define z_error          FLATE_PREFIX(z_error)
#  define zlibCompileFlags          FLATE_PREFIX(zlibCompileFlags)
#  define zlibVersion          FLATE_PREFIX(zlibVersion)
#  define z_verbose          FLATE_PREFIX(z_verbose)
#  define inflateGetHeader          FLATE_PREFIX(inflateGetHeader)
#  define inflatePrime          FLATE_PREFIX(inflatePrime)
#  define adler32_combine          FLATE_PREFIX(adler32_combine)
#  define crc32_combine          FLATE_PREFIX(crc32_combine)
#  define deflate_copyright          FLATE_PREFIX(deflate_copyright)
#  define deflateSetHeader          FLATE_PREFIX(deflateSetHeader)
#  define deflateTune          FLATE_PREFIX(deflateTune)

#  define deflateInit_          FLATE_PREFIX(deflateInit_)
#  define deflate               FLATE_PREFIX(deflate)
#  define deflateEnd            FLATE_PREFIX(deflateEnd)
#  define inflateInit_          FLATE_PREFIX(inflateInit_)
#  define inflate               FLATE_PREFIX(inflate)
#  define inflateEnd            FLATE_PREFIX(inflateEnd)
#  define deflateInit2_         FLATE_PREFIX(deflateInit2_)
#  define deflateSetDictionary  FLATE_PREFIX(deflateSetDictionary)
#  define deflateCopy           FLATE_PREFIX(deflateCopy)
#  define deflateReset          FLATE_PREFIX(deflateReset)
#  define deflateParams         FLATE_PREFIX(deflateParams)
#  define deflateBound          FLATE_PREFIX(deflateBound)
#  define deflatePrime          FLATE_PREFIX(deflatePrime)
#  define inflateInit2_         FLATE_PREFIX(inflateInit2_)
#  define inflateSetDictionary  FLATE_PREFIX(inflateSetDictionary)
#  define inflateSync           FLATE_PREFIX(inflateSync)
#  define inflateSyncPoint      FLATE_PREFIX(inflateSyncPoint)
#  define inflateCopy           FLATE_PREFIX(inflateCopy)
#  define inflateReset          FLATE_PREFIX(inflateReset)
#  define inflateBack           FLATE_PREFIX(inflateBack)
#  define inflateBackEnd        FLATE_PREFIX(inflateBackEnd)
#  define compress              FLATE_PREFIX(compress)
#  define compress2             FLATE_PREFIX(compress2)
#  define compressBound         FLATE_PREFIX(compressBound)
#  define uncompress            FLATE_PREFIX(uncompress)
#  define adler32               FLATE_PREFIX(adler32)
#  define crc32                 FLATE_PREFIX(crc32)
#  define get_crc_table         FLATE_PREFIX(get_crc_table)
#  define zError                FLATE_PREFIX(zError)

#if 0
/*
 * PDFlib GmbH: Avoid these redefinitions since they are not required
 * for typedefs, and can break functions of the same name in other
 * modules.
 */
#  define alloc_func            FLATE_PREFIX(alloc_func)
#  define free_func             FLATE_PREFIX(free_func)
#  define in_func               FLATE_PREFIX(in_func)
#  define out_func              FLATE_PREFIX(out_func)
#endif

/* special handling required on the Mac where Byte is alread defined */
#if !(defined(MAC) || defined(MACOSX))
#  define Byte		z_Byte
#endif

#  define uInt		z_uInt
#  define uLong		z_uLong
#  define Bytef		z_Bytef
#  define charf		z_charf
#  define intf		z_intf
#  define uIntf		z_uIntf
#  define uLongf	z_uLongf
#  define voidpf	z_voidpf
#  define voidp		z_voidp
#endif

#endif /* ZPREFIX_H */
