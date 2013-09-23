/* NeuQuant Neural-Net Quantization Algorithm
 * ------------------------------------------
 *
 * Copyright (c) 1994 Anthony Dekker
 *
 * NEUQUANT Neural-Net quantization algorithm by Anthony Dekker, 1994.
 * See "Kohonen neural networks for optimal colour quantization"
 * in "Network: Computation in Neural Systems" Vol. 5 (1994) pp 351-367.
 * for a discussion of the algorithm.
 * See also  http://members.ozemail.com.au/~dekker/NEUQUANT.HTML
 *
 * Any party obtaining a copy of these files from the author, directly or
 * indirectly, is granted, free of charge, a full and unrestricted irrevocable,
 * world-wide, paid up, royalty-free, nonexclusive right and license to deal
 * in this software and documentation files (the "Software"), including without
 * limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons who receive
 * copies from any such party to do so, with the only requirement being
 * that this copyright notice remain intact.
 * 
 *
 * Modified to process 32bit RGBA images.
 * Stuart Coyle 2004-2006
 *
 * Rewritten by Kornel Lesiński (2009)
 */

//
// XIX : 2010-10-15
// removed the verbose option
// added neuquant32_ prefix so as to reduce name clash
// moved junk from header to this file for the same reason
// marked all globals as static except the ones prefixed with neuquant32_
// switched to expect BGRA colors, which is a little endian U32 of a U8[4] ARGB
// made the alpha super important when doing an inxsearch
// removed the faster version of inxsearch since it seems broken
//
// actually I"m not convinced any of this works very well at all maybe i shall rewrite :)
//

#include "neuquant32.h"
#include <math.h>


#include <stdio.h>
#include <string.h>


#define MAXNETSIZE	256    /* maximum number of colours that can be used. 
				  actual number is now passed to initcolors */


/* four primes near 500 - assume no image has a length so large */
/* that it is divisible by all four primes */
#define prime1		499
#define prime2		491
#define prime3		487
#define prime4		503

#define minpicturebytes	(4*prime4)		/* minimum size for input image */

/* Unbias network to give byte values 0..255 and record position i to prepare for sort
   ----------------------------------------------------------------------------------- */
 static double biasvalue(unsigned int temp);


/* Network Definitions
   ------------------- */
   
#define maxnetpos	(MAXNETSIZE-1)
#define ncycles		100			/* no. of learning cycles */
#define ABS(a) ((a)>=0?(a):-(a))

/* defs for freq and bias */
#define gammashift  10			/* gamma = 1024 */
#define gamma   	((double)(1<<gammashift))
#define betashift  	10
#define beta		(1.0/(1<<betashift))	/* beta = 1/1024 */
#define betagamma	((double)(1<<(gammashift-betashift)))

/* defs for decreasing radius factor */
#define initrad		(MAXNETSIZE>>3)		/* for 256 cols, radius starts */
#define initradius	(initrad*1.0)	/* and decreases by a */
#define radiusdec	30			/* factor of 1/30 each cycle */ 

/* defs for decreasing alpha factor */
#define alphabiasshift	10			/* alpha starts at 1.0 */
#define initalpha	((double)(1<<alphabiasshift))
static double alphadec;					/* biased by 10 bits */

/* radbias and alpharadbias used for radpower calculation */
#define radbiasshift	8
#define radbias         (1<<radbiasshift)
#define alpharadbshift  (alphabiasshift+radbiasshift)
#define alpharadbias    ((double)(1<<alpharadbshift))


/* Types and Global Variables
   -------------------------- */
   
static unsigned char *thepicture;		/* the input image itself */
static unsigned int lengthcount;				/* lengthcount = H*W*3 */


typedef struct /* ABGRc */
{				
    double al,b,g,r;
} nq_pixel;

static nq_pixel network[MAXNETSIZE];			/* the network itself */

static unsigned int netindex[256];			/* for network lookup - really 256 */

static double bias [MAXNETSIZE];			/* bias and freq arrays for learning */
static double freq [MAXNETSIZE];
static double radpower[initrad];			/* radpower for precomputation */

static unsigned int netsize;                             /* Number of colours to use. Made a global instead of #define */

static double gamma_correction; // 1.0/2.2 usually 

/* Initialise network in range (0,0,0,0) to (255,255,255,255) and set parameters
   ----------------------------------------------------------------------- */

static double biasvalues[256];

void neuquant32_initnet(unsigned char *thepic,unsigned int len,unsigned int colours, double gamma_c)
{
	unsigned int i;
    
    gamma_correction = gamma_c;
	
	/* Clear out network from previous runs */
	/* thanks to Chen Bin for this fix */
	memset((void*)network,0,sizeof(network));

	thepicture = thepic;
	lengthcount = len;
	netsize = colours; 
	
    for(i=0;i<256;i++)
    {
        double temp;
        temp = pow(i/255.0, 1.0/gamma_correction) * 255.0;
        temp = floor(temp);
        biasvalues[i] = temp;
    }
    
	for (i=0; i<netsize; i++) {
		network[i].b = network[i].g = network[i].r = biasvalue(i*256/netsize);
        if (i < 16) network[i].al = (i*16); else network[i].al = 255;
		freq[i] = 1.0/netsize;	/* 1/netsize */
		bias[i] = 0;
	}
}

static unsigned int unbiasvalue(double temp)
{
    if (temp < 0) return 0;
    
    temp = pow(temp/255.0, gamma_correction) * 255.0;    
    temp = floor((temp / 255.0 * 256.0));

    if (temp > 255) return 255;
    return temp;
}

static unsigned int round_biased(double temp)
{    
    if (temp < 0) return 0;
    temp = floor((temp / 255.0 * 256.0));
    
    if (temp > 255) return 255;    
    return temp;
}


static double biasvalue(unsigned int temp)
{    
    return biasvalues[temp];
}

/* Output colormap to unsigned char ptr in BGRA format */
void neuquant32_getcolormap(unsigned char *map)
{
	unsigned int j;
	for(j=0; j<netsize; j++)
    {
        *map++ = unbiasvalue(network[j].r);
        *map++ = unbiasvalue(network[j].g);
        *map++ = unbiasvalue(network[j].b);
        *map++ = round_biased(network[j].al);
	}
}


/* Insertion sort of network and building of netindex[0..255] (to do after unbias)
   ------------------------------------------------------------------------------- */

typedef struct 
{
    unsigned char r,g,b,al;  
} nq_colormap;

static nq_colormap colormap[256];

void neuquant32_inxbuild()
{
	nq_colormap tempc;
	unsigned int i,j,smallpos,smallval;
	unsigned int previouscol,startpos;

    for(i=0; i< netsize; i++)
    {
        colormap[i].r =  biasvalue(unbiasvalue(network[i].r));
        colormap[i].g =  biasvalue(unbiasvalue(network[i].g));
        colormap[i].b =  biasvalue(unbiasvalue(network[i].b));
        colormap[i].al = round_biased(network[i].al);        
    }

// sort colormap on green        
	previouscol = 0;
	startpos = 0;
	for (i=0; i<netsize; i++) {
		smallpos = i;
		smallval = (colormap[i].g);			/* index on g */
		/* find smallest in i..netsize-1 */
		for (j=i+1; j<netsize; j++) {
			if ((colormap[j].g) < smallval) {		/* index on g */
				smallpos = j;
				smallval = (colormap[j].g);	/* index on g */
			}
		}
		/* swap colormap[i] (i) and colormap[smallpos] (smallpos) entries */
		if (i != smallpos) {
			nq_pixel temp = network[smallpos];   network[smallpos] = network[i];   network[i] = temp;
			tempc = colormap[smallpos];   colormap[smallpos] = colormap[i];   colormap[i] = tempc;
		}
		/* smallval entry is now in position i */
		if (smallval != previouscol) {
			netindex[previouscol] = (startpos+i)>>1;
			for (j=previouscol+1; j<smallval; j++) netindex[j] = i;
			previouscol = smallval;
			startpos = i;
		}
	}
	netindex[previouscol] = (startpos+maxnetpos)>>1;
	for (j=previouscol+1; j<256; j++) netindex[j] = maxnetpos; /* really 256 */
}

        
 static double colorimportance(double al)
{
    double transparency = 1.0 - al/255.0;
    return (1.0 - transparency*transparency);
}

/* Search for ABGR values 0..255 (after net is unbiased) and return colour index
   ---------------------------------------------------------------------------- */

unsigned int neuquant32_inxsearch( int al, int r, int g, int b)
{
    unsigned int i,best=0;
    double a,bestd=1<<30,dist;
    double colimp = colorimportance(al);

    r=biasvalue(r);
    g=biasvalue(g);
    b=biasvalue(b);
    
    for(i=0; i < netsize; i++)
    {
        a = colormap[i].r - r;
        dist = a*a * colimp;

        a = colormap[i].g - g;
        dist += a*a * colimp;
        
        a = colormap[i].b - b;
        dist += a*a * colimp;
        
        a = colormap[i].al - al;
        dist += a*a*4;
        
        if (dist<bestd) {bestd=dist; best=i;}        
    }
    return best;
}


/* Search for biased ABGR values
   ---------------------------- */

static int contest(double al,double b,double g,double r)
{
	/* finds closest neuron (min dist) and updates freq */
	/* finds best neuron (min dist-bias) and returns position */
	/* for frequently chosen neurons, freq[i] is high and bias[i] is negative */
	/* bias[i] = gamma*((1/netsize)-freq[i]) */

	unsigned int i; double dist,a,betafreq;
	unsigned int bestpos,bestbiaspos;double bestd,bestbiasd;

    double colimp = colorimportance(al);

	bestd = 1<<30;
	bestbiasd = bestd;
	bestpos = 0;
	bestbiaspos = bestpos;

    
	for (i=0; i<netsize; i++) 
    {
        double bestbiasd_biased = bestbiasd + bias[i];
        
		a = network[i].b - b;
		dist = ABS(a) * colimp;
        a = network[i].r - r;
        dist += ABS(a) * colimp;
        
        if (dist < bestd || dist < bestbiasd_biased)
        {                 
            a = network[i].g - g;
            dist += ABS(a) * colimp;
            a = network[i].al - al; 
            dist += ABS(a) ;    
            
            if (dist<bestd) {bestd=dist; bestpos=i;}
            if (dist<bestbiasd_biased) {bestbiasd=dist - bias[i]; bestbiaspos=i;}
        }
		betafreq = freq[i] / (1<< betashift);
		freq[i] -= betafreq;
		bias[i] += betafreq * (1<<gammashift);
	}
	freq[bestpos] += beta;
	bias[bestpos] -= betagamma;
	return(bestbiaspos);
}


/* Move neuron i towards biased (a,b,g,r) by factor alpha
   ---------------------------------------------------- */

static void altersingle(double alpha,unsigned int i,double al,double b,double g,double r)
{    
    double colorimp = 1.0;//0.5;// + 0.7*colorimportance(al);
    
    alpha /= initalpha;
    
    /* alter hit neuron */
	network[i].al -= alpha*(network[i].al - al);
	network[i].b -= colorimp*alpha*(network[i].b - b);
	network[i].g -= colorimp*alpha*(network[i].g - g);
	network[i].r -= colorimp*alpha*(network[i].r - r);
}


/* Move adjacent neurons by precomputed alpha*(1-((i-j)^2/[r]^2)) in radpower[|i-j|]
   --------------------------------------------------------------------------------- */

static void alterneigh(unsigned int rad,unsigned int i,double al,double b,double g,double r)
{
	unsigned int j,hi;
    int k,lo;
	double *q,a;

	lo = i-rad;   if (lo<0) lo=0;
	hi = i+rad;   if (hi>netsize-1) hi=netsize-1;

	j = i+1;
	k = i-1;
	q = radpower;
	while ((j<=hi) || (k>=lo)) {
		a = (*(++q)) / alpharadbias;
		if (j<=hi) {
			network[j].al -= a*(network[j].al - al);
			network[j].b  -= a*(network[j].b  - b) ;
			network[j].g  -= a*(network[j].g  - g) ;
			network[j].r  -= a*(network[j].r  - r) ;
			j++;
		}
		if (k>=lo) {
			network[k].al -= a*(network[k].al - al);
			network[k].b  -= a*(network[k].b  - b) ;
			network[k].g  -= a*(network[k].g  - g) ;
			network[k].r  -= a*(network[k].r  - r) ;
			k--;
		}
	}
}


/* Main Learning Loop
   ------------------ */
/* sampling factor 1..30 */
void neuquant32_learn(unsigned int samplefac)//, unsigned int verbose) /* Stu: N.B. added parameter so that main() could control verbosity. */
{
	unsigned int i,j,al,b,g,r;
	unsigned int rad,step,delta,samplepixels;
    double radius,alpha;
	unsigned char *p;
	unsigned char *lim;
	
	alphadec = 30 + ((samplefac-1)/3);
	p = thepicture;
	lim = thepicture + lengthcount;
	samplepixels = lengthcount/(4*samplefac); 
	delta = samplepixels/ncycles;  /* here's a problem with small images: samplepixels < ncycles => delta = 0 */
	if(delta==0) delta = 1;        /* kludge to fix */
	alpha = initalpha;
	radius = initradius;
	
	rad = radius;
	if (rad <= 1) rad = 0;
	for (i=0; i<rad; i++) 
		radpower[i] = floor( alpha*(((rad*rad - i*i)*radbias)/(rad*rad)) );
	
//	if(verbose) fprintf(stderr,"beginning 1D learning: initial radius=%d\n", rad);

	if ((lengthcount%prime1) != 0) step = 4*prime1;
	else {
		if ((lengthcount%prime2) !=0) step = 4*prime2;
		else {
			if ((lengthcount%prime3) !=0) step = 4*prime3;
			else step = 4*prime4;
		}
	}
//	step=4;
	
	i = 0;
	while (i < samplepixels) 
    {
        if (p[3])
        {
            al =p[3];
            r = biasvalue(p[0]);
            g = biasvalue(p[1]);
            b = biasvalue(p[2]);
        }
        else
        {
            al=r=g=b=0;
        }
        r=r*al/255;
        g=g*al/255;
        b=b*al/255;
            
		j = contest(al,b,g,r);

		altersingle(alpha,j,al,b,g,r);
		if (rad) alterneigh(rad,j,al,b,g,r);   /* alter neighbours */

		p += step;
		while (p >= lim) p -= lengthcount;
	
		i++;
		if (i%delta == 0) {                    /* FPE here if delta=0*/	
			alpha -= alpha / (double)alphadec;
			radius -= radius / (double)radiusdec;
			rad = radius;
			if (rad <= 1) rad = 0;
			for (j=0; j<rad; j++) 
				radpower[j] = floor( alpha*(((rad*rad - j*j)*radbias)/(rad*rad)) );
		}
	}
//	if(verbose) fprintf(stderr,"finished 1D learning: final alpha=%f !\n",((float)alpha)/initalpha);
}
