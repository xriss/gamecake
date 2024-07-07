
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define DJON_C 1
#include "../../c/djon.h"


int main(int argc, char *argv[])
{
	
	double ds[128];
	int dlen=0;
	double d;
	int i,j,k;
	char *cp;
	char cs[256];
	for( i=1 ; i<argc ; i++ )
	{
		d=djon_str_to_double(argv[i],0);
		if( !isnan(d) ) // good number
		{
			ds[dlen++]=d;
		}
		if(dlen>=128) { goto error; }
	}

	for(j=0;j<10;j++)
	{
		for(i=0;i<dlen;i++)
		{
			d=ds[i];
			djon_double_to_str(d,cs);
			for(k=0;k<j;k++)
			{
				d=djon_str_to_double(cs,0);
				djon_double_to_str(d,cs);
			}
			printf("%-32s",cs);
		}
		printf("\n");
	}
	
	return 0;
error:
	return 20;
}
