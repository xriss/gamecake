/*****************************************************************************

gifinto - save GIF on stdin to file if size over set threshold

*****************************************************************************/

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#ifdef _WIN32
#include <io.h>
#endif /* _WIN32 */

#include "gif_lib.h"
#include "getarg.h"

#define PROGRAM_NAME	"gifinto"

#define STRLEN		512

#define DEFAULT_MIN_FILE_SIZE	14     /* More than GIF stamp + screen desc. */
#define	DEFAULT_OUT_NAME	"GifInto.Gif"
#define DEFAULT_TMP_NAME	"TempInto.XXXXXX"

static char
    *VersionStr =
	PROGRAM_NAME
	VERSION_COOKIE
	"	Gershon Elber,	"
	__DATE__ ",   " __TIME__ "\n"
	"(C) Copyright 1989 Gershon Elber.\n";
static char
    *CtrlStr =
	PROGRAM_NAME
	" v%- s%-MinFileSize!d h%- GifFile!*s";

static int
    MinFileSize = DEFAULT_MIN_FILE_SIZE;

/******************************************************************************
 This is simply: read until EOF, then close the output, test its length, and
 if non zero then rename it.
******************************************************************************/
int main(int argc, char **argv)
{
    int	NumFiles;
    bool Error, MinSizeFlag = false, HelpFlag = false;
    char **FileName = NULL, FoutTmpName[STRLEN], FullPath[STRLEN], *p;
    FILE *Fin, *Fout;

    if ((Error = GAGetArgs(argc, argv, CtrlStr, &GifNoisyPrint,
		&MinSizeFlag, &MinFileSize, &HelpFlag,
		&NumFiles, &FileName)) != false ||
		(NumFiles > 1 && !HelpFlag)) {
	if (Error)
	    GAPrintErrMsg(Error);
	else if (NumFiles != 1)
	    GIF_MESSAGE("Error in command line parsing - one GIF file please.");
	GAPrintHowTo(CtrlStr);
	exit(EXIT_FAILURE);
    }

    if (HelpFlag) {
	(void)fprintf(stderr, VersionStr, GIFLIB_MAJOR, GIFLIB_MINOR);
	GAPrintHowTo(CtrlStr);
	exit(EXIT_SUCCESS);
    }

    /* Open the stdin in binary mode and increase its buffer size: */
#ifdef _WIN32
    _setmode(0, O_BINARY);		  /* Make sure it is in binary mode. */
#endif

    Fin = fdopen(0, "rb");   /* Make it into a stream: */

    if (Fin == NULL)
    {
        GIF_EXIT("Failed to open input.");
    }

    /* Isolate the directory where our destination is, and set tmp file name */
    /* in the very same directory. This code is isecure because it creates   */
    /* predictable names, but it's not worth the effort and risk to fix.     */
    if ( *FileName == NULL ) GIF_EXIT("No valid Filename given.");
    if ( strlen(*FileName) > STRLEN-1 ) GIF_EXIT("Filename too long.");
    memset(FullPath, '\0', sizeof(FullPath));
    // cppcheck-suppress redundantCopy
    strncpy(FullPath, *FileName, STRLEN);
    if ((p = strrchr(FullPath, '/')) != NULL ||
	(p = strrchr(FullPath, '\\')) != NULL)
	p[1] = 0;
    else if ((p = strrchr(FullPath, ':')) != NULL)
	p[1] = 0;
    else
	FullPath[0] = 0;		  /* No directory or disk specified. */

    if ( strlen(FullPath) > STRLEN-1 ) GIF_EXIT("Filename too long.");
    strncpy(FoutTmpName, FullPath, STRLEN);   /* First setup the Path */
    /* then add a name for the tempfile */
    if ( (strlen(FoutTmpName) + strlen(DEFAULT_TMP_NAME))  > STRLEN-1 ) GIF_EXIT("Filename too long.");
    strcat(FoutTmpName, DEFAULT_TMP_NAME);
    int FD;
    FD = mkstemp(FoutTmpName); /* returns filedescriptor */
    if (FD == -1 )
    {
	GIF_EXIT("Failed to open output.");
    }
    Fout = fdopen(FD, "w"); /* returns a stream with FD */
    if (Fout == NULL )
    {
	GIF_EXIT("Failed to open output.");
    }

    while (!feof(Fin)) {
	if (putc(getc(Fin), Fout) == EOF)
	    GIF_EXIT("Failed to write output.");
    }

    fclose(Fin);
    if (ftell(Fout) >= (long) MinFileSize) {
	fclose(Fout);
	unlink(*FileName);
	if (rename(FoutTmpName, *FileName) != 0) {
	    char DefaultName[STRLEN];
	    if ( (strlen(FullPath) + strlen(DEFAULT_OUT_NAME)) > STRLEN-1 ) GIF_EXIT("Filename too long.");
	    strncpy(DefaultName, FullPath, STRLEN);
	    // cppcheck-suppress uninitstring
	    strcat(DefaultName, DEFAULT_OUT_NAME);
	    if (rename(FoutTmpName, DefaultName) == 0) {
		char s[STRLEN];
		snprintf(s, STRLEN, "Failed to rename out file - left as %s.",
			      DefaultName);
		GIF_MESSAGE(s);
	    }
	    else {
		unlink(FoutTmpName);
		GIF_MESSAGE("Failed to rename out file - deleted.");
	    }
	}
    }
    else {
	fclose(Fout);
	unlink(FoutTmpName);
	GIF_MESSAGE("File too small - not renamed.");
    }

    return 0;
}

/* end */
