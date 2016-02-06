/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/






struct coll_hit
{
	v3    normal[1]; // normal of surface we hit

	f32		dd; // distance*distance intersect, used to find closest surface if we start off intersecting

	f32   time;	// 1.0f for no hit 0.0f for we didnt get to move at all 0.5f for moved halfway etc etc

	struct coll_point *hit; // data pointer to what we hit if we hit something, 0 for no hit

	void reset()
	{
		normal->reset();
		time=2.0f;
		hit=0;
	}
};



struct coll_point
{
	v3 pos[1]; // start position this frame

	v3 mov[1]; // movement wanted this frame

	f32 radius; // "size" of point

	coll_hit hit[1]; // our best hit so far


	void reset()
	{
		pos->reset();
		mov->reset();
		radius=0.0f;
		hit->reset();
	}
};


bool collide( coll_point *a , coll_point *b );
