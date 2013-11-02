/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/




//
// string scan funcs
//
const char * string_scan_s32(const char *p, s32 *o);
const char * string_scan_u32(const char *p, u32 *o);
const char * string_scan_f32(const char *p, f32 *o);


bool string_scan_token(const char *p, s32 *white_size, s32 *token_size);
