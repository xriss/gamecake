/** \file
 * \brief Cairo/GTK Printer Driver  (UNIX Only)
 *
 * See Copyright Notice in cd.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>
#if GTK_CHECK_VERSION(2, 10, 0)
#include <gtk/gtkprintunixdialog.h>
#endif

#include "cdcairoctx.h"
#include "cdprint.h"

#if GTK_CHECK_VERSION(2, 10, 0)
static gboolean print_enum(GtkPrinter *printer, GtkPrinter **ret_printer)
{
  if (gtk_printer_is_default(printer))
  {
    *ret_printer = printer;
    g_object_ref(printer);
    return TRUE;
  }
  return FALSE;
}

static void finish_send(GtkPrintJob *job, GMainLoop* loop, GError *error)
{
  if (error != NULL)
  {
    GtkWidget *edialog;
    edialog = gtk_message_dialog_new (NULL, 
                                      GTK_DIALOG_DESTROY_WITH_PARENT,
                                      GTK_MESSAGE_ERROR,
                                      GTK_BUTTONS_CLOSE,
                                      "Error printing");
    gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (edialog), "%s", error->message);
    gtk_window_set_modal (GTK_WINDOW (edialog), TRUE);
    g_signal_connect(edialog, "response", G_CALLBACK (gtk_widget_destroy), NULL);

    gtk_window_present(GTK_WINDOW(edialog));
  }

  g_main_loop_quit(loop);
}

static void cdkillcanvas(cdCtxCanvas *ctxcanvas)
{
  GMainLoop* loop = g_main_loop_new (NULL, FALSE);

  cairo_surface_finish(cairo_get_target(ctxcanvas->cr));

  gtk_print_job_send(ctxcanvas->job, (GtkPrintJobCompleteFunc)finish_send, loop, NULL);

  g_main_loop_run(loop);
  g_main_loop_unref(loop);

  cdcairoKillCanvas(ctxcanvas);
}

static char* get_printername_attrib(cdCtxCanvas* ctxcanvas)
{
  return (char*)gtk_printer_get_name(gtk_print_job_get_printer(ctxcanvas->job));
}

static cdAttribute printername_attrib =
{
  "PRINTERNAME",
  NULL,
  get_printername_attrib
}; 

static void cdcreatecanvas(cdCanvas* canvas, void *data)
{
  cdCtxCanvas* ctxcanvas;
  char *data_str = (char*) data;
  char docname[256] = "CD - Canvas Draw Document";
  GtkPrintUnixDialog* dialog = NULL;
  GtkPrinter* printer;
  GtkPrintSettings* settings;
  GtkPageSetup* page_setup = NULL;
  GtkPrintJob* job;
  int show_dialog = 0;

  /* Starting parameters */
  if (data_str == NULL) 
    return;

  if (data_str[0] != 0)
  {
    char *ptr = strstr(data_str, "-d");

    if (ptr != NULL)
      show_dialog = 1;

    if (data_str[0] != '-')
    {
      strcpy(docname, data_str);

      if (show_dialog)
        docname[ptr - data_str - 1] = 0;
    }
  }

  if (show_dialog)
  {
    int response;

    dialog = (GtkPrintUnixDialog*)gtk_print_unix_dialog_new(NULL, NULL);

    gtk_print_unix_dialog_set_manual_capabilities(dialog,
						   GTK_PRINT_CAPABILITY_PAGE_SET |
						   GTK_PRINT_CAPABILITY_COPIES |
						   GTK_PRINT_CAPABILITY_COLLATE |
						   GTK_PRINT_CAPABILITY_REVERSE |
						   GTK_PRINT_CAPABILITY_SCALE);

    gtk_widget_realize(GTK_WIDGET(dialog));

    response = gtk_dialog_run(GTK_DIALOG(dialog));

    if (response == GTK_RESPONSE_CANCEL)
    {
      gtk_widget_destroy(GTK_WIDGET(dialog));  
      return;
    }

    printer = gtk_print_unix_dialog_get_selected_printer(dialog);
    settings = gtk_print_unix_dialog_get_settings(dialog);
    page_setup = gtk_print_unix_dialog_get_page_setup(dialog);
  }
  else
  {
    printer = NULL;
    gtk_enumerate_printers((GtkPrinterFunc)print_enum, &printer, NULL, TRUE);
    if (!printer)
      return;
#if GTK_CHECK_VERSION(2, 13, 0)
    page_setup = gtk_printer_get_default_page_size(printer);
#endif
    if (!page_setup)
      page_setup = gtk_page_setup_new();  /* ?????? */

    settings = gtk_print_settings_new();  /* ?????? */
  }

  job = gtk_print_job_new(docname, printer, settings, page_setup);

  canvas->w_mm = (int)gtk_page_setup_get_page_width(page_setup, GTK_UNIT_MM);
  canvas->h_mm = (int)gtk_page_setup_get_page_height(page_setup, GTK_UNIT_MM);
  canvas->bpp  = 24;
#if GTK_CHECK_VERSION(2, 16, 0)
  canvas->xres = (double)gtk_print_settings_get_resolution_x(settings) / 25.4;
  canvas->yres = (double)gtk_print_settings_get_resolution_y(settings) / 25.4;
#else
  canvas->xres = (double)gtk_print_settings_get_int (settings, GTK_PRINT_SETTINGS_RESOLUTION) / 25.4;
  canvas->yres = (double)gtk_print_settings_get_int (settings, GTK_PRINT_SETTINGS_RESOLUTION) / 25.4;
#endif
  canvas->w = cdRound(canvas->w_mm*canvas->xres);
  canvas->h = cdRound(canvas->h_mm*canvas->yres);

  ctxcanvas = cdcairoCreateCanvas(canvas, cairo_create(gtk_print_job_get_surface(job, NULL)));
  ctxcanvas->job = job;

  cairo_identity_matrix(ctxcanvas->cr);
  cairo_scale(ctxcanvas->cr, 0.25, 0.25);  /* TODO: why this is needed? */

  cdRegisterAttribute(canvas, &printername_attrib);

  if (dialog)
    gtk_widget_destroy(GTK_WIDGET(dialog)); 

  g_object_unref(settings);
}
#else
static void cdcreatecanvas(cdCanvas* canvas, void *data)
{
  /* do nothing */
}
#endif

static void cdinittable(cdCanvas* canvas)
{
  cdcairoInitTable(canvas);
  
  canvas->cxKillCanvas = cdkillcanvas;
}

static cdContext cdPrinterCairoContext =
{
  CD_CAP_ALL & ~(CD_CAP_PLAY | CD_CAP_YAXIS | CD_CAP_REGION | CD_CAP_GETIMAGERGB |
                 CD_CAP_WRITEMODE | CD_CAP_PALETTE | CD_CAP_IMAGESRV),
  0,
  cdcreatecanvas,
  cdinittable,
  NULL,
  NULL,
};

cdContext* cdContextCairoPrinter(void)
{
  return &cdPrinterCairoContext;
}
