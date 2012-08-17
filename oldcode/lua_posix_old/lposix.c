/*
* lposix.c
* POSIX library for Lua 5.1.
* Luiz Henrique de Figueiredo <lhf@tecgraf.puc-rio.br>
* 07 Apr 2006 23:17:49
* Clean up and bug fixes by Leo Razoumov <slonik.az@gmail.com> 2006-10-11 <!LR> 
* Based on original by Claudio Terra for Lua 3.x.
* With contributions by Roberto Ierusalimschy.
*/

#include <sys/stat.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/wait.h>

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#ifndef ANDROID
#include <glob.h>
#endif
#include <grp.h>
#include <libgen.h>
#include <limits.h>
#include <poll.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>

#define MYNAME		"posix"
#define MYVERSION	MYNAME " library for " LUA_VERSION " / Jan 2008"

#ifndef ENABLE_SYSLOG
#define ENABLE_SYSLOG 	1
#endif

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "modemuncher.c"

/* compatibility with Lua 5.0 */
#ifndef LUA_VERSION_NUM
static int luaL_checkoption (lua_State *L, int narg, const char *def,
                                 const char *const lst[]) {
  const char *name = (def) ? luaL_optstring(L, narg, def) :
                             luaL_checkstring(L, narg);
  int i = luaL_findstring(name, lst);
  if (i == -1)
	luaL_argerror(L, narg, lua_pushfstring(L, "invalid option '%s'", name));
  return i;
}
#define lua_pushinteger			lua_pushnumber
#define lua_createtable(L,a,r)		lua_newtable(L)
#define LUA_FILEHANDLE			"FILE*"

#define lua_setfield(l,i,k)
#define lua_getfield(l,i,k)

#endif

static const struct { char c; mode_t b; } M[] =
{
	{'r', S_IRUSR}, {'w', S_IWUSR}, {'x', S_IXUSR},
	{'r', S_IRGRP}, {'w', S_IWGRP}, {'x', S_IXGRP},
	{'r', S_IROTH}, {'w', S_IWOTH}, {'x', S_IXOTH},
};


static void pushmode(lua_State *L, mode_t mode)
{
	char m[9];
	int i;
	for (i=0; i<9; i++) m[i]= (mode & M[i].b) ? M[i].c : '-';
	if (mode & S_ISUID) m[2]= (mode & S_IXUSR) ? 's' : 'S';
	if (mode & S_ISGID) m[5]= (mode & S_IXGRP) ? 's' : 'S';
	lua_pushlstring(L, m, 9);
}

typedef void (*Selector)(lua_State *L, int i, const void *data);

static int doselection(lua_State *L, int i, int n, 
                       const char *const S[], 
                       Selector F, 
                       const void *data)
{
	if (lua_isnone(L, i) || lua_istable(L, i))
	{
		int j;
		if (lua_isnone(L, i)) lua_createtable(L,0,n); else lua_settop(L, i);
		for (j=0; S[j]!=NULL; j++)
		{
			lua_pushstring(L, S[j]);
			F(L, j, data);
			lua_settable(L, -3);
		}
		return 1;
	}
	else
	{
		int k,n=lua_gettop(L);
		for (k=i; k<=n; k++)
		{
			int j=luaL_checkoption(L, k, NULL, S);
			F(L, j, data);
			lua_replace(L, k);
		}
		return n-i+1;
	}
}
#define doselection(L,i,S,F,d) (doselection)(L,i,sizeof(S)/sizeof(*S)-1,S,F,d)

static int pusherror(lua_State *L, const char *info)
{
	lua_pushnil(L);
	if (info==NULL)
		lua_pushstring(L, strerror(errno));
	else
		lua_pushfstring(L, "%s: %s", info, strerror(errno));
	lua_pushinteger(L, errno);
	return 3;
}

static int pushresult(lua_State *L, int i, const char *info)
{
	if (i==-1) return pusherror(L, info);
	lua_pushinteger(L, i);
		return 1;
}

static void badoption(lua_State *L, int i, const char *what, int option)
{
	luaL_argerror(L, 2,
		lua_pushfstring(L, "unknown %s option '%c'", what, option));
}

static uid_t mygetuid(lua_State *L, int i)
{
	if (lua_isnone(L, i))
		return -1;
	else if (lua_isnumber(L, i))
		return (uid_t) lua_tonumber(L, i);
	else if (lua_isstring(L, i))
	{
		struct passwd *p=getpwnam(lua_tostring(L, i));
		return (p==NULL) ? -1 : p->pw_uid;
	}
	else
		return luaL_typerror(L, i, "string or number");
}

static gid_t mygetgid(lua_State *L, int i)
{
	if (lua_isnone(L, i))
		return -1;
	else if (lua_isnumber(L, i))
		return (gid_t) lua_tonumber(L, i);
	else if (lua_isstring(L, i))
	{
		struct group *g=getgrnam(lua_tostring(L, i));
		return (g==NULL) ? -1 : g->gr_gid;
	}
	else
		return luaL_typerror(L, i, "string or number");
}


static int Perrno(lua_State *L)			/** errno([n]) */
{
	int n = luaL_optint(L, 1, errno);
	lua_pushstring(L, strerror(n));
	lua_pushinteger(L, n);
	return 2;
}


static int Pbasename(lua_State *L)		/** basename(path) */
{
	char b[PATH_MAX];
	size_t len;
	const char *path = luaL_checklstring(L, 1, &len);
	if (len>=sizeof(b)) luaL_argerror(L, 1, "too long");
	lua_pushstring(L, basename(strcpy(b,path)));
	return 1;
}


static int Pdirname(lua_State *L)		/** dirname(path) */
{
	char b[PATH_MAX];
	size_t len;
	const char *path = luaL_checklstring(L, 1, &len);
	if (len>=sizeof(b)) luaL_argerror(L, 1, "too long");
	lua_pushstring(L, dirname(strcpy(b,path)));
	return 1;
}


static int Pdir(lua_State *L)			/** dir([path]) */
{
	const char *path = luaL_optstring(L, 1, ".");
	DIR *d = opendir(path);
	if (d == NULL)
		return pusherror(L, path);
	else
	{
		int i;
		struct dirent *entry;
		lua_newtable(L);
		for (i=1; (entry = readdir(d)) != NULL; i++)
		{
			lua_pushstring(L, entry->d_name);
			lua_rawseti(L, -2, i);
		}
		closedir(d);
		lua_pushinteger(L, i-1);
		return 2;
	}
}

#ifndef ANDROID
static int Pglob(lua_State *L)                  /** glob(pattern) */
{
 const char *pattern = luaL_optstring(L, 1, "*");
 glob_t globres;

 if (glob(pattern, 0, NULL, &globres))
   return pusherror(L, pattern);
 else
   {
     int i;
     lua_newtable(L);
     for (i=1; i<=globres.gl_pathc; i++) {
       lua_pushstring(L, globres.gl_pathv[i-1]);
       lua_rawseti(L, -2, i);
     }
     globfree(&globres);
     return 1;
   }
}
#endif

static int aux_files(lua_State *L)
{
	DIR **p = (DIR **)lua_touserdata(L, lua_upvalueindex(1));
	DIR *d = *p;
	struct dirent *entry;
	if (d == NULL) return 0;
	entry = readdir(d);
	if (entry == NULL)
	{
		closedir(d);
		*p=NULL;
		return 0;
	}
	else
	{
		lua_pushstring(L, entry->d_name);
	return 1;
	}
}

static int dir_gc (lua_State *L)
{
	DIR *d = *(DIR **)lua_touserdata(L, 1);
	if (d!=NULL) closedir(d);
	return 0;
}

static int Pfiles(lua_State *L)			/** files([path]) */
{
	const char *path = luaL_optstring(L, 1, ".");
	DIR **d = (DIR **)lua_newuserdata(L, sizeof(DIR *));
	if (luaL_newmetatable(L, MYNAME " dir handle"))
	{
		lua_pushliteral(L, "__gc");
		lua_pushcfunction(L, dir_gc);
		lua_settable(L, -3);
	}
	lua_setmetatable(L, -2);
	*d = opendir(path);
	if (*d == NULL) return pusherror(L, path);
		lua_pushcclosure(L, aux_files, 1);
		return 1;
}


static int Pgetcwd(lua_State *L)		/** getcwd() */
{
	char b[PATH_MAX];
	if (getcwd(b, sizeof(b)) == NULL) return pusherror(L, ".");
	lua_pushstring(L, b);
		return 1;
}


static int Pmkdir(lua_State *L)			/** mkdir(path) */
{
	const char *path = luaL_checkstring(L, 1);
	return pushresult(L, mkdir(path, 0777), path);
}


static int Pchdir(lua_State *L)			/** chdir(path) */
{
	const char *path = luaL_checkstring(L, 1);
	return pushresult(L, chdir(path), path);
}

static int Prmdir(lua_State *L)			/** rmdir(path) */
{
	const char *path = luaL_checkstring(L, 1);
	return pushresult(L, rmdir(path), path);
}


static int Punlink(lua_State *L)		/** unlink(path) */
{
	const char *path = luaL_checkstring(L, 1);
	return pushresult(L, unlink(path), path);
}

static int Plink(lua_State *L)			/** link(old,new,[symbolic]) */
{
	const char *oldpath = luaL_checkstring(L, 1);
	const char *newpath = luaL_checkstring(L, 2);
	return pushresult(L,
		(lua_toboolean(L,3) ? symlink : link)(oldpath, newpath), NULL);
}


static int Preadlink(lua_State *L)		/** readlink(path) */
{
	char b[PATH_MAX];
	const char *path = luaL_checkstring(L, 1);
	int n = readlink(path, b, sizeof(b));
	if (n==-1) return pusherror(L, path);
	lua_pushlstring(L, b, n);
	return 1;
}


static int Paccess(lua_State *L)		/** access(path,[mode]) */
{
	int mode=F_OK;
	const char *path=luaL_checkstring(L, 1);
	const char *s;
	for (s=luaL_optstring(L, 2, "f"); *s!=0 ; s++)
		switch (*s)
		{
			case ' ': break;
			case 'r': mode |= R_OK; break;
			case 'w': mode |= W_OK; break;
			case 'x': mode |= X_OK; break;
			case 'f': mode |= F_OK; break;
			default: badoption(L, 2, "mode", *s); break;
		}
	return pushresult(L, access(path, mode), path);
}


static int myfclose (lua_State *L) {
  FILE **p = (FILE **)lua_touserdata(L, 1);
  int rc = fclose(*p);
  if (rc == 0) *p = NULL;
  return pushresult(L, rc, NULL);
} 

static int pushfile (lua_State *L, int id, const char *mode) {
  FILE **f = (FILE **)lua_newuserdata(L, sizeof(FILE *));
  *f = NULL;
  luaL_getmetatable(L, LUA_FILEHANDLE);
  lua_setmetatable(L, -2);
  lua_getfield(L, LUA_REGISTRYINDEX, "POSIX_PIPEFILE");
  if (lua_isnil(L, -1)) {
    lua_pop(L, 1);
    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_pushcfunction(L, myfclose);
    lua_setfield(L, -2, "__close");
    lua_setfield(L, LUA_REGISTRYINDEX, "POSIX_PIPEFILE");
  }
  lua_setfenv(L, -2);
  *f = fdopen(id, mode);
  return (*f != NULL);
}

static int Ppipe(lua_State *L)			/** pipe() */
{
	int fd[2];
	if (pipe(fd)==-1) return pusherror(L, NULL);
	if (!pushfile(L, fd[0], "r") || !pushfile(L, fd[1], "w"))
		return pusherror(L, "pipe");
	return 2;
}


static int Pfileno(lua_State *L)	/** fileno(filehandle) */
{
	FILE *f = *(FILE**) luaL_checkudata(L, 1, LUA_FILEHANDLE);
	return pushresult(L, fileno(f), NULL);
}


static int Pfdopen(lua_State *L)	/** fdopen(fd, mode) */
{
	int fd = luaL_checkint(L, 1);
	const char *mode = luaL_checkstring(L, 2);
	if (!pushfile(L, fd, mode))
		return pusherror(L, "fdpoen");
	return 1;
}


/* helper func for Pdup */
static const char *filemode(int fd)
{
	const char *m;
	int mode = fcntl(fd, F_GETFL);
	if (mode < 0)
		return NULL;
	switch (mode & O_ACCMODE) {
		case O_RDONLY:  m = "r"; break;
		case O_WRONLY:  m = "w"; break;
		default:    	m = "rw"; break;
	}
	return m;
}

static int Pdup(lua_State *L)			/** dup(old,[new]) */
{
	FILE **oldf = (FILE**)luaL_checkudata(L, 1, LUA_FILEHANDLE);
  	FILE **newf = (FILE **)lua_touserdata(L, 2);
	int fd;
	const char *msg = "dup2";
	fflush(*newf);
	if (newf == NULL) {
		fd = dup(fileno(*oldf));
		msg = "dup";
	} else {
		fflush(*newf);
		fd = dup2(fileno(*oldf), fileno(*newf));
	}

	if ((fd < 0) || !pushfile(L, fd, filemode(fd)))
		return pusherror(L, msg);
	return 1;
}


static int Pmkfifo(lua_State *L)		/** mkfifo(path) */
{
	const char *path = luaL_checkstring(L, 1);
	return pushresult(L, mkfifo(path, 0777), path);
}


static int runexec(lua_State *L, int use_shell)
{
	const char *path = luaL_checkstring(L, 1);
	int i,n=lua_gettop(L);
	char **argv = lua_newuserdata(L,(n+1)*sizeof(char*));
	argv[0] = (char*)path;
	for (i=1; i<n; i++) argv[i] = (char*)luaL_checkstring(L, i+1);
	argv[n] = NULL;
	if (use_shell) {
		execvp(path, argv);
	} else {
		execv(path, argv);
	}
	return pusherror(L, path);
}


static int Pexec(lua_State *L)			/** exec(path,[args]) */
{
	return runexec(L, 0);
}


static int Pexecp(lua_State *L)			/** execp(path,[args]) */
{
	return runexec(L, 1);
}


static int Pfork(lua_State *L)			/** fork() */
{
	return pushresult(L, fork(), NULL);
}

/* from http://lua-users.org/lists/lua-l/2007-11/msg00346.html */
static int Ppoll(lua_State *L)   /** poll(filehandle, timeout) */
{
	struct pollfd fds;
	FILE* file = *(FILE**)luaL_checkudata(L,1,LUA_FILEHANDLE);
	int timeout = luaL_checkint(L,2);
	fds.fd = fileno(file);
	fds.events = POLLIN;
	return pushresult(L, poll(&fds,1,timeout), NULL);
}

static int Pwait(lua_State *L)			/** wait([pid]) */
{
	int status;
	pid_t pid = luaL_optint(L, 1, -1);
	pid = waitpid(pid, &status, 0);
	if (pid == -1) return pusherror(L, NULL);
	lua_pushinteger(L, pid);
	if (WIFEXITED(status))
	{
		lua_pushliteral(L,"exited");
		lua_pushinteger(L, WEXITSTATUS(status));
		return 3;
	}
	else if (WIFSIGNALED(status))
	{
		lua_pushliteral(L,"killed");
		lua_pushinteger(L, WTERMSIG(status));
		return 3;
	}
	else if (WIFSTOPPED(status))
	{
		lua_pushliteral(L,"stopped");
		lua_pushinteger(L, WSTOPSIG(status));
		return 3;
	}
	return 1;
}


static int Pkill(lua_State *L)			/** kill(pid,[sig]) */
{
	pid_t pid = luaL_checkint(L, 1);
	int sig = luaL_optint(L, 2, SIGTERM);
	return pushresult(L, kill(pid, sig), NULL);
}

static int Psetpid(lua_State *L)		/** setpid(option,...) */
{
	const char *what=luaL_checkstring(L, 1);
	switch (*what)
	{
		case 'U':
			return pushresult(L, seteuid(mygetuid(L, 2)), NULL);
		case 'u':
			return pushresult(L, setuid(mygetuid(L, 2)), NULL);
		case 'G':
			return pushresult(L, setegid(mygetgid(L, 2)), NULL);
		case 'g':
			return pushresult(L, setgid(mygetgid(L, 2)), NULL);
		case 's':
			return pushresult(L, setsid(), NULL);
		case 'p':
		{
			pid_t pid  = luaL_checkint(L, 2);
			pid_t pgid = luaL_checkint(L, 3);
			return pushresult(L, setpgid(pid,pgid), NULL);
		}
		default:
			badoption(L, 2, "id", *what);
			return 0;
	}
}


static int Psleep(lua_State *L)			/** sleep(seconds) */
{
	unsigned int seconds = luaL_checkint(L, 1);
	lua_pushinteger(L, sleep(seconds));
	return 1;
}


static int Psetenv(lua_State *L)		/** setenv(name,value,[over]) */
{
	const char *name=luaL_checkstring(L, 1);
	const char *value=luaL_optstring(L, 2, NULL);
	if (value==NULL)
	{
	unsetenv(name);
		return pushresult(L, 0, NULL);
	}
	else
	{
		int overwrite=lua_isnoneornil(L, 3) || lua_toboolean(L, 3);
		return pushresult(L, setenv(name,value,overwrite), NULL);
	}
}


static int Pgetenv(lua_State *L)		/** getenv([name]) */
{
	if (lua_isnone(L, 1))
	{
		extern char **environ;
		char **e;
		lua_newtable(L);
		for (e=environ; *e!=NULL; e++)
		{
			char *s=*e;
			char *eq=strchr(s, '=');
			if (eq==NULL)		/* will this ever happen? */
			{
				lua_pushstring(L,s);
				lua_pushboolean(L,1);
			}
			else
			{
				lua_pushlstring(L,s,eq-s);
				lua_pushstring(L,eq+1);
			}
			lua_settable(L,-3);
		}
	}
	else
		lua_pushstring(L, getenv(luaL_checkstring(L, 1)));
	return 1;
}

static int Pumask(lua_State *L)			/** umask([mode]) */
{/* <!LR> from old lposix-5.0 version */
	char m[10];
	mode_t mode;
	umask(mode=umask(0));
	mode=(~mode)&0777;
	if (!lua_isnone(L, 1))
	{
		if (mode_munch(&mode, luaL_checkstring(L, 1)))
		{
			lua_pushnil(L);
			return 1;
		}
		mode&=0777;
		umask(~mode);
	}
	modechopper(mode, m);
	lua_pushstring(L, m);
	return 1;
}


static int Pchmod(lua_State *L)			/** chmod(path,mode) */
{
	mode_t mode;
	struct stat s;
	const char *path = luaL_checkstring(L, 1);
	const char *modestr = luaL_checkstring(L, 2);
	if (stat(path, &s)) return pusherror(L, path);
	mode = s.st_mode;
	if (mode_munch(&mode, modestr)) luaL_argerror(L, 2, "bad mode");
	return pushresult(L, chmod(path, mode), path);
}


static int Pchown(lua_State *L)			/** chown(path,uid,gid) */
{
	const char *path = luaL_checkstring(L, 1);
	uid_t uid = mygetuid(L, 2);
	gid_t gid = mygetgid(L, 3);
	return pushresult(L, chown(path, uid, gid), path);
}


static int Putime(lua_State *L)			/** utime(path,[mtime,atime]) */
{
	struct utimbuf times;
	time_t currtime = time(NULL);
	const char *path = luaL_checkstring(L, 1);
	times.modtime = luaL_optnumber(L, 2, currtime);
	times.actime  = luaL_optnumber(L, 3, currtime);
	return pushresult(L, utime(path, &times), path);
}


static void FgetID(lua_State *L, int i, const void *data)
{
	switch (i)
	{
		case 0:	lua_pushinteger(L, getegid());	break;
		case 1:	lua_pushinteger(L, geteuid());	break;
		case 2:	lua_pushinteger(L, getgid());	break;
		case 3:	lua_pushinteger(L, getuid());	break;
		case 4:	lua_pushinteger(L, getpgrp());	break;
		case 5:	lua_pushinteger(L, getpid());	break;
		case 6:	lua_pushinteger(L, getppid());	break;
	}
}

static const char *const SgetID[] =
{
	"egid", "euid", "gid", "uid", "pgrp", "pid", "ppid", NULL
};

static int Pgetpid(lua_State *L)		/** getpid([options]) */
{
	return doselection(L, 1, SgetID, FgetID, NULL);
}


static int Phostid(lua_State *L)		/** hostid() */
{
	char b[32];
	sprintf(b,"%ld",gethostid());
	lua_pushstring(L, b);
	return 1;
}


static int Pttyname(lua_State *L)		/** ttyname([fd]) */
{
	int fd=luaL_optint(L, 1, 0);
	lua_pushstring(L, ttyname(fd));
	return 1;
}


static int Pctermid(lua_State *L)		/** ctermid() */
{
	char b[L_ctermid];
	ctermid(b);
	lua_pushstring(L, b);
	return 1;
}


static int Pgetlogin(lua_State *L)		/** getlogin() */
{
	lua_pushstring(L, getlogin());
	return 1;
}


static void Fgetpasswd(lua_State *L, int i, const void *data)
{
	const struct passwd *p=data;
	switch (i)
	{
		case 0: lua_pushstring(L, p->pw_name); break;
		case 1: lua_pushinteger(L, p->pw_uid); break;
		case 2: lua_pushinteger(L, p->pw_gid); break;
		case 3: lua_pushstring(L, p->pw_dir); break;
		case 4: lua_pushstring(L, p->pw_shell); break;
/* not strictly POSIX */
#ifndef ANDROID
		case 5: lua_pushstring(L, p->pw_gecos); break;
		case 6: lua_pushstring(L, p->pw_passwd); break;
#endif
	}
}

static const char *const Sgetpasswd[] =
{
	"name", "uid", "gid", "dir", "shell", "gecos", "passwd", NULL
};


static int Pgetpasswd(lua_State *L)		/** getpasswd(name|id,[sel]) */
{
	struct passwd *p=NULL;
	if (lua_isnoneornil(L, 1))
		p = getpwuid(geteuid());
	else if (lua_isnumber(L, 1))
		p = getpwuid((uid_t)lua_tonumber(L, 1));
	else if (lua_isstring(L, 1))
		p = getpwnam(lua_tostring(L, 1));
	else
		luaL_typerror(L, 1, "string or number");
	if (p==NULL)
		lua_pushnil(L);
	else
		return doselection(L, 2, Sgetpasswd, Fgetpasswd, p);
	return 1;
}


static int Pgetgroup(lua_State *L)		/** getgroup(name|id) */
{
	struct group *g=NULL;
	if (lua_isnumber(L, 1))
		g = getgrgid((gid_t)lua_tonumber(L, 1));
	else if (lua_isstring(L, 1))
		g = getgrnam(lua_tostring(L, 1));
	else
		luaL_typerror(L, 1, "string or number");
	if (g==NULL)
		lua_pushnil(L);
	else
	{
		int i;
		lua_newtable(L);
		lua_pushliteral(L, "name");
		lua_pushstring(L, g->gr_name);
		lua_settable(L, -3);
		lua_pushliteral(L, "gid");
		lua_pushinteger(L, g->gr_gid);
		lua_settable(L, -3);
		for (i=0; g->gr_mem[i]!=NULL; i++)
		{
			lua_pushstring(L, g->gr_mem[i]);
			lua_rawseti(L, -2, i);
		}
	}
	return 1;
}


struct mytimes
{
 struct tms t;
 clock_t elapsed;
};

/* #define pushtime(L,x)	lua_pushnumber(L,((lua_Number)x)/CLOCKS_PER_SEC) */
#define pushtime(L,x)	lua_pushnumber(L, ((lua_Number)x)/clk_tck)

static void Ftimes(lua_State *L, int i, const void *data)
{
    static long clk_tck = 0; 
	const struct mytimes *t=data;

    if( !clk_tck){ clk_tck= sysconf(_SC_CLK_TCK);}
	switch (i)
	{
		case 0: pushtime(L, t->t.tms_utime); break;
		case 1: pushtime(L, t->t.tms_stime); break;
		case 2: pushtime(L, t->t.tms_cutime); break;
		case 3: pushtime(L, t->t.tms_cstime); break;
		case 4: pushtime(L, t->elapsed); break;
	}
}

static const char *const Stimes[] =
{
	"utime", "stime", "cutime", "cstime", "elapsed", NULL
};

static int Ptimes(lua_State *L)			/** times([options]) */
{
	struct mytimes t;
	t.elapsed = times(&t.t);
	return doselection(L, 1, Stimes, Ftimes, &t);
}


static const char *filetype(mode_t m)
{
	if (S_ISREG(m))		return "regular";
	else if (S_ISLNK(m))	return "link";
	else if (S_ISDIR(m))	return "directory";
	else if (S_ISCHR(m))	return "character device";
	else if (S_ISBLK(m))	return "block device";
	else if (S_ISFIFO(m))	return "fifo";
	else if (S_ISSOCK(m))	return "socket";
	else			return "?";
}

static void Fstat(lua_State *L, int i, const void *data)
{
	const struct stat *s=data;
	switch (i)
	{
		case 0: pushmode(L, s->st_mode); break;
		case 1: lua_pushinteger(L, s->st_ino); break;
		case 2: lua_pushinteger(L, s->st_dev); break;
		case 3: lua_pushinteger(L, s->st_nlink); break;
		case 4: lua_pushinteger(L, s->st_uid); break;
		case 5: lua_pushinteger(L, s->st_gid); break;
		case 6: lua_pushinteger(L, s->st_size); break;
		case 7: lua_pushinteger(L, s->st_atime); break;
		case 8: lua_pushinteger(L, s->st_mtime); break;
		case 9: lua_pushinteger(L, s->st_ctime); break;
		case 10:lua_pushstring(L, filetype(s->st_mode)); break;
	}
}

static const char *const Sstat[] =
{
	"mode", "ino", "dev", "nlink", "uid", "gid",
	"size", "atime", "mtime", "ctime", "type",
	NULL
};

static int Pstat(lua_State *L)			/** stat(path,[options]) */
{
	struct stat s;
	const char *path=luaL_checkstring(L, 1);
	if (lstat(path,&s)==-1) return pusherror(L, path);
	return doselection(L, 2, Sstat, Fstat, &s);
}


static int Puname(lua_State *L)			/** uname([string]) */
{
	struct utsname u;
	luaL_Buffer b;
	const char *s;
	if (uname(&u)==-1) return pusherror(L, NULL);
	luaL_buffinit(L, &b);
	for (s=luaL_optstring(L, 1, "%s %n %r %v %m"); *s; s++)
		if (*s!='%')
			luaL_putchar(&b, *s);
		else switch (*++s)
		{
			case '%': luaL_putchar(&b, *s); break;
			case 'm': luaL_addstring(&b,u.machine); break;
			case 'n': luaL_addstring(&b,u.nodename); break;
			case 'r': luaL_addstring(&b,u.release); break;
			case 's': luaL_addstring(&b,u.sysname); break;
			case 'v': luaL_addstring(&b,u.version); break;
			default: badoption(L, 2, "format", *s); break;
		}
	luaL_pushresult(&b);
	return 1;
}


static const int Kpathconf[] =
{
	_PC_LINK_MAX, _PC_MAX_CANON, _PC_MAX_INPUT, _PC_NAME_MAX, _PC_PATH_MAX,
	_PC_PIPE_BUF, _PC_CHOWN_RESTRICTED, _PC_NO_TRUNC, _PC_VDISABLE,
	-1
};

static void Fpathconf(lua_State *L, int i, const void *data)
{
	const char *path=data;
	lua_pushinteger(L, pathconf(path, Kpathconf[i]));
}

static const char *const Spathconf[] =
{
	"link_max", "max_canon", "max_input", "name_max", "path_max",
	"pipe_buf", "chown_restricted", "no_trunc", "vdisable",
	NULL
};

static int Ppathconf(lua_State *L)		/** pathconf([path,options]) */
{
	const char *path = luaL_optstring(L, 1, ".");
	return doselection(L, 2, Spathconf, Fpathconf, path);
}


static const int Ksysconf[] =
{
	_SC_ARG_MAX, _SC_CHILD_MAX, _SC_CLK_TCK, _SC_NGROUPS_MAX, _SC_STREAM_MAX,
	_SC_TZNAME_MAX, _SC_OPEN_MAX, _SC_JOB_CONTROL, _SC_SAVED_IDS, _SC_VERSION,
	-1
};

static void Fsysconf(lua_State *L, int i, const void *data)
{
	lua_pushinteger(L, sysconf(Ksysconf[i]));
}

static const char *const Ssysconf[] =
{
	"arg_max", "child_max", "clk_tck", "ngroups_max", "stream_max",
	"tzname_max", "open_max", "job_control", "saved_ids", "version",
	NULL
};

static int Psysconf(lua_State *L)		/** sysconf([options]) */
{
	return doselection(L, 1, Ssysconf, Fsysconf, NULL);
}

#if ENABLE_SYSLOG
/* syslog funcs */
static int Popenlog(lua_State *L)	/** openlog(ident, [option], [facility]) */
{
	const char *ident = luaL_checkstring(L, 1);
	int option = 0;
	int facility = luaL_optint(L, 3, LOG_USER);
	const char *s = luaL_optstring(L, 2, "");
	while (*s) {
		switch (*s) {
			case ' ': break;
			case 'c': option |= LOG_CONS; break;
			case 'n': option |= LOG_NDELAY; break;
			case 'e': option |= LOG_PERROR; break;
			case 'p': option |= LOG_PID; break;
			default: badoption(L, 2, "option", *s); break;
		}
		s++;
	}
	openlog(ident, option, facility);
	return 0;
}


static int Psyslog(lua_State *L)		/** syslog(priority, message) */
{
	int priority = luaL_checkint(L, 1);
	const char *msg = luaL_checkstring(L, 2);
	syslog(priority, "%s", msg);
	return 0;
}


static int Pcloselog(lua_State *L)		/** closelog() */
{
	closelog();
	return 0;
}
#endif

/*
 * XXX: GNU and BSD handle the forward declaration of crypt() in different
 * and annoying ways (especially GNU). Declare it here just to make sure
 * that it's there
 */
char *crypt(const char *, const char *);

static int Pcrypt(lua_State *L)
{
	const char *str, *salt;
	char *res;

	str = luaL_checkstring(L, 1);
	salt = luaL_checkstring(L, 2);
	if (strlen(salt) < 2)
		luaL_error(L, "not enough salt");

	res = crypt(str, salt);
	lua_pushstring(L, res);

	return 1;
}

static const luaL_reg R[] =
{
	{"access",		Paccess},
	{"basename",		Pbasename},
	{"chdir",		Pchdir},
	{"chmod",		Pchmod},
	{"chown",		Pchown},
	{"crypt",		Pcrypt},
	{"ctermid",		Pctermid},
	{"dirname",		Pdirname},
	{"dir",			Pdir},
	{"dup",			Pdup},
	{"errno",		Perrno},
	{"exec",		Pexec},
	{"execp",		Pexecp},
	{"fdopen",		Pfdopen},
	{"fileno",		Pfileno},
	{"files",		Pfiles},
	{"fork",		Pfork},
	{"getcwd",		Pgetcwd},
	{"getenv",		Pgetenv},
	{"getgroup",		Pgetgroup},
	{"getlogin",		Pgetlogin},
	{"getpasswd",		Pgetpasswd},
	{"getpid",		Pgetpid},
#ifndef ANDROID
	{"glob",		Pglob},
#endif
	{"hostid",		Phostid},
	{"kill",		Pkill},
	{"link",		Plink},
	{"mkdir",		Pmkdir},
	{"mkfifo",		Pmkfifo},
	{"pathconf",		Ppathconf},
	{"pipe",		Ppipe},
	{"readlink",		Preadlink},
	{"rmdir",		Prmdir},
	{"rpoll",		Ppoll},
	{"setenv",		Psetenv},
	{"setpid",		Psetpid},
	{"sleep",		Psleep},
	{"stat",		Pstat},
	{"sysconf",		Psysconf},
	{"times",		Ptimes},
	{"ttyname",		Pttyname},
	{"unlink",		Punlink},
	{"umask",		Pumask},
	{"uname",		Puname},
	{"utime",		Putime},
	{"wait",		Pwait},

#if ENABLE_SYSLOG
	{"openlog",		Popenlog},
	{"syslog",		Psyslog},
	{"closelog",		Pcloselog},
#endif

	{NULL,			NULL}
};

#define set_const(key, value)		\
	lua_pushliteral(L, key);	\
	lua_pushnumber(L, value);	\
	lua_settable(L, -3)

LUALIB_API int luaopen_posix (lua_State *L)
{
	luaL_register(L,MYNAME,R);
	lua_pushliteral(L,"version");		/** version */
	lua_pushliteral(L,MYVERSION);
	lua_settable(L,-3);

#if ENABLE_SYSLOG
	set_const("LOG_AUTH", LOG_AUTH);
	set_const("LOG_AUTHPRIV", LOG_AUTHPRIV);
	set_const("LOG_CRON", LOG_CRON);
	set_const("LOG_DAEMON", LOG_DAEMON);
	set_const("LOG_FTP", LOG_FTP);
	set_const("LOG_KERN", LOG_KERN);
	set_const("LOG_LOCAL0", LOG_LOCAL0);
	set_const("LOG_LOCAL1", LOG_LOCAL1);
	set_const("LOG_LOCAL2", LOG_LOCAL2);
	set_const("LOG_LOCAL3", LOG_LOCAL3);
	set_const("LOG_LOCAL4", LOG_LOCAL4);
	set_const("LOG_LOCAL5", LOG_LOCAL5);
	set_const("LOG_LOCAL6", LOG_LOCAL6);
	set_const("LOG_LOCAL7", LOG_LOCAL7);
	set_const("LOG_LPR", LOG_LPR);
	set_const("LOG_MAIL", LOG_MAIL);
	set_const("LOG_NEWS", LOG_NEWS);
	set_const("LOG_SYSLOG", LOG_SYSLOG);
	set_const("LOG_USER", LOG_USER);
	set_const("LOG_UUCP", LOG_UUCP);

	set_const("LOG_EMERG", LOG_EMERG);
	set_const("LOG_ALERT", LOG_ALERT);
	set_const("LOG_CRIT", LOG_CRIT);
	set_const("LOG_ERR", LOG_ERR);
	set_const("LOG_WARNING", LOG_WARNING);
	set_const("LOG_NOTICE", LOG_NOTICE);
	set_const("LOG_INFO", LOG_INFO);
	set_const("LOG_DEBUG", LOG_DEBUG);
#endif


	return 1;
}

/*EOF*/
