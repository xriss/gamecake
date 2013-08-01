/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// allocate an object
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool t3d_object::setup(struct thunk3d *_thunk3d)
{
	DMEM_ZERO(this);

	master=_thunk3d;

	DHEAD_INIT(points);
	DHEAD_INIT(maps);
	DHEAD_INIT(polys);
	DHEAD_INIT(surfaces);
	DHEAD_INIT(morphs);
	DHEAD_INIT(bones);

	return true;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// allocate an object
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool t3d_object::reset(void)
{
thunk3d *_thunk3d;

	_thunk3d=master;

	clean();
	return setup(_thunk3d);
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// allocate an object
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void t3d_object::clean(void)
{
t3d_morph	*morph;
t3d_morph	*morphnext;

t3d_surface	*surf;
t3d_surface	*surfnext;

t3d_poly	*poly;
t3d_poly	*polynext;

t3d_point	*point;
t3d_point	*pointnext;


	if(points_xox) { free(points_xox); points_xox=0; }
	if(polys_xox)  { free(polys_xox);  polys_xox=0; }

	for( morph=morphs->first ; morphnext=morph->next ; morph=morphnext )
	{
		master->FreeMorph(morph);
	}

	for( surf=surfaces->first ; surfnext=surf->next ; surf=surfnext )
	{
		master->FreeSurface(surf);
	}

	for( poly=polys->first ; polynext=poly->next ; poly=polynext )
	{
		master->FreePoly(poly);
	}

	for( point=points->first ; pointnext=point->next ; point=pointnext )
	{
		master->FreePoint(point);
	}

	for( point=maps->first ; pointnext=point->next ; point=pointnext )
	{
		master->FreePoint(point);
	}

}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// allocate an object
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
t3d_object *thunk3d::AllocObject(void)
{
t3d_object *ret;


	if(!(ret=(t3d_object *)objects->alloc()))
	{
		DBG_Error("Failed to allocate thunk3D.object.\n");
		goto bogus;
	}

	if(!ret->setup(this))
	{
		DBG_Error("Failed to setup thunk3D.object.\n");
		goto bogus;
	}

	DLIST_PASTE(objects->atoms->last,ret,0);

	return ret;

bogus:
	FreeObject(ret);
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// free an object
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void thunk3d::FreeObject(t3d_object *item)
{
	if(item)
	{
		DLIST_CUT(item);

		item->clean();

		objects->free((llatom*)item);
	}
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// allocate and build an object from a LWO layer
// 
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool t3d_object::FillObject(lwObject *lwo, s32 layer_num)
{
lwPolygon	*lwp;
lwPoint		*lwv;
lwLayer		*lwl;
lwSurface	*lws;
lwVMap		*lwm;

s32 i;

s32 tri;

t3d_point   *pm;
t3d_point   *point;
t3d_point   *pointparent;
t3d_poly    *poly;
t3d_surface *surface;
t3d_morph	*morph;

s32 *ip;
f32 **fpp;

f32 len;

	for( lwl=lwo->layer ; lwl ; lwl=lwl->next)
	{
		if(lwl->index==layer_num-1)break;
	}

	if((!lwl)||((lwl->index!=layer_num-1)))
	{
		DBG_Error("Missing LWO layer %d.\n",layer_num);
		goto bogus;
	}

	numof_surfaces=0;
	for( lws=lwo->surf , i=0 ; lws ; lws=lws->next , i++ )
	{
		if(!(surface=master->AllocSurface()))
		{
			DBG_Error("Failed to allocate surface.\n");
			goto bogus;
		}
		DLIST_CUTPASTE(surfaces->last,surface,0);
		numof_surfaces++;


		surface->id=(s32)(lws);

		surface->a=1.0f-lws->transparency.val.val;
		surface->r=lws->color.rgb[0];
		surface->g=lws->color.rgb[1];
		surface->b=lws->color.rgb[2];

		surface->r*=lws->diffuse.val;
		surface->g*=lws->diffuse.val;
		surface->b*=lws->diffuse.val;

		surface->a*=255.0f;
		surface->r*=255.0f;
		surface->g*=255.0f;
		surface->b*=255.0f;

		CLAMPVAL(surface->a,0,255);
		CLAMPVAL(surface->r,0,255);
		CLAMPVAL(surface->g,0,255);
		CLAMPVAL(surface->b,0,255);

		surface->sa=255.0f;
		surface->sr=lws->specularity.val*255.0f;
		surface->sg=lws->specularity.val*255.0f;
		surface->sb=lws->specularity.val*255.0f;

		CLAMPVAL(surface->sa,0,255);
		CLAMPVAL(surface->sr,0,255);
		CLAMPVAL(surface->sg,0,255);
		CLAMPVAL(surface->sb,0,255);

		surface->gloss=lws->glossiness.val;
		CLAMPVAL(surface->gloss,0,1);

		strncpy(surface->name,lws->name,sizeof(surface->name)); surface->name[sizeof(surface->name)-1]=0;

		printf("surface named %s\n",surface->name);
	}


	numof_points=0;
	for( lwv=lwl->point.pt , i=0 ; lwv<lwl->point.pt+lwl->point.count ; lwv++ , i++ )
	{
		if(!(point=master->AllocPoint()))
		{
			DBG_Error("Failed to allocate point.\n");
			goto bogus;
		}
		DLIST_CUTPASTE(points->last,point,0);
		numof_points++;


		point->id=i;
		point->x=lwv->pos[0];
		point->y=lwv->pos[1];
		point->z=lwv->pos[2];

		point->nx=0.0f; // clear normal,  we will add to this later
		point->ny=0.0f;
		point->nz=0.0f;
	}


	numof_morphs=0;
	for( lwm=lwl->vmap , i=0 ; lwm ; lwm=lwm->next , i++ )
	{
		if(lwm->dim==3)
		{
			if(!(morph=master->AllocMorph()))
			{
				DBG_Error("Failed to allocate morph.\n");
				goto bogus;
			}
			DLIST_CUTPASTE(morphs->last,morph,0);
			numof_morphs++;



			morph->id=i;

			strncpy(morph->name,lwm->name,sizeof(morph->name)); morph->name[sizeof(morph->name)-1]=0;

			printf("morph named %s\n",morph->name);

			for( fpp=lwm->val , ip=lwm->vindex ; ip<lwm->vindex+lwm->nverts ; fpp++ , ip++ )
			{
				if( (pointparent=findpoint(*ip)) )
				{
					if(!(point=master->AllocPoint()))
					{
						DBG_Error("Failed to allocate morph point.\n");
						goto bogus;
					}
					DLIST_CUTPASTE(pointparent->morphs->last,point,0);

					point->id=morph->id;
					point->x=(*fpp)[0];
					point->y=(*fpp)[1];
					point->z=(*fpp)[2];

//					DBG_Info("morph %d , point %d , %f,%f,%f\n",morph->id,*ip,point->x,point->y,point->z);
				}
			}

		}
	}

	numof_polys=0;
	tri=0;
	for( lwp=lwl->polygon.pol , i=0 ; lwp<lwl->polygon.pol+lwl->polygon.count ; lwp++ , i++ )
	{
	s32 count;
	v3 a[1];
	v3 b[1];
	v3 d[1];

		for( count=0 ; (count+3)<=lwp->nverts ; count++ )
		{
			if(!(poly=master->AllocPoly()))
			{
				DBG_Error("Failed to allocate poly.\n");
				goto bogus;
			}
			DLIST_CUTPASTE(polys->last,poly,0);
			numof_polys++;


			poly->id=i;
			poly->points[0]=findpoint(lwp->v[0].index);
			poly->points[1]=findpoint(lwp->v[count+1].index);
			poly->points[2]=findpoint(lwp->v[count+2].index);

			poly->surface=findsurface((s32)(lwp->surf));



			a->x= poly->points[1]->x - poly->points[0]->x ;
			a->y= poly->points[1]->y - poly->points[0]->y ;
			a->z= poly->points[1]->z - poly->points[0]->z ;

			b->x= poly->points[2]->x - poly->points[1]->x ;
			b->y= poly->points[2]->y - poly->points[1]->y ;
			b->z= poly->points[2]->z - poly->points[1]->z ;

			v3_cross_v3(d,a,b);

			poly->nx=d->x;
			poly->ny=d->y;
			poly->nz=d->z;

			len=poly->nx*poly->nx + poly->ny*poly->ny + poly->nz*poly->nz ;

			len=f32_sqrt(len);

			if(len!=0.0f)
			{
				len=1.0f/len;
			}

			poly->nx*=len;
			poly->ny*=len;
			poly->nz*=len;


			poly->points[0]->nx+=poly->nx;
			poly->points[0]->ny+=poly->ny;
			poly->points[0]->nz+=poly->nz;

			poly->points[1]->nx+=poly->nx;
			poly->points[1]->ny+=poly->ny;
			poly->points[1]->nz+=poly->nz;

			poly->points[2]->nx+=poly->nx;
			poly->points[2]->ny+=poly->ny;
			poly->points[2]->nz+=poly->nz;

			tri++;

		}
	}

// build morph normals

	for( morph=morphs->first ; morph->next ; morph=morph->next )
	{
		for( poly=polys->first ; poly->next ; poly=poly->next )
		{
		v3 a[1];
		v3 b[1];
		v3 d[1];


			a->x= poly->points[1]->x - poly->points[0]->x ;
			a->y= poly->points[1]->y - poly->points[0]->y ;
			a->z= poly->points[1]->z - poly->points[0]->z ;

			if(pm=poly->points[1]->find_morph_point(morph))
			{
				a->x+=pm->x;
				a->y+=pm->y;
				a->z+=pm->z;
			}
			if(pm=poly->points[0]->find_morph_point(morph))
			{
				a->x-=pm->x;
				a->y-=pm->y;
				a->z-=pm->z;
			}

			b->x= poly->points[2]->x - poly->points[1]->x ;
			b->y= poly->points[2]->y - poly->points[1]->y ;
			b->z= poly->points[2]->z - poly->points[1]->z ;

			if(pm=poly->points[2]->find_morph_point(morph))
			{
				b->x+=pm->x;
				b->y+=pm->y;
				b->z+=pm->z;
			}
			if(pm=poly->points[1]->find_morph_point(morph))
			{
				b->x-=pm->x;
				b->y-=pm->y;
				b->z-=pm->z;
			}

			v3_cross_v3(d,a,b);

			poly->nx=d->x;
			poly->ny=d->y;
			poly->nz=d->z;

			len=poly->nx*poly->nx + poly->ny*poly->ny + poly->nz*poly->nz ;

			len=f32_sqrt(len);

			if(len!=0.0f)
			{
				len=1.0f/len;
			}

			for(i=0;i<3;i++)
			{
				if(pm=poly->points[i]->find_morph_point(morph))
				{
					pm->nx+=poly->nx;
					pm->ny+=poly->ny;
					pm->nz+=poly->nz;
				}
			}
		}
	}


// normalize point normals
	for( point=points->first  ; point->next ; point=point->next )
	{
		len=point->nx*point->nx + point->ny*point->ny + point->nz*point->nz ;

		len=f32_sqrt(len);

		if(len!=0.0f)
		{
			len=1.0f/len;
		}

		point->nx*=len;
		point->ny*=len;
		point->nz*=len;


		for( morph=morphs->first ; morph->next ; morph=morph->next )
		{
			if(pm=point->find_morph_point(morph))
			{
				len=pm->nx*pm->nx + pm->ny*pm->ny + pm->nz*pm->nz ;

				len=f32_sqrt(len);

				if(len!=0.0f)
				{
					len=1.0f/len;
				}

				pm->nx*=len;
				pm->ny*=len;
				pm->nz*=len;
			}
		}

	}

	printf("%d morphs\n",numof_morphs);
	printf("%d points\n",numof_points);
	printf("%d surfaces\n",numof_surfaces);
	printf("%d polys\n",numof_polys);




	return true;
bogus:
	return false;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// build XOX output object format data
// 
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool t3d_object::BuildXOX()
{
s32 last_link_type;
t3d_poly    *strip_start;

t3d_surface *surface;

s32 i;
t3d_poly    *poly;
t3d_point   *point;

XOX0_point *xp;
u16 *ip;
u16 *cp;

s32 a,b;

t3d_point   *pm;
t3d_morph	*morph;

XOX0_morph_point *mp;


	if(!SortPolys()) { goto bogus; }

	if(!SortPoints()) { goto bogus; }


//	for( poly=polys->first ; poly->next ; poly=poly->next )
//	{
//		poly->strip_link=0;
//	}

//
// assign output IDs to the points.
//
	for( point=points->first , i=0 ; point->next ; point=point->next , i++ )
	{
		point->output_index=i;
	}

//
// allocate memory for the output point array
//
	points_xox_sizeof=(sizeof(XOX0_point)*numof_points);
	if(!(points_xox=(float*)calloc(points_xox_sizeof,1)))
	{
		DBG_Error("Failed to allocate memory for XOX output points.\n");
		goto bogus;
	}
//
// fill the output point array
//
	for( point=points->first , xp=(XOX0_point*)points_xox ; point->next ; point=point->next , xp++ )
	{
		xp->x=point->x;
		xp->y=point->y;
		xp->z=point->z;
		xp->nx=point->nx;
		xp->ny=point->ny;
		xp->nz=point->nz;
		xp->u=0.0f;
		xp->v=0.0f;
	}

//
// Clear the surface poly strip pointers
//
	for( surface=surfaces->first ; surface->next ; surface=surface->next )
	{
		surface->polys_xox_indexs=0;
		surface->polys_xox_strips=0;
		surface->polys_xox_numof_strips=0;
	}

//
// step through the polys and find out how much space we need to store the poly strips in and record these numbers
//
	numof_polyindexs=0;
	numof_polystrips=0;
	last_link_type=0;
	for( poly=polys->first ; poly->next ; poly=poly->next )
	{
		poly->strip_length=0;

		if(last_link_type)
		{
			strip_start->strip_length+=1;
			numof_polyindexs+=1;
		}
		else
		{
			poly->surface->polys_xox_numof_strips+=1;
			numof_polyindexs+=3;
			numof_polystrips+=1;
			strip_start=poly;
			strip_start->strip_length=3;
		}

		last_link_type=poly->strip_link;
	}
//
// allocate memory for the output poly array
//
	polys_xox_sizeof=(2*(numof_polyindexs+numof_polystrips));
	if(!(polys_xox=(u16*)calloc(polys_xox_sizeof,1)))
	{
		DBG_Error("Failed to allocate memory for XOX output polys.\n");
		goto bogus;
	}
//
// step through the polys building the poly strips.
//
	last_link_type=0;
	for( poly=polys->first , ip=polys_xox , cp=polys_xox+numof_polyindexs ; poly->next ; poly=poly->next )
	{
		if(poly->surface->polys_xox_strips==0) // first poly of this surface so set up its pointer
		{
			poly->surface->polys_xox_indexs=ip;
			poly->surface->polys_xox_strips=cp;
		}

		if(last_link_type) // continue strip
		{
			a=last_link_type>>4;

			b=last_link_type&0xf;

			switch(last_link_type)
			{
				case 0x01:
				case 0x12:
				case 0x20:
				{
					if( ( poly->prev->points[a]==poly->points[0] ) && ( poly->prev->points[b]==poly->points[2] ) )
					{
						*ip++=poly->points[1]->output_index;
					}
					else
					if( ( poly->prev->points[a]==poly->points[1] ) && ( poly->prev->points[b]==poly->points[0] ) )
					{
						*ip++=poly->points[2]->output_index;
					}
					else
					if( ( poly->prev->points[a]==poly->points[2] ) && ( poly->prev->points[b]==poly->points[1] ) )
					{
						*ip++=poly->points[0]->output_index;
					}
					else
					{
						DBG_Error("Bad poly strip encountered.\n");
						goto bogus;
					}
				}
				break;

				case 0x02:
				case 0x10:
				case 0x21:
				{
					if( ( poly->prev->points[a]==poly->points[0] ) && ( poly->prev->points[b]==poly->points[1] ) )
					{
						*ip++=poly->points[2]->output_index;
					}
					else
					if( ( poly->prev->points[a]==poly->points[1] ) && ( poly->prev->points[b]==poly->points[2] ) )
					{
						*ip++=poly->points[0]->output_index;
					}
					else
					if( ( poly->prev->points[a]==poly->points[2] ) && ( poly->prev->points[b]==poly->points[0] ) )
					{
						*ip++=poly->points[1]->output_index;
					}
					else
					{
						DBG_Error("Bad poly strip encountered.\n");
						goto bogus;
					}
				}
				break;

				default:
				{
					DBG_Error("Bad poly strip encountered.\n");
					goto bogus;
				}
				break;
			}
		}
		else
		{
			*cp++=poly->strip_length;

			switch(poly->strip_link)
			{
				case 0x01:
				{
					*ip++=poly->points[2]->output_index;
					*ip++=poly->points[0]->output_index;
					*ip++=poly->points[1]->output_index;
				}
				break;

				case 0:
				case 0x12:
				{
					*ip++=poly->points[0]->output_index;
					*ip++=poly->points[1]->output_index;
					*ip++=poly->points[2]->output_index;
				}
				break;

				case 0x20:
				{
					*ip++=poly->points[1]->output_index;
					*ip++=poly->points[2]->output_index;
					*ip++=poly->points[0]->output_index;
				}
				break;

				default:
				{
					DBG_Error("Bad poly strip encountered.\n");
					goto bogus;
				}
				break;
			}

		}

		last_link_type=poly->strip_link;
	}


//
// step through the morphs and find out how much space we need to store each one, basicaly the min/max pointid of each
//
	for( morph=morphs->first ; morph->next ; morph=morph->next )
	{
		morph->min_pointid=65536*256;
		morph->max_pointid=0;
		for( point=points->first ; point->next ; point=point->next )
		{
			if(pm=point->find_morph_point(morph))
			{
				if(point->output_index<morph->min_pointid) morph->min_pointid=point->output_index;
				if(point->output_index>morph->max_pointid) morph->max_pointid=point->output_index;
			}
		}

		if( morph->min_pointid > morph->max_pointid ) // no points
		{
			morph->min_pointid=0;
			morph->max_pointid=0;
			morph->numof_points=0;
		}
		else
		{
			morph->numof_points= ( morph->max_pointid - morph->min_pointid ) + 1;
		}
	}

//
// step through morphs and allocate then build output data
//
	for( morph=morphs->first ; morph->next ; morph=morph->next )
	{
		morph->morphs_xox=0;
		morph->morphs_xox_sizeof=sizeof(XOX0_morph_point)*morph->numof_points;

		if(morph->morphs_xox_sizeof)
		{
			if(!(morph->morphs_xox=(f32*)calloc(morph->morphs_xox_sizeof,1)))
			{
				DBG_Error("Failed to allocate memory for XOX output morph points.\n");
				goto bogus;
			}
			for( point=points->first ; point->next ; point=point->next )
			{
				if(pm=point->find_morph_point(morph))
				{
					mp=(XOX0_morph_point*)morph->morphs_xox;
					mp+=point->output_index-morph->min_pointid;

					mp->x=pm->x;
					mp->y=pm->y;
					mp->z=pm->z;

					mp->nx=pm->nx-point->nx;
					mp->ny=pm->ny-point->ny;
					mp->nz=pm->nz-point->nz;
				}
			}

		}
	}

	BuildBounds();

	return true;
bogus:
	return false;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// work out bounding information
// 
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool t3d_object::BuildBounds()
{
t3d_point   *point;

f32 maxradrad;
f32 radrad;

	maxrad=0.0f;
	min->x=min->y=min->z=+F32_HUGE;
	max->x=max->y=max->z=-F32_HUGE;

	maxradrad=0.0f;

	for( point=points->first ; point->next ; point=point->next )
	{
		if(point->x>max->x) max->x=point->x;
		if(point->y>max->y) max->y=point->y;
		if(point->z>max->z) max->z=point->z;

		if(point->x<max->x) min->x=point->x;
		if(point->y<max->y) min->y=point->y;
		if(point->z<max->z) min->z=point->z;


		radrad= (point->x*point->x) + (point->y*point->y) + (point->z*point->z) ;

		if(radrad>maxradrad)
		{
			maxradrad=radrad;
			maxrad=f32_sqrt(radrad);
		}
	}

	return true;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// merge multiple bones into a single bone referencing multiple objects with various weights
//
// these bones will then be mapped onto each point.
//
// unused bones will then be removed.
//
// This then makes for a simple bone definition
//
// The bones are then handed out their own unique ID
// as well as each reference name
// 
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool t3d_object::BuildBones()
{
t3d_bone *bone;
s32 i;



// number the remaining bones

	for( bone=bones->first , i=0 ; bone->next ; bone=bone->next , i++ )
	{
		bone->id=i;
	}

	return true;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// work out polygon and all point normals
// 
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool t3d_object::BuildNormals()
{
t3d_poly    *poly;
t3d_point   *point;

f32 len;
s32 i;


// clear all point normals to 0 so we can add to them as we build polys

	for( point=points->first  ; point->next ; point=point->next )
	{
		point->nx=0;
		point->ny=0;
		point->nz=0;
	}

// build normals

//	for( morph=morphs->first ; morph->next ; morph=morph->next )
	{
		for( poly=polys->first ; poly->next ; poly=poly->next )
		{
		v3 a[1];
		v3 b[1];
		v3 d[1];


			a->x= poly->points[1]->x - poly->points[0]->x ;
			a->y= poly->points[1]->y - poly->points[0]->y ;
			a->z= poly->points[1]->z - poly->points[0]->z ;

/*
			if(pm=poly->points[1]->find_morph_point(morph))
			{
				a->x+=pm->x;
				a->y+=pm->y;
				a->z+=pm->z;
			}
			if(pm=poly->points[0]->find_morph_point(morph))
			{
				a->x-=pm->x;
				a->y-=pm->y;
				a->z-=pm->z;
			}
*/

			b->x= poly->points[2]->x - poly->points[1]->x ;
			b->y= poly->points[2]->y - poly->points[1]->y ;
			b->z= poly->points[2]->z - poly->points[1]->z ;

/*
			if(pm=poly->points[2]->find_morph_point(morph))
			{
				b->x+=pm->x;
				b->y+=pm->y;
				b->z+=pm->z;
			}
			if(pm=poly->points[1]->find_morph_point(morph))
			{
				b->x-=pm->x;
				b->y-=pm->y;
				b->z-=pm->z;
			}
*/

			v3_cross_v3(d,a,b);

			poly->nx=d->x;
			poly->ny=d->y;
			poly->nz=d->z;

			len=poly->nx*poly->nx + poly->ny*poly->ny + poly->nz*poly->nz ;

			len=f32_sqrt(len);

			if(len!=0.0f)
			{
				len=1.0f/len;
			}

			for(i=0;i<3;i++)
			{
//				if(pm=poly->points[i]->find_morph_point(morph))
				{
					poly->points[i]->nx+=poly->nx;
					poly->points[i]->ny+=poly->ny;
					poly->points[i]->nz+=poly->nz;
				}
			}
		}
	}


// normalize all normals

	for( point=points->first  ; point->next ; point=point->next )
	{
		len=point->nx*point->nx + point->ny*point->ny + point->nz*point->nz ;

		len=f32_sqrt(len);

		if(len!=0.0f)
		{
			len=1.0f/len;
		}

		point->nx*=len;
		point->ny*=len;
		point->nz*=len;

/*
		for( morph=morphs->first ; morph->next ; morph=morph->next )
		{
			if(pm=point->find_morph_point(morph))
			{
				len=pm->nx*pm->nx + pm->ny*pm->ny + pm->nz*pm->nz ;

				len=f32_sqrt(len);

				if(len!=0.0f)
				{
					len=1.0f/len;
				}

				pm->nx*=len;
				pm->ny*=len;
				pm->nz*=len;
			}
		}
*/

	}

	return true;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// save data as an XOX output object format data
// 
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool t3d_object::SaveXOX(const char *filename)
{
XOX0_header		xoxhead[1];
XOX0_info		xoxinfo[1];
XOX0_surface	xoxsurf[1];
XOX0_morph		xoxmorph[1];

FILE *fp=0;

bool ret;

t3d_surface *surface;
t3d_morph *morph;



s32 chunk_info			;
s32 chunk_points		;
s32 chunk_morphpoints	;
s32 chunk_polyindexs	;
s32 chunk_polystrips	;
s32 chunk_surfaces		;
s32 chunk_morphs		;

s32 chunk_pos;

char *cp;

XOX0_morph_point *mpoff;

	ret=false;

	if(!BuildXOX()) { goto bogus; }

	if(!(fp=fopen(filename,"wb")))
	{
		DBG_Error("Failed to open output file \"%s\".\n",filename);
		goto bogus;
	}

	xoxinfo->numof_points=numof_points;
	xoxinfo->numof_polys=numof_polys;
	xoxinfo->numof_polyindexs=numof_polyindexs;
	xoxinfo->numof_polystrips=numof_polystrips;
	xoxinfo->numof_surfaces=numof_surfaces;

	xoxinfo->numof_morphs=0;
	xoxinfo->numof_morphpoints=0;

	for( morph=morphs->first ; morph->next ; morph=morph->next )
	{
		xoxinfo->numof_morphs++;
		xoxinfo->numof_morphpoints+=morph->numof_points;
	}


	chunk_info			=	( 1								*	sizeof(XOX0_info)			) ;
	chunk_points		=	( xoxinfo->numof_points			*	sizeof(XOX0_point)			) ;
	chunk_morphpoints	=	( xoxinfo->numof_points			*	sizeof(XOX0_morph_point)	) ;
	chunk_polyindexs	=	( xoxinfo->numof_polyindexs		*	sizeof(u16)					) ;
	chunk_polystrips	=	( xoxinfo->numof_polystrips		*	sizeof(u16)					) ;
	chunk_surfaces		=	( xoxinfo->numof_surfaces		*	sizeof(XOX0_surface)		) ;
	chunk_morphs		=	( xoxinfo->numof_morphs			*	sizeof(XOX0_morph)			) ;
	chunk_pos			=	0;

	xoxhead->id=U32_ID4_XOX0;
	xoxhead->version=XOX0_VERSION;
	xoxhead->filesize= chunk_info + chunk_surfaces + chunk_morphs + chunk_points + chunk_polyindexs + chunk_polystrips ;


	xoxinfo->surfaces=(XOX0_surface*)			(	chunk_info																				);
	xoxinfo->morphs=(XOX0_morph*)				(	chunk_info+chunk_surfaces																						);
	xoxinfo->points=(XOX0_point*)				(	chunk_info+chunk_surfaces+chunk_morphs													);
	xoxinfo->indexs=(u16*)						(	chunk_info+chunk_surfaces+chunk_morphs+chunk_points										);
	xoxinfo->strips=(u16*)						(	chunk_info+chunk_surfaces+chunk_morphs+chunk_points+chunk_polyindexs					);
	xoxinfo->morphpoints=(XOX0_morph_point*)	(	chunk_info+chunk_surfaces+chunk_morphs+chunk_points+chunk_polyindexs+chunk_polystrips	);

	xoxinfo->maxrad=maxrad;

	xoxinfo->max->x=max->x;
	xoxinfo->max->y=max->y;
	xoxinfo->max->z=max->z;

	xoxinfo->min->x=min->x;
	xoxinfo->min->y=min->y;
	xoxinfo->min->z=min->z;

	if(!xoxinfo->numof_morphs)
	{
		xoxinfo->morphs=0;
		xoxinfo->morphpoints=0;
	}


//
// dump out header
//
	xoxhead->twiddle();
	if(1!=fwrite((void*)xoxhead,12,1,fp))
	{
		DBG_Error("Failed to write to output file \"%s\".\n",filename);
		goto bogus;
	}
	xoxhead->twiddle();

//
// dump out info
//
	xoxinfo->twiddle();
	if(1!=fwrite((void*)xoxinfo,sizeof(xoxinfo),1,fp))
	{
		DBG_Error("Failed to write to output file \"%s\".\n",filename);
		goto bogus;
	}
	xoxinfo->twiddle();

//
// dump out surfaces
//
	for( surface=surfaces->first ; surface->next ; surface=surface->next )
	{
		for( cp=xoxsurf->name ; cp < xoxsurf->name+sizeof(xoxsurf->name) ; cp ++ ) { *cp=0; }
		strncpy(xoxsurf->name,surface->name,sizeof(xoxsurf->name)-1);

		xoxsurf->argb=ARGBU32( surface->a  , surface->r  , surface->g  , surface->b  );
		xoxsurf->spec=ARGBU32( surface->sa , surface->sr , surface->sg , surface->sb );
		xoxsurf->gloss=surface->gloss;

		xoxsurf->numof_strips=surface->polys_xox_numof_strips;
		xoxsurf->strips=(u16*)( chunk_info + chunk_surfaces + chunk_morphs + chunk_points + ((surface->polys_xox_strips-polys_xox)*2) );
		xoxsurf->index_base=surface->polys_xox_indexs-polys_xox;

		xoxsurf->min_point=0;
		xoxsurf->max_point=numof_points-1;

		xoxsurf->twiddle();
		if(1!=fwrite((void*)xoxsurf,sizeof(xoxsurf),1,fp))
		{
			DBG_Error("Failed to write to output file \"%s\".\n",filename);
			goto bogus;
		}
		xoxsurf->twiddle();
	}

//
// dump out morphs
//
	mpoff=xoxinfo->morphpoints;

	if(chunk_morphs)
	for( morph=morphs->first ; morph->next ; morph=morph->next )
	{
		for( cp=xoxmorph->name ; cp < xoxmorph->name+sizeof(xoxmorph->name) ; cp ++ ) { *cp=0; }
		strncpy(xoxmorph->name,morph->name,sizeof(xoxmorph->name)-1);

		xoxmorph->min_point=morph->min_pointid;
		xoxmorph->numof_points=morph->numof_points;

		xoxmorph->points=mpoff;
		mpoff+=xoxmorph->numof_points;

		xoxmorph->twiddle();
		if(1!=fwrite((void*)xoxmorph,sizeof(xoxmorph),1,fp))
		{
			DBG_Error("Failed to write to output file \"%s\".\n",filename);
			goto bogus;
		}
		xoxmorph->twiddle();
	}

//
// dump out points
//
	if(sizeof(XOX0_point)!=fwrite((void*)points_xox,numof_points,sizeof(XOX0_point),fp))
	{
		DBG_Error("Failed to write to output file \"%s\".\n",filename);
		goto bogus;
	}

//
// dump out polys
//
	if(2!=fwrite((void*)polys_xox,numof_polyindexs+numof_polystrips,2,fp))
	{
		DBG_Error("Failed to write to output file \"%s\".\n",filename);
		goto bogus;
	}
//
// dump out morph points
//
	for( morph=morphs->first ; morph->next ; morph=morph->next )
	{
		if(morph->morphs_xox)
		{
			if(sizeof(XOX0_morph_point)!=fwrite((void*)morph->morphs_xox,morph->numof_points,sizeof(XOX0_morph_point),fp))
			{
				DBG_Error("Failed to write to output file \"%s\".\n",filename);
				goto bogus;
			}
		}
	}


	ret=true;
bogus:
	if(fp) { fclose(fp); }
	return ret;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// sort points, by polygon order
// 
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool t3d_object::SortPoints()
{
t3d_poly    *poly;
t3d_point   *point;
t3d_point   *pointnext;
t3d_point   *point_insert;

	point_insert=points->first->prev;

//
// step through all the polys
//
	for( poly=polys->first ; poly->next ; poly=poly->next )
	{
//
// and move the points used in each of the polys up the list if they havent already been moved
//
		for( point=point_insert->next , pointnext=point->next ; pointnext ; point=pointnext , pointnext=point->next )
		{
			if( ( point==poly->points[0] ) ||  ( point==poly->points[1] ) ||  ( point==poly->points[2] ) )
			{
				DLIST_CUTPASTE(point_insert,point,0);
				point_insert=point;
			}
		}
	}

	return true;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// sort polygons, by surface ID and then into adjacent tris for striping
// 
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool t3d_object::SortPolys()
{
t3d_poly    *polya;
t3d_poly    *polyanext;
t3d_poly    *polyb;
t3d_poly    *polybprev;

//s32 links;

u32 link_type;
u32 next_link_type;

s32 strip;

s32 count;

/*
	DBG_Info("before\n");
	for( polya=polys->first , polyanext=polya->next ; polyanext ; polya=polyanext , polyanext=polya->next )
	{
		DBG_Info("%d\n",polya->surface->id);
	}
	DBG_Info("before\n");
*/

//
// sort by surface
//
	for( polya=polys->first , polyanext=polya->next ; polyanext ; polya=polyanext , polyanext=polya->next )
	{
		for( polyb=polya->prev , polybprev=polyb->prev ; polybprev ; polyb=polybprev , polybprev=polyb->prev )
		{
			if( (polya->surface && polyb->surface) && (polyb->surface->id < polya->surface->id ) )
			{
				break;
			}
		}
		DLIST_CUTPASTE(polyb,polya,0);
	}
/*
	DBG_Info("after\n");
	for( polya=polys->first , polyanext=polya->next ; polyanext ; polya=polyanext , polyanext=polya->next )
	{
		DBG_Info("%d\n",polya->surface->id);
	}
	DBG_Info("after\n");
	DBG_Info("after\n");
*/

//
// Sort by shared edges, look ahead into current surface only and do simple reorder, wont be optimum but will be better
//

/*
	links=0;
	for( polya=polys->first ; polya->next ; polya=polya->next )
	{
		polyb=polya->next;

			if	(
					(polyb->next)
					&&
					( polyb->surface->id == polya->surface->id )
					&&
					(
						( ( polya->points[0]->id == polyb->points[0]->id ) && ( polya->points[1]->id == polyb->points[2]->id ) ) ||
						( ( polya->points[0]->id == polyb->points[1]->id ) && ( polya->points[1]->id == polyb->points[0]->id ) ) ||
						( ( polya->points[0]->id == polyb->points[2]->id ) && ( polya->points[1]->id == polyb->points[1]->id ) ) ||

						( ( polya->points[1]->id == polyb->points[0]->id ) && ( polya->points[2]->id == polyb->points[2]->id ) ) ||
						( ( polya->points[1]->id == polyb->points[1]->id ) && ( polya->points[2]->id == polyb->points[0]->id ) ) ||
						( ( polya->points[1]->id == polyb->points[2]->id ) && ( polya->points[2]->id == polyb->points[1]->id ) ) ||

						( ( polya->points[2]->id == polyb->points[0]->id ) && ( polya->points[0]->id == polyb->points[2]->id ) ) ||
						( ( polya->points[2]->id == polyb->points[1]->id ) && ( polya->points[0]->id == polyb->points[0]->id ) ) ||
						( ( polya->points[2]->id == polyb->points[2]->id ) && ( polya->points[0]->id == polyb->points[1]->id ) )
					)
				)
			{
				links++;
			}
	}

	DBG_Info("links before == %d\n",links);
*/

	count=0;
	strip=0;
	link_type=0;
	for( polya=polys->first ; polya->next ; polya=polya->next )
	{
		count++;
		polya->strip=strip;

		polya->strip_link=0;

		for( polyb=polya->next ; polyb->next ; polyb=polyb->next )
		{
			if( (polya->surface && polyb->surface) && ( polyb->surface->id != polya->surface->id ) ) break; // stop looking when we reach next surface

			switch(link_type)
			{
				case 0: // any

				case 0x01:
				{
					if( ( polya->points[0]->id == polyb->points[0]->id ) && ( polya->points[1]->id == polyb->points[2]->id ) )
					{
						DLIST_CUTPASTE(polya,polyb,0);
						polya->strip_link=0x01;
						next_link_type=0x21;
						goto found;
					}
					if( ( polya->points[0]->id == polyb->points[1]->id ) && ( polya->points[1]->id == polyb->points[0]->id ) )
					{
						DLIST_CUTPASTE(polya,polyb,0);
						polya->strip_link=0x01;
						next_link_type=0x02;
						goto found;
					}
					if( ( polya->points[0]->id == polyb->points[2]->id ) && ( polya->points[1]->id == polyb->points[1]->id ) )
					{
						DLIST_CUTPASTE(polya,polyb,0);
						polya->strip_link=0x01;
						next_link_type=0x10;
						goto found;
					}
				}
				if(link_type) break;

				case 0x12:
				{
					if( ( polya->points[1]->id == polyb->points[0]->id ) && ( polya->points[2]->id == polyb->points[2]->id ) )
					{
						DLIST_CUTPASTE(polya,polyb,0);
						polya->strip_link=0x12;
						next_link_type=0x21;
						goto found;
					}
					if( ( polya->points[1]->id == polyb->points[1]->id ) && ( polya->points[2]->id == polyb->points[0]->id ) )
					{
						DLIST_CUTPASTE(polya,polyb,0);
						polya->strip_link=0x12;
						next_link_type=0x02;
						goto found;
					}
					if( ( polya->points[1]->id == polyb->points[2]->id ) && ( polya->points[2]->id == polyb->points[1]->id ) )
					{
						DLIST_CUTPASTE(polya,polyb,0);
						polya->strip_link=0x12;
						next_link_type=0x10;
						goto found;
					}
				}
				if(link_type) break;

				case 0x20:
				{
					if( ( polya->points[2]->id == polyb->points[0]->id ) && ( polya->points[0]->id == polyb->points[2]->id ) )
					{
						DLIST_CUTPASTE(polya,polyb,0);
						polya->strip_link=0x20;
						next_link_type=0x21;
						goto found;
					}
					if( ( polya->points[2]->id == polyb->points[1]->id ) && ( polya->points[0]->id == polyb->points[0]->id ) )
					{
						DLIST_CUTPASTE(polya,polyb,0);
						polya->strip_link=0x20;
						next_link_type=0x02;
						goto found;
					}
					if( ( polya->points[2]->id == polyb->points[2]->id ) && ( polya->points[0]->id == polyb->points[1]->id ) )
					{
						DLIST_CUTPASTE(polya,polyb,0);
						polya->strip_link=0x20;
						next_link_type=0x10;
						goto found;
					}
				}
				break;


				case 0x02:
				{
					if( ( polya->points[0]->id == polyb->points[0]->id ) && ( polya->points[2]->id == polyb->points[1]->id ) )
					{
						DLIST_CUTPASTE(polya,polyb,0);
						polya->strip_link=0x02;
						next_link_type=0x12;
						goto found;
					}
					if( ( polya->points[0]->id == polyb->points[1]->id ) && ( polya->points[2]->id == polyb->points[2]->id ) )
					{
						DLIST_CUTPASTE(polya,polyb,0);
						polya->strip_link=0x02;
						next_link_type=0x20;
						goto found;
					}
					if( ( polya->points[0]->id == polyb->points[2]->id ) && ( polya->points[2]->id == polyb->points[0]->id ) )
					{
						DLIST_CUTPASTE(polya,polyb,0);
						polya->strip_link=0x02;
						next_link_type=0x01;
						goto found;
					}
				}
				break;

				case 0x10:
				{
					if( ( polya->points[1]->id == polyb->points[0]->id ) && ( polya->points[0]->id == polyb->points[1]->id ) )
					{
						DLIST_CUTPASTE(polya,polyb,0);
						polya->strip_link=0x10;
						next_link_type=0x12;
						goto found;
					}
					if( ( polya->points[1]->id == polyb->points[1]->id ) && ( polya->points[0]->id == polyb->points[2]->id ) )
					{
						DLIST_CUTPASTE(polya,polyb,0);
						polya->strip_link=0x10;
						next_link_type=0x20;
						goto found;
					}
					if( ( polya->points[1]->id == polyb->points[2]->id ) && ( polya->points[0]->id == polyb->points[0]->id ) )
					{
						DLIST_CUTPASTE(polya,polyb,0);
						polya->strip_link=0x10;
						next_link_type=0x01;
						goto found;
					}
				}
				break;

				case 0x21:
				{
					if( ( polya->points[2]->id == polyb->points[0]->id ) && ( polya->points[1]->id == polyb->points[1]->id ) )
					{
						DLIST_CUTPASTE(polya,polyb,0);
						polya->strip_link=0x21;
						next_link_type=0x12;
						goto found;
					}
					if( ( polya->points[2]->id == polyb->points[1]->id ) && ( polya->points[1]->id == polyb->points[2]->id ) )
					{
						DLIST_CUTPASTE(polya,polyb,0);
						polya->strip_link=0x21;
						next_link_type=0x20;
						goto found;
					}
					if( ( polya->points[2]->id == polyb->points[2]->id ) && ( polya->points[1]->id == polyb->points[0]->id ) )
					{
						DLIST_CUTPASTE(polya,polyb,0);
						polya->strip_link=0x21;
						next_link_type=0x01;
						goto found;
					}
				}
				break;

			}
		}
//		DBG_Info("surf %d count %d\n",polya->surface->id,count);
		count=0;
		strip++;
		next_link_type=0;
found:
		link_type=next_link_type;
//		DBG_Info("surf %d linktype %d\n",polya->surface->id,link_type);
	}

//	DBG_Info("number of strips == %d\n",strip);

/*
	links=0;
	for( polya=polys->first ; polya->next ; polya=polya->next )
	{
		polyb=polya->next;

			if	(
					(polyb->next)
					&&
					( polyb->surface->id == polya->surface->id )
					&&
					(
						( ( polya->points[0]->id == polyb->points[0]->id ) && ( polya->points[1]->id == polyb->points[2]->id ) ) ||
						( ( polya->points[0]->id == polyb->points[1]->id ) && ( polya->points[1]->id == polyb->points[0]->id ) ) ||
						( ( polya->points[0]->id == polyb->points[2]->id ) && ( polya->points[1]->id == polyb->points[1]->id ) ) ||

						( ( polya->points[1]->id == polyb->points[0]->id ) && ( polya->points[2]->id == polyb->points[2]->id ) ) ||
						( ( polya->points[1]->id == polyb->points[1]->id ) && ( polya->points[2]->id == polyb->points[0]->id ) ) ||
						( ( polya->points[1]->id == polyb->points[2]->id ) && ( polya->points[2]->id == polyb->points[1]->id ) ) ||

						( ( polya->points[2]->id == polyb->points[0]->id ) && ( polya->points[0]->id == polyb->points[2]->id ) ) ||
						( ( polya->points[2]->id == polyb->points[1]->id ) && ( polya->points[0]->id == polyb->points[0]->id ) ) ||
						( ( polya->points[2]->id == polyb->points[2]->id ) && ( polya->points[0]->id == polyb->points[1]->id ) )
					)
				)
			{
				links++;
			}
	}
	DBG_Info("links after == %d\n",links);
*/

	return true;
}




/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// turn a point index into a pointer to a point
// 
/*+-----------------------------------------------------------------------------------------------------------------+*/
t3d_point * t3d_object::findpoint(s32 index)
{
t3d_point *point;

	for( point=points->first ; point->next ; point=point->next )
	{
		if(point->id==index)
		{
			return point;
		}
	}

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// turn a map index into a pointer to a point
// 
/*+-----------------------------------------------------------------------------------------------------------------+*/
t3d_point * t3d_object::findmap(s32 index)
{
t3d_point *point;

	for( point=maps->first ; point->next ; point=point->next )
	{
		if(point->id==index)
		{
			return point;
		}
	}

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// turn a map index into a pointer to a point
// 
/*+-----------------------------------------------------------------------------------------------------------------+*/
t3d_poly * t3d_object::findpoly(s32 index)
{
t3d_poly *poly;

	for( poly=polys->first ; poly->next ; poly=poly->next )
	{
		if(poly->id==index)
		{
			return poly;
		}
	}

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// turn a surface index into a pointer to a surface
// 
/*+-----------------------------------------------------------------------------------------------------------------+*/
t3d_surface * t3d_object::findsurface(s32 index)
{
t3d_surface *surface;

	for( surface=surfaces->first ; surface->next ; surface=surface->next )
	{
		if(surface->id==index)
		{
			return surface;
		}
	}

	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// find morph point for given morph?
// 
/*+-----------------------------------------------------------------------------------------------------------------+*/
t3d_point * t3d_point::find_morph_point(struct t3d_morph *morph)
{
t3d_point *ret;

	for( ret=morphs->first ; ret->next ; ret=ret->next )
	{
		if( ret->id == morph->id )
		{
			return ret;
		}
	}

	return 0;
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// make or return a previously made bone, (different weights make extra bones)
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
t3d_bone * t3d_object::ref_bone( const char *name0, f32 weight0 , const char *name1, f32 weight1 )
{
t3d_bone *bone;


// first try to find

	for( bone=bones->first ; bone->next ; bone=bone->next )
	{
		if(strcmp(name0,bone->name[0])==0)
		{
			if( weight0 == bone->weight[0] )
			{
				if(strcmp(name1,bone->name[1])==0)
				{
					if( weight1 == bone->weight[1] )
					{
						return bone;
					}
				}
			}
		}
	}

// then create new


	bone=master->AllocBone();
	if(bone)
	{
		DLIST_CUTPASTE(bones->last,bone,0);
		numof_bones++;

		strncpy(bone->name[0],name0,sizeof(bone->name[0])); bone->name[0][sizeof(bone->name[0])-1]=0;
		bone->weight[0]=weight0;

		strncpy(bone->name[1],name1,sizeof(bone->name[1])); bone->name[1][sizeof(bone->name[1])-1]=0;
		bone->weight[1]=weight1;
	}

	return bone;
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// make or return a previously made bone, (different weights make extra bones)
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
t3d_bone * t3d_object::findbone(s32 index)
{
t3d_bone *bone;
s32 i;

// first try to find

	for( bone=bones->first , i=0 ; bone->next ; bone=bone->next , i++ )
	{
		if( i==index )
		{
			return bone;
		}
	}

	return 0;
}



