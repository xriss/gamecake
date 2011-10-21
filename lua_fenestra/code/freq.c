/* This file contains all routines for creating and managing a file
 * requestor.  The programmer's only interface to the file requestor
 * is the function GetFile().  See the description for that function
 * for usage information (it's pretty trivial).
 *
 * Originally written by Allen Martin (amartin@cs.wpi.edu).
 *
 * Significant modifications by me (Dominic Giampaolo, dbg@sgi.com)
 * to clean up some bugs, do double-clicks correctly, and make
 * relative path browsing work better.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <limits.h>
#include <sys/times.h>

#include "libsx.h"

#ifndef CLK_TCK
#include <unistd.h>
#define  CLK_TCK  sysconf(_SC_CLK_TCK)    /* seems to work right */
#endif  /* CLK_TCK */


/*
 * Some ugly hacks to make this work better under ultrix.
 */
#ifdef ultrix

/*
 * Can you say "Let's be pinheaded and follow the letter of the spec without
 * regard for what might make sense"?  I knew you could.  And so can the
 * boys and girls at DEC.
 *
 * Which is why they don't provide strdup() in their libc.  Gay or what?
 */
char *strdup(const char *str)
{
  char *new;

  new = malloc(strlen(str)+1);
  if (new)
    strcpy(new, str);

  return new;
}

#endif /* ultrix */


/*
 * No one seems to have a prototype for strdup().  What a pain
 * in the butt.  Why on earth isn't strdup() in the POSIX standard
 * but something completely useless like mbstowcs() is?
 */
char *strdup(const char *str);


/*
 * Here's where the real code begins.
 */

typedef struct {
  Widget freq_window;
  Widget file_path;
  Widget file_name;
  Widget file_list;

  char **dirlist;            /* used by the list widget */
  char fpath[MAXPATHLEN];
  char fname[MAXPATHLEN];
  
  clock_t last_click;        /* time of last click */

} FReqData;

static void load_cancel(Widget w, FReqData *fdata);
static void load_ok(Widget w, FReqData *fdata);
static void load_list(Widget w, char *string, int index, FReqData *fdata);
static void load_dir(Widget w, char *string, FReqData *fdata);
static void load_name(Widget w, char *string, FReqData *fdata);
static int mystrcmp(const void *a, const void *b);

char **get_dir_list(char *pname, int *num);
void free_dirlist(char **table);

/*
 * GetFile() - This is the entry point to the file requestor.  A single
 *             argument is passed - the path name for the initial list.
 *             If this path name is passed as NULL, the current directory
 *             is used instead.  The function returns a character string
 *             that is the name of the selected file, path included.  If
 *             an error occurs, or the user selects CANCEL, NULL is returned.
 */
char *freq_GetFile(const char *_path)
{
  FReqData fdata;
  Widget w[8];
  int num_dir;
  char path[MAXPATHLEN];

  if(!_path || strcmp(_path, ".") == 0 || strcmp(_path, "./") == 0)
    getcwd(path, MAXPATHLEN);
  else
    strcpy(path, _path);
  
  if(path[strlen(path)-1] != '/')
    strcat(path, "/");

  if(!(fdata.dirlist = get_dir_list(path, &num_dir)))
    return(NULL);
  
  qsort(fdata.dirlist, num_dir, sizeof(char *), mystrcmp);

  fdata.freq_window = MakeWindow("File Requestor", SAME_DISPLAY,
				 EXCLUSIVE_WINDOW);

  w[0]  = MakeLabel("Path:");
  w[1]  = MakeStringEntry(path, 300, (void *)load_dir,         &fdata);
  w[2]  = MakeScrollList(fdata.dirlist, 350, 300, (void *)load_list, &fdata);
  w[3]  = MakeLabel("File:");
  w[4]  = MakeStringEntry("", 300, (void *)load_name,          &fdata);
  w[5]  = MakeButton("Ok",         (void *)load_ok,            &fdata);
  w[6]  = MakeLabel("Select a File from the List or Enter a Name");
  w[7]  = MakeButton("Cancel",     (void *)load_cancel,        &fdata);
  
  SetWidgetPos(w[1], PLACE_RIGHT, w[0], NO_CARE,     NULL);
  SetWidgetPos(w[2], PLACE_UNDER, w[0], NO_CARE,     NULL);
  SetWidgetPos(w[3], PLACE_UNDER, w[2], NO_CARE,     NULL);
  SetWidgetPos(w[4], PLACE_UNDER, w[2], PLACE_RIGHT, w[3]);
  SetWidgetPos(w[5], PLACE_UNDER, w[3], NO_CARE,     NULL);
  SetWidgetPos(w[6], PLACE_UNDER, w[3], PLACE_RIGHT, w[5]);
  SetWidgetPos(w[7], PLACE_UNDER, w[3], PLACE_RIGHT, w[6]);

  /* save the file name & file list widgets, so we can use them later */
  fdata.file_path = w[1];
  fdata.file_list = w[2];
  fdata.file_name = w[4];

  fdata.last_click = 0;

  /* set up the file path */
  strcpy(fdata.fpath, path);
  
  ShowDisplay();
  MainLoop();
  
  /* free the directory list */
  if (fdata.dirlist)
    free_dirlist(fdata.dirlist);

  SetCurrentWindow(ORIGINAL_WINDOW);
//  CloseWindow();
CheckForEvent();

  if(fdata.fname[0] == '\0')
    return(NULL);
  else
    return(strdup(fdata.fname));
}

/*
 * load_cancel() - Callback routine for CANCEL button
 */
static void load_cancel(Widget w, FReqData *fdata)
{
  SetCurrentWindow(fdata->freq_window);
  CloseWindow();
  strcpy(fdata->fname, "");
}

/*
 * load_ok() - Callback routine for OK button
 */
static void load_ok(Widget w, FReqData *fdata)
{
  char *fpath, *fname;
  char fullname[MAXPATHLEN];
  
  fpath = GetStringEntry(fdata->file_path);
  fname = GetStringEntry(fdata->file_name);

  sprintf(fullname, "%s%s", fpath, fname);

  /* right here we should check the validity of the file name */
  /* and abort if invalid */

  strcpy(fdata->fname, fullname);

  SetCurrentWindow(fdata->freq_window);
  CloseWindow();
}

/*
 * load_list() - Callback routine for scrollable list widget
 */
static void load_list(Widget w, char *string, int index, FReqData *fdata)
{
  char newpath[MAXPATHLEN], *cptr, *fpath, fullname[MAXPATHLEN];
  char **old_dirlist=NULL;
  static char oldfile[MAXPATHLEN] = { '\0', };
  clock_t cur_click;
  struct tms junk_tms;  /* not used, but passed to times() */
  int num_dir;
  float tdiff;    /* difference in time between two clicks as % of a second */
  
  /*
   * First we check fora double click.
   *
   * If the time between the last click and this click is greater than
   * 0.5 seconds or the last filename and the current file name
   * are different, then it's not a double click, so we just return.
   */
  cur_click = times(&junk_tms);
  tdiff = ((float)(cur_click - fdata->last_click) / CLK_TCK);

  if(tdiff > 0.50 || strcmp(oldfile, string) != 0)
   {
     fdata->last_click = cur_click;
     strcpy(oldfile, string);
     SetStringEntry(fdata->file_name, string);
     return;
   }
  
  /* check if a directory was selected */
  if(string[strlen(string)-1] != '/')   /* a regular file double click */
   {
     fpath = GetStringEntry(fdata->file_path);
     
     sprintf(fullname, "%s%s", fpath, string);
     
     /* right here we should check the validity of the file name */
     /* and abort if invalid */
     
     strcpy(fdata->fname, fullname);
     
     SetCurrentWindow(fdata->freq_window);
     CloseWindow();
     return;
   }

  /*
   * Else, we've got a directory name and should deal with it
   * as approrpriate.
   */

  /* check for special cases "./" and "../" */
  if(strcmp(string, "./") == 0)
   {
     if (fdata->fpath)
       strcpy(newpath, fdata->fpath);
     else
       strcpy(newpath, "./");
   }
  else if(strcmp(string, "../") == 0)
   {
     strcpy(newpath, fdata->fpath);
     
     if (strcmp(newpath, "./") == 0)
       strcpy(newpath, string);
     else 
      {
	/*
	 * chop off the last path component and look at what it 
	 * is to determine what to do with the `..' we just got.
	 */
	cptr = strrchr(newpath, '/');
	if (cptr)
	  *cptr = '\0';
	cptr = strrchr(newpath, '/');
	if (cptr)
	  *cptr = '\0';

	if (  (cptr != NULL && strcmp(cptr+1,  "..") == 0)
	    ||(cptr == NULL && strcmp(newpath, "..") == 0))
	 {
	   if (cptr)
	     *cptr = '/';

	   strcat(newpath, "/");      /* put back the / we took out */
	   strcat(newpath, "../");    /* and append the new ../ */
	 }
	else
	 {
	   if(cptr == NULL && strcmp(fdata->fpath, "/") == 0)
	     strcpy(newpath, "/");
	   else if (cptr == NULL)
	     strcpy(newpath, "./");

	   if (newpath[strlen(newpath)-1] != '/')
	     strcat(newpath, "/");
	 }
      }
   }
  else /* not a `./' or `../', so it's just a regular old directory name */
   {
     if (fdata->fpath[strlen(fdata->fpath)-1] == '/')
       sprintf(newpath, "%s%s", fdata->fpath, string);
     else
       sprintf(newpath, "%s/%s", fdata->fpath, string);
   }

  old_dirlist = fdata->dirlist;
  if(!(fdata->dirlist = get_dir_list(newpath, &num_dir)))
    /* should beep the display or something here */
    return;
  
  qsort(fdata->dirlist, num_dir, sizeof(char *), mystrcmp);
  
  strcpy(fdata->fpath, newpath);
  SetStringEntry(fdata->file_path, fdata->fpath);
  SetStringEntry(fdata->file_name, "");
  strcpy(fdata->fpath, newpath);
  ChangeScrollList(fdata->file_list, fdata->dirlist);
  
  /* free the directory list */
  if (old_dirlist)
    free_dirlist(old_dirlist);
  
  
  fdata->last_click = 0;  /* re-init double-click time */
  
  return;
}

/*
 * load_dir() - Callback routine for pathname string entry widget
 */
static void load_dir(Widget w, char *string, FReqData *fdata)
{
  char **old_dirlist, temp[MAXPATHLEN];
  int num_dir;

  /* make sure the name has a '/' at the end */
  strcpy(temp, string);
  if(temp[strlen(temp)-1] != '/')
    strcat(temp, "/");
  
  old_dirlist = fdata->dirlist;
  
  if(!(fdata->dirlist = get_dir_list(temp, &num_dir)))
    {
      /* bad path - reset the file path and return */
      SetStringEntry(fdata->file_path, fdata->fpath);
      return;
    }
  
  qsort(fdata->dirlist, num_dir, sizeof(char *), mystrcmp);

  strcpy(fdata->fpath, temp);
  SetStringEntry(fdata->file_path, temp);
  ChangeScrollList(fdata->file_list, fdata->dirlist);

  /* free the directory list */
  if (old_dirlist)
    free_dirlist(old_dirlist);
}

/*
 * load_name() - Callback routine for file name string entry widget
 */
static void load_name(Widget w, char *string, FReqData *fdata)
{
  char *fpath, fullname[MAXPATHLEN];
  
  fpath = GetStringEntry(fdata->file_path);
  
  sprintf(fullname, "%s%s", fpath, string);
  
  /* right here we should check the validity of the file name */
  /* and abort if invalid */
  
  strcpy(fdata->fname, fullname);

  SetCurrentWindow(fdata->freq_window);
  CloseWindow();
  return;
}


/*
 * This function is just a wrapper for mystrcmp(), and is called by qsort()
 * (if used) down below.
 */
static int mystrcmp(const void *a, const void *b)
{
  return strcmp(*(char **)a, *(char **)b);
}

