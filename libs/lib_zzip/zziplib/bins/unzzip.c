/*
 *	Copyright (c) 2003 Guido Draheim <guidod@gmx.de>
 *      Use freely under the restrictions of the ZLIB license.
 *
 *      This file is used as an example to clarify zzip api usage.
 */

#include <zzip/zzip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <zzip/__debug.h>
#include <zzip/__mkdir.h>
#include <zzip/__param.h>
#include <zzip/__string.h>

/* common code */
#include "unzzip-states.h"
#include "unzzipcat-zip.h"
#include "unzzipdir-zip.h"

static const char usage[] = /* .. */
    {"unzzip <dir>.. \n"
     "  - unzzip the files contained in a zip archive.\n"
     "  -p            print content of files to pipe\n"
     "  -l            list names in archive (short format)\n"};

#define BASENAME(x) (strchr((x), '/') ? strrchr((x), '/') + 1 : (x))

static int
unzzip_version(void)
{
    printf("%s version %s %s\n", BASENAME(__FILE__), ZZIP_PACKAGE_NAME, ZZIP_PACKAGE_VERSION);
    return 0;
}

static int
unzzip_help(void)
{
    printf(usage);
    return 0;
}

/* Functions used by unzzipcat-*.c: */
int
exitcode(int e)
{
    switch (e) {
    case ZZIP_NO_ERROR:
        return EXIT_OK;
    case ZZIP_OUTOFMEM: /* out of memory */
        return EXIT_ENOMEM;
    case ZZIP_DIR_OPEN: /* failed to open zipfile, see errno for details */
        return EXIT_ZIP_NOT_FOUND;
    case ZZIP_DIR_STAT: /* failed to fstat zipfile, see errno for details */
    case ZZIP_DIR_SEEK: /* failed to lseek zipfile, see errno for details */
    case ZZIP_DIR_READ: /* failed to read zipfile, see errno for details */
    case ZZIP_DIR_TOO_SHORT:
    case ZZIP_DIR_EDH_MISSING:
        return EXIT_FILEFORMAT;
    case ZZIP_DIRSIZE:
        return EXIT_EARLY_END_OF_FILE;
    case ZZIP_ENOENT:
        return EXIT_FILE_NOT_FOUND;
    case ZZIP_UNSUPP_COMPR:
        return EXIT_UNSUPPORTED_COMPRESSION;
    case ZZIP_CORRUPTED:
    case ZZIP_UNDEF:
    case ZZIP_DIR_LARGEFILE:
        return EXIT_FILEFORMAT;
    }
    return EXIT_ERRORS;
}

/* like realpath but without resolving symlinks */
static int
normpath(const char *path, char *buf, size_t buflen)
{
	char *max_path, *new_path, *old_path;
	size_t path_len;

	if (path == NULL || buf == NULL || buflen < 3) {
		return EINVAL;
	}
	if (*path == '\0') {
		return ENOENT;
	}
	path_len = strlen(path);
	if (path_len >= buflen - 2) {
		return ENAMETOOLONG;
	}
	max_path = buf + buflen - 2; /* except end-NUL */
	new_path = buf;
    old_path = path;
	while (*old_path != '\0') {
		/* ignore extra "/" */
		if (*old_path == '/') {
			old_path++;
			continue;
		}
		if (*old_path == '.') {
			/* ignore "./" */
			if (old_path[1] == '\0' || old_path[1] == '/') {
				old_path++;
				continue;
			}
			if (old_path[1] == '.') {
				if (old_path[2] == '\0' || old_path[2] == '/') {
					old_path += 2;
					/* error "../" at root */
					if (new_path == buf)
						return EBADMSG;
					/* resolve "../" by removing earlier path component */
					while (--new_path > buf && new_path[-1] != '/');
					continue;
				}
			}
		}
		/* copy the next pathname component. */
		while (*old_path != '\0' && *old_path != '/') {
			if (new_path > max_path) {
				return ENAMETOOLONG;
			}
			*new_path++ = *old_path++;
		}
		*new_path++ = '/';
	}
    if (old_path > path && old_path[-1] != '/') {
	    /* Delete trailing slash but not a lone slash. */
	    if (new_path != buf + 1 && new_path[-1] == '/')
            new_path--;
    }
	*new_path = '\0';
	return 0; /* OK */
}

static void
makedirs(const char* name)
{
    char* p = strrchr(name, '/');
    if (p) {
        char* dir_name = _zzip_strndup(name, p - name);
        makedirs(dir_name);
        free(dir_name);
    }
    if (_zzip_mkdir(name, 0775) == -1 && errno != EEXIST) {
        DBG3("while mkdir %s : %s", name, strerror(errno));
    }
    errno = 0;
}

FILE*
create_fopen(char* name, char* mode, int subdirs)
{
    char file_name[PATH_MAX];
    int err = normpath(name, file_name, PATH_MAX);
    if (err) {
        fprintf(stderr, "ERROR: %s: %s\n", strerror(err), name);
        return NULL;
    }

    if (subdirs) {
        char* p = strrchr(file_name, '/');
        if (p) {
            char* dir_name = _zzip_strndup(file_name, p - file_name);
            makedirs(dir_name);
            free(dir_name);
        }
    }
    return fopen(file_name, mode);
}

int
main(int argc, char** argv)
{
    int          argn;
    int          exitcode = 0;
    zzip_error_t error;

    if (argc <= 1 || ! strcmp(argv[1], "--help")) {
        return unzzip_help();
    }
    if (! strcmp(argv[1], "--version")) {
        return unzzip_version();
    }
    if (! strcmp(argv[1], "-l") || ! strcmp(argv[1], "--list")) {
        argc -= 1;
        argv += 1;
        return unzzip_show_list(argc, argv);
    }
    if (! strcmp(argv[1], "-v") || ! strcmp(argv[1], "--versions")) {
        if (argc == 2)
            return unzzip_version(); /* compatible with info-zip */
        argc -= 1;
        argv += 1;
        return unzzip_long_list(argc, argv);
    }
    if (! strcmp(argv[1], "-p") || ! strcmp(argv[1], "--pipe")) {
        argc -= 1;
        argv += 1;
        return unzzip_print(argc, argv);
    }

    if (! strcmp(argv[1], "-")) {
        fprintf(stderr, "unknown option %s", argv[1]);
        return EXIT_INVALID_OPTION;
    }

    return unzzip_extract(argc, argv);
}
