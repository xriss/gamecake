/*
    C-midialsa.c - ALSA sequencer bindings for Lua

   This Lua5 module is Copyright (c) 2011, Peter J Billam
                     www.pjb.com.au

 This module is free software; you can redistribute it and/or
       modify it under the same terms as Lua5 itself.
*/

#include <lua.h>
#include <lauxlib.h>
#include <alsa/asoundlib.h>

snd_seq_t *seq_handle;
int queue_id = -1;
int ninputports, noutputports, createqueue;
int firstoutputport, lastoutputport;

static int c_client(lua_State *L) {
	/* Lua stack: client_name, ninputports, noutputports, createqueue */
	size_t len;
	const char *client_name  = lua_tolstring(L, 1, &len);
	lua_Integer ninputports  = lua_tointeger(L, 2);
	lua_Integer noutputports = lua_tointeger(L, 3);
	int createqueue          = lua_toboolean(L, 4);

	int portid, n;
	if (snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_DUPLEX, 0) < 0) {
		fprintf(stderr, "Error creating ALSA client.\n");
		lua_pushboolean(L, 0);
		return 1;
	}
	snd_seq_set_client_name(seq_handle, client_name );

	if ( createqueue )
		queue_id = snd_seq_alloc_queue(seq_handle);
	else
		queue_id = SND_SEQ_QUEUE_DIRECT;

	/*  Clemens Ladisch says (comp.music.midi, 2014041):
	> If you want to allow other clients to send events to the port,
	> set the WRITE flag.
	> If you want to allow other clients to create a subscription to
	> the port, set the WRITE and SUBS_WRITE flags.
	> If you want to allow other clients to create a subscription from
	> the port, set the READ and SUBS_READ flags.
	> (Setting only the READ flag does not make sense because these flags
	> specify what *other* clients are allowed to do.)
	> The DUPLEX flag is purely informational, but you should set it if
	> the port supports both directions.
	*/
	for ( n=0; n < ninputports; n++ ) {
		if (( portid = snd_seq_create_simple_port(seq_handle, "Input port",
				SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
				SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
			fprintf(stderr, "Error creating input port %d.\n", n );
			lua_pushboolean(L, 0);
			return 1;
		}
		if( createqueue ) {
			/* set timestamp info of port  */
			snd_seq_port_info_t *pinfo;
			snd_seq_port_info_alloca(&pinfo);
			snd_seq_get_port_info(seq_handle, portid, pinfo);
			snd_seq_port_info_set_timestamping(pinfo, 1);
			snd_seq_port_info_set_timestamp_queue(pinfo, queue_id);
			snd_seq_port_info_set_timestamp_real(pinfo, 1);
			snd_seq_set_port_info(seq_handle, portid, pinfo);
		}
	}

	for ( n=0; n < noutputports; n++ ) {
		/* 1.20 mark WRITE to allow UNSUBSCRIBE message from System */
		if (( portid = snd_seq_create_simple_port(seq_handle, "Output port",
				SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ
				|SND_SEQ_PORT_CAP_WRITE, SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
			fprintf(stderr, "Error creating output port %d.\n", n );
			lua_pushboolean(L, 0);
			return 1;
		}
	}
	firstoutputport = ninputports;
	lastoutputport  = noutputports + ninputports - 1;
	lua_pushboolean(L, 1);
	return 1;
}

static int c_queue_id(lua_State *L) {
	if (seq_handle == NULL) { return(0); }  /* avoid segfaults */
	lua_pushinteger(L, queue_id);
	return 1;
}

static int c_start(lua_State *L) {
	if (seq_handle == NULL) { return(0); }  /* avoid segfaults */
	int rc = snd_seq_start_queue(seq_handle, queue_id, NULL);
	snd_seq_drain_output(seq_handle);
	lua_pushboolean(L, rc);
	return 1;
}

static int c_stop(lua_State *L) {
	if (seq_handle == NULL) { return(0); }  /* avoid segfaults */
	int rc = snd_seq_stop_queue(seq_handle, queue_id, NULL);
	snd_seq_drain_output(seq_handle);
	lua_pushboolean(L, rc);
	return 1;
}

static int c_status(lua_State *L) {
	if (seq_handle == NULL) { return(0); }  /* avoid segfaults */
	snd_seq_queue_status_t *queue_status;
	int running, events;
	const snd_seq_real_time_t *rt;
	snd_seq_queue_status_malloc( &queue_status );
	snd_seq_get_queue_status( seq_handle, queue_id, queue_status );
	rt      = snd_seq_queue_status_get_real_time( queue_status );
	running = snd_seq_queue_status_get_status( queue_status );
	events  = snd_seq_queue_status_get_events( queue_status );
	/* returns: running, time in floating-point seconds, events */
	lua_pushboolean(L, running);
	lua_pushnumber(L, 1.0*rt->tv_sec + 1.0e-9*rt->tv_nsec);
	lua_pushinteger(L, events);
	snd_seq_queue_status_free( queue_status );
	return 3;
}

static int c_parse_address(lua_State *L) {
	/* 1.11 */
	size_t len;
	const char *port_name  = lua_tolstring(L, 1, &len);
	snd_seq_addr_t *addr;
    addr = alloca(sizeof(snd_seq_addr_t));
    int rc = snd_seq_parse_address(seq_handle, addr, port_name);
    if (rc < 0) {
        /* fprintf(stderr, "Invalid port %s - %s\n", port_name, snd_strerror(rc)); */
        return(0);
    }
    lua_pushinteger(L, addr->client);
    lua_pushinteger(L, addr->port);
	return 2;
}

static int c_connectfrom(lua_State *L) {
	if (seq_handle == NULL) { return(0); }  /* avoid segfaults */
	/* Lua stack: inputport, src_client, src_port */
	lua_Integer myport     = lua_tointeger(L, 1);
	lua_Integer src_client = lua_tointeger(L, 2);
	lua_Integer src_port   = lua_tointeger(L, 3);
	/* Modify dest port if out of bounds 1.01 */
	/* printf ( "firstoutputport=%d\n", firstoutputport ); */
    if (myport >= firstoutputport) myport = firstoutputport - 1;
	int rc = snd_seq_connect_from( seq_handle, myport, src_client, src_port);
	/* returns 0 on success, or a negative error code */
	/* http://alsa-project.org/alsa-doc/alsa-lib/seq.html */
	lua_pushboolean(L, rc==0);
	return 1;
}

static int c_connectto(lua_State *L) {
	if (seq_handle == NULL) { return(0); }  /* avoid segfaults */
	/* Lua stack: outputport, dest_client, dest_port */
	lua_Integer myport      = lua_tointeger(L, 1);
	lua_Integer dest_client = lua_tointeger(L, 2);
	lua_Integer dest_port   = lua_tointeger(L, 3);
	/* Modify source port if out of bounds 1.01 */
    if ( myport < firstoutputport ) myport= firstoutputport;
    else if ( myport > lastoutputport ) myport = lastoutputport;
	int rc = snd_seq_connect_to( seq_handle, myport, dest_client, dest_port);
	/* returns 0 on success, or a negative error code */
	/* http://alsa-project.org/alsa-doc/alsa-lib/seq.html */
	lua_pushboolean(L, rc==0);
	return 1;
}

static int c_disconnectfrom(lua_State *L) {
	if (seq_handle == NULL) { return(0); }  /* avoid segfaults */
	/* Lua stack: inputport, src_client, src_port */
	lua_Integer myport     = lua_tointeger(L, 1);
	lua_Integer src_client = lua_tointeger(L, 2);
	lua_Integer src_port   = lua_tointeger(L, 3);
	/* Modify dest port if out of bounds 1.01 */
	/* printf ( "firstoutputport=%d\n", firstoutputport ); */
    if (myport >= firstoutputport) myport = firstoutputport - 1;
	int rc = snd_seq_disconnect_from(seq_handle,myport,src_client,src_port);
	/* returns 0 on success, or a negative error code ? */
	/* http://alsa-project.org/alsa-doc/alsa-lib/seq.html */
	lua_pushboolean(L, rc==0);
	return 1;
}

static int c_disconnectto(lua_State *L) {
	if (seq_handle == NULL) { return(0); }  /* avoid segfaults */
	/* Lua stack: outputport, dest_client, dest_port */
	lua_Integer myport      = lua_tointeger(L, 1);
	lua_Integer dest_client = lua_tointeger(L, 2);
	lua_Integer dest_port   = lua_tointeger(L, 3);
	/* Modify source port if out of bounds 1.01 */
    if ( myport < firstoutputport ) myport= firstoutputport;
    else if ( myport > lastoutputport ) myport = lastoutputport;
	int rc = snd_seq_disconnect_to( seq_handle,myport,dest_client,dest_port);
	/* returns 0 on success, or a negative error code ? */
	/* http://alsa-project.org/alsa-doc/alsa-lib/seq.html */
	lua_pushboolean(L, rc==0);
	return 1;
}

static int c_fd(lua_State *L) {
	if (seq_handle == NULL) { return(0); }  /* avoid segfaults */
	int npfd;
	struct pollfd *pfd;
	npfd = snd_seq_poll_descriptors_count(seq_handle, POLLIN);
	pfd = (struct pollfd *)alloca(npfd * sizeof(struct pollfd));
	snd_seq_poll_descriptors(seq_handle, pfd, npfd, POLLIN);
	lua_pushinteger(L, pfd->fd);
	return 1;
}

static int c_id(lua_State *L) {
	if (seq_handle == NULL) { return(0); }  /* avoid segfaults */
	lua_pushinteger(L, snd_seq_client_id( seq_handle ));
	return 1;
}

static int c_input(lua_State *L) {
	if (seq_handle == NULL) { return(0); }  /* avoid segfaults */
	snd_seq_event_t *ev;
	int err;
	err = snd_seq_event_input( seq_handle, &ev );
	if (err < 0) { return(0); }  /* 1.04 survive SIGINT */
	/* returns: (type, flags, tag, queue, time, src_client, src_port,
	   dest_client, dest_port, data...)
	   We flatten out the list here so as not to have to use userdata
	   and we use one Time in secs, rather than separate secs and nsecs
	*/
	lua_pushinteger(L, ev->type);
	lua_pushinteger(L, ev->flags);
	lua_pushinteger(L, ev->tag);
	lua_pushinteger(L, ev->queue);
	lua_pushnumber( L, ev->time.time.tv_sec + 1.0e-9 * ev->time.time.tv_nsec);
	lua_pushinteger(L, ev->source.client);
	lua_pushinteger(L, ev->source.port);
	lua_pushinteger(L, ev->dest.client);
	lua_pushinteger(L, ev->dest.port);

	switch( ev->type ) {
		case SND_SEQ_EVENT_NOTE:
		case SND_SEQ_EVENT_NOTEON:
		case SND_SEQ_EVENT_NOTEOFF:
		case SND_SEQ_EVENT_KEYPRESS:
			lua_pushinteger(L, ev->data.note.channel);
			lua_pushinteger(L, ev->data.note.note);
			lua_pushinteger(L, ev->data.note.velocity);
			lua_pushinteger(L, ev->data.note.off_velocity);
			lua_pushinteger(L, ev->data.note.duration);
			return 14;
			break;

		case SND_SEQ_EVENT_CONTROLLER:
		case SND_SEQ_EVENT_PGMCHANGE:
		case SND_SEQ_EVENT_CHANPRESS:
		case SND_SEQ_EVENT_PITCHBEND:
			lua_pushinteger(L, ev->data.control.channel);
			lua_pushinteger(L, ev->data.control.unused[0]);
			lua_pushinteger(L, ev->data.control.unused[1]);
			lua_pushinteger(L, ev->data.control.unused[2]);
			lua_pushinteger(L, ev->data.control.param);
			lua_pushinteger(L, ev->data.control.value);
			return 15;
            break;

		case SND_SEQ_EVENT_SYSEX:
			lua_pushstring(L, ev->data.ext.ptr);  /* length? */
			return 10;
			break;

		default:
			/* lua_pushinteger(L, ev->data.note.channel);
			lua_pushinteger(L, ev->data.note.note);
			lua_pushinteger(L, ev->data.note.velocity);
			lua_pushinteger(L, ev->data.note.off_velocity);
			lua_pushinteger(L, ev->data.note.duration); */
			return 9;
			break;
	}
}

static int c_inputpending(lua_State *L) {
	if (seq_handle == NULL) { return(0); }
	if (queue_id < 0) { lua_pushinteger(L,0) ; return 1; }
	lua_pushinteger(L, snd_seq_event_input_pending(seq_handle, 1));
	return 1;
}

static int c_output(lua_State *L) {
	if (seq_handle == NULL) { return(0); }
	/* Lua stack: type, flags, tag, queue, time (float, in secs),
	 src_client, src_port, dest_client, dest_port, data... */
	snd_seq_event_t ev;
	ev.type          = lua_tointeger(L, 1);
	ev.flags         = lua_tointeger(L, 2) | SND_SEQ_TIME_STAMP_REAL; /*1.15*/
	ev.tag           = lua_tointeger(L, 3);
	ev.queue         = lua_tointeger(L, 4);
	lua_Number t     = lua_tonumber( L, 5);  /* not just double! 1.01 */
	ev.time.time.tv_sec  = (int) t;
	ev.time.time.tv_nsec = (int) (1.0e9 * (t - (double) ev.time.time.tv_sec));
	/* printf ( "c_output: t=%g\n", t); */
	ev.source.client = lua_tointeger(L, 6);
	ev.source.port   = lua_tointeger(L, 7);
	ev.dest.client   = lua_tointeger(L, 8);
	ev.dest.port     = lua_tointeger(L, 9);
	static int * data;
	char * sysex_data;
	switch( ev.type ) {
		case SND_SEQ_EVENT_NOTE:
		case SND_SEQ_EVENT_NOTEON:
		case SND_SEQ_EVENT_NOTEOFF:
		case SND_SEQ_EVENT_KEYPRESS:
			ev.data.note.channel      = lua_tointeger(L, 10);
			ev.data.note.note         = lua_tointeger(L, 11);
			ev.data.note.velocity     = lua_tointeger(L, 12);
			ev.data.note.off_velocity = lua_tointeger(L, 13);
			ev.data.note.duration     = lua_tointeger(L, 14);
			break;

		case SND_SEQ_EVENT_CONTROLLER:
		case SND_SEQ_EVENT_PGMCHANGE:
		case SND_SEQ_EVENT_CHANPRESS:
		case SND_SEQ_EVENT_PITCHBEND:
			ev.data.control.channel   = lua_tointeger(L, 10);
			ev.data.control.unused[0] = lua_tointeger(L, 11);
			ev.data.control.unused[1] = lua_tointeger(L, 12);
			ev.data.control.unused[2] = lua_tointeger(L, 13);
			ev.data.control.param     = lua_tointeger(L, 14);
			ev.data.control.value     = lua_tointeger(L, 15);
			break;

		case SND_SEQ_EVENT_SYSEX:
			sysex_data = (char *) luaL_checkstring(L, 16);
			snd_seq_ev_set_variable( &ev, lua_tointeger(L, 10), sysex_data);
            break;
	}
	/* If not a direct event, use the queue */
	if ( ev.queue != SND_SEQ_QUEUE_DIRECT )
		ev.queue = queue_id;
	/* Modify source port if out of bounds */
	if ( ev.source.port < firstoutputport ) 
		snd_seq_ev_set_source(&ev, firstoutputport );
	else if ( ev.source.port > lastoutputport )
		snd_seq_ev_set_source(&ev, lastoutputport );
	/* Use subscribed ports, except if ECHO event, or dest_client>0 1.12 */
	if (ev.type!=SND_SEQ_EVENT_ECHO && ( !ev.dest.client
	  || ev.dest.client == snd_seq_client_id( seq_handle)))  /* 1.14 */
		snd_seq_ev_set_subs(&ev);
	int rc = snd_seq_event_output_direct( seq_handle, &ev );
	lua_pushinteger(L, rc);
	return 1;
}

static int c_listclients(lua_State *L) {
	/* stuff for version 1.03 - see aconnect.c
	alsa-utils.sourcearchive.com/documentation/1.0.20/aconnect_8c-source.html 
	*/
	if (seq_handle == NULL) { return(0); }
	lua_Integer getnumports = lua_tointeger(L, 1);
	snd_seq_client_info_t *cinfo;
	snd_seq_port_info_t *pinfo;
	snd_seq_client_info_alloca(&cinfo);
	snd_seq_port_info_alloca(&pinfo);
	snd_seq_client_info_set_client(cinfo, -1);
	unsigned int iST = 0;
	while (snd_seq_query_next_client(seq_handle, cinfo) >= 0) {
		/* reset query info */
		snd_seq_port_info_set_client(pinfo,
		  snd_seq_client_info_get_client(cinfo));
		snd_seq_port_info_set_port(pinfo, -1);
		lua_pushinteger(L, snd_seq_client_info_get_client(cinfo));
		iST++;
		if (getnumports) {
	 		lua_pushinteger(L, snd_seq_client_info_get_num_ports(cinfo));
		} else {
			lua_pushstring(L, snd_seq_client_info_get_name(cinfo));
		}
		iST++;
	}
	return iST;
}

static int c_listconnections (lua_State *L) {
	/* stuff for version 1.03 - see aconnect.c
	alsa-utils.sourcearchive.com/documentation/1.0.20/aconnect_8c-source.html 
	*/
	if (seq_handle == NULL) { return(0); }
	lua_Integer from      = lua_tointeger(L, 1);
	snd_seq_client_info_t *cinfo;
	snd_seq_port_info_t *pinfo;
	snd_seq_query_subscribe_t *subs;
	snd_seq_client_info_alloca(&cinfo);
	snd_seq_port_info_alloca(&pinfo);
	snd_seq_query_subscribe_alloca(&subs);
	snd_seq_get_client_info(seq_handle, cinfo);
	unsigned int iST = 0;
	/* reset query info */
	snd_seq_query_subscribe_set_type(subs,
	  from ? SND_SEQ_QUERY_SUBS_WRITE : SND_SEQ_QUERY_SUBS_READ);
	snd_seq_port_info_set_client(pinfo,
	  snd_seq_client_info_get_client(cinfo));
	snd_seq_port_info_set_port(pinfo, -1);
	while (snd_seq_query_next_port(seq_handle, pinfo) >= 0) {
		snd_seq_query_subscribe_set_root(subs,
		  snd_seq_port_info_get_addr(pinfo));
		snd_seq_query_subscribe_set_port(subs,
		  snd_seq_port_info_get_addr(pinfo)->port);
		snd_seq_query_subscribe_set_index(subs, 0);
		/* At least, the client id, the port id, the index number
		 and the query type must be set to perform a proper query. */
		while (snd_seq_query_port_subscribers(seq_handle, subs) >= 0) {
			const snd_seq_addr_t *addr;
			addr = snd_seq_query_subscribe_get_addr(subs);
			lua_pushinteger(L, snd_seq_port_info_get_addr(pinfo)->port);
			iST++;
			lua_pushinteger(L, addr->client);
			iST++;
			lua_pushinteger(L, addr->port);
			iST++;
			snd_seq_query_subscribe_set_index(subs,
			  snd_seq_query_subscribe_get_index(subs) + 1);
		}
	}
	return iST;
}


static int c_syncoutput(lua_State *L) {
	if (seq_handle == NULL) { return(0); }
	int rc = snd_seq_sync_output_queue( seq_handle );
	lua_pushinteger(L, rc);
	return 1;
}

struct constant {  /* Gems p. 334 */
	const char * name;
	int value;
};
static const struct constant constants[] = {
	{"SND_SEQ_EVENT_BOUNCE", SND_SEQ_EVENT_BOUNCE},
	{"SND_SEQ_EVENT_CHANPRESS", SND_SEQ_EVENT_CHANPRESS},
	{"SND_SEQ_EVENT_CLIENT_CHANGE", SND_SEQ_EVENT_CLIENT_CHANGE},
	{"SND_SEQ_EVENT_CLIENT_EXIT", SND_SEQ_EVENT_CLIENT_EXIT},
	{"SND_SEQ_EVENT_CLIENT_START", SND_SEQ_EVENT_CLIENT_START},
	{"SND_SEQ_EVENT_CLOCK", SND_SEQ_EVENT_CLOCK},
	{"SND_SEQ_EVENT_CONTINUE", SND_SEQ_EVENT_CONTINUE},
	{"SND_SEQ_EVENT_CONTROL14", SND_SEQ_EVENT_CONTROL14},
	{"SND_SEQ_EVENT_CONTROLLER", SND_SEQ_EVENT_CONTROLLER},
	{"SND_SEQ_EVENT_ECHO", SND_SEQ_EVENT_ECHO},
	{"SND_SEQ_EVENT_KEYPRESS", SND_SEQ_EVENT_KEYPRESS},
	{"SND_SEQ_EVENT_KEYSIGN", SND_SEQ_EVENT_KEYSIGN},
	{"SND_SEQ_EVENT_NONE", SND_SEQ_EVENT_NONE},
	{"SND_SEQ_EVENT_NONREGPARAM", SND_SEQ_EVENT_NONREGPARAM},
	{"SND_SEQ_EVENT_NOTE", SND_SEQ_EVENT_NOTE},
	{"SND_SEQ_EVENT_NOTEOFF", SND_SEQ_EVENT_NOTEOFF},
	{"SND_SEQ_EVENT_NOTEON", SND_SEQ_EVENT_NOTEON},
	{"SND_SEQ_EVENT_OSS", SND_SEQ_EVENT_OSS},
	{"SND_SEQ_EVENT_PGMCHANGE", SND_SEQ_EVENT_PGMCHANGE},
	{"SND_SEQ_EVENT_PITCHBEND", SND_SEQ_EVENT_PITCHBEND},
	{"SND_SEQ_EVENT_PORT_CHANGE", SND_SEQ_EVENT_PORT_CHANGE},
	{"SND_SEQ_EVENT_PORT_EXIT", SND_SEQ_EVENT_PORT_EXIT},
	{"SND_SEQ_EVENT_PORT_START", SND_SEQ_EVENT_PORT_START},
	{"SND_SEQ_EVENT_PORT_SUBSCRIBED", SND_SEQ_EVENT_PORT_SUBSCRIBED},
	{"SND_SEQ_EVENT_PORT_UNSUBSCRIBED", SND_SEQ_EVENT_PORT_UNSUBSCRIBED},
	{"SND_SEQ_EVENT_QFRAME", SND_SEQ_EVENT_QFRAME},
	{"SND_SEQ_EVENT_QUEUE_SKEW", SND_SEQ_EVENT_QUEUE_SKEW},
	{"SND_SEQ_EVENT_REGPARAM", SND_SEQ_EVENT_REGPARAM},
	{"SND_SEQ_EVENT_RESET", SND_SEQ_EVENT_RESET},
	{"SND_SEQ_EVENT_RESULT", SND_SEQ_EVENT_RESULT},
	{"SND_SEQ_EVENT_SENSING", SND_SEQ_EVENT_SENSING},
	{"SND_SEQ_EVENT_SETPOS_TICK", SND_SEQ_EVENT_SETPOS_TICK},
	{"SND_SEQ_EVENT_SETPOS_TIME", SND_SEQ_EVENT_SETPOS_TIME},
	{"SND_SEQ_EVENT_SONGPOS", SND_SEQ_EVENT_SONGPOS},
	{"SND_SEQ_EVENT_SONGSEL", SND_SEQ_EVENT_SONGSEL},
	{"SND_SEQ_EVENT_START", SND_SEQ_EVENT_START},
	{"SND_SEQ_EVENT_STOP", SND_SEQ_EVENT_STOP},
	{"SND_SEQ_EVENT_SYNC_POS", SND_SEQ_EVENT_SYNC_POS},
	{"SND_SEQ_EVENT_SYSEX", SND_SEQ_EVENT_SYSEX},
	{"SND_SEQ_EVENT_SYSTEM", SND_SEQ_EVENT_SYSTEM},
	{"SND_SEQ_EVENT_TEMPO", SND_SEQ_EVENT_TEMPO},
	{"SND_SEQ_EVENT_TICK", SND_SEQ_EVENT_TICK},
	{"SND_SEQ_EVENT_TIMESIGN", SND_SEQ_EVENT_TIMESIGN},
	{"SND_SEQ_EVENT_TUNE_REQUEST", SND_SEQ_EVENT_TUNE_REQUEST},
	{"SND_SEQ_EVENT_USR0", SND_SEQ_EVENT_USR0},
	{"SND_SEQ_EVENT_USR1", SND_SEQ_EVENT_USR1},
	{"SND_SEQ_EVENT_USR2", SND_SEQ_EVENT_USR2},
	{"SND_SEQ_EVENT_USR3", SND_SEQ_EVENT_USR3},
	{"SND_SEQ_EVENT_USR4", SND_SEQ_EVENT_USR4},
	{"SND_SEQ_EVENT_USR5", SND_SEQ_EVENT_USR5},
	{"SND_SEQ_EVENT_USR6", SND_SEQ_EVENT_USR6},
	{"SND_SEQ_EVENT_USR7", SND_SEQ_EVENT_USR7},
	{"SND_SEQ_EVENT_USR8", SND_SEQ_EVENT_USR8},
	{"SND_SEQ_EVENT_USR9", SND_SEQ_EVENT_USR9},
	{"SND_SEQ_EVENT_USR_VAR0", SND_SEQ_EVENT_USR_VAR0},
	{"SND_SEQ_EVENT_USR_VAR1", SND_SEQ_EVENT_USR_VAR1},
	{"SND_SEQ_EVENT_USR_VAR2", SND_SEQ_EVENT_USR_VAR2},
	{"SND_SEQ_EVENT_USR_VAR3", SND_SEQ_EVENT_USR_VAR3},
	{"SND_SEQ_EVENT_USR_VAR4", SND_SEQ_EVENT_USR_VAR4},
	{"SND_SEQ_QUEUE_DIRECT", SND_SEQ_QUEUE_DIRECT},
	{"SND_SEQ_TIME_STAMP_REAL", SND_SEQ_TIME_STAMP_REAL},
	{NULL, 0}
};

static const luaL_Reg prv[] = {  /* private functions */
	{"client",          c_client},
	{"connectfrom",     c_connectfrom},
	{"connectto",       c_connectto},
	{"disconnectfrom",  c_disconnectfrom},
	{"disconnectto",    c_disconnectto},
	{"listclients",     c_listclients},
	{"listconnections", c_listconnections},
	{"parse_address",   c_parse_address},
	{"queue_id",        c_queue_id},
	{"start",           c_start},
	{"status",          c_status},
	{"stop",            c_stop},
	{"fd",              c_fd},
	{"id",              c_id},
	{"input",           c_input},
	{"inputpending",    c_inputpending},
	{"output",          c_output},
	{"syncoutput",      c_syncoutput},
	{NULL, NULL}
};

static int initialise(lua_State *L) {  /* Lua Programming Gems p. 335 */
	/* Lua stack: aux table, prv table, dat table */
	int index;  /* define constants in module namespace */
	for (index = 0; constants[index].name != NULL; ++index) {
		lua_pushinteger(L, constants[index].value);
		lua_setfield(L, 3, constants[index].name);
	}
	/* lua_pushvalue(L, 1);   * set the aux table as environment */
	/* lua_replace(L, LUA_ENVIRONINDEX);
	   unnecessary here, fortunately, because it fails in 5.2 */
	lua_pushvalue(L, 2); /* register the private functions */
#if LUA_VERSION_NUM >= 502
	luaL_setfuncs(L, prv, 0);    /* 5.2 */
	return 0;
#else
	luaL_register(L, NULL, prv); /* 5.1 */
	return 0;
#endif
}

int luaopen_midialsa_core(lua_State *L) {
	lua_pushcfunction(L, initialise);
	return 1;
}
