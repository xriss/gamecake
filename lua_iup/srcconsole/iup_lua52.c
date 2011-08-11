/*
** $Id: iup_lua52.c,v 1.3 2010/10/28 03:53:47 scuri Exp $
** Lua stand-alone interpreter
** See Copyright Notice in lua.h
*/


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define lua_c

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"

/******************* IUP *********************/
#ifdef USE_STATIC
#include "iup.h"
#include "iuplua.h"

#ifndef IUPLUA_NO_GL
#include "iupgl.h"
#include "iupluagl.h"
/* #include "luagl.h" */
#endif

#ifndef IUPLUA_NO_CD
#include "iupcontrols.h"
#include "iupluacontrols.h"
#include "iup_pplot.h"
#include "iuplua_pplot.h"
#include <cd.h>
#include <cdgdiplus.h>
#include <cdlua.h>
#include <cdluaiup.h>
#endif

#ifndef IUPLUA_NO_IM
#include "iupluaim.h"
#include <im.h>
#include <im_image.h>
#include <imlua.h>
#endif

#ifndef IUPLUA_NO_IM
#ifndef IUPLUA_NO_CD
#include <cdluaim.h>
#endif
#endif

#ifdef IUPLUA_TUIO
#include "iupluatuio.h"
#endif

#endif
/******************* IUP *********************/


#if !defined(LUA_PROMPT)
#define LUA_PROMPT		"> "
#define LUA_PROMPT2		">> "
#endif

#if !defined(LUA_PROGNAME)
#define LUA_PROGNAME		"lua"
#endif

#if !defined(LUA_MAXINPUT)
#define LUA_MAXINPUT		512
#endif

#if !defined(LUA_INIT_VAR)
#define LUA_INIT_VAR		"LUA_INIT"
#endif


/*
** lua_stdin_is_tty detects whether the standard input is a 'tty' (that
** is, whether we're running lua interactively).
*/
#if defined(LUA_USE_ISATTY)
#include <unistd.h>
#define lua_stdin_is_tty()      isatty(0)
#elif defined(LUA_WIN)
#include <io.h>
#include <stdio.h>
#define lua_stdin_is_tty()      _isatty(_fileno(stdin))
#else
#define lua_stdin_is_tty()      1  /* assume stdin is a tty */
#endif


/*
** lua_readline defines how to show a prompt and then read a line from
** the standard input.
** lua_saveline defines how to "save" a read line in a "history".
** lua_freeline defines how to free a line read by lua_readline.
*/
#if defined(LUA_USE_READLINE)

#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#define lua_readline(L,b,p)     ((void)L, ((b)=readline(p)) != NULL)
#define lua_saveline(L,idx) \
        if (lua_rawlen(L,idx) > 0)  /* non-empty line? */ \
          add_history(lua_tostring(L, idx));  /* add it to history */
#define lua_freeline(L,b)       ((void)L, free(b))

#elif !defined(lua_readline)

#define lua_readline(L,b,p)     \
        ((void)L, fputs(p, stdout), fflush(stdout),  /* show prompt */ \
        fgets(b, LUA_MAXINPUT, stdin) != NULL)  /* get line */
#define lua_saveline(L,idx)     { (void)L; (void)idx; }
#define lua_freeline(L,b)       { (void)L; (void)b; }

#endif




static lua_State *globalL = NULL;

static const char *progname = LUA_PROGNAME;



static void lstop (lua_State *L, lua_Debug *ar) {
  (void)ar;  /* unused arg. */
  lua_sethook(L, NULL, 0, 0);
  luaL_error(L, "interrupted!");
}


static void laction (int i) {
  signal(i, SIG_DFL); /* if another SIGINT happens before lstop,
                              terminate process (default action) */
  lua_sethook(globalL, lstop, LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);
}


static void print_usage (void) {
  fprintf(stderr,
  "usage: %s [options] [script [args]]\n"
  "Available options are:\n"
  "  -e stat  execute string " LUA_QL("stat") "\n"
  "  -l name  require library " LUA_QL("name") "\n"
  "  -i       enter interactive mode after executing " LUA_QL("script") "\n"
  "  -v       show version information\n"
  "  --       stop handling options\n"
  "  -        execute stdin and stop handling options\n"
  ,
  progname);
  fflush(stderr);
}


static void l_message (const char *pname, const char *msg) {
  if (pname) fprintf(stderr, "%s: ", pname);
  fprintf(stderr, "%s\n", msg);
  fflush(stderr);
}


static int report (lua_State *L, int status) {
  if (status != LUA_OK && !lua_isnil(L, -1)) {
    const char *msg = lua_tostring(L, -1);
    if (msg == NULL) msg = "(error object is not a string)";
    l_message(progname, msg);
    lua_pop(L, 1);
    /* force a complete garbage collection in case of errors */
    lua_gc(L, LUA_GCCOLLECT, 0);
  }
  return status;
}


/* the next function is called unprotected, so it must avoid errors */
static void finalreport (lua_State *L, int status) {
  if (status != LUA_OK) {
    const char *msg = (lua_type(L, -1) == LUA_TSTRING) ? lua_tostring(L, -1)
                                                       : NULL;
    if (msg == NULL) msg = "(error object is not a string)";
    l_message(progname, msg);
    lua_pop(L, 1);
  }
}


static int traceback (lua_State *L) {
  const char *msg = lua_tostring(L, 1);
  if (msg)
    luaL_traceback(L, L, msg, 1);
  else if (!lua_isnoneornil(L, 1)) {  /* is there an error object? */
    if (!luaL_callmeta(L, 1, "__tostring"))  /* try its 'tostring' metamethod */
      lua_pushliteral(L, "(no error message)");
  }
  return 1;
}


static int docall (lua_State *L, int narg, int clear) {
  int status;
  int base = lua_gettop(L) - narg;  /* function index */
  lua_pushcfunction(L, traceback);  /* push traceback function */
  lua_insert(L, base);  /* put it under chunk and args */
  globalL = L;  /* to be available to 'laction' */
  signal(SIGINT, laction);
  status = lua_pcall(L, narg, (clear ? 0 : LUA_MULTRET), base);
  signal(SIGINT, SIG_DFL);
  lua_remove(L, base);  /* remove traceback function */
  return status;
}


static void print_version (void) {
  printf("%s\n", LUA_COPYRIGHT);
}


static int getargs (lua_State *L, char **argv, int n) {
  int narg;
  int i;
  int argc = 0;
  while (argv[argc]) argc++;  /* count total number of arguments */
  narg = argc - (n + 1);  /* number of arguments to the script */
  luaL_checkstack(L, narg + 3, "too many arguments to script");
  for (i=n+1; i < argc; i++)
    lua_pushstring(L, argv[i]);
  lua_createtable(L, narg, n + 1);
  for (i=0; i < argc; i++) {
    lua_pushstring(L, argv[i]);
    lua_rawseti(L, -2, i - n);
  }
  return narg;
}


static int dofile (lua_State *L, const char *name) {
  int status = luaL_loadfile(L, name);
  if (status == LUA_OK) status = docall(L, 0, 1);
  return report(L, status);
}


static int dostring (lua_State *L, const char *s, const char *name) {
  int status = luaL_loadbuffer(L, s, strlen(s), name);
  if (status == LUA_OK) status = docall(L, 0, 1);
  return report(L, status);
}


static int dolibrary (lua_State *L, const char *name) {
  lua_getfield(L, LUA_ENVIRONINDEX, "require");
  lua_pushstring(L, name);
  return report(L, docall(L, 1, 1));
}


static const char *get_prompt (lua_State *L, int firstline) {
  const char *p;
  lua_getfield(L, LUA_ENVIRONINDEX, firstline ? "_PROMPT" : "_PROMPT2");
  p = lua_tostring(L, -1);
  if (p == NULL) p = (firstline ? LUA_PROMPT : LUA_PROMPT2);
  lua_pop(L, 1);  /* remove global */
  return p;
}

/* mark in error messages for incomplete statements */
#define mark	"<eof>"
#define marklen	(sizeof(mark) - 1)

static int incomplete (lua_State *L, int status) {
  if (status == LUA_ERRSYNTAX) {
    size_t lmsg;
    const char *msg = lua_tolstring(L, -1, &lmsg);
    if (lmsg >= marklen && strcmp(msg + lmsg - marklen, mark) == 0) {
      lua_pop(L, 1);
      return 1;
    }
  }
  return 0;  /* else... */
}


static int pushline (lua_State *L, int firstline) {
  char buffer[LUA_MAXINPUT];
  char *b = buffer;
  size_t l;
  const char *prmt = get_prompt(L, firstline);
  if (lua_readline(L, b, prmt) == 0)
    return 0;  /* no input */
  l = strlen(b);
  if (l > 0 && b[l-1] == '\n')  /* line ends with newline? */
    b[l-1] = '\0';  /* remove it */
  if (firstline && b[0] == '=')  /* first line starts with `=' ? */
    lua_pushfstring(L, "return %s", b+1);  /* change it to `return' */
  else
    lua_pushstring(L, b);
  lua_freeline(L, b);
  return 1;
}


static int loadline (lua_State *L) {
  int status;
  lua_settop(L, 0);
  if (!pushline(L, 1))
    return -1;  /* no input */
  for (;;) {  /* repeat until gets a complete line */
    size_t l;
    const char *line = lua_tolstring(L, 1, &l);
    status = luaL_loadbuffer(L, line, l, "=stdin");
    if (!incomplete(L, status)) break;  /* cannot try to add lines? */
    if (!pushline(L, 0))  /* no more input? */
      return -1;
    lua_pushliteral(L, "\n");  /* add a new line... */
    lua_insert(L, -2);  /* ...between the two lines */
    lua_concat(L, 3);  /* join them */
  }
  lua_saveline(L, 1);
  lua_remove(L, 1);  /* remove line */
  return status;
}


static void dotty (lua_State *L) {
  int status;
  const char *oldprogname = progname;
  progname = NULL;
  while ((status = loadline(L)) != -1) {
    if (status == LUA_OK) status = docall(L, 0, 0);
    report(L, status);
    if (status == LUA_OK && lua_gettop(L) > 0) {  /* any result to print? */
      luaL_checkstack(L, LUA_MINSTACK, "too many results to print");
      lua_getfield(L, LUA_ENVIRONINDEX, "print");
      lua_insert(L, 1);
      if (lua_pcall(L, lua_gettop(L)-1, 0, 0) != LUA_OK)
        l_message(progname, lua_pushfstring(L,
                               "error calling " LUA_QL("print") " (%s)",
                               lua_tostring(L, -1)));
    }
  }
  lua_settop(L, 0);  /* clear stack */
  luai_writestring("\n", 1);
  fflush(stdout);
  progname = oldprogname;
}


static int handle_script (lua_State *L, char **argv, int n) {
  int status;
  const char *fname;
  int narg = getargs(L, argv, n);  /* collect arguments */
  lua_setfield(L, LUA_ENVIRONINDEX, "arg");
  fname = argv[n];
  if (strcmp(fname, "-") == 0 && strcmp(argv[n-1], "--") != 0)
    fname = NULL;  /* stdin */
  status = luaL_loadfile(L, fname);
  lua_insert(L, -(narg+1));
  if (status == LUA_OK)
    status = docall(L, narg, 0);
  else
    lua_pop(L, narg);
  return report(L, status);
}


/* check that argument has no extra characters at the end */
#define noextrachars(x)		{if ((x)[2] != '\0') return -1;}


static int collectargs (char **argv, int *pi, int *pv, int *pe) {
  int i;
  for (i = 1; argv[i] != NULL; i++) {
    if (argv[i][0] != '-')  /* not an option? */
        return i;
    switch (argv[i][1]) {  /* option */
      case '-':
        noextrachars(argv[i]);
        return (argv[i+1] != NULL ? i+1 : 0);
      case '\0':
        return i;
      case 'i':
        noextrachars(argv[i]);
        *pi = 1;  /* go through */
      case 'v':
        noextrachars(argv[i]);
        *pv = 1;
        break;
      case 'e':
        *pe = 1;  /* go through */
      case 'l':
        if (argv[i][2] == '\0') {
          i++;
          if (argv[i] == NULL) return -1;
        }
        break;
      default: return -1;  /* invalid option */
    }
  }
  return 0;
}


static int runargs (lua_State *L, char **argv, int n) {
  int i;
  for (i = 1; i < n; i++) {
    if (argv[i] == NULL) continue;
    lua_assert(argv[i][0] == '-');
    switch (argv[i][1]) {  /* option */
      case 'e': {
        const char *chunk = argv[i] + 2;
        if (*chunk == '\0') chunk = argv[++i];
        lua_assert(chunk != NULL);
        if (dostring(L, chunk, "=(command line)") != LUA_OK)
          return 0;
        break;
      }
      case 'l': {
        const char *filename = argv[i] + 2;
        if (*filename == '\0') filename = argv[++i];
        lua_assert(filename != NULL);
        if (dolibrary(L, filename) != LUA_OK)
          return 0;  /* stop if file fails */
        break;
      }
      default: break;
    }
  }
  return 1;
}


static int handle_luainit (lua_State *L) {
  const char *init = getenv(LUA_INIT_VAR);
  if (init == NULL) return LUA_OK;
  else if (init[0] == '@')
    return dofile(L, init+1);
  else
    return dostring(L, init, "=" LUA_INIT_VAR);
}


/******************* IUP *********************/
#ifdef USE_STATIC
#ifdef IUPLUA_IMGLIB
int luaopen_iupluaimglib(lua_State* L);
#endif
#endif

static void iuplua_openlibs (lua_State *L) {
  lua_pushliteral(L, LUA_COPYRIGHT);
  lua_setglobal(L, "_COPYRIGHT");  /* set global _COPYRIGHT */

#ifdef USE_STATIC
  /* iuplua initialization */
  iuplua_open(L);

#ifdef IUPLUA_IMGLIB
  luaopen_iupluaimglib(L);
#endif
#ifdef IUPLUA_TUIO
  iuptuiolua_open(L);
#endif

/* luaopen_lfs(L); */

#ifndef IUPLUA_NO_GL
  iupgllua_open(L);
/*  luaopen_luagl(L); */
#endif
#ifndef IUPLUA_NO_CD
  iupcontrolslua_open(L);
  iup_pplotlua_open(L);
  cdlua_open(L);
  cdluaiup_open(L);
#endif
#ifndef IUPLUA_NO_IM
  iupimlua_open(L);
  imlua_open(L);
  imlua_open_process(L);
#endif
#ifndef IUPLUA_NO_IM
#ifndef IUPLUA_NO_CD
  cdluaim_open(L);
#endif
#endif
#endif
}

static void iuplua_input (lua_State *L) 
{
#ifdef IUPLUA_USELOH
#include "indent.loh"
#include "console5.loh"
#else
#ifdef IUPLUA_USELZH
#include "indent.lzh"
#include "console5.lzh"
#else
  luaL_dofile(L, "indent.lua");
  luaL_dofile(L, "console5.lua");
#endif
#endif
}
/******************* IUP *********************/


static int pmain (lua_State *L) {
  int argc = lua_tointeger(L, 1);
  char **argv = (char **)lua_touserdata(L, 2);
  int script;
  int has_i = 0, has_v = 0, has_e = 0;
  if (argv[0] && argv[0][0]) progname = argv[0];
  script = collectargs(argv, &has_i, &has_v, &has_e);
  if (script < 0) {  /* invalid args? */
    print_usage();
    return 0;
  }
  if (has_v) print_version();
  /* open standard libraries */
  luaL_checkversion(L);
  lua_gc(L, LUA_GCSTOP, 0);  /* stop collector during initialization */
  luaL_openlibs(L);  /* open libraries */
  lua_gc(L, LUA_GCRESTART, 0);
/******************* IUP *********************/
  iuplua_openlibs(L);
/******************* IUP *********************/
  /* run LUA_INIT */
  if (handle_luainit(L) != LUA_OK) return 0;
  /* execute arguments -e and -l */
  if (!runargs(L, argv, (script > 0) ? script : argc)) return 0;
  /* execute main script (if there is one) */
  if (script && handle_script(L, argv, script) != LUA_OK) return 0;
  if (has_i)  /* -i option? */
    dotty(L);
  else if (script == 0 && !has_e && !has_v) {  /* no arguments? */
    if (lua_stdin_is_tty()) {
      print_version();
/******************* IUP *********************/
/*    dotty(L);                              */
      iuplua_input(L);
/******************* IUP *********************/
    }
    else dofile(L, NULL);  /* executes stdin as a file */
  }
  lua_pushboolean(L, 1);  /* signal no errors */
  return 1;
}


int main (int argc, char **argv) {
  static lua_CFunction ppmain = &pmain;
  int status, result;
  lua_State *L = luaL_newstate();  /* create state */
  if (L == NULL) {
    l_message(argv[0], "cannot create state: not enough memory");
    return EXIT_FAILURE;
  }
  /* call 'pmain' in protected mode */
  lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_CPCALL);  /* calling function */
  lua_pushlightuserdata(L, &ppmain);
  lua_pushinteger(L, argc); 
  lua_pushlightuserdata(L, argv);
  status = lua_pcall(L, 3, 1, 0);
  result = lua_toboolean(L, -1);  /* get result */
  finalreport(L, status);
  lua_close(L);
  return (result && status == LUA_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
}

