/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// how much memory does an xox need to fit this info with no other allocations
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
s32 fenestra_ogl::xox_sizeof(const struct XOX0_info *xox_info)
{
	s32 mem=0;
	mem+= sizeof(struct XOX)*1;
	mem+= sizeof(struct XOX_surface)*xox_info->numof_surfaces;
	mem+= sizeof(struct XOX_point)*xox_info->numof_points;
	mem+= sizeof(struct XOX_morph)*xox_info->numof_morphs;
	
	return mem;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// test junk
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void fenestra_ogl::xox_setup( struct XOX *xox,const struct XOX0_info *xox_info)
{
// we expect all memory needed to already have been allocated following this struct...
	memset((u8*)xox,0,xox_sizeof(xox_info));

	xox->info=xox_info;
	
	xox->numof_surfaces=xox_info->numof_surfaces;
	xox->numof_points=xox_info->numof_points;
	xox->numof_morphs=xox_info->numof_morphs;
	
	xox->surfaces=(struct XOX_surface *)(xox+1);
	xox->points=(struct XOX_point *)(xox->surfaces+xox->numof_surfaces);
	xox->morphs=(struct XOX_morph *)(xox->points+xox->numof_points);
	
struct XOX_surface *surf;
struct XOX0_surface *surf0;

struct XOX_point *point;
struct XOX0_point *point0;

// copy surfaces so we may fiddlw with them
	for( surf=xox->surfaces , surf0=xox->info->surfaces ; surf<xox->surfaces+xox->numof_surfaces ; surf++ , surf0++ )
	{
		surf->argb			=surf0->argb;
		surf->flags			=surf0->flags;
		surf->gloss			=2.0f + surf0->gloss*512.0f;
		surf->index_base	=surf0->index_base;
		surf->max_point		=surf0->max_point;
		surf->min_point		=surf0->min_point;
		surf->numof_polys	=surf0->numof_polys;
		surf->numof_strips	=surf0->numof_strips;
		surf->spec			=surf0->spec;
		surf->strips		=surf0->strips;
		
		strncpy(surf->name,surf0->name,32);
	}
// copy points so we may fiddle with them	
	for( point=xox->points , point0=xox->info->points ; point<xox->points+xox->numof_points ; point++ , point0++ )
	{
		point->x=point0->x;
		point->y=point0->y;
		point->z=point0->z;
		point->nx=point0->nx;
		point->ny=point0->ny;
		point->nz=point0->nz;
		point->u=point0->u;
		point->v=point0->v;
		point->argb=point0->argb;
	}
	
	xox->need_rebuild=true;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// test junk
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void fenestra_ogl::xox_clean( struct XOX *xox)
{
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// test junk
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void fenestra_ogl::xox_update( struct XOX *xox)
{
	if(!xox->need_rebuild) { return; }
	xox->need_rebuild=false;
	
XOX_point *v;
XOX0_point *p;
XOX0_morph_point *mp;
XOX_morph *mb;
f32 f;
XOX0_morph *m;


	if(xox->flipX)
	{
		for( v=xox->points , p=xox->info->points ; p<xox->info->points+xox->info->numof_points ; v++ , p++)
		{
			v->x=-p->x;
			v->y=p->y;
			v->z=p->z;
			v->nx=-p->nx;
			v->ny=p->ny;
			v->nz=p->nz;
		}

		for( m=xox->info->morphs , mb=xox->morphs ; m<xox->info->morphs+xox->info->numof_morphs ; m++ , mb++)
		{
			f=mb->f;
			if(f!=0.0f)
			{
				for( v=xox->points+m->min_point , mp=m->points ; mp<m->points+m->numof_points ; v++ , mp++)
				{
					v->x-=mp->x*f;
					v->y+=mp->y*f;
					v->z+=mp->z*f;
					v->nx-=mp->nx*f;
					v->ny+=mp->ny*f;
					v->nz+=mp->nz*f;
				}
			}
		}
	}
	else
	{
		for( v=xox->points , p=xox->info->points ; p<xox->info->points+xox->info->numof_points ; v++ , p++)
		{
			v->x=p->x;
			v->y=p->y;
			v->z=p->z;
			v->nx=p->nx;
			v->ny=p->ny;
			v->nz=p->nz;
		}

		for( m=xox->info->morphs , mb=xox->morphs ; m<xox->info->morphs+xox->info->numof_morphs ; m++ , mb++)
		{
			f=mb->f;
			if(f!=0.0f)
			{
				for( v=xox->points+m->min_point , mp=m->points ; mp<m->points+m->numof_points ; v++ , mp++)
				{
					v->x+=mp->x*f;
					v->y+=mp->y*f;
					v->z+=mp->z*f;
					v->nx+=mp->nx*f;
					v->ny+=mp->ny*f;
					v->nz+=mp->nz*f;
				}
			}
		}
	}
	
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// test junk
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void fenestra_ogl::xox_draw( struct XOX *xox)
{
u16 *slist;
XOX_surface *surf;
s32 count;
s32 idx;
u32 argb;

GLfloat color[4];// = { 0.0f, 0.0f, 0.0f, 1.0f };

	glEnable(GL_CULL_FACE);
	
	if(xox->flipX)
	{
		glCullFace(GL_FRONT);
	}
	else
	{
		glCullFace(GL_BACK);
	}

	glEnableClientState(GL_VERTEX_ARRAY);

	glVertexPointer(3, GL_FLOAT, 9*4, &(xox->points[0].x) );

	glEnableClientState(GL_NORMAL_ARRAY);

	glNormalPointer(GL_FLOAT, 9*4, &(xox->points[0].nx) );
	
	glDisable( GL_COLOR_MATERIAL );
	
	for( surf=xox->surfaces ; surf < xox->surfaces + xox->numof_surfaces ; surf++ )
	{
	
		argb=surf->argb;
		if(force_diffuse) { argb=force_diffuse; }
		color[0]= ((argb>>16)&0xff) / 255.0f ;
		color[1]= ((argb>> 8)&0xff) / 255.0f ;
		color[2]= ((argb>> 0)&0xff) / 255.0f ;
		color[3]= ((argb>>24)&0xff) / 255.0f ;
		glMaterialfv ( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color );
		
		argb=surf->spec;
		if(force_spec) { argb=force_spec; }
//		argb=0;
		color[0]= ((argb>>16)&0xff) / 255.0f ;
		color[1]= ((argb>> 8)&0xff) / 255.0f ;
		color[2]= ((argb>> 0)&0xff) / 255.0f ;
		color[3]= ((argb>>24)&0xff) / 255.0f ;
		glMaterialfv ( GL_FRONT_AND_BACK, GL_SPECULAR, color );
		
		if(force_gloss>=0)
		{
			glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, force_gloss );
		}
		else
		{
			glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, surf->gloss );
		}
		
//		glEnable(GL_COLOR_SUM_EXT);

//		argb=surf->argb;		
//		glColor4ub( (argb>>16)&0xff , (argb>>8)&0xff , (argb)&0xff , (argb>>24)&0xff );
		
//		argb=surf->spec;
//		glSecondaryColor3ub( (argb>>16)&0xff , (argb>>8)&0xff , (argb)&0xff );
		
					
		for( slist=surf->strips , idx=surf->index_base ; slist < surf->strips + surf->numof_strips ; slist++ )
		{
			count=*slist;

			glDrawElements( GL_TRIANGLE_STRIP , count, GL_UNSIGNED_SHORT , xox->info->indexs+idx );

			idx+=count;
		}
		
	}
	

glEnable( GL_COLOR_MATERIAL );

glDisableClientState(GL_VERTEX_ARRAY);
glDisableClientState(GL_NORMAL_ARRAY);


}
