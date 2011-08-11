#ifndef _CD_VOID_
#define _CD_VOID_

#ifdef __cplusplus
extern "C" {
#endif

cdContext* cdContextVoid(void);
void cdlua_setvoidstate(cdCanvas* cnv, lua_State * L);

#define CD_VOID cdContextVoid()

#ifdef __cplusplus
}
#endif

#endif /* ifndef _CD_VOID_ */

