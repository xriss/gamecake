
// hax to be honest, all this to create a lua_pack_toluserdata function
// if lua or luajit change then this will break
// it is however still better than not having any bounds checking

// even more evil hack
// we create some userdata of various sizes and find where lua or luajit is keeping the size value.
// then we remember that location
static int hax_userdata_size_offset=0;
static int hax_get_userdata_size_offset( lua_State *l )
{
	int i;
	uint32_t *p;
	uint32_t t;
	if(hax_userdata_size_offset==0) // go fish
	{
		for(i=-1;i>-16;i--)
		{
			p=(uint32_t*)lua_newuserdata(l,42);
			t=*(p+i);
			lua_pop(l,1);
			if( t == 42 )
			{
				p=(uint32_t*)lua_newuserdata(l,23);
				t=*(p+i);
				lua_pop(l,1);
				if( t == 23 )
				{
					p=(uint32_t*)lua_newuserdata(l,19);
					t=*(p+i);
					lua_pop(l,1);
					if( t == 19 )
					{
						hax_userdata_size_offset=i;
						break;
					}
				}
			}
		}
	}
	return hax_userdata_size_offset;
}



static unsigned char * hax_toluserdata (lua_State *l, int idx, size_t *len)
{
	int hax=hax_get_userdata_size_offset(l);
	uint32_t *t=(uint32_t*)lua_touserdata(l,idx);
	if(!t) { return 0; }
	if(len)
	{
		if(lua_islightuserdata(l,idx))
		{
			*len=0x7fffffff;
		}
		else
		{
			*len=(size_t)(*(t+hax));
		}
	}
	
	return (unsigned char *)t;
}

