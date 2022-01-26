/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"




/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// some macros to make the sourcecode more readable
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

//
// peek next char
//
#define SCAN_GET_C(p)		((int)(unsigned char)(*((p))))


//
// get next char and advance pointer forwards
//
#define SCAN_GET_CPP(p)		((int)(unsigned char)(*((p)++)))


//
// skip whitespace
//
#define SCAN_SKIP_WHITE(p)	while( isspace(SCAN_GET_C(p)) ) ++p;


bool is_alnum(char c) { return ( isalnum(c) || (c=='_') ); }


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// scan an s32 out of this string, returns pointer to after the read number or 0 for error
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
const char * string_scan_s32(const char *p, s32 *o)
{

s32 c;
s32 sign;

s32 check;
s32 total;

// reset total and overflow check

	total = 0;
	check = 0;


// skip leading whitespace

	SCAN_SKIP_WHITE(p);

// grab sign and advance, allow - or +

	sign = c = SCAN_GET_CPP(p);

	if (c == '-' || c == '+')
	{
		c = SCAN_GET_CPP(p);
	}

// loop reading decimal digits

	if(!isdigit(c)) { return 0; } //not a number

	while(isdigit(c))
	{
		check=total;
		total = (10 * total) + (c - '0');
		c = SCAN_GET_CPP(p);

		if(check>total) //overflow
		{
			return 0;
		}
	}
	--p; //step back since we just read a non digit

// apply cached sign and store result

	if(sign == '-')
	{
		*o=-total;
	}
	else
	{
		*o=total;
	}

// skip trailing whitespace

	SCAN_SKIP_WHITE(p);

	return p;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// scan a u32 out of this string, returns pointer to after the read number or 0 for error
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
const char * string_scan_u32(const char *p, u32 *o)
{

s32 c;

u32 check;
u32 total;


// reset total and overflow check

	total = 0;
	check = 0;

// skip leading whitespace

	SCAN_SKIP_WHITE(p);

// loop reading decimal digits

	c = SCAN_GET_CPP(p);
	if(!isdigit(c)) { return 0; } // not a number
	while(isdigit(c))
	{
		check=total;
		total = (10 * total) + (c - '0');
		c = SCAN_GET_CPP(p);

		if(check>total) //overflow
		{
			return 0;
		}
	}
	--p; //step back since we just read a non digit

// store result

	*o=total;

// skip trailing whitespace

	SCAN_SKIP_WHITE(p);

	return p;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// scan an u32 out of this string, returns pointer to after the read number or 0 for error
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
const char * string_scan_f32(const char *p, f32 *o)
{
	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// scan for a token, returns amount of whitespace to skip and strlen of token to then copy
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool string_scan_token(const char *p, s32 *white_size, s32 *token_size)
{
const char *u;
char c;

s32 count;

// skip leading whitespace

	u=p;
	SCAN_SKIP_WHITE(p);
	white_size[0]=p-u;

	u=p;

	c=*p;
	switch(c)	// do single and double char operator token tests
	{
		case '=' :
		{
			if( p[1] == '=' )	token_size[0]=2;
			else				token_size[0]=1;
			return true;
		}
		break;

		case '+' :
		{
			if( p[1] == '+' )	token_size[0]=2;
			else				token_size[0]=1;
			return true;
		}
		break;

		case '-' :
		{
			if( p[1] == '-' )	token_size[0]=2;
			else				token_size[0]=1;
			return true;
		}
		break;

		case '/' :
		case '*' :
		case '(' :
		case ')' :
		case ',' :
		case ';' :
		case ':' :
		{
			token_size[0]=1;
			return true;
		}
		break;
	}

	if(isdigit(c))
	{
		count=0;
		while(isdigit(*p++))
		{
			count++;
		}
		token_size[0]=count;
		return true;
	}

	if(is_alnum(c))
	{
		count=0;
		while(is_alnum(*p++))
		{
			count++;
		}
		token_size[0]=count;
		return true;
	}

	
	
	
	return false;
}

