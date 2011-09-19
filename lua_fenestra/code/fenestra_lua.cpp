/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

// this is a huge amount of junk so only include in this file
#if defined(X11)

#include <gtkmm.h>

#endif

// this will however fuck with microsofts crazy header cache scheme, so turn that off for this file
#include "all.h"


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

static int core_setup(lua_State *l)
{
	struct fenestra *core = (struct fenestra *)lua_newuserdata(l, sizeof(fenestra));
	
#if defined(WIN32)

HWND into_hwnd=0;

	lua_pushstring(l,"into_hwnd");
	lua_rawget(l,1);
	if( lua_isuserdata(l,-1) )
	{
		into_hwnd=(HWND)lua_touserdata(l,-1);
	}
	lua_pop(l,1);

#endif

	core->prepare();
	
	core->l=l;
	
	lua_pushlightuserdata(l,core);
	lua_pushvalue(l,1);
	lua_settable(l, LUA_REGISTRYINDEX); // save main tab associated with main struct

	lua_pushstring(l,"width");
	lua_rawget(l,1);
	if( lua_isnumber(l,-1) )
	{
		core->width=(s32)lua_tonumber(l,-1);
	}
	lua_pop(l,1);

	lua_pushstring(l,"height");
	lua_rawget(l,1);
	if( lua_isnumber(l,-1) )
	{
		core->height=(s32)lua_tonumber(l,-1);
	}
	lua_pop(l,1);

	
#if defined(WIN32)

	core->setup(into_hwnd);

#elif defined(X11)

	core->setup();

#endif
	
	return 1;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_clean(lua_State *l)
{
	struct fenestra *core = (struct fenestra *)lua_touserdata(l, 1 );

	core->clean();

	lua_pushlightuserdata(l,core);
	lua_pushnil(l);
	lua_settable(l, LUA_REGISTRYINDEX); // clear main tab associated with main struct

	return 0;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_msg(lua_State *l)
{
int ret=1;

	struct fenestra *core = (struct fenestra *)lua_touserdata(l, 1 );

	const char *cmd = luaL_checkstring (l, 2);
	
	

#if defined(WIN32)

MSG msg;

	if(strcmp(cmd,"peek")==0) // check if there is a msg
	{
		if( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
		{
			lua_pushboolean(l,1);
		}
		else
		{
			lua_pushboolean(l,0);
		}
		return 1;
	}
	else
	if(strcmp(cmd,"wait")==0) // wait for a msg, but wait no more than 1ms
	{
		MsgWaitForMultipleObjects(1, &core->proc,FALSE, 1, QS_ALLINPUT);
	}

// handle all msgs that are waiting but do not wait for any

	while( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
	{
		if( !GetMessage( &msg, NULL, 0, 0 ) ) { ret=0; } // Simon says shutdown
		TranslateMessage(&msg); 
		DispatchMessage(&msg);
	}

#elif defined(X11)

	if(strcmp(cmd,"peek")==0) // check if there is a msg
	{
		if( XPending( core->dsp ) > 0 )
		{
			lua_pushboolean(l,1);
		}
		else
		{
			lua_pushboolean(l,0);
		}
		return 1;
	}
	else
	if(strcmp(cmd,"wait")==0) // wait for a msg, but wait no more than 1ms
	{
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 1000; // 1ms
		select(core->fp_dsp, &core->set_dsp, 0, 0, &tv);
	}

// handle all msgs that are waiting but do not wait for any

	XEvent e[1];
	while( XPending( core->dsp ) > 0 )
	{
		XNextEvent(core->dsp, e);
		core->event_handler( e );
	}
	
#endif

// return false if window has been closed
	lua_pushboolean(l,ret);

	return 1;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_time(lua_State *l)
{
	struct fenestra *core = (struct fenestra *)lua_touserdata(l, 1 );

	lua_pushnumber(l,core->time());

	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_setwin(lua_State *l)
{
	struct fenestra *core = (struct fenestra *)lua_touserdata(l, 1 );

	const char *sv = luaL_checkstring (l, 2);
	
	const char *si=0;
	int i=0;
	if(lua_isstring(l,3))
	{
		si = luaL_checkstring (l, 3);
	}
	else
	{
		i = (int)lua_tonumber(l,3);
	}
	
	if(strcmp(sv,"call_update")==0)
	{
		core->call_update=i?true:false;
	}
	
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_getwin(lua_State *l)
{
	struct fenestra *core = (struct fenestra *)lua_touserdata(l, 1 );

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// show a file requester
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_choose_file(lua_State *l)
{
	struct fenestra *core = (struct fenestra *)lua_touserdata(l, 1 );

#if defined(WIN32)
	return 0;	
#endif

#if defined(X11)

	GtkWindow *gtkWindow;
	const char *title="Choose File to Load";
	GtkFileChooserAction mode=GTK_FILE_CHOOSER_ACTION_OPEN;
	const char *butt=GTK_STOCK_OPEN;
	
	if(lua_isstring(l,3))
	{
		if(strcmp(lua_tostring(l,3),"save")==0)
		{
			mode=GTK_FILE_CHOOSER_ACTION_SAVE;
			title="Choose File to Save";
			butt=GTK_STOCK_SAVE;
		}
	}
			
	gtk_init(&core->argc,&core->argv); // safe to call more than once?

	gtkWindow = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));

	GtkWidget *dialog;
	dialog = gtk_file_chooser_dialog_new (title,
	                                      (gtkWindow),
	                                      mode,
	                                      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
	                                      butt, GTK_RESPONSE_ACCEPT,
	                                      NULL);

	if (lua_isstring(l,2))
	{
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog),lua_tostring(l,2));
	}

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		lua_pushstring(l,filename);
		g_free (filename);
	}
	else
	{
		lua_pushnil(l);
	}

	gtk_widget_destroy(dialog);
	gtk_widget_destroy(GTK_WIDGET(gtkWindow));
	while (gtk_events_pending())
	{
		gtk_main_iteration();    // need to do this to really kill the window
	}

	return 1;
	
#endif

}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static const struct luaL_reg core_lib[] = {
	{"setup",		core_setup},
	{"clean",		core_clean},
	{"msg",			core_msg},
	{"time",		core_time},
	{"setwin",		core_setwin},
	{"getwin",		core_getwin},
	{"choose_file",	core_choose_file},	
	{NULL, NULL},
};

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int luaopen_fenestra_core (lua_State *l) {


	luaL_openlib (l, "fenestra.core", core_lib, 0);

	lua_pushstring(l,"data");
	luaopen_fenestra_core_data(l);
	lua_rawset(l,-3);

	lua_pushstring(l,"ogl");
	luaopen_fenestra_core_ogl(l);
	lua_rawset(l,-3);


	return 1;
}

