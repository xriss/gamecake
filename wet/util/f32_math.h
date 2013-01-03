/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

//
// vector/matrix/quat all junk that involves arrays of f32s or just f32 math
//



/*------------------------------------------------------------------------------------------------------------------------------*/
//
// some usefukll macro sets using stems
//
/*------------------------------------------------------------------------------------------------------------------------------*/

#define  SETXYZ( stem , xx , yy , zz ) stem  x = xx ; stem  y = yy ; stem  z = zz
#define HSETXYZ( stem , xx , yy , zz ) stem##x = xx ; stem##y = yy ; stem##z = zz

#define SSETXYZ( stema , stemb , xx , yy , zz ) stema x stemb = xx ; stema y stemb = yy ; stema z stemb = zz

#define  SETARGB( stem , aa , rr , gg , bb ) stem  a = aa ; stem  r = rr ; stem  g = gg ; stem  b = bb
#define HSETARGB( stem , aa , rr , gg , bb ) stem##a = aa ; stem##r = rr ; stem##g = gg ; stem##b = bb

#define SSETARGB( stema , stemb  , aa , rr , gg , bb ) stema a stemb = aa ; stema r stemb = rr ; stema g stemb = gg ; stema b smb = bb

#define CLAMPVAL(val,min,max) if(val>(max)) { val=(max); } if(val<(min)) { val=(min); }


// forced cast cross product
#define FORCE_V3_CROSS_V3( d , a , b ) v3_cross_v3((v3*)d,(const v3*)a,(const v3*)b)

/*------------------------------------------------------------------------------------------------------------------------------*/
//
// Useful constants
//
/*------------------------------------------------------------------------------------------------------------------------------*/
const f32 F32_PI       =  3.14159265358979323846f; // Pi
const f32 F32_2_PI     =  6.28318530717958623200f; // 2 * Pi
const f32 F32_PI_DIV_2 =  1.57079632679489655800f; // Pi / 2
const f32 F32_PI_DIV_4 =  0.78539816339744827900f; // Pi / 4
const f32 F32_INV_PI   =  0.31830988618379069122f; // 1 / Pi
const f32 F32_DEGTORAD =  0.01745329251994329547f; // Degrees to Radians
const f32 F32_RADTODEG = 57.29577951308232286465f; // Radians to Degrees
const f32 F32_HUGE     =  1.0e+38f;                // Huge number for FLOAT
const f32 F32_EPSILON  =  1.0e-5f;                 // Tolerance for FLOATs



const f32 F32_RAD360   =  F32_2_PI;
const f32 F32_RAD180   =  F32_PI;
const f32 F32_RAD90    =  F32_PI_DIV_2;
const f32 F32_RAD45    =  F32_PI_DIV_4;



/*------------------------------------------------------------------------------------------------------------------------------*/
//
// functions
//
/*------------------------------------------------------------------------------------------------------------------------------*/
void m44_mul_m44( struct m44 * q, const struct m44 * a, const struct m44 * b );

bool m44_invert( struct m44 * q, const struct m44 * a );


void m44_set_m44( struct m44 * mat, const struct m44 * from );

void m44_set_quat( struct m44 * mat, const struct v4 * quat );

void m44_set_tfm( struct m44 * mat, const struct tfm * tfm );

bool v3_mul_m44( struct v3 * vDest, const struct v3 * vSrc, const struct m44 * mat);

void v3_cross_v3( struct v3 * vDest , const struct v3 * vA , const struct v3 * vB );

void v3_mul_quat( v3 * vDest, const v3 * vSrc, const v4 * q);
void v3_mul_nquat( v3 * vDest, const v3 * vSrc, const v4 * q);

void quat_set_rot( struct v4 * quat , const struct v3 * v, f32 rot );
void quat_set_rotx( struct v4 * quat, f32 rot );
void quat_set_roty( struct v4 * quat, f32 rot );
void quat_set_rotz( struct v4 * quat, f32 rot );

void quat_rotx( struct v4 * quat, f32 rot );
void quat_roty( struct v4 * quat, f32 rot );
void quat_rotz( struct v4 * quat, f32 rot );

void rotx_quat( struct v4 * quat, f32 rot );
void roty_quat( struct v4 * quat, f32 rot );
void rotz_quat( struct v4 * quat, f32 rot );

void quat_set_ypr( struct v4 * quat , f32 yaw, f32 pitch, f32 roll );

void quat_mul_quat( struct v4 * q,  const struct v4 * a, const struct v4 * b );
void quat_mul_nquat( struct v4 * q,  const struct v4 * a, const struct v4 * b );

//void quat_slerp_quat( struct v4 * q, const struct v4 *a , const struct v4 *b , f32 fAlpha );
void quat_slerp_quat( struct v4 * q,  struct v4 *a ,  struct v4 *b , f32 fAlpha ); // broken

f32 f32_sqrt(f32 n);


#include <math.h>

inline f32 f32_floor( f32 num )
{
	return (f32)floor( (float)(num) );
}

inline f32 f32_ceil( f32 num )
{
	return (f32)ceil( (float)(num) );
}

/*------------------------------------------------------------------------------------------------------------------------------*/
//
// Fuzzy zero test
//
/*------------------------------------------------------------------------------------------------------------------------------*/
inline bool f32_IsZero( f32 a, f32 fTol = F32_EPSILON )
{
	return ( a <= 0.0f ) ? ( a >= -fTol ) : ( a <= fTol );
}

/*------------------------------------------------------------------------------------------------------------------------------*/
//
// some float snaps
//
/*------------------------------------------------------------------------------------------------------------------------------*/
inline f32 f32_lowsnap( f32 f , f32 step = 1.0f )
{
	return f32_floor(f/step)*step;
}

inline f32 f32_highsnap( f32 f , f32 step = 1.0f )
{
	return f32_ceil(f/step)*step;
}



/*------------------------------------------------------------------------------------------------------------------------------*/
//
// some simple curvy math functions, useful for smooth movement, is a nicer more natural curve than sine and without the lookup
//
// input -1 to 0 to +1 and output 1 to 0 to 1 smoothing to and from the 0 and 1
//
/*------------------------------------------------------------------------------------------------------------------------------*/
inline f32 f32_spine( f32 a )
{
f32 aa;

	if(a<0.0f) a=-a;

	aa=a*a;

	return ((aa+(aa*2.0f))-((aa*a)*2.0f));
}

/*------------------------------------------------------------------------------------------------------------------------------*/
//
// some simple curvy math functions, useful for smooth movement, is a nicer more natural curve than sine and without the lookup
//
// input 0 to +1 and output 1 to 0 to 1 with no sharp bits so it loops smoothly
//
/*------------------------------------------------------------------------------------------------------------------------------*/
inline f32 f32_spineloop( f32 a )
{
	return f32_spine((a*2.0f)-1.0f);
}

/*------------------------------------------------------------------------------------------------------------------------------*/
//
// some simple curvy math functions, useful for smooth movement, is a nicer more natural curve than sine and without the lookup
//
// input 0 to 1 and output 0 to 1 with a sharp up end on 1
//
/*------------------------------------------------------------------------------------------------------------------------------*/
inline f32 f32_spinebot( f32 a )
{
f32 aa;

	if(a<0.0f) a=-a;
	a*=0.5f;

	aa=a*a;

	return ((aa+(aa*2.0f))-((aa*a)*2.0f))*2.0f;
}

/*------------------------------------------------------------------------------------------------------------------------------*/
//
// some simple curvy math functions, usefull for smooth movement, is a nicer more natural curve than sine and without the lookup
//
// input 0 to 1 and output 0 to 1 with a sharp up end on 0
//
/*------------------------------------------------------------------------------------------------------------------------------*/
inline f32 f32_spinetop( f32 a )
{
f32 aa;

	if(a<0.0f) a=-a;
	a*=0.5f;
	a+=0.5f;

	aa=a*a;

	return (((aa+(aa*2.0f))-((aa*a)*2.0f))-0.5f)*2.0f;
}

/*------------------------------------------------------------------------------------------------------------------------------*/
//
// some simple curvy math functions, usefull for smooth movement, is a nicer more natural curve than sine and without the lookup
//
// input 0 to 1 and output 0 to 1 to 0 again with shape ends at the 0 points
//
/*------------------------------------------------------------------------------------------------------------------------------*/
inline f32 f32_spinearc( f32 a )
{
f32 aa;

	a-=0.5f;
	if(a<0.0f) a=-a;

	aa=a*a;

	return 1.0f-(((aa+(aa*2.0f))-((aa*a)*2.0f))*2.0f);
}


/*------------------------------------------------------------------------------------------------------------------------------*/
//
// simple Z only rot matrix
//
/*------------------------------------------------------------------------------------------------------------------------------*/
struct m22
{
	f32	xx,xy;
	f32	yx,yy;

	inline void reset(void)
	{
		xy=yx=0.0f;
		xx=yy=1.0f;
	}
};


/*------------------------------------------------------------------------------------------------------------------------------*/
//
// m33 class definition, its an array of 3x3 x32s
//
/*------------------------------------------------------------------------------------------------------------------------------*/
struct m33
{
	f32	xx,xy,xz;	// this row is a column :)
	f32 yx,yy,yz;
	f32 zx,zy,zz;

	inline void reset(void)
	{
		xy=xz=yx=yz=zx=zy=0.0f;
		xx=yy=zz=1.0f;
	}
};

/*------------------------------------------------------------------------------------------------------------------------------*/
//
// m33 class definition, its an array of 3x3 x32s
//
/*------------------------------------------------------------------------------------------------------------------------------*/
struct m44
{
	f32	xx,xy,xz,xw;	// this row is a column :)
	f32 yx,yy,yz,yw;
	f32 zx,zy,zz,zw;
	f32 wx,wy,wz,ww;

	inline void reset(void)
	{
		xy=xz=xw=yx=yz=yw=zx=zy=zw=wx=wy=wz=0.0f;
		xx=yy=zz=ww=1.0f;
	}

	inline void set(struct tfm *tfm)
	{
		m44_set_tfm(this,tfm);
	}

	inline void set(struct m44 *mat)
	{
		m44_set_m44(this,mat);
	}

	inline void set(const struct quat *q)
	{
		m44_set_quat(this,(const v4*)q);
	}

	inline void mul(const struct m44 *m)
	{
		m44_mul_m44(this,this,m);
	}


};




/*------------------------------------------------------------------------------------------------------------------------------*/
//
// v2 class definition, its an array of 2 x32s
//
/*------------------------------------------------------------------------------------------------------------------------------*/
struct v2
{
	f32	x;
	f32 y;

	inline void reset(void)
	{
		x=y=0.0f;
	}
};

/*------------------------------------------------------------------------------------------------------------------------------*/
//
// v3 class definition, its an array of 3 x32s
//
/*------------------------------------------------------------------------------------------------------------------------------*/
struct v3
{
	f32 x;
	f32 y;
	f32 z;

	inline void reset(void)
	{
		x=y=z=0.0f;
	}

	inline void set( const v3 *v ) // copy vector
	{
		x=v->x;
		y=v->y;
		z=v->z;
	}

	inline void set( f32 all ) // set vector
	{
		x=all;
		y=all;
		z=all;
	}

	inline f32 dot( const v3 *v ) // copy vector
	{
		return x*v->x + y*v->y + z*v->z;
	}

	inline void rot( const v4 *q ) // rotate by a quat
	{
		v3_mul_quat(this,this,q);
	}

	inline void nrot( const v4 *q ) // rotate by a quat
	{
		v3_mul_nquat(this,this,q);
	}

	inline void neg(void) // neg the vector
	{
		x=-x;
		y=-y;
		z=-z;
	}

	inline f32 lenlen(void) // length of vector squared
	{
		return x*x + y*y + z*z ;
	}

	inline f32 len(void) // length of vector
	{
		return f32_sqrt( x*x + y*y + z*z );
	}

	inline void norm(void) // scale to a length of 1
	{
	f32 len;

		len=lenlen();
		if(len>0.0f)
		{
			len=f32_sqrt(len);
			len=1.0f/len;

			x*=len;
			y*=len;
			z*=len;
		}
		else
		{
			x=0.0f;
			y=1.0f;
			z=0.0f;
		}
	}

};


/*------------------------------------------------------------------------------------------------------------------------------*/
//
// v3o its a v3 but with an ooz cache member
//
/*------------------------------------------------------------------------------------------------------------------------------*/
struct v3o : public v3
{
	f32 ooz;

	inline void reset(void)
	{
		x=y=z=0.0f;
		ooz=1.0f;
	}
};

/*------------------------------------------------------------------------------------------------------------------------------*/
//
// v4 class definition, its an array of 4 f32s , probably holds a quat
//
/*------------------------------------------------------------------------------------------------------------------------------*/
struct v4 : public v3
{
	f32 w;

	inline void reset(void)
	{
		x=y=z=w=0.0f;
	}

	inline void set( const v4 *v ) // copy vector
	{
		w=v->w;
		x=v->x;
		y=v->y;
		z=v->z;
	}

	inline void set( f32 all ) // set vector
	{
		w=all;
		x=all;
		y=all;
		z=all;
	}

	inline void neg(void) // neg the vector
	{
		w=-w;
		x=-x;
		y=-y;
		z=-z;
	}

};

/*------------------------------------------------------------------------------------------------------------------------------*/
//
// 2d box definition, its an array of 4 f32s  x,y gives the top left position , w,h gives the width and height of the area
//
// inline functions for center and radius can be used if you want to change values relative to a center
//
/*------------------------------------------------------------------------------------------------------------------------------*/
struct box 
{
	f32 x;
	f32 y;
	f32 w;
	f32 h;

	inline void reset(void)
	{
		x=y=w=h=0.0f;
	}

	inline void set(const box *a)
	{
		x=a->x;
		y=a->y;
		w=a->w;
		h=a->h;
	}

	inline void set(f32 _x, f32 _y, f32 _w, f32 _h)
	{
		x=_x;
		y=_y;
		w=_w;
		h=_h;
	}

	inline f32  get_radius(void)
	{
		return w*0.5f;
	}
	inline f32  get_radius_w(void)
	{
		return w*0.5f;
	}
	inline f32  get_radius_h(void)
	{
		return h*0.5f;
	}
	inline void set_radius(f32 rw, f32 rh)
	{
	f32 cx,cy;
		cx=get_center_x();
		cy=get_center_y();
		w=rw*2.0f;
		h=rh*2.0f;
		set_center_x(cx);
		set_center_y(cy);
	}
	inline void set_radius_w(f32 rw)
	{
	f32 cx;
		cx=get_center_x();
		w=rw*2.0f;
		set_center_x(cx);
	}
	inline void set_radius_h(f32 rh)
	{
	f32 cy;
		cy=get_center_y();
		h=rh*2.0f;
		set_center_y(cy);
	}
	inline void set_radius(f32 r)
	{
		set_radius(r,r);
	}


	inline f32 get_center_x(void)
	{
		return x+(w*0.5f);
	}
	inline void set_center_x(f32 c)
	{
		x=c-(w*0.5f);
	}

	inline f32 get_center_y(void)
	{
		return y+(h*0.5f);
	}
	inline void set_center_y(f32 c)
	{
		y=c-(h*0.5f);
	}

	inline bool inside(f32 xx,f32 yy)
	{
		return (xx>=x)&&(yy>=y)&&(xx<=x+w)&&(yy<=y+h);
	}
};

/*------------------------------------------------------------------------------------------------------------------------------*/
//
// 3d box definition, its an array of 5 f32s  x,y,z gives the top left position , w,h,d gives the width and height of the area
//
// inline functions for center and radius can be used if you want to change values relative to a center
//
/*------------------------------------------------------------------------------------------------------------------------------*/

struct cube 
{
	f32 x;
	f32 y;
	f32 z;
	f32 w;
	f32 h;
	f32 d;

	inline void reset(void)
	{
		x=y=z=w=h=d=0.0f;
	}

	inline void set(const cube *a)
	{
		x=a->x;
		y=a->y;
		z=a->z;
		w=a->w;
		h=a->h;
		d=a->d;
	}

	inline void set(f32 _x, f32 _y, f32 _z, f32 _w, f32 _h, f32 _d)
	{
		x=_x;
		y=_y;
		z=_z;
		w=_w;
		h=_h;
		d=_d;
	}

	inline f32  get_radius(void)
	{
		return w*0.5f;
	}
	inline f32  get_radius_w(void)
	{
		return w*0.5f;
	}
	inline f32  get_radius_h(void)
	{
		return h*0.5f;
	}
	inline f32  get_radius_d(void)
	{
		return d*0.5f;
	}
	inline void set_radius(f32 rw, f32 rh, f32 rd)
	{
	f32 cx,cy,cz;
		cx=get_center_x();
		cy=get_center_y();
		cz=get_center_z();
		w=rw*2.0f;
		h=rh*2.0f;
		d=rd*2.0f;
		set_center_x(cx);
		set_center_y(cy);
		set_center_y(cz);
	}
	inline void set_radius_w(f32 rw)
	{
	f32 cx;
		cx=get_center_x();
		w=rw*2.0f;
		set_center_x(cx);
	}
	inline void set_radius_h(f32 rh)
	{
	f32 cy;
		cy=get_center_y();
		h=rh*2.0f;
		set_center_y(cy);
	}
	inline void set_radius_d(f32 rd)
	{
	f32 cz;
		cz=get_center_z();
		d=rd*2.0f;
		set_center_z(cz);
	}
	inline void set_radius(f32 r)
	{
		set_radius(r,r,r);
	}


	inline f32 get_center_x(void)
	{
		return x+(w*0.5f);
	}
	inline void set_center_x(f32 c)
	{
		x=c-(w*0.5f);
	}

	inline f32 get_center_y(void)
	{
		return y+(h*0.5f);
	}
	inline void set_center_y(f32 c)
	{
		y=c-(h*0.5f);
	}

	inline f32 get_center_z(void)
	{
		return z+(d*0.5f);
	}
	inline void set_center_z(f32 c)
	{
		z=c-(d*0.5f);
	}
};

/*------------------------------------------------------------------------------------------------------------------------------*/
//
// quat class definition, its an array of 4 f32s
//
/*------------------------------------------------------------------------------------------------------------------------------*/
struct quat : public v4
{
	inline void reset(void)
	{
		x=y=z=0.0f;
		w=1.0f;
	}

	inline void rot(quat *rot)
	{
		quat_mul_quat(this,this,rot);
	}

	inline void nrot(quat *rot)
	{
		quat_mul_nquat(this,this,rot);
	}

	inline void rotx(f32 rot)
	{
		rotx_quat(this,rot);
	}

	inline void roty(f32 rot)
	{
		roty_quat(this,rot);
	}
 
	inline void rotz(f32 rot)
	{
		rotz_quat(this,rot); 
	}

	inline void norm(void)
	{
	f32 len;

		len= x*x + y*y + z*z + w*w ;
		len=f32_sqrt(len);
		len=1.0f/len;

		x*=len;
		y*=len;
		z*=len;
		w*=len; 
	}

	inline void normish(void)
	{
	f32 len;

		len= x*x + y*y + z*z + w*w ;

		if( (len>(1.0f+(1.0f/1024))*(1.0f+(1.0f/1024)))
			||
			(len<(1.0f-(1.0f/1024))*(1.0f-(1.0f/1024)))
		  )
		{
			len=f32_sqrt(len);
			len=1.0f/len;

			x*=len;
			y*=len;
			z*=len;
			w*=len;
		}
	}

	inline void scale( f32 s)
	{
		x*=s;
		y*=s;
		z*=s;

		if(w>=0.0f)
		{
			w=1.0f-((1.0f-w)*s);
		}
		else
		{
			w=-1.0f-((-1.0f-w)*s);
		}
	}

	inline void neg(void)
	{
		w=-w;
	}

};


/*------------------------------------------------------------------------------------------------------------------------------*/
//
// a generic transformation structure, deals with position rotation and scale
//
/*------------------------------------------------------------------------------------------------------------------------------*/
struct tfm
{
	v3		vec[1];
	quat	rot[1];
	v3		siz[1];

	inline void reset(void)
	{
		vec->reset();
		rot->reset();
		siz->x=1.0f;
		siz->y=1.0f;
		siz->z=1.0f;
	}

	inline void set(tfm *to)
	{
		vec->set(to->vec);
		rot->set(to->rot);
		siz->set(to->siz);
	}

};


/*------------------------------------------------------------------------------------------------------------------------------*/
//
// a generic velocity structure, deals with position and rotation movement
//
/*------------------------------------------------------------------------------------------------------------------------------*/
struct vel
{
	v3		vec[1];
	quat	rot[1];


	inline void reset(void)
	{
		vec->reset();
		rot->reset();
	}

	inline void set(vel *to)
	{
		vec->set(to->vec);
		rot->set(to->rot);
	}
};


/*------------------------------------------------------------------------------------------------------------------------------*/
//
// a generic acc structure, deals with position and rotation movement movement
//
/*------------------------------------------------------------------------------------------------------------------------------*/
struct acc
{
	v3		vec[1];
	quat	rot[1];


	inline void reset(void)
	{
		vec->reset();
		rot->reset();
	}

	inline void set(acc *to)
	{
		vec->set(to->vec);
		rot->set(to->rot);
	}

};


/*------------------------------------------------------------------------------------------------------------------------------*/
//
// a generic transformation structure with velocity and acceleration
//
/*------------------------------------------------------------------------------------------------------------------------------*/
struct TVA
{
	tfm		t[1];
	vel		v[1];
	acc		a[1];

	f32		friction;	// fraction to mul vel by to bring it closer to zero


	void update(void);

	void update_vel(void);

	void update_tfm(f32 by);

	inline void reset(void)
	{
		t->reset();
		v->reset();
		a->reset();

		friction=1.0f;
	}

	inline void set(TVA *to)
	{
		t->set(to->t);
		v->set(to->v);
		a->set(to->a);

		friction=to->friction;
	}
};








/*------------------------------------------------------------------------------------------------------------------------------*/
//
// a simple tweened value, uses spine functions to start/stop smoothly
//
/*------------------------------------------------------------------------------------------------------------------------------*/

struct f32_change_controler
{
	f32		now;	// where the body currently is

	f32		base;	// where we where

	f32		diff;	// change to where we are going

	f32		fade;	// 0-1, when it reaches 1 we have hit the destination

	f32		speed;	// amount to add to fade per sec till we hit one 


	void set(f32 val)
	{
		now=base=val;
		diff=fade=speed=0.0f;
	}

	void start(f32 _diff, f32 _speed)
	{
		base=now;
		diff=_diff;
		fade=0;
		speed=_speed;
	}

	void finish(void)
	{
		now=base+diff;
		base=now;
		diff=fade=speed=0.0f;
	}

	void update(f32 by)
	{
		if(speed)
		{
			fade+=speed*by;

			if(fade>=1.0f)
			{
				finish();
			}
			else
			{
				now=base + diff*f32_spinetop(fade);
			}
		}
	}

};






/*------------------------------------------------------------------------------------------------------------------------------*/
//
// cross product
//
/*------------------------------------------------------------------------------------------------------------------------------*/
inline void v3_cross_v3( struct v3 * vDest , const struct v3 * vA , const struct v3 * vB )
{
	vDest->x= vA->y * vB->z - vA->z * vB->y ;
	vDest->y= vA->z * vB->x - vA->x * vB->z ;
	vDest->z= vA->x * vB->y - vA->y * vB->x ;
}


/*------------------------------------------------------------------------------------------------------------------------------*/
//
// square root of an f32
//
/*------------------------------------------------------------------------------------------------------------------------------*/
inline f32 f32_sqrt(f32 n)
{
	return (f32) sqrt(n);
}

/*------------------------------------------------------------------------------------------------------------------------------*/
//
// set an m44 from an m44
//
/*------------------------------------------------------------------------------------------------------------------------------*/
inline void m44_set_m44( struct m44 * mat, const struct m44 * from )
{
	((f32*)mat)[0]=((f32*)from)[0];
	((f32*)mat)[1]=((f32*)from)[1];
	((f32*)mat)[2]=((f32*)from)[2];
	((f32*)mat)[3]=((f32*)from)[3];
	((f32*)mat)[4]=((f32*)from)[4];
	((f32*)mat)[5]=((f32*)from)[5];
	((f32*)mat)[6]=((f32*)from)[6];
	((f32*)mat)[7]=((f32*)from)[7];
	((f32*)mat)[8]=((f32*)from)[8];
	((f32*)mat)[9]=((f32*)from)[9];
	((f32*)mat)[10]=((f32*)from)[10];
	((f32*)mat)[11]=((f32*)from)[11];
	((f32*)mat)[12]=((f32*)from)[12];
	((f32*)mat)[13]=((f32*)from)[13];
	((f32*)mat)[14]=((f32*)from)[14];
	((f32*)mat)[15]=((f32*)from)[15];

//	memcpy(mat,from,sizeof(m44));
}

/*------------------------------------------------------------------------------------------------------------------------------*/
//
// set an m44 from a tfm
//
/*------------------------------------------------------------------------------------------------------------------------------*/
inline void m44_set_tfm( struct m44 * mat, const struct tfm * tfm )
{

// rotate
	m44_set_quat(mat,tfm->rot);

// scale

	mat->xx*=tfm->siz->x;
	mat->xy*=tfm->siz->x;
	mat->xz*=tfm->siz->x;
	mat->yx*=tfm->siz->y;
	mat->yy*=tfm->siz->y;
	mat->yz*=tfm->siz->y;
	mat->zx*=tfm->siz->z;
	mat->zy*=tfm->siz->z;
	mat->zz*=tfm->siz->z;


	mat->ww=1.0f;

// translate
	mat->wx=tfm->vec->x;
	mat->wy=tfm->vec->y;
	mat->wz=tfm->vec->z;
}


/*------------------------------------------------------------------------------------------------------------------------------*/
//
// multiply two m44 matrices
//
// q=a*b
//
// q can be a or b
//
/*------------------------------------------------------------------------------------------------------------------------------*/
inline void m44_mul_m44( m44 * q, const m44 * a, const m44 * b )
{
	f32* pA = (f32*)a;
	f32* pB = (f32*)b;
	f32  m[16]={0};


	for( int i=0; i<4; i++ ) 
		for( int j=0; j<4; j++ ) 
			for( int k=0; k<4; k++ ) 
				m[4*i+j] += pA[4*k+j] * pB[4*i+k];
/*
	m44 * m;

	m->xx = a->xx*b->xx + a->yx*b->xy + a->zx*b->xz + a->wx*b->xw ;
	m->xy = a->xy*b->xx + a->yy*b->xy + a->zy*b->xz + a->wy*b->xw ;
	m->xz = a->xz*b->xx + a->yz*b->xy + a->zz*b->xz + a->wz*b->xw ;
	m->xw = a->xw*b->xx + a->yw*b->xy + a->zw*b->xz + a->ww*b->xw ;

	m->yx = a->xx*b->yx + a->yx*b->yy + a->zx*b->yz + a->wx*b->yw ;
	m->yy = a->xy*b->yx + a->yy*b->yy + a->zy*b->yz + a->wy*b->yw ;
	m->yz = a->xz*b->yx + a->yz*b->yy + a->zz*b->yz + a->wz*b->yw ;
	m->yw = a->xw*b->yx + a->yw*b->yy + a->zw*b->yz + a->ww*b->yw ;

	m->zx = a->xx*b->zx + a->yx*b->zy + a->zx*b->zz + a->wx*b->zw ;
	m->zy = a->xy*b->zx + a->yy*b->zy + a->zy*b->zz + a->wy*b->zw ;
	m->zz = a->xz*b->zx + a->yz*b->zy + a->zz*b->zz + a->wz*b->zw ;
	m->zw = a->xw*b->zx + a->yw*b->zy + a->zw*b->zz + a->ww*b->zw ;

	m->wx = a->xx*b->wx + a->yx*b->wy + a->zx*b->wz + a->wx*b->ww ;
	m->wy = a->xy*b->wx + a->yy*b->wy + a->zy*b->wz + a->wy*b->ww ;
	m->wz = a->xz*b->wx + a->yz*b->wy + a->zz*b->wz + a->wz*b->ww ;
	m->ww = a->xw*b->wx + a->yw*b->wy + a->zw*b->wz + a->ww*b->ww ;
*/

	m44_set_m44(q,(const struct m44*)m);
//	memcpy( q, m, sizeof(m44) );
}



/*------------------------------------------------------------------------------------------------------------------------------*/
//
// invert a m44 matrices
//
// this function only works for matrices with [0 0 0 1] for the 4th column.
//
// q must not be the same pointer as a
//
/*------------------------------------------------------------------------------------------------------------------------------*/
inline bool m44_invert( m44 * q, const m44 * a )
{
	if( fabs(a->ww - 1.0f) > .001f) return false;
	if( fabs(a->xw) > .001f || fabs(a->yw) > .001f || fabs(a->zw) > .001f ) return false;

    f32 fDetInv = 1.0f / ( a->xx * ( a->yy * a->zz - a->yz * a->zy ) -
                             a->xy * ( a->yx * a->zz - a->yz * a->zx ) +
                             a->xz * ( a->yx * a->zy - a->yy * a->zx ) );

    q->xx =  fDetInv * ( a->yy * a->zz - a->yz * a->zy );
    q->xy = -fDetInv * ( a->xy * a->zz - a->xz * a->zy );
    q->xz =  fDetInv * ( a->xy * a->yz - a->xz * a->yy );
    q->xw = 0.0f;

    q->yx = -fDetInv * ( a->yx * a->zz - a->yz * a->zx );
    q->yy =  fDetInv * ( a->xx * a->zz - a->xz * a->zx );
    q->yz = -fDetInv * ( a->xx * a->yz - a->xz * a->yx );
    q->yw = 0.0f;

    q->zx =  fDetInv * ( a->yx * a->zy - a->yy * a->zx );
    q->zy = -fDetInv * ( a->xx * a->zy - a->xy * a->zx );
    q->zz =  fDetInv * ( a->xx * a->yy - a->xy * a->yx );
    q->zw = 0.0f;

    q->wx = -( a->wx * q->xx + a->wy * q->yx + a->wz * q->zx );
    q->wy = -( a->wx * q->xy + a->wy * q->yy + a->wz * q->zy );
    q->wz = -( a->wx * q->xz + a->wy * q->yz + a->wz * q->zz );
    q->ww = 1.0f;

    return true;
}




/*------------------------------------------------------------------------------------------------------------------------------*/
//
// multiply v3 by m44
//
// vector source and dest may be the same
//
/*------------------------------------------------------------------------------------------------------------------------------*/
inline bool v3_mul_m44( v3 * vDest, const v3 * vSrc, const m44 * mat)
{
	f32 x = vSrc->x*mat->xx + vSrc->y*mat->yx + vSrc->z* mat->zx + mat->wx;
	f32 y = vSrc->x*mat->xy + vSrc->y*mat->yy + vSrc->z* mat->zy + mat->wy;
	f32 z = vSrc->x*mat->xz + vSrc->y*mat->yz + vSrc->z* mat->zz + mat->wz;
	f32 w = vSrc->x*mat->xw + vSrc->y*mat->yw + vSrc->z* mat->zw + mat->ww;

	if( fabs( w ) < F32_EPSILON ) return false;

	vDest->x = x/w;
	vDest->y = y/w;
	vDest->z = z/w;

	return true;
}



/*------------------------------------------------------------------------------------------------------------------------------*/
//
// multiply v3 by quat
//
// vector source and dest may be the same
//
/*------------------------------------------------------------------------------------------------------------------------------*/
inline void v3_mul_quat( v3 * vDest, const v3 * vSrc, const v4 * q)
{
f32 x,y,z;

	x=vSrc->x;
	y=vSrc->y;
	z=vSrc->z;

	vDest->x=	q->w*q->w*x +
				2*q->y*q->w*z -
				2*q->z*q->w*y +
				q->x*q->x*x +
				2*q->y*q->x*y +
				2*q->z*q->x*z -
				q->z*q->z*x -
				q->y*q->y*x;

	vDest->y=	2*q->x*q->y*x +
				q->y*q->y*y +
				2*q->z*q->y*z +
				2*q->w*q->z*x -
				q->z*q->z*y +
				q->w*q->w*y -
				2*q->x*q->w*z -
				q->x*q->x*y;

	vDest->z=	2*q->x*q->z*x +
				2*q->y*q->z*y +
				q->z*q->z*z -
				2*q->w*q->y*x -
				q->y*q->y*z +
				2*q->w*q->x*y -
				q->x*q->x*z +
				q->w*q->w*z;


	return;
}


/*------------------------------------------------------------------------------------------------------------------------------*/
//
// multiply v3 by negative quat (reverse the rotation)
//
// vector source and dest may be the same
//
/*------------------------------------------------------------------------------------------------------------------------------*/
inline void v3_mul_nquat( v3 * vDest, const v3 * vSrc, const v4 * q)
{
f32 x,y,z;

	x=vSrc->x;
	y=vSrc->y;
	z=vSrc->z;

	vDest->x=	(-q->w)*(-q->w)*x +
				2*q->y*(-q->w)*z -
				2*q->z*(-q->w)*y +
				q->x*q->x*x +
				2*q->y*q->x*y +
				2*q->z*q->x*z -
				q->z*q->z*x -
				q->y*q->y*x;

	vDest->y=	2*q->x*q->y*x +
				q->y*q->y*y +
				2*q->z*q->y*z +
				2*(-q->w)*q->z*x -
				q->z*q->z*y +
				(-q->w)*(-q->w)*y -
				2*q->x*(-q->w)*z -
				q->x*q->x*y;

	vDest->z=	2*q->x*q->z*x +
				2*q->y*q->z*y +
				q->z*q->z*z -
				2*(-q->w)*q->y*x -
				q->y*q->y*z +
				2*(-q->w)*q->x*y -
				q->x*q->x*z +
				(-q->w)*(-q->w)*z;


	return;
}


/*------------------------------------------------------------------------------------------------------------------------------*/
//
// convert a normalised axis and angle to quat
//
/*------------------------------------------------------------------------------------------------------------------------------*/
inline void quat_set_rot( v4 * quat , const v3 * v, f32 rot )
{
    quat->x = (f32)sin(rot/2) * v->x;
    quat->y = (f32)sin(rot/2) * v->y;
    quat->z = (f32)sin(rot/2) * v->z;
    quat->w = (f32)cos(rot/2);
}

/*------------------------------------------------------------------------------------------------------------------------------*/
//
// build a quat thats just an x axis rotation
//
/*------------------------------------------------------------------------------------------------------------------------------*/
inline void quat_set_rotx( v4 * quat, f32 rot )
{
    quat->x = (f32)sin(rot/2);
    quat->y = 0.0f;
    quat->z = 0.0f;
    quat->w = (f32)cos(rot/2);
}

/*------------------------------------------------------------------------------------------------------------------------------*/
//
// build a quat thats just an x axis rotation
//
/*------------------------------------------------------------------------------------------------------------------------------*/
inline void quat_set_roty( v4 * quat, f32 rot )
{
    quat->x = 0.0f;
    quat->y = (f32)sin(rot/2);
    quat->z = 0.0f;
    quat->w = (f32)cos(rot/2);
}

/*------------------------------------------------------------------------------------------------------------------------------*/
//
// build a quat thats just an x axis rotation
//
/*------------------------------------------------------------------------------------------------------------------------------*/
inline void quat_set_rotz( v4 * quat, f32 rot )
{
    quat->x = 0.0f;
    quat->y = 0.0f;
    quat->z = (f32)sin(rot/2);
    quat->w = (f32)cos(rot/2);
}

/*------------------------------------------------------------------------------------------------------------------------------*/
//
// rotate a quat about the x axis a bit
//
/*------------------------------------------------------------------------------------------------------------------------------*/
inline void quat_rotx( struct v4 * quat, f32 rot )
{
v4 newquat[1];
	quat_set_rotx(newquat,rot);
	quat_mul_quat(quat,quat,newquat);
}

/*------------------------------------------------------------------------------------------------------------------------------*/
//
// rotate a quat about the y axis a bit
//
/*------------------------------------------------------------------------------------------------------------------------------*/
inline void quat_roty( struct v4 * quat, f32 rot )
{
v4 newquat[1];
	quat_set_roty(newquat,rot);
	quat_mul_quat(quat,quat,newquat);
}

/*------------------------------------------------------------------------------------------------------------------------------*/
//
// rotate a quat about the z axis a bit
//
/*------------------------------------------------------------------------------------------------------------------------------*/
inline void quat_rotz( struct v4 * quat, f32 rot )
{
v4 newquat[1];
	quat_set_rotz(newquat,rot);
	quat_mul_quat(quat,quat,newquat);
}

/*------------------------------------------------------------------------------------------------------------------------------*/
//
// rotate a quat about the x axis a bit
//
/*------------------------------------------------------------------------------------------------------------------------------*/
inline void rotx_quat( struct v4 * quat, f32 rot )
{
v4 newquat[1];
	quat_set_rotx(newquat,rot);
	quat_mul_quat(quat,newquat,quat);
}

/*------------------------------------------------------------------------------------------------------------------------------*/
//
// rotate a quat about the x axis a bit
//
/*------------------------------------------------------------------------------------------------------------------------------*/
inline void roty_quat( struct v4 * quat, f32 rot )
{
v4 newquat[1];
	quat_set_roty(newquat,rot);
	quat_mul_quat(quat,newquat,quat);
}

/*------------------------------------------------------------------------------------------------------------------------------*/
//
// rotate a quat about the x axis a bit
//
/*------------------------------------------------------------------------------------------------------------------------------*/
inline void rotz_quat( struct v4 * quat, f32 rot )
{
v4 newquat[1];
	quat_set_rotz(newquat,rot);
	quat_mul_quat(quat,newquat,quat);
}



/*------------------------------------------------------------------------------------------------------------------------------*/
//
// build a quat from yaw pitch roll
//
/*------------------------------------------------------------------------------------------------------------------------------*/
inline void quat_set_ypr( v4 * quat , f32 yaw, f32 pitch, f32 roll )
{
	f32 SinYaw   = (f32)sin(yaw/2);
	f32 SinPitch = (f32)sin(pitch/2);
	f32 SinRoll  = (f32)sin(roll/2);
	f32 CosYaw   = (f32)cos(yaw/2);
	f32 CosPitch = (f32)cos(pitch/2);
	f32 CosRoll  = (f32)cos(roll/2);

	quat->x = SinRoll * CosPitch * CosYaw - CosRoll * SinPitch * SinYaw;
	quat->y = CosRoll * SinPitch * CosYaw + SinRoll * CosPitch * SinYaw;
	quat->z = CosRoll * CosPitch * SinYaw - SinRoll * SinPitch * CosYaw;
	quat->w = CosRoll * CosPitch * CosYaw + SinRoll * SinPitch * SinYaw;
}


/*------------------------------------------------------------------------------------------------------------------------------*/
//
// build an m44 from quat
//
/*------------------------------------------------------------------------------------------------------------------------------*/
inline void m44_set_quat( m44 * mat, const v4 * quat )
{
f32 wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2; 
	
	
// calculate coefficients
	
	x2 = quat->x + quat->x;
	y2 = quat->y + quat->y; 
	z2 = quat->z + quat->z;
	
	xx = quat->x * x2;
	xy = quat->x * y2;
	xz = quat->x * z2;
	
	yy = quat->y * y2;
	yz = quat->y * z2;
	zz = quat->z * z2;
	
	wx = quat->w * x2;
	wy = quat->w * y2;
	wz = quat->w * z2;
	
	
	mat->xx = 1.0f - (yy + zz);
	mat->yx = xy - wz;
	mat->zx = xz + wy;
	mat->wx = 0.0f;
	
	mat->xy = xy + wz;
	mat->yy = 1.0f - (xx + zz);
	mat->zy = yz - wx;
	mat->wy = 0.0f;
	
	
	mat->xz = xz - wy;
	mat->yz = yz + wx;
	mat->zz = 1.0f - (xx + yy);
	mat->wz = 0.0f;
	
	
	mat->xw = 0.0f;
	mat->yw = 0.0f;
	mat->zw = 0.0f;
	mat->ww = 1.0f;

}



/*------------------------------------------------------------------------------------------------------------------------------*/
//
// multiply two quats together, q=a*b  , q can point to a or b since its cached 
//
/*------------------------------------------------------------------------------------------------------------------------------*/
inline void quat_mul_quat( v4 * q,  const v4 * a, const v4 * b )
{
f32 A, B, C, D, E, F, G, H;
	
	
	A = (a->w + a->x) * (b->w + b->x);
	B = (a->z - a->y) * (b->y - b->z);
	C = (a->w - a->x) * (b->y + b->z); 
	D = (a->y + a->z) * (b->w - b->x);
	E = (a->x + a->z) * (b->x + b->y);
	F = (a->x - a->z) * (b->x - b->y);
	G = (a->w + a->y) * (b->w - b->z);
	H = (a->w - a->y) * (b->w + b->z);
	
	
	q->w = B + (-E - F + G + H) * 0.5f;
	q->x = A - ( E + F + G + H) * 0.5f;
	q->y = C + ( E - F + G - H) * 0.5f;
	q->z = D + ( E - F - G + H) * 0.5f;
	
}


inline void quat_mul_nquat( v4 * q,  const v4 * a, const v4 * b )
{
f32 A, B, C, D, E, F, G, H;
	
	
	A = (a->w + a->x) * ((-b->w) + b->x);
	B = (a->z - a->y) * (b->y - b->z);
	C = (a->w - a->x) * (b->y + b->z); 
	D = (a->y + a->z) * ((-b->w) - b->x);
	E = (a->x + a->z) * (b->x + b->y);
	F = (a->x - a->z) * (b->x - b->y);
	G = (a->w + a->y) * ((-b->w) - b->z);
	H = (a->w - a->y) * ((-b->w) + b->z);
	
	
	q->w = B + (-E - F + G + H) * 0.5f;
	q->x = A - ( E + F + G + H) * 0.5f;
	q->y = C + ( E - F + G - H) * 0.5f;
	q->z = D + ( E - F - G + H) * 0.5f;
	
}





/*------------------------------------------------------------------------------------------------------------------------------*/
//
// slerp between two quats by the alpha amount.
//
// This is broken and destroys input data....
//
// in fact this is very broken :)
//
/*------------------------------------------------------------------------------------------------------------------------------*/
inline void quat_slerp_quat( v4 * q,   v4 * a,  v4 * b , f32 alpha )
{
    f32 fScale1;
    f32 fScale2;

	f32 fCosTheta;

    // Compute dot product, aka cos(theta):
    fCosTheta = a->x*b->x + a->y*b->y + a->z*b->z + a->w*b->w;

    if( fCosTheta < 0.0f )
    {
        // Flip start quaternion
        a->x = -a->x; a->y = -a->y; a->x = -a->z; a->w = -a->w;
        fCosTheta = -fCosTheta;
    }

    if( fCosTheta + 1.0f > 0.05f )
    {
        // If the quaternions are close, use linear interploation
        if( 1.0f - fCosTheta < 0.05f )
        {
            fScale1 = 1.0f - alpha;
            fScale2 = alpha;
        }
        else // Otherwise, do spherical interpolation
        {
            f32 fTheta    = (f32)acos( fCosTheta );
            f32 fSinTheta = (f32)sin( fTheta );
            
            fScale1 = (f32)sin( fTheta * (1.0f-alpha) ) / fSinTheta;
            fScale2 = (f32)sin( fTheta * alpha ) / fSinTheta;
        }
    }
    else
    {
        b->x = -a->y;
        b->y =  a->x;
        b->z = -a->w;
        b->w =  a->z;
        fScale1 = (f32)sin( F32_PI * (0.5f - alpha) );
        fScale2 = (f32)sin( F32_PI * alpha );
    }

    q->x = fScale1 * a->x + fScale2 * b->x;
    q->y = fScale1 * a->y + fScale2 * b->y;
    q->z = fScale1 * a->z + fScale2 * b->z;
    q->w = fScale1 * a->w + fScale2 * b->w;
}






/*------------------------------------------------------------------------------------------------------------------------------*/
//
// appply acc and decay to vel and vel to pos
//
/*------------------------------------------------------------------------------------------------------------------------------*/
inline void TVA::update(void)
{
	v->vec->x=(v->vec->x+a->vec->x)*friction;
	v->vec->y=(v->vec->y+a->vec->y)*friction;
	v->vec->z=(v->vec->z+a->vec->z)*friction;

	t->vec->x+=v->vec->x;
	t->vec->y+=v->vec->y;
	t->vec->z+=v->vec->z;

	quat_mul_quat(v->rot , a->rot , v->rot );
	v->rot->scale(friction);
	v->rot->normish();

	quat_mul_quat(t->rot , v->rot , t->rot );
	t->rot->normish();
}


/*------------------------------------------------------------------------------------------------------------------------------*/
//
// appply acc and decay to vel and vel to pos
//
/*------------------------------------------------------------------------------------------------------------------------------*/
inline void TVA::update_vel(void)
{
	v->vec->x=(v->vec->x+a->vec->x)*friction;
	v->vec->y=(v->vec->y+a->vec->y)*friction;
	v->vec->z=(v->vec->z+a->vec->z)*friction;

	quat_mul_quat(v->rot , a->rot , v->rot );
	v->rot->scale(friction);
	v->rot->normish();

}


/*------------------------------------------------------------------------------------------------------------------------------*/
//
// appply acc and decay to vel and vel to pos
//
/*------------------------------------------------------------------------------------------------------------------------------*/
inline void TVA::update_tfm(f32 by)
{

	t->vec->x+=v->vec->x*by;
	t->vec->y+=v->vec->y*by;
	t->vec->z+=v->vec->z*by;

	quat_mul_quat(t->rot , v->rot , t->rot );
	t->rot->normish();
}






