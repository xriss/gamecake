
#include "node_api.h"


// stack sniffing not working with wasm
//#define DJON_MAX_STACK (0)

#define DJON_FILE (0)
#define DJON_C 1
#include "djon.h"


// is this what we need?
extern void * napi_wasm_malloc( size_t siz ) { return malloc(siz); }

// check error state and optionally throw an exception
static int js_error(napi_env env,int err)
{
	// check pending
	bool pending=0;
	napi_is_exception_pending( env , &pending );
	if(pending) { return 1; }

	// check err (status), 0 is OK (napi_ok)
	if(err)
	{
		napi_throw_error( env , NULL , "error" );
		return 1;
	}
	
	// everything good
	return 0;
}

typedef struct js_struct_functions{
	const char *name;
	napi_value (*func)(napi_env env, napi_callback_info info);
} js_struct_functions ;

static napi_value js_export_functions(napi_env env, napi_value exports, const js_struct_functions funcs[], void *data)
{
	if(js_error(env,0)){return 0;};
	for( const js_struct_functions *fp=funcs ; fp->name ; fp++ )
	{
		napi_value exported_function;
		if(js_error(env,

			napi_create_function(env , fp->name , strlen(fp->name) , fp->func , data , &exported_function )

		)){return 0;}
		if(js_error(env,

			napi_set_named_property(env , exports , fp->name , exported_function )

		)){return 0;}
	}
	return exports;
}


static napi_value js_null(napi_env env)
{
	napi_value ret;
	if(js_error(env,

		napi_get_null(env ,  &ret )

	)){return 0;}
	return ret;
}

static napi_value js_bool(napi_env env,int num)
{
	napi_value ret;
	if(js_error(env,

		napi_get_boolean(env, num , &ret )

	)){return 0;}
	return ret;
}

static napi_value js_object(napi_env env)
{
	napi_value ret;
	if(js_error(env,
	
		napi_create_object(env, &ret )

	)){return 0;}
	return ret;
}

static napi_value js_array(napi_env env,int len)
{
	napi_value ret;
	if(js_error(env,
	
		napi_create_array_with_length(env, len , &ret )
	
	)){return 0;}
	return ret;
}

static napi_value js_number(napi_env env,double num)
{
	napi_value ret;
	if(js_error(env,

		napi_create_double(env, num , &ret )

	)){return 0;}
	return ret;
}

static napi_value js_string(napi_env env,const char *cp)
{
	napi_value ret;
	if(js_error(env,

		napi_create_string_utf8(env, cp , strlen(cp) , &ret )

	)){return 0;}
	return ret;
}

static napi_value js_string_len(napi_env env,const char *cp,int len)
{
	napi_value ret;
	if(js_error(env,

		napi_create_string_utf8(env, cp , len , &ret )

	)){return 0;}
	return ret;
}

static napi_value js_buffer(napi_env env,const char *cp,int len)
{
	napi_value ret;
	if(js_error(env,

		napi_create_buffer_copy(env, len , cp , NULL , &ret)

	)){return 0;}
	return ret;
}

static int js_typeof(napi_env env,napi_value val)
{
	napi_valuetype ret=napi_undefined;
	if(js_error(env,

		napi_typeof(env, val, &ret)

	)){return napi_undefined;}
	return ret;
}

static int js_is_array(napi_env env,napi_value val)
{
	bool ret=0;
	if(js_error(env,

		napi_is_array(env, val, &ret)

	)){return 0;}
	return ret;
}

static int js_is_object(napi_env env,napi_value val)
{
	napi_valuetype ret=napi_undefined;
	if(js_error(env,
	
		napi_typeof(env, val, &ret)

	)){return 0;}
	return (ret==napi_object);
}

static int js_is_bool(napi_env env,napi_value val)
{
	napi_valuetype ret=napi_undefined;
	if(js_error(env,

		napi_typeof(env, val, &ret)

	)){return 0;}
	return (ret==napi_boolean);
}

static int js_is_number(napi_env env,napi_value val)
{
	napi_valuetype ret=napi_undefined;
	if(js_error(env,

		napi_typeof(env, val, &ret)
		
	)){return 0;}
	return (ret==napi_number);
}

static int js_is_string(napi_env env,napi_value val)
{
	napi_valuetype ret=napi_undefined;
	if(js_error(env,

		napi_typeof(env, val, &ret)

	)){return 0;}
	return (ret==napi_string);
}

static double js_to_bool(napi_env env,napi_value val)
{
	bool ret;
	if(js_error(env,

		napi_get_value_bool(env,val,&ret)

	)){return 0;}
	return ret;
}

static double js_to_double(napi_env env,napi_value val)
{
	double ret;
	if(js_error(env,

		napi_get_value_double(env,val,&ret)

	)){return 0;}
	return ret;
}

static int js_len_string(napi_env env,napi_value val)
{
	size_t size=0;
	if(js_error(env,

		napi_get_value_string_utf8(env,val,0,0,&size)

	)){return 0;}
	return (int)size;
}

// write a atring into ptr
static int js_ptr_string(napi_env env,napi_value val,char *cp,int size)
{
	size_t len=0;
	if(js_error(env,

		napi_get_value_string_utf8(env,val,cp,size,&len)

	)){return 0;}
	return len;
}


// alloced, so you will need to free this
static const char * js_to_string(napi_env env,napi_value val)
{
	char *cp;
	size_t size=0;
	size_t len=0;

	if(js_error(env,

		napi_get_value_string_utf8(env,	val , 0 , 0 , &size )

	)){return 0;}

	size=size+1; // term
	cp=malloc(size);
	if(!cp){ napi_throw_error(env, NULL, "out of memory" ); return 0; }

	if(js_error(env,

		napi_get_value_string_utf8(env, val , cp , size , &len )

	)){return 0;}

	return cp;
}

// get all object keys as array of values
static napi_value js_key_array(napi_env env,napi_value obj)
{
	napi_value ret;
	if(js_error(env,

		napi_get_property_names(env, obj, &ret )

	)){return 0;}
	return ret;
}

static int js_len_array(napi_env env,napi_value arr)
{
	uint32_t size=0;
	if(js_error(env,

		napi_get_array_length(env,arr,&size)

	)){return 0;}
	return (int)size;
}

static napi_value js_val_set(napi_env env,napi_value obj, napi_value key, napi_value val)
{
	if(js_error(env,

		napi_set_property(env, obj , key , val )

	)){return 0;}
	return obj;
}
static napi_value js_val_get(napi_env env,napi_value obj, napi_value key)
{
	napi_value ret;
	if(js_error(env,

		napi_get_property(env, obj , key , &ret )

	)){return 0;}
	return ret;
}
static napi_value js_str_set(napi_env env,napi_value obj, const char *key, napi_value val)
{
	if(js_error(env,
	
		napi_set_named_property(env, obj , key , val )

	)){return 0;}
	return obj;
}
static napi_value js_str_get(napi_env env,napi_value obj, const char *key)
{
	napi_value ret;
	if(js_error(env,

		napi_get_named_property(env, obj , key , &ret )

	)){return 0;}
	return ret;
}
static napi_value js_idx_set(napi_env env,napi_value arr, int idx, napi_value val)
{
	if(js_error(env,
	
		napi_set_element(env, arr , idx , val )

	)){return 0;}
	return arr;
}
static napi_value js_idx_get(napi_env env,napi_value arr, int idx)
{
	napi_value ret;
	if(js_error(env,

		napi_get_element(env, arr , idx , &ret )

	)){return 0;}
	return ret;
}

static napi_value djon_core_locate(napi_env env, napi_callback_info info)
{
	size_t argc=8;
	napi_value argv[8];
	napi_value thisjs;
	djon_state *ds;
	if(js_error(env,
		napi_get_cb_info(env, info, &argc, argv, &thisjs, (void**)&ds )
	)){return 0;}

	napi_value a=js_array(env,4);
	if(ds->error_string)
	{
		js_idx_set(env,a,0,js_string(env,(const char *)ds->error_string));
	}
	else
	{
		js_idx_set(env,a,0,js_null(env));
	}
	js_idx_set(env,a,1,js_number(env,ds->error_line));
	js_idx_set(env,a,2,js_number(env,ds->error_char));
	js_idx_set(env,a,3,js_number(env,ds->error_idx));
	return a;
}

static napi_value djon_core_get_value(napi_env env,djon_state *ds,int idx)
{
djon_value *v=djon_get(ds,idx);
int ai,len;
napi_value arr;
napi_value obj;
	if(!v) { return js_null(env); }
	switch(v->typ&DJON_TYPEMASK)
	{
		case DJON_ARRAY:
			len=0;
			for( int vi=v->lst ; vi ; vi=djon_get(ds,vi)->nxt )
			{
				len++;
			}
			arr=js_array(env,len);
			ai=0;
			for( int vi=v->lst ; vi ; vi=djon_get(ds,vi)->nxt )
			{
				js_idx_set( env , arr , ai , djon_core_get_value( env , ds ,vi ) );
				ai++;
			}
			return arr;
		break;
		case DJON_OBJECT:
			obj=js_object(env);
			for( int ki=v->lst ; ki ; ki=djon_get(ds,ki)->nxt )
			{
				js_val_set( env , obj , djon_core_get_value( env , ds , ki ) , djon_core_get_value( env , ds , djon_get(ds,ki)->lst ) );
			}
			return obj;
		break;
		case DJON_STRING:
			return js_string_len( env , v->str , v->len );
		break;
		case DJON_NUMBER:
			return js_number( env , v->num );
		break;
		case DJON_BOOL:
			return js_bool( env , v->num ? true : false );
		break;
	}
	
	return js_null(env);
}
static napi_value djon_core_get(napi_env env, napi_callback_info info)
{
	size_t argc=8;
	napi_value argv[8];
	napi_value thisjs;
	djon_state *ds;
	if(js_error(env,
		napi_get_cb_info(env, info, &argc, argv, &thisjs, (void**)&ds )
	)){return 0;}

	int comments=0;
	for(int i=0;(size_t)i<argc;i++) // check string flags in args
	{
		const char *cp=js_to_string(env,argv[i]);
		if( strcmp(cp,"comments")==0 ) { comments=1; }
		free((void*)cp);
	}
	if(comments)
	{
		ds->parse_value=djon_value_to_vca(ds,ds->parse_value);
	}
	return djon_core_get_value(env,ds,ds->parse_value);
}


int djon_core_set_value(napi_env env,djon_state *ds,const napi_value val){
int di=0;
djon_value *dv=0;
	if( js_is_array(env,val) )
	{
		di=djon_alloc(ds); if(!di) { napi_throw_error(env, NULL, "out of memory" ); return 0; }
		dv=djon_get(ds,di);
		dv->typ=DJON_ARRAY;

		int len=js_len_array(env,val);
		int li=0;
		for(int i=0;i<len;i++)
		{
			int vi=djon_core_set_value(env,ds,js_idx_get(env,val,i));

			dv=djon_get(ds,di); // realloc safe
			if( dv->lst==0) // first
			{
				dv->lst=vi;
			}
			else // chain
			{
				djon_get(ds,li)->nxt=vi;
				djon_get(ds,vi)->prv=li;
			}
			djon_get(ds,vi)->par=di;
			li=vi;
		}
		return di;
	}
	else
	if( js_is_object(env,val) )
	{
		di=djon_alloc(ds); if(!di) { napi_throw_error(env, NULL, "out of memory" ); return 0; }
		dv=djon_get(ds,di);
		dv->typ=DJON_OBJECT;
		
		napi_value arr = js_key_array(env,val);
		int len=js_len_array(env,arr);
		int li=0;
		for(int i=0;i<len;i++)
		{
			napi_value k=js_idx_get(env,arr,i);
			napi_value v=js_val_get(env,val,k);

			int ki=djon_core_set_value(env,ds,k);
			int vi=djon_core_set_value(env,ds,v);
			djon_get(ds,ki)->lst=vi; // key to value

			dv=djon_get(ds,di); // realloc safe
			if( dv->lst==0) // first
			{
				dv->lst=ki;
			}
			else // chain
			{
				djon_get(ds,li)->nxt=ki;
				djon_get(ds,ki)->prv=li;
			}
			djon_get(ds,ki)->typ=DJON_KEY|DJON_STRING; // this is a key
			djon_get(ds,ki)->par=di;
			djon_get(ds,vi)->par=ki;
			li=ki;

		}
		return di;
	}
	else
	if( js_is_string(env,val) )
	{
		int len=js_len_string(env,val);
		int p=djon_write_data(ds,0,len+1); // allocate string but write nothing and return idx
		js_ptr_string(env,val,ds->write_data+p,len+1);
		di=djon_alloc(ds); if(!di) { napi_throw_error(env, NULL, "out of memory" ); return 0; }
		dv=djon_get(ds,di);
		dv->typ=DJON_STRING;
		dv->str=((char *)(0))+p; // we will need to add base memory address when we finish
		dv->len=len;
		return di;
	}
	else
	if( js_is_number(env,val) )
	{
		di=djon_alloc(ds); if(!di) { napi_throw_error(env, NULL, "out of memory" ); return 0; }
		dv=djon_get(ds,di);
		dv->typ=DJON_NUMBER;
		dv->num=js_to_double(env,val);
		return di;
	}
	else
	if( js_is_bool(env,val) )
	{
		di=djon_alloc(ds); if(!di) { napi_throw_error(env, NULL, "out of memory" ); return 0; }
		dv=djon_get(ds,di);
		dv->typ=DJON_BOOL;
		dv->num=js_to_bool(env,val)?1.0:0.0; // bool to double
		return di;
	}
	else // everything else is a null
	{
		di=djon_alloc(ds); if(!di) { napi_throw_error(env, NULL, "out of memory" ); return 0; }
		dv=djon_get(ds,di);
		dv->typ=DJON_NULL;
		return di;
	}
	return 0;
}

// we need to fix all of the string pointers
void djon_core_set_fix(djon_state *ds,int idx,char *base){
djon_value *v=djon_get(ds,idx);
int vi=0;
int ki=0;
	switch(v->typ&DJON_TYPEMASK)
	{
		case DJON_ARRAY:
			vi=v->lst;
			while( vi )
			{
				djon_core_set_fix( ds , vi , base );
				vi=djon_get(ds,vi)->nxt;
			}
		break;
		case DJON_OBJECT:
			ki=v->lst;
			while( ki )
			{
				djon_core_set_fix( ds , ki , base );
				djon_core_set_fix( ds , djon_get(ds,ki)->lst , base );
				ki=djon_get(ds,ki)->nxt;
			}
		break;
		case DJON_STRING:
			v->str=(v->str-((char*)0))+base;
		break;
	}
}

static napi_value djon_core_set(napi_env env, napi_callback_info info)
{
	size_t argc=8;
	napi_value argv[8];
	napi_value thisjs;
	djon_state *ds;
	if(js_error(env,
		napi_get_cb_info(env, info, &argc, argv, &thisjs, (void**)&ds )
	)){return 0;}

	int comments=0;
	for(int i=1;(size_t)i<argc;i++) // check string flags in args
	{
		const char *cp=js_to_string(env,argv[i]);
		if( strcmp(cp,"comments")==0 ) { comments=1; }
		free((void*)cp);
	}


	ds->write_data=0;
	ds->parse_value=djon_core_set_value(env,ds,argv[0]);
	// keep string data and fix all the pointers
	ds->data=ds->write_data;
	ds->data_len=ds->write_size;
	ds->write_data=0;
	djon_core_set_fix(ds,ds->parse_value,ds->data);

	if(comments)
	{
		ds->parse_value=djon_vca_to_value(ds,ds->parse_value);
	}

	return NULL;
}

static napi_value djon_core_load(napi_env env, napi_callback_info info)
{
	size_t argc=8;
	napi_value argv[8];
	napi_value thisjs;
	djon_state *ds;
	if(js_error(env,
		napi_get_cb_info(env, info, &argc, argv, &thisjs, (void**)&ds )
	)){return 0;}

	// this will get auto free on object cleanup
	ds->data = (char*) js_to_string(env,argv[0]);
	if(!ds->data){return NULL;}
	ds->data_len=strlen(ds->data);

	ds->strict=0;
	for(int i=1;(size_t)i<argc;i++) // check string flags in args
	{
		const char *cp=js_to_string(env,argv[i]);
		if( strcmp(cp,"strict")==0 ) { ds->strict=1; }
		free((void*)cp);
	}

	djon_parse(ds);

	return NULL;
}

static napi_value djon_core_save(napi_env env, napi_callback_info info)
{
	size_t argc=8;
	napi_value argv[8];
	napi_value thisjs;
	djon_state *ds;
	if(js_error(env,
		napi_get_cb_info(env, info, &argc, argv, &thisjs, (void**)&ds )
	)){return 0;}

	napi_value ret;

	int write_djon=0;

	ds->write=&djon_write_data; // we want to write to a string
	ds->write_data=0; //  and reset

	ds->compact=0;
	ds->strict=0;
	for(int i=0;(size_t)i<argc;i++) // check string flags in args
	{
		const char *cp=js_to_string(env,argv[i]);
		if( strcmp(cp,"djon")==0 ) { write_djon=1; }
		if( strcmp(cp,"compact")==0 ) { ds->compact=1; }
		if( strcmp(cp,"strict")==0 ) { ds->strict=1; }
		free((void*)cp);
	}

	if(write_djon)
	{
		djon_write_djon(ds,ds->parse_value);
	}
	else
	{
		djon_write_json(ds,ds->parse_value);
	}
	
	if(ds->write_data)
	{
		ret=js_string_len( env , ds->write_data , ds->write_len );
	}
	
	free(ds->write_data); // and free write buffer
	ds->write_data=0;


	return ret;
}

static void djon_core_finalizer(napi_env env, void *data, void *hint)
{
	djon_state *ds=(djon_state *)data;
	djon_clean(ds);
}

static napi_value djon_core_new(napi_env env, napi_callback_info info)
{
	const js_struct_functions funcs[]={

		{		"locate"	,	djon_core_locate	},
		{		"get"		,	djon_core_get		},
		{		"set"		,	djon_core_set		},
		{		"load"		,	djon_core_load		},
		{		"save"		,	djon_core_save		},

		{0,0}
	};
	djon_state *ds=0;
	napi_value exports;
	
	ds=djon_setup();
	if(!ds){return NULL;}
	
	if(js_error(env,

		napi_create_object(env, &exports)
	
	)){return 0;}
	if(js_error(env,

		napi_add_finalizer(env, exports , ds , djon_core_finalizer , 0 , 0 )

	)){return 0;}
                               
	return js_export_functions(env,exports,funcs,ds);
}


NAPI_MODULE_INIT() {
	const js_struct_functions funcs[]={

		{		"djoncore"	,	djon_core_new	},

		{0,0}
	};
	return js_export_functions(env,exports,funcs,0);
}

