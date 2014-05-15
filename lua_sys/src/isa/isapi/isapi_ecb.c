/* Lua System: Internet Server Application: ISAPI Extension Control Block */

#define ECB_TYPENAME	"sys.isa.isapi"

#define ECB_STATUS_MASK		0x0FFF
#define ECB_STATUS_HEADERS	0x1000
#define ECB_STATUS_HEADERS_SEND	0x2000
#define ECB_STATUS_PENDING	0x4000


/*
 * Returns: ecb_udata
 */
static int
ecb_new (lua_State *L)
{
  lua_boxpointer(L, NULL);
  luaL_getmetatable(L, ECB_TYPENAME);
  lua_setmetatable(L, -2);
  return 1;
}

/*
 * Arguments: ecb_udata, [handle (ludata)]
 * Returns: [ecb_udata | handle (ludata)]
 */
static int
ecb_handle (lua_State *L)
{
  LPEXTENSION_CONTROL_BLOCK *ecbp = checkudata(L, 1, ECB_TYPENAME);

  if (lua_gettop(L) > 1) {
    *ecbp = lua_touserdata(L, 2);
    lua_settop(L, 1);
  } else {
    if (!*ecbp) lua_pushnil(L);
    else lua_pushlightuserdata(L, *ecbp);
  }
  return 1;
}

/*
 * Arguments: ecb_udata, variable_name (string)
 * Returns: value (string | number)
 */
static int
ecb_getvar (lua_State *L)
{
  LPEXTENSION_CONTROL_BLOCK ecb = lua_unboxpointer(L, 1, ECB_TYPENAME);
  const char *name = luaL_checkstring(L, 2);
  const char *val = NULL;

  if (!strcmp(name, "REQUEST_METHOD"))
    val = ecb->lpszMethod;
  else if (!strcmp(name, "QUERY_STRING"))
    val = ecb->lpszQueryString;
  else if (!strcmp(name, "PATH_INFO"))
    val = ecb->lpszPathInfo;
  else if (!strcmp(name, "PATH_TRANSLATED"))
    val = ecb->lpszPathTranslated;
  else if (!strcmp(name, "CONTENT_TYPE"))
    val = ecb->lpszContentType;
  else if (!strcmp(name, "CONTENT_LENGTH")) {
    lua_pushinteger(L, ecb->cbTotalBytes);
    return 1;
  }
  if (val) {
    lua_pushstring(L, val);
    return 1;
  } else {
    char buf[SYS_BUFSIZE];
    DWORD len = sizeof(buf);
    union sys_rwptr name_ptr;  /* to avoid "const cast" warning */

    name_ptr.r = name;
    if (ecb->GetServerVariable(ecb->ConnID, name_ptr.w, buf, &len)) {
      lua_pushlstring(L, buf, len - 1);
      return 1;
    }
  }
  return sys_seterror(L, 0);
}

/*
 * Arguments: ecb_udata
 * Returns: data (string)
 */
static int
ecb_data (lua_State *L)
{
  LPEXTENSION_CONTROL_BLOCK ecb = lua_unboxpointer(L, 1, ECB_TYPENAME);

  lua_pushlstring(L, (const char *) ecb->lpbData, ecb->cbAvailable);
  return 1;
}

/*
 * Arguments: ecb_udata, [membuf_udata, count (number)]
 * Returns: [string | count (number)]
 */
static int
ecb_read (lua_State *L)
{
  LPEXTENSION_CONTROL_BLOCK ecb = lua_unboxpointer(L, 1, ECB_TYPENAME);
  size_t n = !lua_isnumber(L, -1) ? ~((size_t) 0)
   : (size_t) lua_tointeger(L, -1);
  const size_t len = n;  /* how much total to read */
  size_t rlen;  /* how much to read */
  int nr;  /* number of bytes actually read */
  struct sys_thread *td = sys_thread_get();
  struct sys_buffer sb;
  char buf[SYS_BUFSIZE];
  int res = 0;

  sys_buffer_write_init(L, 2, &sb, buf, sizeof(buf));
  do {
    rlen = (n <= sb.size) ? n : sb.size;
    if (td) sys_vm2_leave(td);
    {
      DWORD l;
      nr = ecb->ReadClient(ecb->ConnID, sb.ptr.w, &l) ? (int) l : -1;
    }
    if (td) sys_vm2_enter(td);
    if (nr == -1) break;
    n -= nr;  /* still have to read `n' bytes */
  } while ((n != 0L && nr == (int) rlen)  /* until end of count or eof */
   && sys_buffer_write_next(L, &sb, buf, 0));
  if (nr <= 0 && len == n) {
    res = -1;
  } else {
    if (!sys_buffer_write_done(L, &sb, buf, nr))
      lua_pushinteger(L, len - n);
  }
  if (td) sys_thread_check(td, L);
  if (!res) return 1;
  return sys_seterror(L, 0);
}

/*
 * Arguments: ecb_udata, {string | membuf_udata} ...
 * Returns: [success/partial (boolean), count (number)]
 */
static int
ecb_write (lua_State *L)
{
  LPEXTENSION_CONTROL_BLOCK ecb = lua_unboxpointer(L, 1, ECB_TYPENAME);
  size_t n = 0;  /* number of chars actually write */
  int i, narg = lua_gettop(L);

  if (ecb->dwHttpStatusCode & ECB_STATUS_HEADERS_SEND) {
    ecb->dwHttpStatusCode ^= ECB_STATUS_HEADERS_SEND;

    lua_pushfstring(L, "HTTP/1.1 %d\r\n",
     (ecb->dwHttpStatusCode & ECB_STATUS_MASK));

    lua_rawgetp(L, LUA_REGISTRYINDEX, ecb);

    lua_pushliteral(L, "\r\n");
    lua_concat(L, 3);

    lua_insert(L, 2);
    ++narg;
  }

  for (i = 2; i <= narg; ++i) {
    struct sys_buffer sb;
    int nw;

    if (!sys_buffer_read_init(L, i, &sb)
     || sb.size == 0)  /* don't close the connection */
      continue;
    sys_vm_leave(L);
    {
      DWORD l = sb.size;
      nw = ecb->WriteClient(ecb->ConnID, sb.ptr.w, &l, 0)
       ? (int) l : -1;
    }
    sys_vm_enter(L);
    if (nw == -1) {
      if (n > 0) break;
      return sys_seterror(L, 0);
    }
    n += nw;
    sys_buffer_read_next(&sb, nw);
    if ((size_t) nw < sb.size) break;
  }
  lua_pushboolean(L, (i > narg));
  lua_pushinteger(L, n);
  return 2;
}

/*
 * Arguments: ecb_udata, name (string), value (string | number)
 */
static int
ecb_header (lua_State *L)
{
  LPEXTENSION_CONTROL_BLOCK ecb = lua_unboxpointer(L, 1, ECB_TYPENAME);
  const int headers = (ecb->dwHttpStatusCode & ~ECB_STATUS_MASK);
  const char *name = luaL_checkstring(L, 2);

  if (headers == ECB_STATUS_HEADERS)
    luaL_error(L, "Headers already sent");

  if (!strcmp(name, "Status")) {
    ecb->dwHttpStatusCode &= ~ECB_STATUS_MASK;
    ecb->dwHttpStatusCode |= lua_tointeger(L, 3) & ECB_STATUS_MASK;
    return 0;
  }
  luaL_checktype(L, 3, LUA_TSTRING);

  lua_pushliteral(L, ": ");
  lua_insert(L, 3);
  lua_pushliteral(L, "\r\n");

  if (headers) {
    lua_rawgetp(L, LUA_REGISTRYINDEX, ecb);
  } else {
    lua_pushliteral(L, "");
  }

  lua_concat(L, 5);
  lua_rawsetp(L, LUA_REGISTRYINDEX, ecb);

  ecb->dwHttpStatusCode |= ECB_STATUS_HEADERS | ECB_STATUS_HEADERS_SEND;
  return 0;
}

/*
 * Arguments: ecb_udata
 */
static int
ecb_req_pending (lua_State *L)
{
  LPEXTENSION_CONTROL_BLOCK ecb = lua_unboxpointer(L, 1, ECB_TYPENAME);

  ecb->dwHttpStatusCode |= ECB_STATUS_PENDING;
  return 0;
}

/*
 * Arguments: ecb_udata
 * Returns: [ecb_udata]
 */
static int
ecb_req_done (lua_State *L)
{
  LPEXTENSION_CONTROL_BLOCK ecb = lua_unboxpointer(L, 1, ECB_TYPENAME);
  DWORD res = HSE_STATUS_SUCCESS;

  if (ecb->dwHttpStatusCode & ~ECB_STATUS_MASK) {
    ecb->dwHttpStatusCode &= ECB_STATUS_MASK;

    lua_pushnil(L);
    lua_rawsetp(L, LUA_REGISTRYINDEX, ecb);
  }

  if (ecb->ServerSupportFunction(ecb->ConnID, HSE_REQ_DONE_WITH_SESSION,
   &res, NULL,NULL)) {
    lua_settop(L, 1);
    return 1;
  }
  return sys_seterror(L, 0);
}


static luaL_Reg ecb_meth[] = {
  {"handle",		ecb_handle},
  {"getvar",		ecb_getvar},
  {"data",		ecb_data},
  {"read",		ecb_read},
  {"write",		ecb_write},
  {"header",		ecb_header},
  {"req_pending",	ecb_req_pending},
  {"req_done",		ecb_req_done},
  {SYS_BUFIO_TAG,	NULL},  /* can operate with buffers */
  {NULL, NULL}
};
