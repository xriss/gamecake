/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/


//
// Functions that wrap onto the devil library
//


s32  grd_create( s32 fmt , s32 w, s32 h, s32 d );
s32  grd_load( const char *filename , s32 fmt , grd_info *grd );
s32 grd_duplicate( s32 ID );


bool grd_convert( s32 ID , s32 fmt );

bool grd_getinfo( s32 ID , grd_info *grd );
bool grd_getpalinfo( s32 ID , grd_info *grd );


bool grd_save( s32 ID , const char *filename );
void grd_free( s32 ID );



bool grd_conscale( s32 ID , f32 base , f32 scale);

bool grd_scale( s32 ID , s32 w, s32 h, s32 d);

