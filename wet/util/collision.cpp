/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"




/*------------------------------------------------------------------------------------------------------------------------------*/
//
// see if two points hit during a frames movement.
//
/*------------------------------------------------------------------------------------------------------------------------------*/
bool collide( coll_point *a , coll_point *b )
{
f32 r;	// radius of both points combined
f32 rr;	// r*r
f32 oor; // one over total radius

v3 pos[1];
v3 mov[1];

v3 perp[1];

f32 perp_mov;
f32 perp_len;

v3 mov_norm[1];

f32 pos_lenlen;

f32 mov_len;

f32 need_len;

f32 hit_time;
v3 hit_norm[1];

f32 dot; // the dot product of pos and mov

	r=a->radius+b->radius;
	rr=r*r;

// combine the 2 positions / movement
// set b at the origin and stationary and calculate a combined position/vector for a relative for this

	pos->x=a->pos->x-b->pos->x;
	pos->y=a->pos->y-b->pos->y;
	pos->z=a->pos->z-b->pos->z;

	mov->x=a->mov->x-b->mov->x;
	mov->y=a->mov->y-b->mov->y;
	mov->z=a->mov->z-b->mov->z;


// are we already intersecting ?

	pos_lenlen=pos->lenlen();

	if( rr-(1.0f/256.0f) >= pos_lenlen )
	{
DBG_Info("Intersection %f\n",rr-pos_lenlen);

		if( (a->hit->time>0.0f) || (a->hit->dd<pos_lenlen) )
		{
			a->hit->hit=b;
			a->hit->time=0.0f;
			a->hit->normal->set( pos );
			a->hit->normal->norm();
			a->hit->dd=rr-pos_lenlen;
		}

		if( (b->hit->time>0.0f) || (b->hit->dd<pos_lenlen) )
		{
			b->hit->hit=a;
			b->hit->time=0.0f;
			b->hit->normal->set( pos );
			b->hit->normal->neg();
			b->hit->normal->norm();
			b->hit->dd=rr-pos_lenlen;
		}

		return true;
	}

// will we ever intersect? IE are we moving towards each other

	dot=pos->dot(mov);

	if(dot>=0.0f) // we never get any closer so will never hit
	{
		return false;
	}

// find vector perpendicular to the line which is at closest point line gets

	mov_norm->set(mov);
	mov_norm->norm();		// normal of movement vector

	perp_mov=-pos->dot(mov_norm); // distance along mov to perpendicular point

	perp->x=pos->x+mov_norm->x*perp_mov; // position of perpendicular point
	perp->y=pos->y+mov_norm->y*perp_mov;
	perp->z=pos->z+mov_norm->z*perp_mov;

	perp_len=perp->len(); // distance at closest point

	if(perp_len>r) // we will never hit?
	{
		return false;
	}


	need_len=perp_mov-f32_sqrt(rr-perp_len*perp_len); // how far along mov_norm we need to move before we hit

	mov_len=mov->len();

	if(need_len>mov_len) // we dobnt get close enough
	{
		return false;
	}


	hit_time=need_len/mov_len; // how far we move before we hit

	hit_norm->set(pos);
	hit_norm->x+=mov->x*hit_time;
	hit_norm->y+=mov->y*hit_time;
	hit_norm->z+=mov->z*hit_time;

	oor=1.0f/r; // hit_norm must have a length of r at this point so normalise using r

	hit_norm->x*=oor;
	hit_norm->y*=oor;
	hit_norm->z*=oor;

	if( (a->hit->time>hit_time) )
	{
		a->hit->hit=b;
		a->hit->time=hit_time;
		a->hit->normal->set( hit_norm );
		a->hit->normal->norm();
	}

	if( (b->hit->time>hit_time) )
	{
		b->hit->hit=a;
		b->hit->time=hit_time;
		b->hit->normal->set( hit_norm );
		b->hit->normal->neg();
		b->hit->normal->norm();
	}

	return true;

}


