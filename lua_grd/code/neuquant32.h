/* NeuQuant Neural-Net Quantization Algorithm Interface
 * ----------------------------------------------------
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
 */

/* Modified to quantize 32bit RGBA images for the pngnq program.
 * Also modified to accept a numebr of colors arguement. 
 * Copyright (c) Stuart Coyle 2004-2006
 */

/*
 * Rewritten by Kornel Lesiński (2009)
 * Euclidean distance, color matching dependent on alpha channel 
 * and with gamma correction. code refreshed for modern compilers/architectures:
 * ANSI C, floats, removed pointer tricks and used arrays and structs.
 */

//
// XIX : 2010-10-15
// see .c file for slight changes
//



/* Initialise network in range (0,0,0,0) to (255,255,255,255) and set parameters
   ----------------------------------------------------------------------- */
void neuquant32_initnet(unsigned char *thepic, unsigned int len, unsigned int colours, double gamma);
		
/* Main Learning Loop
   ------------------ */
void neuquant32_learn(unsigned int samplefactor);//, unsigned int verbose);

/* Insertion sort of network and building of netindex[0..255] (to do after unbias)
   ------------------------------------------------------------------------------- */
void neuquant32_inxbuild();


/* Output colour map
   ----------------- */
void neuquant32_getcolormap(unsigned char *map);

/* Search for ABGR values 0..255 (after net is unbiased) and return colour index
   ---------------------------------------------------------------------------- */
unsigned int neuquant32_inxsearch( int al,  int b,  int g,  int r);


/* Program Skeleton
   ----------------
   	[select samplefac in range 1..30]
   	pic = (unsigned char*) malloc(4*width*height);
   	[read image from input file into pic]
	initnet(pic,4*width*height,samplefac,colors);
	learn();
	unbiasnet();
	[write output image header, using writecolourmap(f),
	possibly editing the loops in that function]
	inxbuild();
	[write output image using inxsearch(a,b,g,r)]		*/
