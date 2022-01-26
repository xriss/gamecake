/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"




/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// setup
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool thunk3d::setup(void)
{

	polys->setup(sizeof(t3d_poly),256);
	points->setup(sizeof(t3d_point),256);

	objects->setup(sizeof(t3d_object),16);
	surfaces->setup(sizeof(t3d_surface),16);
	morphs->setup(sizeof(t3d_morph),16);
	bones->setup(sizeof(t3d_bone),16);


	scenes->setup(sizeof(t3d_scene),16);
	items->setup(sizeof(t3d_item),32);

	streams->setup(sizeof(t3d_stream),256);
	keys->setup(sizeof(t3d_key),256);

	return true;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// reset
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool thunk3d::reset(void)
{
	clean();
	return setup();
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// clean
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void thunk3d::clean(void)
{
#define FREE_STUFF(typ,lst,fnc)\
	{\
	typ *item,*itemprev;\
\
		if(lst->atoms->last)\
		{\
			for( item=(typ*)(lst->atoms->last) ; itemprev=item->prev ; item=itemprev )\
			{\
				fnc(item);\
			}\
			lst->clean();\
		}\
	}\


	FREE_STUFF(t3d_object,objects,FreeObject)

	FREE_STUFF(t3d_surface,surfaces,FreeSurface)
	FREE_STUFF(t3d_poly,polys,FreePoly)
	FREE_STUFF(t3d_point,points,FreePoint)
	FREE_STUFF(t3d_morph,morphs,FreeMorph)
	FREE_STUFF(t3d_bone,bones,FreeBone)

	FREE_STUFF(t3d_scene,scenes,FreeScene)
	FREE_STUFF(t3d_item,items,FreeItem)
	FREE_STUFF(t3d_stream,streams,FreeStream)
	FREE_STUFF(t3d_key,keys,FreeKey)


#undef FREEE_STUFF



}
