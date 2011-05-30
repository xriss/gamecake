
local bit=require('bit')


-- very simple xml parsing into a table
local sxml=require("simpxml")

local string=string
local table=table
local ipairs=ipairs
local math=math
local loadstring=loadstring
local pcall=pcall
local error=error
local io=io
local type=type
local tonumber=tonumber

-- imported global functions
local sub = string.sub
local match = string.match
local find = string.find
local push = table.insert
local pop = table.remove
local append = table.insert
local concat = table.concat
local floor = math.floor
local write = io.write
local read = io.read
local type = type
local setfenv = setfenv
local tostring=tostring
local pairs=pairs
local ipairs=ipairs
local unpack=unpack

local _G = _G

local wetlua=wetlua
local wldir=wetlua.dir or ""

module("fenestra.avatar")

--
-- avatar part group information
--
-- group is where the object is located, IE head
-- then look up the actual xox filename using the easy name used on a soul 
-- finally you now have a chunk of data that describes this xox
--
xox_groups={

inmouth =
{ 
	joint					= { xox=	"inmouth_joint"			; boi=1; grl=1; active=1; };
	fag						= { xox=	"inmouth_fag"			; boi=1; grl=1; active=1; };
	pipe					= { xox=	"inmouth_pipe"			; boi=1; grl=1; active=1; };
	cigar					= { xox=	"inmouth_cigar"			; boi=1; grl=1; active=1; };
	sushi_nigiri			= { xox=	"inmouth_sushi_nigiri"	; boi=1; grl=1; active=1; };
},

inhand =
{ 
	dagger					= { xox=	"item_dagger_fist"			; boi=1; grl=1; active=1; };
	hammer					= { xox=	"item_hammer_fist"			; boi=1; grl=1; active=1; };
	pistol					= { xox=	"item_pistol_fist"			; boi=1; grl=1; active=1; };
	joint					= { xox=	"item_joint_fist"			; boi=1; grl=1; active=1; };
	fag						= { xox=	"item_fag_fist"				; boi=1; grl=1; active=1; };
	joint					= { xox=	"item_joint_fist"			; boi=1; grl=1; active=1; };
	pipe					= { xox=	"item_pipe_fist"			; boi=1; grl=1; active=1; };
	cigar					= { xox=	"item_cigar_fist"			; boi=1; grl=1; active=1; };
	bottle					= { xox=	"item_bottle_fist"			; boi=1; grl=1; active=1; };
	bottle_smashed			= { xox=	"item_bottle_smashed_fist"	; boi=1; grl=1; active=1; };
},

beard =
{ 
	default					= { xox=	"beard_circle"			; boi=1; grl=0; active=1; };
	circle					= { xox=	"beard_circle"			; boi=1; grl=0; active=1; };
	circle_point			= { xox=	"beard_circle_point"	; boi=1; grl=0; active=1; };
	tash					= { xox=	"beard_tash"			; boi=1; grl=0; active=1; };
	whiskers				= { xox=	"beard_whiskers"		; boi=0; grl=0; active=1; };
},

body =
{ 
	
	bare					= { xox=	"body_bare"				; boi=1; grl=0; active=1; };
	bare_boobs				= { xox=	"body_bare_boobs"		; boi=0; grl=1; active=1; };
	
	robox					= { xox=	"body_robox"			; boi=0; grl=0; active=1; };
	
	bodess					= { xox=	"body_bodess"			; boi=0; grl=1; active=1; };
	
	tshirt					= { xox=	"body_tshirt"			; boi=1; grl=0; active=1; };
	tshirt_boobs			= { xox=	"body_tshirt_boobs"		; boi=0; grl=1; active=1; };
	tshirt_boobs_low		= { xox=	"body_tshirt_boobs_low"	; boi=0; grl=1; active=1; };
	
	coat					= { xox=	"body_coat"				; boi=1; grl=0; active=1; };
	overalls				= { xox=	"body_overalls"			; boi=1; grl=0; active=1; };
	vest					= { xox=	"body_vest"				; boi=1; grl=0; active=1; };
	vest_boobs				= { xox=	"body_vest_boobs"		; boi=0; grl=1; active=1; };
	
	tie						= { xox=	"body_tie"				; boi=1; grl=0; active=1; };
	tie_boobs				= { xox=	"body_tie_boobs"		; boi=0; grl=1; active=1; };

	default					= { xox=	"body"					; boi=1; grl=0; active=1; };
	bare_boobs_small		= { xox=	"body_bare_boobsa"		; boi=0; grl=1; active=0; };
	bare_boobs_medium		= { xox=	"body_bare_boobsc"		; boi=0; grl=1; active=0; };
	bare_boobs_large		= { xox=	"body_bare_boobse"		; boi=0; grl=1; active=0; };
	bare_chest				= { xox=	"body_bare_chest"		; boi=1; grl=0; active=0; };
	bare_gut				= { xox=	"body_bare_gut"			; boi=1; grl=0; active=0; };
	bodess_medium			= { xox=	"body_bodessc"			; boi=0; grl=1; active=0; };
	bodess_large			= { xox=	"body_bodesse"			; boi=0; grl=1; active=0; };
	tshirt_boobs_small		= { xox=	"body_tshirt_boobsa"	; boi=0; grl=1; active=0; };
	tshirt_boobs_medium		= { xox=	"body_tshirt_boobsc"	; boi=0; grl=1; active=0; };
	tshirt_boobs_large		= { xox=	"body_tshirt_boobse"	; boi=0; grl=1; active=0; };
	tshirt_chest			= { xox=	"body_tshirt_chest"		; boi=1; grl=0; active=0; };
	tshirt_gut				= { xox=	"body_tshirt_gut"		; boi=1; grl=0; active=0; };
	tshirt_lowcut_large		= { xox=	"body_tshirt_lowcute"	; boi=0; grl=3; active=0; };
	tshirt_skinny			= { xox=	"body_tshirt_skinny"	; boi=1; grl=0; active=0; };
	
},

tail =
{ 
	default					= { xox=	"tail"					; boi=1; grl=1; active=1; };
	bunny					= { xox=	"tail_bunny"			; boi=0; grl=1; active=1; };
	devil					= { xox=	"tail_devil"			; boi=0; grl=1; active=1; };
},


ear =
{ 
	default					= { xox=	"ear"					; boi=1; grl=1; active=1; };
	big						= { xox=	"ear_big"				; boi=1; grl=1; active=1; };
	big_sticky				= { xox=	"ear_big_sticky"		; boi=1; grl=1; active=1; };
	robox					= { xox=	"ear_robox"				; boi=0; grl=0; active=1; };
},


eye =
{ 
	default					= { xox=	"eye"					; boi=1; grl=1; active=1; };
	bigbrow					= { xox=	"eye_bigbrow"			; boi=1; grl=1; active=1; };
	brow					= { xox=	"eye_brow"				; boi=0; grl=0; active=1; };
	tribrow					= { xox=	"eye_tribrow"			; boi=1; grl=1; active=1; };
	robox					= { xox=	"eye_robox"				; boi=0; grl=0; active=1; };
},


eyeball =
{ 
	default					= { xox=	"eyeball"				; boi=5; grl=5; active=1; };
	diamond					= { xox=	"eyeball_cat"			; boi=1; grl=1; active=1; };
},


foot =
{ 
	default					= { xox=	"foot"					; boi=1; grl=1; active=1; };
	bare					= { xox=	"foot_bare"				; boi=1; grl=1; active=1; };
	boot					= { xox=	"foot_boot"				; boi=1; grl=1; active=1; };
	flipflop				= { xox=	"foot_flipflop"			; boi=1; grl=1; active=1; };
	slipper					= { xox=	"foot_slipper"			; boi=1; grl=1; active=1; };
	heel					= { xox=	"foot_heel"				; boi=0; grl=1; active=1; };
	shoe					= { xox=	"foot_shoe"				; boi=1; grl=1; active=1; };
	hoof					= { xox=	"foot_hoof"				; boi=0; grl=0; active=1; };
	robox					= { xox=	"foot_robox"			; boi=0; grl=0; active=1; };
},


hair =
{ 
	topspiked_short			= { xox=	"hair_topspiked_short"		; boi=1; grl=1; active=1; };
	peak					= { xox=	"hair_peak"					; boi=1; grl=1; active=1; };
	
	bob						= { xox=	"hair_bob"					; boi=1; grl=1; active=1; };
	goth_long				= { xox=	"hair_goth_long"			; boi=1; grl=1; active=1; };
	spikey_short			= { xox=	"hair_spikey_short"			; boi=1; grl=1; active=1; };
	trihawk_short			= { xox=	"hair_trihawk_short"		; boi=1; grl=1; active=1; };
	trihawk_hi				= { xox=	"hair_trihawk_hi"			; boi=1; grl=1; active=1; };
	hedgehog				= { xox=	"hair_hedgehog"				; boi=1; grl=1; active=1; };
	afro					= { xox=	"hair_afro"					; boi=1; grl=1; active=1; };
	afro_tall				= { xox=	"hair_afro_tall"			; boi=1; grl=1; active=1; };
		
	quiff					= { xox=	"hair_quiff"				; boi=1; grl=1; active=1; };
	curl_left				= { xox=	"hair_curl_left"			; boi=1; grl=1; active=1; };
	curl_right				= { xox=	"hair_curl_right"			; boi=1; grl=1; active=1; };
},

hair_base =
{ 
	default					= { xox=	"hair"						; boi=1; grl=1; active=1; };
	bowl					= { xox=	"hair_bowl"					; boi=1; grl=1; active=1; };
},

hair_xtra =
{ 
	ponytail				= { xox=	"hair_ponytail"				; boi=1; grl=1; active=1; };
	pigtails				= { xox=	"hair_pigtails"				; boi=1; grl=1; active=1; };
	long					= { xox=	"hair_long"					; boi=1; grl=1; active=1; };
	bunches					= { xox=	"hair_bunches"				; boi=1; grl=1; active=1; };
	bang					= { xox=	"hair_bang_base"			; boi=1; grl=1; active=1; };
	bang_zigs				= { xox=	"hair_bang_zigs"			; boi=1; grl=1; active=1; };
	bang_goff				= { xox=	"hair_bang_goff"			; boi=1; grl=1; active=1; };
	bang_sidel				= { xox=	"hair_bang_sidel"			; boi=1; grl=1; active=1; };
	bang_sider				= { xox=	"hair_bang_sider"			; boi=1; grl=1; active=1; };
	bang_emol				= { xox=	"hair_bang_emol"			; boi=1; grl=1; active=1; };
	bang_emor				= { xox=	"hair_bang_emor"			; boi=1; grl=1; active=1; };
	bang_nerd				= { xox=	"hair_bang_nerd"			; boi=1; grl=1; active=1; };
	bang_hugh				= { xox=	"hair_bang_hugh"			; boi=1; grl=1; active=1; };
},

hat =
{ 
	baseball				= { xox=	"hat_baseball"				; boi=1; grl=1; active=1; };
	pirate					= { xox=	"hat_pirate"				; boi=1; grl=1; active=1; };
	kerchief				= { xox=	"hat_kerchief"				; boi=1; grl=1; active=1; };
	bunny_ears				= { xox=	"hat_bunny_ears"			; boi=1; grl=1; active=1; };
},


hand =
{ 
	default					= { xox=	"hand"					; boi=1; grl=1; active=1; };
	foot					= { xox=	"hand_foot"				; boi=0; grl=0; active=1; };
	hoof					= { xox=	"hand_hoof"				; boi=0; grl=0; active=1; };
	robox					= { xox=	"hand_robox"			; boi=0; grl=0; active=1; };
},


head =
{ 
	default					= { xox=	"head"					; boi=1; grl=1; active=1; };
	cheekbones				= { xox=	"head_cheakbones"		; boi=1; grl=1; active=0; };
	chub					= { xox=	"head_chub"				; boi=1; grl=1; active=0; };
	thin					= { xox=	"head_thin"				; boi=1; grl=1; active=0; };
	skull					= { xox=	"head_skull"			; boi=0; grl=0; active=1; };
	robox					= { xox=	"head_robox"			; boi=0; grl=0; active=1; };
	chinless				= { xox=	"head_chinless"			; boi=0; grl=1; active=1; };
},


mouth =
{ 
	default					= { xox=	"mouth"					; boi=1; grl=1; active=1; };
	bow						= { xox=	"mouth_bow"				; boi=1; grl=1; active=1; };
	bow_fat					= { xox=	"mouth_bow_fat"			; boi=1; grl=1; active=1; };
	bow_thin				= { xox=	"mouth_bow_thin"		; boi=1; grl=1; active=1; };
	fat						= { xox=	"mouth_fat"				; boi=1; grl=1; active=1; };
	thin					= { xox=	"mouth_thin"			; boi=1; grl=1; active=1; };
	beak					= { xox=	"mouth_beak"			; boi=0; grl=0; active=1; };
	squid					= { xox=	"mouth_squid"			; boi=0; grl=0; active=1; };
	jaw						= { xox=	"mouth_jaw"				; boi=0; grl=0; active=1; };
	robox					= { xox=	"mouth_robox"			; boi=0; grl=0; active=1; };
},


nose =
{ 
	default					= { xox=	"nose"					; boi=1; grl=1; active=1; };
	small					= { xox=	"nose_small"			; boi=1; grl=1; active=1; };
	small_up				= { xox=	"nose_small_up"			; boi=1; grl=1; active=1; };
	snub					= { xox=	"nose_snub"				; boi=1; grl=1; active=1; };
	wide					= { xox=	"nose_wide"				; boi=1; grl=1; active=1; };
	wide_up					= { xox=	"nose_wide_up"			; boi=1; grl=1; active=1; };
	clown					= { xox=	"nose_clown"			; boi=0; grl=0; active=1; };
	snout					= { xox=	"nose_snout"			; boi=0; grl=0; active=1; };
	robox					= { xox=	"nose_robox"			; boi=0; grl=0; active=1; };
},


specs =
{ 
	default					= { xox=	"specs"					; boi=1; grl=1; active=1; };
	round					= { xox=	"specs_round"			; boi=1; grl=1; active=1; };
	eyepatch_left			= { xox=	"specs_eyepatch_left"	; boi=1; grl=1; active=1; };
	eyepatch_right			= { xox=	"specs_eyepatch_right"	; boi=1; grl=1; active=1; };
},

}
for g,t in pairs(xox_groups) do
	for n,v in pairs(t) do
		v.group=g
		v.name=n
	end
end

-----------------------------------------------------------------------------
--
-- split a string into a table
--
-----------------------------------------------------------------------------
local function split(div,str)

  if (div=='') or not div then error("div expected", 2) end
  if (str=='') or not str then error("str expected", 2) end
  
  local pos,arr = 0,{}
  
  -- for each divider found
  for st,sp in function() return string.find(str,div,pos,false) end do
	table.insert(arr,sub(str,pos,st-1)) -- Attach chars left of current divider
	pos = sp + 1 -- Jump past current divider
  end
  
  if pos~=0 then
	table.insert(arr,sub(str,pos)) -- Attach chars right of last divider
  else
	table.insert(arr,str) -- return entire string
  end
  
  
  return arr
end


-----------------------------------------------------------------------------
--
-- find the basename from a filename
--
-----------------------------------------------------------------------------
local function get_basename(name)

	local a1=split("%/",name)
	local a2=split("%.",a1[#a1])
	local a3=split("%_",a2[1])
	local s=a3[1]
	
	if s=="left" or s=="right" then s=a3[2] end
	
	return s
end

function setup(win)

	local function print(...)
		win._g.print(...)
	end

	local ogl=win.ogl

	local it={}
	
	it.basename_map={
		[	"body"			]	=	"body"				,
		[	"head"			]	=	"head"				,
		[	"foot"			]	=	"left_foot"			,
		[	"foot_flip"		]	=	"right_foot"		,
		[	"hair"			]	=	"hair"				,
		[	"ear"			]	=	"left_ear"			,
		[	"ear_flip"		]	=	"right_ear"			,
		[	"nose"			]	=	"nose"				,
		[	"mouth"			]	=	"mouth"				,
		[	"eye"			]	=	"left_eye"			,
		[	"eye_flip"		]	=	"right_eye"			,
		[	"tail"			]	=	"tail"				,
		[	"hand"			]	=	"left_hand"			,
		[	"hand_flip" 	]	=	"right_hand"		,
		[	"eyeball"		]	=	"left_eyeball"		,
		[	"eyeball_flip"	]	=	"right_eyeball"		,
	}
	
	it.bodyparts={
		body				={"body"},
		head				={"head"},
		left_foot			={"foot"},
		right_foot			={"foot"},
		hair				={"hair"},
		left_ear			={"ear"},
		right_ear			={"ear"},
		nose				={"nose"},
		mouth				={"mouth"},
		left_eye			={"eye"},
		right_eye			={"eye"},
		tail				={"tail"},
		left_hand			={"hand"},
		right_hand			={"hand"},
		left_eyeball		={"eyeball"},
		right_eyeball		={"eyeball"},
	}
	
	function it.clean()

	end


--
-- Load avatar parts into an xsx for display
--	
	function it.map_xsx(xsx,soul)
	

		for i,v in ipairs(xsx.items) do
		
			local name=get_basename(v.name)
			local flip=((v.flags%2)==1)
			if flip then name=name.."_flip" end
			
			local name=it.basename_map[name]
			
--			print(name or i)
			
			if name then
				local part=soul.vanilla.parts[name]
				if part then
					v.fname=v.fname or {}
					v.data=v.data or {}
					v.soulinfo=v.soulinfo or {}
					
					v.size[1]=part.xsize or 1 -- v.size[1]
					v.size[2]=part.ysize or 1 -- v.size[2]
					v.size[3]=part.zsize or 1 -- v.size[3]
					
					v.morphs[1]=part.morph0 or 0
					v.morphs[2]=part.morph1 or 0
					v.morphs[3]=part.morph2 or 0
					v.morphs[4]=part.morph3 or 0
					
					v.pos[1]=part.xpos or 0 -- v.size[1]
					v.pos[2]=part.ypos or 0 -- v.size[2]
					v.pos[3]=part.zpos or 0 -- v.size[3]
					
					for i=1,4 do
					
						v.soulinfo[i]=part[i]
							
						if part[i] and part[i].xox_info and part[i].xox_info.xox then
					
							local fname="data/avatar/xox/"..part[i].xox_info.xox..".xox"
							
							if v.fname[i]~=fname then
							
								if v[i] then v[i].clean() end
								
								v.data[i]=win.data.load(fname)
								v[i]=win.xox(v.data[i])
								v.fname[i]=fname
								
							end
							
							if v[i] then
								for i,v in ipairs(v[i].surfaces or {}) do
								
									local surf=soul.vanilla.surfaces[v.name]
									if surf then
										v.argb =surf.argb  or v.argb
										v.spec =surf.spec  or v.spec
										v.gloss=surf.gloss or v.gloss
									end
--print("surf:"..v.name.." "..v.spec.." "..v.gloss)
								
								end
								
								v[i].set()
							end
							
							
						else
							if v[i] then v[i].clean() end
							v[i]=nil
							v.fname[i]=nil
							v.data[i]=nil
						end
					end
					
				end
			end
		end
		
		xsx.set()

	end
	
--
-- Load an avatars soul (xml file)
--	
	function it.load_soul(filename,d)
	
		local soul={}
		if not d then -- did not pass in data, read it from a file
--print("soul : "..filename)
			local fp=io.open(filename,"r")
			d=fp:read("*a")
			fp:close()
		end
		
		local tab=sxml.parse(d)
--print(sxml.tree_to_string(tab))
		local t=tab
		
		local function isa(t,s)
			if type(t)~="table" then return false end
			if t[0]==s then return true end
			return false
		end
		
		local function fromhex(s)
			if s then
				if s:sub(1,2)=="0x" then
					return tonumber(s:sub(3),16)
				else
					return tonumber(s,16)
				end
			end
			return nil
		end
		local function fromnum(s)
			if s then return tonumber(s) end
			return nil
		end
		
		t=sxml.child(t,"soul")
		for i,v in ipairs(t) do
		
			if isa(v,"flavour") then -- for each flavour
			
				local dat={}
				soul[v.name]=dat
				dat.parts={}
				dat.surfaces={}
			
				for i,v in ipairs(v) do
				
					if isa(v,"part") then -- for each part
					
						local p={}
						dat.parts[v.name]=p
						
						for i,v in pairs(v) do
							if type(i)=="string" then -- set all attribs
								if i~="name" then
									p[i]=v
								end
							end
						end
						
						local idx=1
						for i,v in ipairs(v) do
							if type(v)=="table" and v[0]=="object" then -- an object
								local o={}
								p[idx]=o
								
								for i,v in pairs(v) do
									if type(i)=="string" then -- set all attribs
										o[i]=v
									end
								end
								
								o.xox_info=xox_groups[ o.group ]
								if o.xox_info then
									o.xox_info=o.xox_info[ o.name ]
								end
								
								
								idx=idx+1
								if idx>4 then break end
							end
						end
						
						p.xpos=fromnum(p.xpos)
						p.ypos=fromnum(p.ypos)
						p.zpos=fromnum(p.zpos)
						
						p.xsize=fromnum(p.xsize)
						p.ysize=fromnum(p.ysize)
						p.zsize=fromnum(p.zsize)
						
						p.morph0=fromnum(p.morph0)
						p.morph1=fromnum(p.morph1)
						p.morph2=fromnum(p.morph2)
						p.morph3=fromnum(p.morph3)

					
					elseif isa(v,"surface") then -- for each surface
					
						local p={}
						dat.surfaces[v.name]=p
						
						for i,v in pairs(v) do
							if type(i)=="string" then -- set all attribs
								if i~="name" then
									p[i]=v
								end
							end
						end
						
						p.argb=bit.bor(fromhex(p.argb),0xff000000)
						p.spec=bit.bor(fromhex(p.spec),0xff000000)
						p.gloss=2+(tonumber(p.gloss)*126) -- make 0 a "good gloss number"
						
					end
					
				end
				
			end
		end
		
--		print(soul)


-- whoops spilling
		soul.vanilla=soul.vanilla or soul.vanila
		soul.vanila=nil
-- whoops spilling

		return soul
	end
	
--
-- Save an avatars soul (xml file)
--	
	function it.save_soul(soul,filename)
	
		local tree={}
		tree[1]={[0]="soul",version="1.0"}
		for i,v in pairs(soul) do
			local t=tree[1]
			t[#t+1]={[0]="flavour",name=i}
			t=t[#t]
			for i,v in pairs(v.parts) do
				t[#t+1]={[0]="part",name=i}
				local p=t[#t]
				for ii,vv in pairs(v) do
					if type(ii)=="string" then
						p[ii]=tostring(vv)
					end
				end
				for i=1,4 do
					p[#p+1]={[0]="object"}
					local o=p[#p]
					for ii,vv in pairs(v[i] or {}) do
						if type(ii)=="string" and type(vv)~="table" then
							o[ii]=tostring(vv)
						end
					end
				end
			end
			for i,v in pairs(v.surfaces) do
				t[#t+1]={[0]="surface",name=i}
				local s=t[#t]
				for ii,vv in pairs(v) do
					if type(ii)=="string" then
						s[ii]=tostring(vv)
					end
				end
				if s.argb then s.argb=string.format("0x%08x",s.argb) end -- use hex
				if s.spec then s.spec=string.format("0x%08x",s.spec) end -- use hex
				if s.gloss then s.gloss=(s.gloss-2)/126 end -- slight number fiddle, sorry
			end
		end
	
		local fp=io.open(filename,"w")
		if not fp then return false,"open failed" end
		fp:write(
		sxml.tree_to_string(tree))
		fp:close()
		
		return true
	end
	
	return it
end
