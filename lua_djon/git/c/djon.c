
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define DJON_C 1
#include "djon.h"


int main(int argc, char *argv[])
{
//printf("START %d\n",argc);

	FILE *fp=0;
	int checkopts=1;
	int write_djon=0;
	int compact=0;
	int strict=0;
	char *fname1=0;
	char *fname2=0;
	int i;
	char *cp;
	for( i=1 ; i<argc ; i++ )
	{
		cp=argv[i];
		if( checkopts && ( cp[0]=='-' && cp[1]=='-' ) ) // option
		{
			if( 0==strcmp(cp,"--") )
			{
				checkopts=0;
			}
			else
			if( 0==strcmp(cp,"--djon") )
			{
				write_djon=1;
			}
			else
			if( 0==strcmp(cp,"--json") )
			{
				write_djon=0;
			}
			else
			if( 0==strcmp(cp,"--compact") )
			{
				compact=1;
			}
			else
			if( 0==strcmp(cp,"--pretty") )
			{
				compact=0;
			}
			else
			if( 0==strcmp(cp,"--strict") )
			{
				strict=1;
			}
			else
			if( 0==strcmp(cp,"--help") )
			{
				printf("\n\
djon input.filename output.filename\n\
\n\
	If no output.filename then write to stdout\n\
	If no input.filename then read from stdin\n\
\n\
Possible options are:\n\
\n\
	--djon    : output djon format\n\
	--json    : output json format\n\
	--compact : output compact\n\
	--pretty  : output pretty\n\
	--strict  : input/output strict\n\
	--        : stop parsing options\n\
\n\
We default to pretty output.\n\
\n\
");
				return 0;
			}
			else
			{
				fprintf(stderr,"Unknown option %s\n",cp);
				return 20;
			}
		}
		else // filename
		{
			if(!fname1) { fname1=cp; }
			else
			if(!fname2) { fname2=cp; }
			else
			{
				fprintf(stderr,"Unknown option %s\n",cp);
				return 20;
			}
		}
	}
	

	djon_state *ds=djon_setup();

	ds->strict=strict; // set strict mode from command line options
	ds->compact=compact; // set compact output flat from command line options
	
	if(fname1)
	{
		djon_load_file(ds,fname1); // filename
	}
	else
	{
		djon_load_file(ds,0); // stdin
	}
	if( ds->error_string ){ goto error; }

	
	djon_parse(ds);
	
	djon_value *v=djon_get(ds,ds->parse_value);

	if(fname2)
	{
		fp=fopen(fname2,"wb");
		if(!fp)
		{
			djon_set_error(ds,"output file error");
			goto error;
		}
		ds->fp=fp;
	}
	else
	{
		ds->fp=stdout;
	}

	if( ds->error_string ) // print parse error but still try and write json
	{
		fprintf(stderr,"%s\n",ds->error_string);
		fprintf(stderr,"line %d char %d byte %d\n",ds->error_line,ds->error_char,ds->error_idx);
		djon_set_error(ds,0);// clear error state
	}

	if(ds->parse_value)
	{
		if(write_djon)
		{
			djon_write_djon(ds,ds->parse_value);
		}
		else
		{
			djon_write_json(ds,ds->parse_value);
		}
	}
	
	if( ds->error_string ){ goto error; }

	if(fp)
	{
		fclose(fp);
	}
	djon_clean(ds);

	return 0;
error:
	if( ds->error_string )
	{
		fprintf(stderr,"%s\n",ds->error_string);
		fprintf(stderr,"line %d char %d byte %d\n",ds->error_line,ds->error_char,ds->error_idx);
	}
	if(fp)
	{
		fclose(fp);
	}
	if(ds)
	{
		djon_clean(ds);
	}
	return 20;
}
