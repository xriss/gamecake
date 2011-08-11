
int cgm_getfilepos ( void );

int cgmb_exec_comand ( int, int );
int cgmb_geti8  ( signed char * );
int cgmb_geti16 ( short * );
int cgmb_geti24 ( long * );
int cgmb_geti32 ( long * );
int cgmb_getu8  ( unsigned char * );
int cgmb_getu16 ( unsigned short * );
int cgmb_getu24 ( unsigned long * );
int cgmb_getu32 ( unsigned long * );
int cgmb_getfl32 ( float * );
int cgmb_getfl64 ( double * );
int cgmb_getfx32 ( float * );
int cgmb_getfx64 ( double * );
int cgmb_ter ( void );
int cgmb_rch ( void );
int cgmb_ci ( unsigned long * );
int cgmb_cd ( unsigned long * );
int cgmb_rgb ( unsigned long *, unsigned long *, unsigned long * );
int cgmb_ix ( long * );
int cgmb_e ( short * );
int cgmb_i ( long * );
int cgmb_u ( unsigned long * );
int cgmb_r ( double * );
int cgmb_s ( char ** );
int cgmb_vdc ( double * );
int cgmb_p ( double *, double * );
int cgmb_co ( void * );
int cgmb_getpixel ( void *, int );
int cgmb_getc ( unsigned char * );

char *cgmt_getsep ( void );
void cgmt_getcom ( void );
char *cgmt_getparentheses ( void );
int cgmt_ter ( void );
int cgmt_rch ( void );
int cgmt_i ( long * );
int cgmt_ci ( unsigned long * );
int cgmt_cd ( unsigned long * );
int cgmt_rgb ( unsigned long *, unsigned long *, unsigned long * );
int cgmt_ix ( long * );
int cgmt_e ( short *, const char ** );
int cgmt_r ( double * );
int cgmt_s ( char ** );
int cgmt_vdc ( double * );
int cgmt_p ( double *, double * );
int cgmt_co ( void * );



