/***************************************************************************

(C) Kriss@XIXs.com 2017 + http://opensource.org/licenses/MIT and 
released into the public domain. If this is not good enough for you 
then I suggest you do some random reformatting and tell your lawyers 
that you rewrote it from base algorithms.




REFERENCE LINKS TO OTHER ALGORITHMS
###################################

1.	http://www.imagemagick.org/script/quantize.php
2.	https://www.researchgate.net/publication/232079905_Kohonen_neural_networks_for_optimal_colour_quantization
3.	https://scientificgems.wordpress.com/stuff/neuquant-fast-high-quality-image-quantization/

*/




/***************************************************************************

External functions, AKA the part of this header file that is actually a 
header.

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
	unsigned char *output , const unsigned char *palette ,
	int width , int dither );

#endif // SWANKYQUANT_H



/***************************************************************************

This code will *only* be included if you #define SWANKYQUANT_C before 
including this file. So do this in one .C file only unless you also define 
SWANKYQUANT_STATIC which would keep all the functions static.

*/
#ifdef SWANKYQUANT_C

#include <math.h>



/***************************************************************************

Compare two colors and return a distance value. This is non linear, as we do not 
need to bother with the sqrt since the numbers are only used for comparison.

*/
static inline double color_distance(int ar,int ag,int ab,int aa,int br,int bg,int bb,int ba)
{
	return (double)( (ar-br)*(ar-br) + (ag-bg)*(ag-bg) + (ab-bb)*(ab-bb) + (aa-ba)*(aa-ba) );
}

/***************************************************************************

Compare how close a match it would be if we added our color(weighted) to a bucket.

Each bucket is 5 doubles and contains an rgba accumulator in 
[0][1][2][3]  and a number of samples in [5] Doubles must be used 
rather than floats or we will encounter number overflow for images that 
are larger than 256x256 pixels. 

(A float or int version for smaller images might be worthwhile.)

The weight should be a natural 1.0 but since we want to encourage early 
bucket jumping we fudge it high at the start and then lower it on later 
passes to bring stability to the feedback system. This is simply to 
reduce the number of passes over the data required to get good results.

This function also accounts for most of the thinking time so 
could/should be optimised more.

*/
static inline double color_distance_weight(const double *a,const unsigned char *b,double weight)
{
	double d=a[4]+weight; if(d<=0.0) { d=1.0; } d=1.0/d;
	return color_distance(
		(int)(0.5+(a[0]+(double)b[0]*weight)*d),
		(int)(0.5+(a[1]+(double)b[1]*weight)*d),
		(int)(0.5+(a[2]+(double)b[2]*weight)*d),
		(int)(0.5+(a[3]+(double)b[3]*weight)*d),
		(int)b[0],(int)b[1],(int)b[2],(int)b[3]);
}

/***************************************************************************

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
const unsigned char *pi;	// pointer to input
unsigned char *po;			// pointer to output
unsigned char *pp;			// pointer to output palette
double *pb;					// pointer to internal buckets
int i,l;
int best_idx;
double distance,best_distance,weight;

	double buckets[256*5]={0}; // buckets for color values [r,g,b,a,v]
	
// start by assigning each pixel a "random" bucket as a starting state
	for( i=0 , pi=input , po=output ; i<length ; i++ , pi+=4 , po++ )
	{
		po[0]=( (colors-1)*(pi[0]+pi[1]+pi[2])/(3*255) )%colors; // a greyscale map
		pb=buckets+(po[0]*5);
		pb[0]+=(double)pi[0];
		pb[1]+=(double)pi[1];
		pb[2]+=(double)pi[2];
		pb[3]+=(double)pi[3];
		pb[4]+=1.0;
	}

// loop over image data and swap to another bucket if we find a better one
	for( l=0 , weight=length/colors/4.0 ; (l<quality) ; l++ , weight=weight/4.0 )
	{
// use a large starting weight and reduce it with each pass
// but never go below a natural weight of 1.0
		if(weight<1.0){weight=1.0;} 
		
		for( pi=input , po=output ; po<output+length ; pi+=4 , po++ )
		{
			pb=buckets+(po[0]*5); // remove from old bucket
			pb[0]-=(double)pi[0];
			pb[1]-=(double)pi[1];
			pb[2]-=(double)pi[2];
			pb[3]-=(double)pi[3];
			pb[4]-=1.0;

			best_idx=po[0];
			best_distance=65546.0*65546.0;
			for( i=0 , pb=buckets ; i<colors ; i++ , pb+=5 ) // search for best bucket
			{
				distance=color_distance_weight(pb,pi,weight);
				if(distance<best_distance)
				{
					best_distance=distance;
					best_idx=i;
				}
			}

			po[0]=best_idx; // write out
			
			pb=buckets+(best_idx*5); // add to new bucket
			pb[0]+=(double)pi[0];
			pb[1]+=(double)pi[1];
			pb[2]+=(double)pi[2];
			pb[3]+=(double)pi[3];
			pb[4]+=1.0;
		}
	}

// build the final output palette from our buckets
	for( i=0 , pp=palette , pb=buckets ; i<colors ; i++ , pp+=4 , pb+=5 )
	{
		double d=pb[4]; if(d<=0) { d=1; } d=1.0/d;
		pp[0]=(unsigned char)floor(0.5+pb[0]*d);
		pp[1]=(unsigned char)floor(0.5+pb[1]*d);
		pp[2]=(unsigned char)floor(0.5+pb[2]*d);
		pp[3]=(unsigned char)floor(0.5+pb[3]*d);
	}
}

/***************************************************************************

Perform an final remap on a swanky_quant output image with an optional 
amount of ordered dithering.

	dither = 0  ==   1 bit pattern  ( no dithering )
	dither = 1  ==   3 bit patterns
	dither = 2  ==   5 bit patterns
	dither = 3  ==   9 bit patterns
	dither = 4  ==  17 bit patterns ( recommended dithering )
	dither = 5  ==  33 bit patterns
	dither = 6  ==  65 bit patterns ( maximum dithering )

*/
SWANKYQUANT_DEF void swanky_quant_remap(const unsigned char *input,
	unsigned int length, unsigned int colors, 
	unsigned char *output , const unsigned char *palette ,
	int width , int dither )
{
const unsigned char *pi;	// pointer to input
unsigned char *po;			// pointer to output
const unsigned char *pp;	// pointer to input palette
int xy,i,j;
int step;
int best1_idx;
int best2_idx;
int best_dither;
double best_distance;
double best1_distance;
double best2_distance;
double distance;

int cr,cg,cb,ca;
int c1r,c1g,c1b,c1a;
int c2r,c2g,c2b,c2a;

int x8,y8;

const int pattern[64]={
	22,38,26,42,23,39,27,43,
	54, 6,58,10,55, 7,59,11,
	30,46,18,34,31,47,19,35,
	62,14,50, 2,63,15,51, 3,
	24,40,28,44,21,37,25,41,
	56, 8,60,12,53, 5,57, 9,
	32,48,20,36,29,45,17,33,
	64,16,52, 4,61,13,49, 1,
};

	step=64; // default to no dither
	switch(dither)
	{
		case 1: step=32; break;
		case 2: step=16; break;
		case 3: step=8;  break;
		case 4: step=4;  break;
		case 5: step=2;  break;
		case 6: step=1;  break;
	}

	for( xy=0 , pi=input , po=output ; xy<length ; xy++ , pi+=4 , po++ )
	{
		x8=(xy%width)%8; // x 0-7 for dither
		y8=(xy/width)%8; // y 0-7 for dither
		best1_idx=0;
		best1_distance=65536.0*65536.0;
		best2_idx=0;
		best2_distance=65536.0*65536.0;
		for( i=0 , pp=palette ; i<colors ; i++ , pp+=4 ) // search for the two best colors
		{
			distance=color_distance(pp[0],pp[1],pp[2],pp[3],pi[0],pi[1],pi[2],pi[3]);
			if(distance<best1_distance)
			{
				best2_distance=best1_distance;
				best2_idx=best1_idx;
				best1_distance=distance; // push the previous best to the 2nd best
				best1_idx=i;
			}
			else
			if(distance<best2_distance) // check for second best
			{
				best2_distance=distance;
				best2_idx=i;
			}
		}
		
		
		if(step==64) // no dither
		{
			po[0]=best1_idx; // write out
		}
		else
		{
			if(best2_idx < best1_idx) { i=best1_idx; best1_idx=best2_idx; best2_idx=i; } // maintain order
			
			c1r=palette[ best1_idx*4 + 0 ];
			c1g=palette[ best1_idx*4 + 1 ];
			c1b=palette[ best1_idx*4 + 2 ];
			c1a=palette[ best1_idx*4 + 3 ];

			c2r=palette[ best2_idx*4 + 0 ];
			c2g=palette[ best2_idx*4 + 1 ];
			c2b=palette[ best2_idx*4 + 2 ];
			c2a=palette[ best2_idx*4 + 3 ];

			best_dither=0;
			best_distance=65536.0*65536.0;
			for( i=0 ; i<=64 ; i+=step ) // check each dither option
			{
				j=64-i;
				
				cr = ( 32 + c1r*i + c2r*j ) / 64 ;
				cg = ( 32 + c1g*i + c2g*j ) / 64 ;
				cb = ( 32 + c1b*i + c2b*j ) / 64 ;
				ca = ( 32 + c1a*i + c2a*j ) / 64 ;
				
				distance=color_distance(cr,cg,cb,ca,pi[0],pi[1],pi[2],pi[3]);
				if(distance<best_distance)
				{
					best_distance=distance;
					best_dither=i;
				}
			}
			
			if( pattern[ x8 + y8*8 ] <= best_dither )
			{
				po[0]=best1_idx; // write out
			}
			else
			{
				po[0]=best2_idx; // write out
			}
			
		}		
	}
}

#endif // SWANKYQUANT_C
