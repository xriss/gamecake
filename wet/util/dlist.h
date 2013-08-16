/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/


/*------------------------------------------------------------------------------------------------------------------------------*/
//
// derive me baby
//
/*------------------------------------------------------------------------------------------------------------------------------*/
struct dlist
{

	dlist	*next;
	dlist	*prev;

};
#define DLIST_DEF(s)	struct s *next;	struct s *prev;



dlist* dlist_cut(dlist *me);

dlist* dlist_paste(dlist *after , dlist *me , dlist *before);


/*------------------------------------------------------------------------------------------------------------------------------*/
//
// its a header, if you dont know how this junk works u is stoopid :)
//
/*------------------------------------------------------------------------------------------------------------------------------*/
struct dhead
{

	dlist	*first;
	dlist	*null;
	dlist	*last;

};
#define DHEAD_DEF(s)			struct s *first;	struct s *null;		struct s *last;

#define DHEAD_DEF_LAB(s,lab)	struct s *lab##first;	struct s *lab##null;		struct s *lab##last;



void dhead_init(dhead *head);

void dlist_build(dhead *head , u8 *data , s32 size , s32 count);


//
// Forced casts :)
//

#define DLIST_CUT(a)			dlist_cut((dlist*)(a))

#define DLIST_PASTE(a,b,c)		dlist_paste((dlist*)(a),(dlist*)(b),(dlist*)(c))

#define DLIST_CUTPASTE(a,b,c)	dlist_cut((dlist*)(b)); dlist_paste((dlist*)(a),(dlist*)(b),(dlist*)(c))

#define DHEAD_INIT(a)			dhead_init((dhead *)(a))

#define DLIST_BUILD(a,b,c,d)	dlist_build((dhead*)a,(u8*)b,c,d)


#define DLIST_PASTE_BEFORE(a,b)		((a)->prev=(b)->prev)->next=(a); (a)->next=(b); (b)->prev=(a)
#define DLIST_PASTE_AFTER(a,b)		((a)->next=(b)->next)->prev=(a); (a)->prev=(b); (b)->next=(a)

#define DLIST_CUT_PASTE_BEFORE(a,b)	dlist_cut((dlist*)(a));	((a)->prev=(b)->prev)->next=(a); (a)->next=(b); (b)->prev=(a)
#define DLIST_CUT_PASTE_AFTER(a,b)	dlist_cut((dlist*)(a));	((a)->next=(b)->next)->prev=(a); (a)->prev=(b); (b)->next=(a)
