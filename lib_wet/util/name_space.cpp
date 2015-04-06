/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"




/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Setup junk
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool name_space::setup(void)
{
	chunk=0;

	ignore_case=true;

	return true;
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Reset junk
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool name_space::reset(void)
{
	clean();
	return setup();
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Clean junk
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void name_space::clean(void)
{
struct name_space_chunk *nsc,*lnsc;

//free in reverse order of allocation just to be nice

	while(chunk)
	{
		lnsc=0;
		nsc=chunk;
		while(nsc->next) // get last chunk into nsc and previous chunk or null into lnsc
		{
			lnsc=nsc;
			nsc=nsc->next;
		}
		if(lnsc) //linked one
		{
			lnsc->next=0;
			MEM_free(nsc);
		}
		else // first one no linked parent just base struc
		{
			chunk=0;
			MEM_free(nsc);
		}
	}
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// turn a string into a const string. the address of this const string may then be used as an ID number for this string
// in this name space. so string comparisons can be done with a simple == test on these const strings
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
const char * name_space::Make(const char *name)
{
struct name_space_chunk *nsc,*lnsc;
char *cp,*lcp;
const char *tcp;
//char *ret;

// test for first time call

	if(chunk==0)
	{
		if(!(chunk=(name_space_chunk*)MEM_calloc(sizeof(name_space_chunk))))
		{
			return(0); //coouldnt even allocate the first chunk
		}
	}

// step through chunks and search for strings

	for( nsc=chunk ; nsc ; nsc=nsc->next )
	{
//search for string in cura chunk
		lcp=cp=nsc->data;
		while(*cp) //finish on double null
		{
			tcp=name; //test pos
			while(*cp)
			{
				if(tcp)
				{
					if(ignore_case)
					{
						if(tolower(*cp)==tolower(*tcp)) //check cura char
							//     if((*cp)==(*tcp)) //check cura char
						{
							tcp++; //next name char
						}
						else
						{
							tcp=0; //no match just skip to end of check string to find start of next
						}
					}
					else
					{
						if((*cp)==(*tcp)) //check cura char
						{
							tcp++; //next name char
						}
						else
						{
							tcp=0; //no match just skip to end of check string to find start of next
						}
					}
				}
				cp++; // next test char
			}
			if(tcp)
			{
				if(*tcp==0) //its a match
				{
					return(lcp);
				}
			}
			cp++; //skip null term
			lcp=cp; // remember start of test string
		}
		lnsc=nsc; //remember last chunk we checked
	}

// couldnt find it so need to add it to the chunks

	if(!lnsc) return(0); //something went wrong somewhere

	if( (strlen(name)+2) <= (sizeof(lnsc->data))-(lcp-lnsc->data) ) //enougth space to splat on the end?
	{
		strcpy(lcp,name);
		return(lcp);
	}
	else //need to allocate a new chunk and put it there
	{
		if(!(lnsc->next=(name_space_chunk*)MEM_calloc(sizeof(name_space_chunk))))
		{
			return(0); //couldnt allocate a new chunk
		}
		if( sizeof(lnsc->data) >= (strlen(name)+2) ) //paranoid checks r us
		{
			strcpy(lnsc->next->data,name);
			return(lnsc->next->data);
		}
		else
		{
			return(0); //no can do
		}
	}
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// As make only doesnt create the string if it doesnt exist
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
const char * name_space::Find(const char *name)
{
struct name_space_chunk *nsc,*lnsc;
char *cp,*lcp;
const char *tcp;
//char *ret;

// test for first time call

	if(chunk==0)
	{
		return(0);
	}

// step through chunks and search for strings

	for( nsc=chunk ; nsc ; nsc=nsc->next )
	{
//search for string in cura chunk
		lcp=cp=nsc->data;
		while(*cp) //finish on double null
		{
			tcp=name; //test pos
			while(*cp)
			{
				if(tcp)
				{
					if(ignore_case)
					{
						if(tolower(*cp)==tolower(*tcp)) //check cura char
							//     if((*cp)==(*tcp)) //check cura char
						{
							tcp++; //next name char
						}
						else
						{
							tcp=0; //no match just skip to end of check string to find start of next
						}
					}
					else
					{
						if((*cp)==(*tcp)) //check cura char
						{
							tcp++; //next name char
						}
						else
						{
							tcp=0; //no match just skip to end of check string to find start of next
						}
					}
				}
				cp++; // next test char
			}
			if(tcp)
			{
				if(*tcp==0) //its a match
				{
					return(lcp);
				}
			}
			cp++; //skip null term
			lcp=cp; // remember start of test string
		}
		lnsc=nsc; //remember last chunk we checked
	}

	return(0);

}





