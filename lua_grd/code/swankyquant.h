/*******************************************************************************

(C) Kriss@XIXs.com 2017

*/




/*******************************************************************************

External functions, the part of this file that is actually a header.

*/
#ifndef SWANKYQUANT_H
#define SWANKYQUANT_H

#ifdef SWANKYQUANT_STATIC
#define SWANKYQUANT_DEF static
#else
#ifdef __cplusplus
#define SWANKYQUANT_DEF extern "C"
#else
#define SWANKYQUANT_DEF extern
#endif
#endif

SWANKYQUANT_DEF void swanky_quant(const unsigned char *input,
	unsigned int length, unsigned int colors, 
	unsigned char *output , unsigned char *palette ,
	int quality );

SWANKYQUANT_DEF void swanky_quant_remap(const unsigned char *input,
	unsigned int length, unsigned int colors, 
	unsigned char *output , unsigned char *palette );

#endif // SWANKYQUANT_H



/*******************************************************************************

This code will *only* be included if you #define SWANKYQUANT_C before 
including this file.

*/
#ifdef SWANKYQUANT_C

#include <math.h>



/*******************************************************************************

Compare two colors and return a distance value. This is non linear, as we do not 
need to bother with the sqrt since the numbers are only used for comparison.

*/
static inline double color_distance(int ar,int ag,int ab,int aa,int br,int bg,int bb,int ba)
{
	return (double)( (ar-br)*(ar-br) + (ag-bg)*(ag-bg) + (ab-bb)*(ab-bb) + (aa-ba)*(aa-ba) );
}

/*******************************************************************************

Compare how close a match it would be if we added our color(weighted) to a bucket.

Each bucket is 5 doubles and contains an rgba accumulator in 
[0][1][2][3]  and a number of samples in [5] Doubles must be used 
rather than floats or we will encounter number overflow for images that 
are larger than 256x256 pixels. 

(A float version for smaller images might be worthwhile.)

The weight should be a natural 1.0 but since we want to encourage early 
bucket jumping we fudge it high at the start and then lower it on later 
passes to bring stability to the feedback system. This is simply to 
reduce the number of passes over the data required to get good results.

This function also accounts for most of the thinking time so 
could/should be optimised more.

*/
static inline double color_distance_weight(const double *a,const unsigned char *b,double weight)
{
	double d=a[4]+weight; if(d<=0) { d=1; } d=1.0/d;
	return color_distance(
		(int)(0.5+(a[0]+(double)b[0]*weight)*d),
		(int)(0.5+(a[1]+(double)b[1]*weight)*d),
		(int)(0.5+(a[2]+(double)b[2]*weight)*d),
		(int)(0.5+(a[3]+(double)b[3]*weight)*d),
		(int)b[0],(int)b[1],(int)b[2],(int)b[3]);
}

/*******************************************************************************

Reduce input rgba data to the requested number of index colors(2-256)

reads data from input[4*length]

writes data to output[length] and palette[4*colors]

Make sure this memory is available.

*/
SWANKYQUANT_DEF void swanky_quant(const unsigned char *input,
	unsigned int length, unsigned int colors, 
	unsigned char *output , unsigned char *palette ,
	int quality)
{
const unsigned char *pi;
const unsigned char *pc;
unsigned char *po;
unsigned char *pp;
double *pa;
int i,l;
int best_idx;
double distance,best_distance,weight;

	double acc[256*5]={0}; // accumulator for color values [r,g,b,a,v]
	
// start by assigning each pixel a "random" index as a starting state
	for( i=0 , pi=input , po=output ; i<length ; i++ , pi+=4 , po++ )
	{
		po[0]=( (colors-1)*(pi[0]+pi[1]+pi[2])/(3*255) )%colors; // a greyscale map
		pa=acc+(po[0]*5);
		pa[0]+=(double)pi[0];
		pa[1]+=(double)pi[1];
		pa[2]+=(double)pi[2];
		pa[3]+=(double)pi[3];
		pa[4]+=1.0;
	}

// loop over image data and swap to another group if we find a better one
// use a large weight based on average colors in each bucket
// and reduce it with each pass
	for( l=0 , weight=length/colors/4.0 ; (l<quality) ; l++ , weight=weight/4.0 )
	{
		if(weight<1.0){weight=1.0;} // never go below a natural weight of 1.0
		
		for( pi=input , po=output ; po<output+length ; pi+=4 , po++ )
		{
			pa=acc+(po[0]*5); // remove from old acc bucket
			pa[0]-=(double)pi[0];
			pa[1]-=(double)pi[1];
			pa[2]-=(double)pi[2];
			pa[3]-=(double)pi[3];
			pa[4]-=1.0;

			best_idx=po[0];
			best_distance=65546.0*65546.0;
			for( i=0 , pa=acc ; i<colors ; i++ , pa+=5 ) // search for best bucket
			{
				distance=color_distance_weight(pa,pi,weight);
				if(distance<best_distance)
				{
					best_distance=distance;
					best_idx=i;
				}
			}

			po[0]=best_idx; // write out
			
			pa=acc+(best_idx*5); // add to new acc bucket
			pa[0]+=(double)pi[0];
			pa[1]+=(double)pi[1];
			pa[2]+=(double)pi[2];
			pa[3]+=(double)pi[3];
			pa[4]+=1.0;
		}
	}

// build the final output palette from our accumulator buckets
	for( i=0 , pp=palette , pa=acc ; i<colors ; i++ , pp+=4 , pa+=5 )
	{
		double d=pa[4]; if(d<=0) { d=1; } d=1.0/d;
		pp[0]=(unsigned char)floor(0.5+pa[0]*d);
		pp[1]=(unsigned char)floor(0.5+pa[1]*d);
		pp[2]=(unsigned char)floor(0.5+pa[2]*d);
		pp[3]=(unsigned char)floor(0.5+pa[3]*d);
	}
}

/*******************************************************************************

Perform an optional final remap on a swanky_quant output image, this is 
really only necessary for for low quality values which may change 
considerably during the early passes and need this as a final cleanup.

The output image from swanky_quant should be considered just as a 
thinking buffer and you would probably want to do a dithering pass 
instead of this anyway.

*/
SWANKYQUANT_DEF void swanky_quant_remap(const unsigned char *input,
	unsigned int length, unsigned int colors, 
	unsigned char *output , unsigned char *palette )
{
const unsigned char *pi;
unsigned char *po;
unsigned char *pp;
int i;
int best_idx;
double distance,best_distance;
	for( pi=input , po=output ; po<output+length ; pi+=4 , po++ )
	{
		best_idx=0;
		best_distance=65546.0*65546.0;
		for( i=0 , pp=palette ; i<colors ; i++ , pp+=4 ) // search for best bucket
		{
			distance=color_distance(pp[0],pp[1],pp[2],pp[3],pi[0],pi[1],pi[2],pi[3]);
			if(distance<best_distance)
			{
				best_distance=distance;
				best_idx=i;
			}
		}
		po[0]=best_idx; // write out
	}
}

#endif // SWANKYQUANT_C
