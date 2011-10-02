/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/




/*+-----------------------------------------------------------------------------------------------------------------+*/

struct llatom
{
	DLIST_DEF(llatom);
};

#define LLATOM_DEF(s)	struct s *next;	struct s *prev;



/*+-----------------------------------------------------------------------------------------------------------------+*/

struct llpuddle
{
	DLIST_DEF(llpuddle);
};

/*+-----------------------------------------------------------------------------------------------------------------+*/

struct llatoms
{
	dhead atoms[1];
	dhead puddles[1];
	dhead freeatoms[1];

	long sizeofatoms;	// size of each single atom
	long numberofatoms;	// number of atoms in each puddle


llpuddle *	add_puddle(void);
llatom *	alloc(void); // returned data will be cleared to 0
void		free(llatom *atom);


bool setup(s32 size , s32 numof);
bool reset(void);
void clean(void);

};

/*+-----------------------------------------------------------------------------------------------------------------+*/







