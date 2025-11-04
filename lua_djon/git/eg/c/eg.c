
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define DJON_C 1
#include "djon.h"


// find index of = separating a path from a value
// return -1 if no = is found
int str_find_end_of_path(char *str)
{
	int ret=-1;
	int idx=0;
	int mode=0;
	for( char *cp=str ; *cp ; cp++ )
	{
		switch(mode)
		{
			case '\'':
				if(*cp=='\'')
				{
					mode=0; // end string
				}
				else
				if( (cp[0]=='\\') && (cp[1]=='\'') )
				{
					cp++; // skip next char
				}
			break;
			case '"':
				if(*cp=='"')
				{
					mode=0; // end string
				}
				else
				if( (cp[0]=='\\') && (cp[1]=='"') )
				{
					cp++; // skip next char
				}
			break;
			default:
				if(*cp=='=' ) { ret=idx; break; }
				else
				if(*cp=='\'') { mode='\''; }
				else
				if(*cp=='"') { mode='"'; }
			break;
		}
		idx++;
	}
	return ret;
}

int main(int argc, char *argv[])
{
	int error_code=20;
	
	// sanity in case of insane futures
	if(sizeof **argv != 1) { printf("invalid universe, char must be 1 byte!\n"); return 20; }

	int checkopts;
	FILE *fp=0;

	int idx;
	char *fname="eg.djon";
	char *cp;

	checkopts=1;
	for( int i=1 ; i<argc ; i++ )
	{
		char *cp=argv[i];
		if( checkopts && ( cp[0]=='-' && cp[1]=='-' ) ) // option
		{
			if( 0==strcmp(cp,"--") )
			{
				checkopts=0;
			}
			else
			if( 0==strncmp(cp,"--file=",7) )
			{
				fname=cp+7;
			}
			else
			if( 0==strcmp(cp,"--help") )
			{
				printf("\n\
eg is djon C example code\n\
\n\
	--file     : filename of djon file to load and save\n\
\n\
	path.path[path]\n\
		print the value of this path\n\
\n\
	path.path[path]=value\n\
		set the value of this path\n\
\n\
Objects can be created with a value of {} and arrays with [] any values\n\
made of digits will be converted to numbers. Wrapping a value in quotes\n\
will force it to be a string.\n\
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
	}
	

	printf("Loading %s\n",fname);
	djon_state *ds=djon_setup();
	
	{
		FILE *fp=fopen(fname,"r");
		if(fp) // try and load
		{
			fclose(fp);
			djon_load_file(ds,fname);
			if( ds->error_string ){ goto error; }
			djon_parse(ds);
			if( ds->error_string ){ goto error; }
		}
		else // create an {} base file
		{
			ds->parse_value=djon_alloc(ds);
			if(!ds->parse_value){ goto error; }
			djon_value_set(ds,ds->parse_value,DJON_OBJECT,0,0,0);
		}
	}

	if(ds->parse_value) // good read
	{

// now we can set/print from command line
		checkopts=1;
		for( int i=1 ; i<argc ; i++ )
		{
			cp=argv[i];
			if( checkopts && ( cp[0]=='-' && cp[1]=='-' ) ) // option
			{
				if( 0==strcmp(cp,"--") )
				{
					checkopts=0;
					cp=0;
				}
				else
				if( 0==strncmp(cp,"--",2) ) // ignore args that start with --
				{
					cp=0;
				}
			}
			if(cp) // a set or get depending on presence of an = in the string
			{
				int vi;
				const char *lastkey;
				char *path=cp;
				char *value="";
				char buff[256];
				int eop=str_find_end_of_path(cp);
				if(eop>=0) // this is a set
				{
					cp[eop]=0;
					value=cp+eop+1;
					printf("setting %s\n",path);
					int pi=djon_value_by_path(ds,ds->parse_value,path,&lastkey); // get last parent
					if( (djon_value_get_typ(ds,pi)&DJON_TYPEMASK)==DJON_OBJECT )
					{
						vi=djon_value_newkey(ds,pi,0,lastkey);
						if( !vi ){ goto error; }
					}
					else
					if( (djon_value_get_typ(ds,pi)&DJON_TYPEMASK)==DJON_ARRAY )
					{
						int lastidx=(int)djon_str_to_number(ds, (char*)lastkey , 0 );
						vi=djon_value_newindex(ds,pi,0,lastidx);
						if( !vi ){ goto error; }
					}
					else 
					{
						djon_value_set(ds,pi,DJON_OBJECT,0,0,0); // force convert to object
						vi=djon_value_newkey(ds,pi,0,lastkey);
						if( !vi ){ goto error; }
					}
					
					if( strcmp( value , "{}" )==0 )
					{
						djon_value_set(ds,vi,DJON_OBJECT,0,0,0);
					}
					else
					if( strcmp( value , "[]" )==0 )
					{
						djon_value_set(ds,vi,DJON_ARRAY,0,0,0);
					}
					else
					if( value[0]=='\'' || value[0]=='"' ) // quoted string
					{
						char q=value[0];
						for( char *ca=value+1,*cb=value,*cl=value+strlen(value) ; ca<=cl ; ) // unquote
						{
							if(*ca=='\\') // escape next
							{
								ca++; //  skip back quote
								*cb++=*ca++; // and force copy next
							}
							else
							if(*ca==q) // end quote
							{
								*cb++=0;
								break;
							}
							else // copy ( possibly over itself )
							{
								*cb++=*ca++;
							}
						}
						djon_value_set(ds,vi,DJON_STRING,0,0,value);
					}
					else
					{
						int isnumber=value[0]?1:0; // number can not be a zero length string
						for( char *ca=value,*cl=value+strlen(value) ; ca<cl ; ca++ ) // unquote
						{
							char c=*ca; // assume a number is any combination of 0123456789.
							if(! (((c>='0')&&(c<='9'))||(c=='.')) )
							{ isnumber=0; }
						}
						if(isnumber)
						{
							double d=djon_str_to_number(ds, (char*)value , 0 );
							djon_value_set(ds,vi,DJON_NUMBER,d,0,0);
						}
						else
						{
							djon_value_set(ds,vi,DJON_STRING,0,0,value);
						}
					}
				}
				else // this is a get and print
				{
					printf("getting %s\n",path);
					vi=djon_value_by_path(ds,ds->parse_value,path,0);
				}
				if(!vi)
				{
					printf("\tno value found at path\n");
				}
				else
				{
					switch( (djon_value_get_typ(ds,vi)&DJON_TYPEMASK) )
					{
						case DJON_STRING :
							djon_value_copy_str(ds,vi,buff+1,(sizeof buff)-2);
							buff[0]='"'; // wrap strings in quotes
							buff[ strlen(buff)+1 ]=0;
							buff[ strlen(buff)   ]='"';
							value=buff;
						break;
						case DJON_NUMBER :
							djon_double_to_str( djon_value_get_num(ds,vi) ,buff);
							value=buff;
						break;
						case DJON_BOOL :
							value=( (djon_value_get_num(ds,vi)==0.0) ? "FALSE" : "TRUE" );
						break;
						case DJON_OBJECT :
							value="{}";
						break;
						case DJON_ARRAY :
							value="[]";
						break;
						case DJON_NULL :
							value="NULL";
						break;
						default : // something went terribly wrong
							value="ERROR";
						break;
					}
					printf("\t=\t%s\n",value);
				}
			}

		}

// iterate over all values in json
		printf("Printing all paths and values\n");
		for( int vi=ds->parse_value ; vi ; vi=djon_value_all(ds,ds->parse_value,vi) )
		{
			int vt=djon_value_get_typ(ds,vi);
			if((vt&DJON_FLAGMASK)!=DJON_KEY) // ignore keys (which are also strings)
			{
				char *value="";
				char buff[256];
				switch(vt&DJON_TYPEMASK)
				{
					case DJON_STRING :
						djon_value_copy_str(ds,vi,buff+1,(sizeof buff)-2);
						buff[0]='"'; // wrap strings in quotes
						buff[ strlen(buff)+1 ]=0;
						buff[ strlen(buff)   ]='"';
						value=buff;
					break;
					case DJON_NUMBER :
						djon_double_to_str( djon_value_get_num(ds,vi) ,buff);
						value=buff;
					break;
					case DJON_BOOL :
						value=( (djon_value_get_num(ds,vi)==0.0) ? "FALSE" : "TRUE" );
					break;
					case DJON_OBJECT :
						value="{}";
					break;
					case DJON_ARRAY :
						value="[]";
					break;
					case DJON_NULL :
						value="NULL";
					break;
				}
				printf("%32s = %s\n", djon_value_to_path(ds,0,vi) , value );
			}
		}

		printf("Saving %s\n",fname);
		fp=fopen(fname,"wb");
		if(!fp)
		{
			djon_set_error(ds,"output file error");
			goto error;
		}
		ds->fp=fp;

		djon_write_djon(ds,ds->parse_value);
		if( ds->error_string ){ goto error; }
	}
	
	error_code=0; // not an error
error:
	if(fp)
	{
		fclose(fp); fp=0;
	}
	if(ds)
	{
		if( ds->error_string )
		{
			fprintf(stderr,"%s\n",ds->error_string);
			fprintf(stderr,"line %d char %d byte %d\n",ds->error_line,ds->error_char,ds->error_idx);
		}
		djon_clean(ds); ds=0;
	}
	return error_code;
}
