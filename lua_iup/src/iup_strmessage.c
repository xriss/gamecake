/** \file
 * \brief String Utilities
 *
 * See Copyright Notice in "iup.h"
 */

 
#include <string.h>  
#include <stdlib.h>  
#include <stdio.h>  
#include <limits.h>  
#include <stdarg.h>

#include "iup.h"

#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_strmessage.h"
#include "iup_table.h"


static Itable *istrmessage_table = NULL;   /* the function hast table indexed by the name string */

void iupStrMessageInit(void)
{
  istrmessage_table = iupTableCreate(IUPTABLE_STRINGINDEXED);
}

void iupStrMessageFinish(void)
{
  iupTableDestroy(istrmessage_table);
  istrmessage_table = NULL;
}

char* iupStrMessageGet(const char* message)
{
  return (char*)iupTableGet(istrmessage_table, message);
}

static void iStrMessageSet(const char* message, const char* str)
{
  iupTableSet(istrmessage_table, message, (char*)str, IUPTABLE_POINTER);
}

void iupStrMessageShowError(Ihandle* parent, const char* message)
{
  Ihandle* dlg = IupMessageDlg();
  char* title = NULL, *str_message;

  if (parent)
  {
    IupSetAttributeHandle(dlg, "PARENTDIALOG", parent);
    title = IupGetAttribute(parent, "TITLE");
  }

  if (!title)
    title = iupStrMessageGet("IUP_ERROR");

  IupSetAttribute(dlg, "TITLE", title);
  IupSetAttribute(dlg, "DIALOGTYPE", "ERROR");
  IupSetAttribute(dlg, "BUTTONS", "OK");

  str_message = iupStrMessageGet(message);
  if (!str_message)
    str_message = (char*)message;
  IupSetAttribute(dlg, "VALUE", str_message);

  IupPopup(dlg, IUP_CURRENT, IUP_CURRENT);

  IupDestroy(dlg);
}

typedef struct _IstdMessage
{
  const char* code;
  const char* lng_msg[3]; /* 2+1 for expansion */
} IstdMessage;

/* Edit this table to add support for more languages */

static IstdMessage iStdMessages[] =
{
  {"IUP_ERROR", {"Error", "Erro", NULL}},
  {"IUP_YES", {"Yes", "Sim", NULL}},
  {"IUP_NO", {"No", "N�o", NULL}},
  {"IUP_INVALIDDIR", {"Invalid directory.", "Diret�rio inv�lido.", NULL}},
  {"IUP_FILEISDIR", {"The selected name is a directory.", "O nome selecionado � um diret�rio.", NULL}},
  {"IUP_FILENOTEXIST", {"File does not exist.", "Arquivo inexistente.", NULL}},
  {"IUP_FILEOVERWRITE", {"Overwrite existing file?", "Sobrescrever arquivo?", NULL}},
  {"IUP_CREATEFOLDER", {"Create Folder", "Criar Diret�rio", NULL}},
  {"IUP_NAMENEWFOLDER", {"Name of the new folder:", "Nome do novo diret�rio:", NULL}},
  {"IUP_SAVEAS", {"Save As", "Salvar Como", NULL}},
  {"IUP_OPEN", {"Open", "Abrir", NULL}},
  {"IUP_SELECTDIR", {"Select Directory", "Selecionar Diret�rio", NULL}},
  {"IUP_OK", {"OK", "OK", NULL}},
  {"IUP_CANCEL", {"Cancel", "Cancelar", NULL}},
  {"IUP_GETCOLOR", {"Color Selection", "Sele��o de Cor", NULL}},
  {"IUP_HELP", {"Help", "Ajuda", NULL}},
  {"IUP_RED", {"&Red:", "&Vermelho:", NULL}},
  {"IUP_GREEN", {"&Green:", "V&erde:", NULL}},
  {"IUP_BLUE", {"&Blue:", "&Azul:", NULL}},
  {"IUP_HUE", {"&Hue:", "&Matiz:", NULL}},
  {"IUP_SATURATION", {"&Saturation:", "&Satura��o:", NULL}},
  {"IUP_INTENSITY", {"&Intensity:", "&Intensidade:", NULL}},
  {"IUP_OPACITY", {"&Opacity:", "&Opacidade:", NULL}},
  {"IUP_PALETTE", {"&Palette:", "&Paleta:", NULL}},
  {"IUP_TRUE", {"True", "Verdadeiro", NULL}},
  {"IUP_FALSE", {"False", "Falso", NULL}},
  {"IUP_FAMILY", {"Family:", "Fam�lia:", NULL}},
  {"IUP_STYLE", {"Style:", "Estilo:", NULL}},
  {"IUP_SIZE", {"Size:", "Tamanho:", NULL}},
  {"IUP_SAMPLE", {"Sample:", "Exemplo:", NULL}},
  {NULL, {NULL, NULL, NULL}}
};

static void iStrRegisterInternalMessages(int lng)
{
  IstdMessage* messages = iStdMessages;
  while (messages->code)
  {
    iStrMessageSet(messages->code, messages->lng_msg[lng]);
    messages++;
  }
}

void iupStrMessageUpdateLanguage(const char* language)
{
  int lng = 0;
  if (iupStrEqualNoCase(language, "PORTUGUESE"))
    lng = 1;
  iStrRegisterInternalMessages(lng);
}

void IupSetLanguage(const char *language)
{
  IupStoreGlobal("LANGUAGE", language);
}

char *IupGetLanguage(void)
{
  return IupGetGlobal("LANGUAGE");
}
