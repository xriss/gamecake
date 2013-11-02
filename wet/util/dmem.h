/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/




void dmem_copy( cu8* from , u8* dest , int size);

void dmem_set( u8 dat , u8* dest , int size);




//
// zero a structure, make sure its all cleared to zero, pass in pointer/label and this uses sizeof to find size
//
#define DMEM_ZERO(a) dmem_set(0,(u8*)(a),sizeof(*a))


//
// get number of items in an array
//
#define NUMOF(a) ((s32)(sizeof((a))/sizeof(*(a))))


//
// add bytes to a pointer, IE ignore pointer type and treat it as a u8*
//
// by assigning it back to val it helps to catch errors as it forces a to be an assignable value
//
#define RAWPTR_INC(val,add) ((void*)(val))=((void*)(((u8*)(val))+(add)))
//
// as above without the assignment
//
#define RAWPTR_ADD(val,add) ((void*)(((u8*)(val))+(add)))



//
// copy a string into a buffer, will clip the string if it is bigger
//
#define COPY_STR_TO_BUFFER(buf,str)	strncpy(buf,str,sizeof(buf));buf[sizeof(buf)-1]=0;


/*------------------------------------------------------------------------------------------------------------------------------*/
//
//
//
/*------------------------------------------------------------------------------------------------------------------------------*/



