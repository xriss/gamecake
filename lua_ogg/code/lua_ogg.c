/*
-- Copyright (C) 2012 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
*/

#include "all.h"



//
// we can use either this string as a string identifier
// or its address as a light userdata identifier, both unique
//
const char *lua_dogg_meta_name="dogg*ptr";



static int lua_ogg_test (lua_State *l)
{
ogg_int16_t convbuffer[4096]; /* take 8k out of the data segment, not the stack */
int convsize=4096;

  ogg_sync_state   oy; /* sync and verify incoming physical bitstream */
  ogg_stream_state os; /* take physical pages, weld into a logical
                          stream of packets */
  ogg_page         og; /* one Ogg bitstream page. Vorbis packets are inside */
  ogg_packet       op; /* one raw packet of data for decode */

  vorbis_info      vi; /* struct that stores all the static vorbis bitstream
                          settings */
  vorbis_comment   vc; /* struct that stores all the bitstream user comments */
  vorbis_dsp_state vd; /* central working state for the packet->PCM decoder */
  vorbis_block     vb; /* local working space for packet->PCM decode */

  char *buffer;
  int  bytes;

  /********** Decode setup ************/

  ogg_sync_init(&oy); /* Now we can read pages */
  
  while(1){ /* we repeat if the bitstream is chained */
    int eos=0;
    int i;

    /* grab some data at the head of the stream. We want the first page
       (which is guaranteed to be small and only contain the Vorbis
       stream initial header) We need the first page to get the stream
       serialno. */

    /* submit a 4k block to libvorbis' Ogg layer */
    buffer=ogg_sync_buffer(&oy,4096);
    bytes=fread(buffer,1,4096,stdin);
    ogg_sync_wrote(&oy,bytes);
    
    /* Get the first page. */
    if(ogg_sync_pageout(&oy,&og)!=1){
      /* have we simply run out of data?  If so, we're done. */
      if(bytes<4096)break;
      
      /* error case.  Must not be Vorbis data */
      fprintf(stderr,"Input does not appear to be an Ogg bitstream.\n");
      exit(1);
    }
  
    /* Get the serial number and set up the rest of decode. */
    /* serialno first; use it to set up a logical stream */
    ogg_stream_init(&os,ogg_page_serialno(&og));
    
    /* extract the initial header from the first page and verify that the
       Ogg bitstream is in fact Vorbis data */
    
    /* I handle the initial header first instead of just having the code
       read all three Vorbis headers at once because reading the initial
       header is an easy way to identify a Vorbis bitstream and it's
       useful to see that functionality seperated out. */
    
    vorbis_info_init(&vi);
    vorbis_comment_init(&vc);
    if(ogg_stream_pagein(&os,&og)<0){ 
      /* error; stream version mismatch perhaps */
      fprintf(stderr,"Error reading first page of Ogg bitstream data.\n");
      exit(1);
    }
    
    if(ogg_stream_packetout(&os,&op)!=1){ 
      /* no page? must not be vorbis */
      fprintf(stderr,"Error reading initial header packet.\n");
      exit(1);
    }
    
    if(vorbis_synthesis_headerin(&vi,&vc,&op)<0){ 
      /* error case; not a vorbis header */
      fprintf(stderr,"This Ogg bitstream does not contain Vorbis "
              "audio data.\n");
      exit(1);
    }
    
    /* At this point, we're sure we're Vorbis. We've set up the logical
       (Ogg) bitstream decoder. Get the comment and codebook headers and
       set up the Vorbis decoder */
    
    /* The next two packets in order are the comment and codebook headers.
       They're likely large and may span multiple pages. Thus we read
       and submit data until we get our two packets, watching that no
       pages are missing. If a page is missing, error out; losing a
       header page is the only place where missing data is fatal. */
    
    i=0;
    while(i<2){
      while(i<2){
        int result=ogg_sync_pageout(&oy,&og);
        if(result==0)break; /* Need more data */
        /* Don't complain about missing or corrupt data yet. We'll
           catch it at the packet output phase */
        if(result==1){
          ogg_stream_pagein(&os,&og); /* we can ignore any errors here
                                         as they'll also become apparent
                                         at packetout */
          while(i<2){
            result=ogg_stream_packetout(&os,&op);
            if(result==0)break;
            if(result<0){
              /* Uh oh; data at some point was corrupted or missing!
                 We can't tolerate that in a header.  Die. */
              fprintf(stderr,"Corrupt secondary header.  Exiting.\n");
              exit(1);
            }
            result=vorbis_synthesis_headerin(&vi,&vc,&op);
            if(result<0){
              fprintf(stderr,"Corrupt secondary header.  Exiting.\n");
              exit(1);
            }
            i++;
          }
        }
      }
      /* no harm in not checking before adding more */
      buffer=ogg_sync_buffer(&oy,4096);
      bytes=fread(buffer,1,4096,stdin);
      if(bytes==0 && i<2){
        fprintf(stderr,"End of file before finding all Vorbis headers!\n");
        exit(1);
      }
      ogg_sync_wrote(&oy,bytes);
    }
    
    /* Throw the comments plus a few lines about the bitstream we're
       decoding */
    {
      char **ptr=vc.user_comments;
      while(*ptr){
        fprintf(stderr,"%s\n",*ptr);
        ++ptr;
      }
      fprintf(stderr,"\nBitstream is %d channel, %ldHz\n",vi.channels,vi.rate);
      fprintf(stderr,"Encoded by: %s\n\n",vc.vendor);
    }
    
    convsize=4096/vi.channels;

    /* OK, got and parsed all three headers. Initialize the Vorbis
       packet->PCM decoder. */
    if(vorbis_synthesis_init(&vd,&vi)==0){ /* central decode state */
      vorbis_block_init(&vd,&vb);          /* local state for most of the decode
                                              so multiple block decodes can
                                              proceed in parallel. We could init
                                              multiple vorbis_block structures
                                              for vd here */
      
      /* The rest is just a straight decode loop until end of stream */
      while(!eos){
        while(!eos){
          int result=ogg_sync_pageout(&oy,&og);
          if(result==0)break; /* need more data */
          if(result<0){ /* missing or corrupt data at this page position */
            fprintf(stderr,"Corrupt or missing data in bitstream; "
                    "continuing...\n");
          }else{
            ogg_stream_pagein(&os,&og); /* can safely ignore errors at
                                           this point */
            while(1){
              result=ogg_stream_packetout(&os,&op);
              
              if(result==0)break; /* need more data */
              if(result<0){ /* missing or corrupt data at this page position */
                /* no reason to complain; already complained above */
              }else{
                /* we have a packet.  Decode it */
                float **pcm;
                int samples;
                
                if(vorbis_synthesis(&vb,&op)==0) /* test for success! */
                  vorbis_synthesis_blockin(&vd,&vb);
                /* 
                   
                **pcm is a multichannel float vector.  In stereo, for
                example, pcm[0] is left, and pcm[1] is right.  samples is
                the size of each channel.  Convert the float values
                (-1.<=range<=1.) to whatever PCM format and write it out */
                
                while((samples=vorbis_synthesis_pcmout(&vd,&pcm))>0){
                  int j;
                  int clipflag=0;
                  int bout=(samples<convsize?samples:convsize);
                  
                  /* convert floats to 16 bit signed ints (host order) and
                     interleave */
                  for(i=0;i<vi.channels;i++){
                    ogg_int16_t *ptr=convbuffer+i;
                    float  *mono=pcm[i];
                    for(j=0;j<bout;j++){
#if 1
                      int val=floor(mono[j]*32767.f+.5f);
#else /* optional dither */
                      int val=mono[j]*32767.f+drand48()-0.5f;
#endif
                      /* might as well guard against clipping */
                      if(val>32767){
                        val=32767;
                        clipflag=1;
                      }
                      if(val<-32768){
                        val=-32768;
                        clipflag=1;
                      }
                      *ptr=val;
                      ptr+=vi.channels;
                    }
                  }
                  
                  if(clipflag)
                    fprintf(stderr,"Clipping in frame %ld\n",(long)(vd.sequence));
                  
                  
                  fwrite(convbuffer,2*vi.channels,bout,stdout);
                  
                  vorbis_synthesis_read(&vd,bout); /* tell libvorbis how
                                                      many samples we
                                                      actually consumed */
                }            
              }
            }
            if(ogg_page_eos(&og))eos=1;
          }
        }
        if(!eos){
          buffer=ogg_sync_buffer(&oy,4096);
          bytes=fread(buffer,1,4096,stdin);
          ogg_sync_wrote(&oy,bytes);
          if(bytes==0)eos=1;
        }
      }
      
      /* ogg_page and ogg_packet structs always point to storage in
         libvorbis.  They're never freed or manipulated directly */
      
      vorbis_block_clear(&vb);
      vorbis_dsp_clear(&vd);
    }else{
      fprintf(stderr,"Error: Corrupt header during playback initialization.\n");
    }

    /* clean up this logical bitstream; before exit we see if we're
       followed by another [chained] */
    
    ogg_stream_clear(&os);
    vorbis_comment_clear(&vc);
    vorbis_info_clear(&vi);  /* must be called last */
  }

  /* OK, clean up the framer */
  ogg_sync_clear(&oy);
  
  fprintf(stderr,"Done.\n");
  return(0);
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// test
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_ogg_test2 (lua_State *l)
{
	printf("test ogg");

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// create
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_dogg_create(lua_State *l)
{
	dogg **dd;

// create a dogg userdata pointer pointer
	dd = (dogg**)lua_newuserdata(l, sizeof(dogg*));
	(*dd)=0;
	luaL_getmetatable(l, lua_dogg_meta_name);
	lua_setmetatable(l, -2);

//open the actual ogg decoder
	(*dd)=(dogg*)calloc(sizeof(dogg),1);
	if(!(*dd)) { return 0; } // error failed to alloc

//return the userdata	
	return 1;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get **ptr and error if it is not the right udata
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
dogg ** lua_dogg_ptr (lua_State *l,int idx)
{
dogg **dd;
	dd = (dogg**)luaL_checkudata(l, idx , lua_dogg_meta_name);
	return dd;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get *ptr and error if it is 0
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
dogg * lua_dogg_check (lua_State *l,int idx)
{	
dogg **dd;
	dd = lua_dogg_ptr (l,idx);
	if(!*dd)
	{
		luaL_error(l,"dogg is null");
	}
	return *dd;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open and close a dogg
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static void lua_dogg_open_ptr(dogg *dd)
{
	memset(dd,0,sizeof(dogg)); // reset everything to 0
	ogg_sync_init(&dd->oy); /* Now we can read pages */
	dd->done_open=1;
}
static void lua_dogg_close_ptr(dogg *dd)
{
	if(dd->done_open)
	{
		ogg_stream_clear(&dd->os);
		vorbis_comment_clear(&dd->vc);
		vorbis_info_clear(&dd->vi);  /* must be called last */
		/* OK, clean up the framer */
		ogg_sync_clear(&dd->oy);
		dd->done_open=0;
	}
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// __GC for ptr (may be null)
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_dogg_destroy (lua_State *l)
{	
dogg **dd;

	dd = lua_dogg_ptr(l, 1 );
	
	if(*dd)
	{
		lua_dogg_close_ptr(*dd);
		free(*dd);
		(*dd)=0;
	}
	
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// fill a table with info
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_dogg_info (lua_State *l)
{	
dogg *dd;
const char *s;

	dd = lua_dogg_check(l, 1 );
	if(lua_istable(l,2)) // reuse
	{
		lua_pushvalue(l,2);
	}
	else // new
	{
		lua_newtable(l);
	}
	
//	lua_pushstring(l,"fmt");			lua_pushnumber(l,sd->fmt);			lua_settable(l,-3);	
	if(dd->err)
	{
		lua_pushstring(l,"err"); lua_pushstring(l,dd->err); lua_settable(l,-3);
	}
	else
	{
		lua_pushstring(l,"err"); lua_pushnil(l); lua_settable(l,-3);
	}

	return 1;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open stream processing
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_dogg_open (lua_State *l)
{	
dogg *dd;
	dd = lua_dogg_check(l, 1 );
	lua_dogg_open_ptr(dd);
	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// close stream processing
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_dogg_close (lua_State *l)
{	
dogg *dd;
	dd = lua_dogg_check(l, 1 );
	lua_dogg_close_ptr(dd);
	return 0;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// push ogg into the engine
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_dogg_push (lua_State *l)
{	
dogg *dd;
const char *s;
int slen;

	dd = lua_dogg_check(l, 1 );
	s = lua_tolstring(l, 2 ,&slen);

	dd->buffer=ogg_sync_buffer(&dd->oy,slen);
	dd->bytes=slen;
	memcpy(dd->buffer,s,slen);
	ogg_sync_wrote(&dd->oy,dd->bytes);
	
	dd->err=0;

	return 0;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// pull wav out of the engine
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_dogg_pull(lua_State *l)
{	
int result;
dogg *dd;
	dd = lua_dogg_check(l, 1 );
	
	while(1)
	{
		if(!dd->done_stream_init)
		{
			result=ogg_sync_pageout(&dd->oy,&dd->og);
			if(result<0)
			{
				dd->err="Corrupt or missing data in bitstream.";
				return 0;
			}
			if(result==0)
			{
				dd->err="push"; /* request a push of more data */
				return 0;
			}
		}

		if(!dd->done_init) // need to setup headers
		{
			/* Get the serial number and set up the rest of decode. */
			/* serialno first; use it to set up a logical stream */
			ogg_stream_init(&dd->os,ogg_page_serialno(&dd->og));

			/* extract the initial header from the first page and verify that the
			Ogg bitstream is in fact Vorbis data */

			/* I handle the initial header first instead of just having the code
			read all three Vorbis headers at once because reading the initial
			header is an easy way to identify a Vorbis bitstream and it's
			useful to see that functionality seperated out. */

			vorbis_info_init(&dd->vi);
			vorbis_comment_init(&dd->vc);
			if(ogg_stream_pagein(&dd->os,&dd->og)<0)
			{
				/* error; stream version mismatch perhaps */
				dd->err="Error reading first page of Ogg bitstream data.";
				return 0;
			}

			if(ogg_stream_packetout(&dd->os,&dd->op)!=1)
			{ 
				/* no page? must not be vorbis */
				dd->err="Error reading initial header packet.";
				return 0;
			}

			if(vorbis_synthesis_headerin(&dd->vi,&dd->vc,&dd->op)<0)
			{ 
				/* error case; not a vorbis header */
				dd->err="This Ogg bitstream does not contain Vorbis audio data.";
				return 0;
			}
			
			dd->done_init=1;
			continue; // get more data, maybe
		}
		
		while(dd->result_head<2)
		{
			ogg_stream_pagein(&dd->os,&dd->og);
			while(dd->result_head<2)
			{
				result=ogg_stream_packetout(&dd->os,&dd->op);
				if(result==0)
				{
					dd->err="push"; /* request a push of more data */
					return 0;
				}
				if(result<0)
				{
					dd->err="Corrupt secondary 1header.";
					return 0;
				}
				
				result=vorbis_synthesis_headerin(&dd->vi,&dd->vc,&dd->op);
				if(result<0)
				{
					dd->err="Corrupt secondary 2header.";
					return 0;
				}
				
				dd->result_head++;
				continue; // get more data, maybe
			}	
		}
		
		if(!dd->done_synthesis_init)
		{
			/* Throw the comments plus a few lines about the bitstream we're
			   decoding */
			char **ptr=dd->vc.user_comments;
			while(*ptr)
			{
				printf("%s\n",*ptr);
				++ptr;
			}
			printf("\nBitstream is %d channel, %ldHz\n",dd->vi.channels,dd->vi.rate);
			printf("Encoded by: %s\n\n",dd->vc.vendor);
			
			dd->convsize=4096/dd->vi.channels;

			/* OK, got and parsed all three headers. Initialize the Vorbis
			   packet->PCM decoder. */
			if(vorbis_synthesis_init(&dd->vd,&dd->vi)!=0)
			{
				  dd->err="Corrupt header during playback initialization.";
				  return 0;
			}
			
			dd->done_synthesis_init=1;
			continue; // get more data, maybe
		}

		if(!dd->done_block_init)
		{
printf("vorbis_block_init\n");
			vorbis_block_init(&dd->vd,&dd->vb);          /* local state for most of the decode
											  so multiple block decodes can
											  proceed in parallel. We could init
											  multiple vorbis_block structures
											  for vd here */
			dd->done_block_init=1;
		}

/* The rest is a decode loop until end of stream */

		if(!dd->done_stream_init)
		{
printf("ogg_stream_pagein\n");
			ogg_stream_pagein(&dd->os,&dd->og);
			dd->done_stream_init=1;
		}

while(1){
		result=ogg_stream_packetout(&dd->os,&dd->op);	  
		if(result==0)
		{
			dd->done_stream_init=0;
			dd->err="push"; /* request a push of more data */
			return 0;
		}
		if(result>0)
		{
			/* we have a packet.  Decode it */
			float **pcm;
			int samples;

			result=vorbis_synthesis(&dd->vb,&dd->op);
printf("vorbis_synthesis %d\n",result);

			if(result==0) /* test for success! */
			{
printf("vorbis_synthesis_blockin\n");
				vorbis_synthesis_blockin(&dd->vd,&dd->vb);
			}
			

			samples=vorbis_synthesis_pcmout(&dd->vd,&pcm);
			
printf("vorbis_synthesis_pcmout %d\n",samples);

			while((samples)>0)
			{
				int i;
				int j;
				int clipflag=0;
				int bout=(samples<dd->convsize?samples:dd->convsize);

				/* convert floats to 16 bit signed ints (host order) and
				interleave */
				for(i=0;i<dd->vi.channels;i++)
				{
					ogg_int16_t *ptr=dd->convbuffer+i;
					float  *mono=pcm[i];
					for(j=0;j<bout;j++)
					{
						int val=floor(mono[j]*32767.f+.5f);
						*ptr=val;
						ptr+=dd->vi.channels;
					}
				}
				vorbis_synthesis_read(&dd->vd,bout); /* tell libvorbis how
								  many samples we
								  actually consumed */
								  
				if(ogg_page_eos(&dd->og)) // check for end of file
				{
					dd->err="end";
				}
				
				lua_pushlstring(l,(const char *)dd->convbuffer,2*dd->vi.channels*bout);
				return 1;
			}
		}
}

		dd->done_stream_init=0;

		/* ogg_page and ogg_packet structs always point to storage in
		libvorbis.  They're never freed or manipulated directly */

printf("vorbis_block_clear\n");

		vorbis_block_clear(&dd->vb);
		vorbis_dsp_clear(&dd->vd);
            
	}
	
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_wetgenes_ogg_core (lua_State *l)
{
	const luaL_reg lib[] =
	{
		{"test",			lua_ogg_test},
		{"create",			lua_dogg_create},
		{"destroy",			lua_dogg_destroy},
		{"info",			lua_dogg_info},

		{"open",			lua_dogg_open},
		{"push",			lua_dogg_push},
		{"pull",			lua_dogg_pull},
		{"close",			lua_dogg_close},

		{0,0}
	};
	const luaL_reg meta_dogg[] =
	{
		{"__gc",			lua_dogg_destroy},
		{0,0}
	};

	luaL_newmetatable(l, lua_dogg_meta_name);
	luaL_openlib(l, NULL, meta_dogg, 0);
	lua_pop(l,1);
		
	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	return 1;
}

