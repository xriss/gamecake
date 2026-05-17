--
-- (C) 2025 Kriss@XIXs.com
--

--[[

Module read / write

Lets start with schizm tracker IT mods
https://github.com/schismtracker/schismtracker/wiki/ITTECH.TXT

Maybe add Milky tracker XM mods later

They seem to be the current good trackers

The plan is to read/write trackers not to edit/write them.

And support playback but not all the "bugs" so you need to compose for 
this not just throw in an old mod.

]]

-- these bit twiddlers work in vanilla lua and fats is luajit ffi optimized
-- we cant just use luajit ffi as we want to be wasm compatible
local wpack=require("wetgenes.pack") -- slower struct style access
local fats=require("wetgenes.fats") -- faster for large arrays of bytes / floats etc

local DUMP=require("wetgenes.logs"):export("dump")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
local bitsynth_tracker=M


--[[

		MODULE
        0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
      ┌───┬───┬───┬───┬───────────────────────────────────────────────┐
0000: │'I'│'M'│'P'│'M'│ Song Name, max 26 characters, includes NULL   │
      ├───┴───┴───┴───┴───────────────────────────────────────┬───────┤
0010: │.......................................................│PHiligt│
      ├───────┬───────┬───────┬───────┬───────┬───────┬───────┼───────┤
0020: │OrdNum │InsNum │SmpNum │PatNum │ Cwt/v │ Cmwt  │ Flags │Special│
      ├───┬───┼───┬───┼───┬───┼───────┼───────┴───────┼───────┴───────┤
0030: │GV │MV │IS │IT │Sep│PWD│MsgLgth│Message Offset │   Reserved    │
      ├───┴───┴───┴───┴───┴───┴───────┴───────────────┴───────────────┤
0040: │ Chnl Pan (64 bytes)...........................................│
      ├───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┤

      ├───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┤
0080: │ Chnl Vol (64 bytes)...........................................│
      ├───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┤

      ├───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┤
00C0: │ Orders, Length = OrdNum                                       │
      ├───────────────────────────────────────────────────────────────┤
xxxx: │ 'Long' Offset of instruments, Length = InsNum*4 (1)           │
      ├───────────────────────────────────────────────────────────────┤
xxxx: │ 'Long' Offset of samples headers, Length = SmpNum*4 (2)       │
      ├───────────────────────────────────────────────────────────────┤
xxxx: │ 'Long' Offset of patterns, Length = PatNum*4 (3)              │
      └───────────────────────────────────────────────────────────────┘

]]    
  
bitsynth_tracker.format_module={
	4,		"magick",
	26,		"u8s_name",
	"u16",	"philigt",
	"u16",	"ordnum",
	"u16",	"insnum",
	"u16",	"smpnum",
	"u16",	"patnum",
	"u16",	"cwtv",
	"u16",	"cmwt",
	"u16",	"u16_flags",
	"u16",	"u16_special",
	"u8",	"gv",
	"u8",	"mv",
	"u8",	"is",
	"u8",	"it",
	"u8",	"sep",
	"u8",	"pwd",
	"u16",	"message_length",
	"u32",	"message_offset",
	"u32",	"reserved",
	64,		"u8s_chanel_pan",
	64,		"u8s_chanel_vol",
}

--[[

      INSTRUMENT
        0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
      ┌───┬───┬───┬───┬───────────────────────────────────────────────┐
0000: │'I'│'M'│'P'│'I'│ DOS FileName (12345678.123)                   │
      ├───┼───┼───┼───┼───────┬───┬───┬───┬───┬───┬───┬───────┬───┬───┤
0010: │00h│NNA│DCT│DCA│FadeOut│PPS│PPC│GbV│DfP│RV │RP │TrkVers│NoS│ x │
      ├───┴───┴───┴───┴───────┴───┴───┴───┴───┴───┴───┴───────┴───┴───┤
0020: │ Instrument Name, max 26 bytes, includes NUL...................│
      ├───────────────────────────────────────┬───┬───┬───┬───┬───────┤
0030: │.......................................│IFC│IFR│MCh│MPr│MIDIBnk│
      ├───────────────────────────────────────┴───┴───┴───┴───┴───────┤
0040: │ Note-Sample/Keyboard Table, Length = 240 bytes................│
      ├───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┤

      ├───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┤
0130: │ Envelopes.....................................................│
      ├───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┤

]]

bitsynth_tracker.format_instrument={
	4,		"magick",
	13,		"u8s_filename",
	"u8",	"u8_nna",
	"u8",	"u8_dct",
	"u8",	"u8_dca",
	"u16",	"fadeout",
	"u8",	"pps",
	"u8",	"ppc",
	"u8",	"gbv",
	"u8",	"dfp",
	"u8",	"rv",
	"u8",	"rp",
	"u16",	"trkvers",
	"u8",	"nos",
	"u8",	"x",
	26,		"u8s_name",
	"u8",	"ifc",
	"u8",	"ifr",
	"u8",	"mch",
	"u8",	"mpr",
	"u16",	"midibnk",
	240,	"u8s_keyboard",
	0x52,	"u8s_envelope_volume",
	0x52,	"u8s_envelope_panning",
	0x52,	"u8s_envelope_pitch",
}

--[[

		ENVELOPE
        0   1   2   3   4   5   6.......
      ┌───┬───┬───┬───┬───┬───┬───────────────────────────────────┬───┐
xxxx: │Flg│Num│LpB│LpE│SLB│SLE│ Node points, 25 sets, 75 bytes....│ x │
      ├───┼───┼───┼───┼───┼───┼───┬───┬───┬───┬───┬───┬───┬───┬───┼───┤

]]

bitsynth_tracker.format_envelope={
	"u8",	"u8_flg",
	"u8",	"num",
	"u8",	"lpb",
	"u8",	"lpe",
	"u8",	"slb",
	"u8",	"sle",
	75,		"u8s_points",
}

--[[

		SAMPLE
        0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
      ┌───┬───┬───┬───┬───────────────────────────────────────────────┐
0000: │'I'│'M'│'P'│'S'│ DOS Filename (12345678.123)                   │
      ├───┼───┼───┼───┼───────────────────────────────────────────────┤
0010: │00h│GvL│Flg│Vol│ Sample Name, max 26 bytes, includes NUL.......│
      ├───┴───┴───┴───┴───────────────────────────────────────┬───┬───┤
0020: │.......................................................│Cvt│DfP│
      ├───────────────┬───────────────┬───────────────┬───────┴───┴───┤
0030: │ Length        │ Loop Begin    │ Loop End      │ C5Speed       │
      ├───────────────┼───────────────┼───────────────┼───┬───┬───┬───┤
0040: │ SusLoop Begin │ SusLoop End   │ SamplePointer │ViS│ViD│ViR│ViT│
      └───────────────┴───────────────┴───────────────┴───┴───┴───┴───┘

The cache file has the following pieces of information added on:

        0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
      ┌───────────────┬───────┬───────┬───┬───────────────────────────┐
0050: │ File Size     │ Date  │ Time  │Fmt│...........................│
      └───────────────┴───────┴───────┴───┴───────────────────────────┘

]]

bitsynth_tracker.format_sample={
	4,		"magick",
	13,		"u8s_filename",
	"u8",	"gvl",
	"u8",	"u8_flg",
	"u8",	"vol",
	26,		"u8s_name",
	"u8",	"u8_cvt",
	"u8",	"dfp",
	"u32",	"data_size",
	"u32",	"loop_begin",
	"u32",	"loop_end",
	"u32",	"c5speed",
	"u32",	"susloop_begin",
	"u32",	"susloop_end",
	"u32",	"data_addr",
	"u8",	"vis",
	"u8",	"vid",
	"u8",	"vir",
	"u8",	"u8_vit",
}

--[[

		PATTERN
        0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
      ┌───────┬───────┬───┬───┬───┬───┬───────────────────────────────┐
0000: │Length │ Rows  │ x │ x │ x │ x │ Packed data................   │
      ├───┬───┼───┬───┼───┼───┼───┼───┼───┬───┬───┬───┬───┬───┬───┬───┤

]]

bitsynth_tracker.format_pattern={
	"u16",	"length",
	"u16",	"rows",
	4,		"pad",
}

local clean_format=function(tab,format)

	for i=1,#format,2 do
		local name=format[i+1]
		for _,n in ipairs({ "u8s_", "u8_", "u16_", "u32_", }) do
			if name:sub(1,#n)==n then -- binary prefix
				tab[name]=nil
			end
		end
	end

end

-- honor null terminated string in data
local extract_cstring=function(data,start,maxlength)
	if not start then start=1 end
	local stop=(string.find(data,"\0",start))
	if maxlength then
		if		not stop				then	stop=start+maxlength
		elseif	stop>(start+maxlength)	then	stop=start+maxlength
		end
	end
	stop=stop-1
	return string.sub( data , start , stop )
end
-- returns a null terminated/padded string of the given size
local pad_cstring=function(str,size)
	if #str > size-1 then
		str=str:sub(1,size-1)
	end
	return str .. string.rep("\0",size-#str)
end

-- table of true/false for bits 0-7 in the given unsigned byte number 0-255
local extract_bits=function(num,bitcount)
	if not bitcount then bitcount=8 end
	local bits={}
	for i=1,bitcount do
		if num%2==1 then bits[i]=true else bits[i]=false end
		num=math.floor(num/2)
	end
	return bits
end
local compact_bits=function(bits)
	local num=0
	for i=1,#bits do
		if bits[i] then
			num=num+( 2^(i-1) )
		end
	end
	return num
end

bitsynth_tracker.IT_to_mod=function(data)

	local mod={}
	mod.head={}
	
	wpack.load( data , bitsynth_tracker.format_module , 0 , mod.head )

	assert( mod.head.magick == "IMPM" )

	mod.head.name=extract_cstring(mod.head.u8s_name)
	mod.head.flags    = extract_bits( mod.head.u16_flags , 16 )
	mod.head.special  = extract_bits( mod.head.u16_special , 16 )
	mod.head.chanel_pan=fats.uint8s_to_table( mod.head.u8s_chanel_pan )
	mod.head.chanel_vol=fats.uint8s_to_table( mod.head.u8s_chanel_vol )

	mod.head.message=extract_cstring( data , mod.head.message_offset+1 , mod.head.message_length )
	if mod.head.message:find("\r") then -- fix windows \r to \n
		if mod.head.message:find("\n") then
			mod.head.message=mod.head.message:gsub("\r","")
		else
			mod.head.message=mod.head.message:gsub("\r","\n")
		end
	end
	
	clean_format( mod.head , bitsynth_tracker.format_module )

	local loc={
		192,
		192+mod.head.ordnum,
		192+mod.head.ordnum+(mod.head.insnum*4),
		192+mod.head.ordnum+(mod.head.insnum*4)+(mod.head.smpnum*4),
		192+mod.head.ordnum+(mod.head.insnum*4)+(mod.head.smpnum*4)+(mod.head.patnum*4),
	}
	mod.orders              = fats.uint8s_to_table(  data:sub(loc[1]+1,loc[2]) )
	mod.instruments_offsets = fats.uint32s_to_table( data:sub(loc[2]+1,loc[3]) )
	mod.samples_offsets     = fats.uint32s_to_table( data:sub(loc[3]+1,loc[4]) )
	mod.patterns_offsets    = fats.uint32s_to_table( data:sub(loc[4]+1,loc[5]) )

	mod.instruments={}
	for idx=1,mod.head.insnum do
		local instrument={}
		mod.instruments[idx]=instrument
		wpack.load( data , bitsynth_tracker.format_instrument , mod.instruments_offsets[idx] , instrument )

		assert( instrument.magick == "IMPI" )
		
		instrument.filename=extract_cstring(instrument.u8s_filename)
		instrument.nna=extract_bits( instrument.u8_nna , 8 )
		instrument.dct=extract_bits( instrument.u8_dct , 8 )
		instrument.dca=extract_bits( instrument.u8_dca , 8 )
		instrument.name=extract_cstring(instrument.u8s_name)
		instrument.keyboard=fats.uint8s_to_table( instrument.u8s_keyboard )
		
		local parse_envelope=function(data)
			local t=fats.uint8s_to_table( data )

			envelope={}

			wpack.load(  data , bitsynth_tracker.format_envelope, 0 , envelope )
			envelope.flg=extract_bits( envelope.u8_flg , 8 )
			
			envelope.nodes={}
			if envelope.num>25 then envelope.num=25 end --clamp
			for i=1,envelope.num do
				envelope.nodes[i]={ t[ 7+((i-1)*3)+1 ] , t[ 7+((i-1)*3)+2 ] + t[ 7+((i-1)*3)+3 ]*256 }
			end
			
			clean_format( envelope , bitsynth_tracker.format_envelope )
			return envelope
		end

		instrument.envelope_volume  = parse_envelope( instrument.u8s_envelope_volume  ) -- fats.uint8s_to_table( data:sub(0x130+1,0x130+0x52) ) )
		instrument.envelope_panning = parse_envelope( instrument.u8s_envelope_panning ) -- fats.uint8s_to_table( data:sub(0x182+1,0x182+0x52) ) )
		instrument.envelope_pitch   = parse_envelope( instrument.u8s_envelope_pitch   ) -- fats.uint8s_to_table( data:sub(0x1d4+1,0x1d4+0x52) ) )

		clean_format( instrument , bitsynth_tracker.format_instrument )
	end
	mod.instruments_offsets=nil
	
	mod.samples={}
	for idx=1,mod.head.smpnum do
		local sample={}
		mod.samples[idx]=sample

		wpack.load( data , bitsynth_tracker.format_sample , mod.samples_offsets[idx] , sample )
		
		assert( sample.magick == "IMPS" )

		sample.filename=extract_cstring(sample.u8s_filename)

		sample.flg = extract_bits( sample.u8_flg , 8 )
		sample.cvt = extract_bits( sample.u8_cvt , 8 )
		sample.vit = extract_bits( sample.u8_vit , 8 )

		sample.name=extract_cstring(sample.u8s_name)
		
		if sample.flg[1] then -- got a sample
			local maxs=0
			local adds=0
			if sample.flg[2] then -- 16 bit
				sample.data=fats.int16s_to_table( data:sub( sample.data_addr+1 , sample.data_addr+(sample.data_size*2) ) )
				maxs=0x8000
				if sample.cvt[1] then -- signed
					adds=0
				else
					adds=-0x8000
				end
			else
				sample.data=fats.int8s_to_table( data:sub( sample.data_addr+1 , sample.data_addr+(sample.data_size*1) ) )
				maxs=0x80
				if sample.cvt[1] then -- signed
					adds=0
				else
					adds=-0x80
				end
			end
			for i=1,#sample.data do
				local v=sample.data[i]
				sample.data[i]=(v+adds)/maxs -- fix signed and normalize 
			end
		end

		clean_format( sample , bitsynth_tracker.format_sample )
	end
	mod.samples_offsets=nil

	mod.patterns={}
	for idx=1,mod.head.patnum do
		local pattern={}
		mod.patterns[idx]=pattern

		local t=fats.uint16s_to_table( data:sub( mod.patterns_offsets[idx]+1 , mod.patterns_offsets[idx]+4 ) )
		
		local length=t[1]
		local rows=t[2]

		local dat=fats.uint8s_to_table( data:sub( mod.patterns_offsets[idx]+8+1 , mod.patterns_offsets[idx]+8+length ) )

		local idx=0
		local pull=function() idx=idx+1 ; return dat[idx] end
		local states={}
		local get_state=function(idx)
			if not states[idx] then states[idx]={0,0,0,0,0,mask=extract_bits(0,8)} end
			return states[idx]
		end
		for i=1,rows do
			local row={}
			pattern[i]=row
			local max_c=0
			while true do
				local s
				local c=pull()

				if c==0 then break end
				if c>128 then
					c=c-128
					s=get_state(c)
					s.mask=extract_bits(pull(),8)
				else
					s=get_state(c)
				end
				local d={0,0,0,0,0}
				if c>max_c then max_c=c end
				row[c]=d

				if s.mask[1] then s[1]=pull() end
				if s.mask[2] then s[2]=pull() end
				if s.mask[3] then s[3]=pull() end
				if s.mask[4] then s[4]=pull() s[5]=pull() end

				if s.mask[1] or s.mask[5] then d[1]=s[1] end
				if s.mask[2] or s.mask[6] then d[2]=s[2] end
				if s.mask[3] or s.mask[7] then d[3]=s[3] end
				if s.mask[4] or s.mask[8] then d[4]=s[4] d[5]=s[5] end

			end
			for i=1,max_c do
				if not row[i] then row[i]=false end
			end
		end
	end
	mod.patterns_offsets=nil
	
	return mod
end


bitsynth_tracker.mod_to_IT=function(mod)

	local alloc_addr=0
	local alloc=function(size)
		local r=alloc_addr%16
		if r~=0 then
			alloc_addr=alloc_addr+16-r -- round up to 16 bytes
		end
		local ret=alloc_addr
		alloc_addr=alloc_addr+size
		return ret
	end

	local write_addr=0
	local write_datas={}
	local write=function(data)
		write_addr=write_addr+#data
		write_datas[#write_datas+1]=data
	end
	local write_align=function(align)
		if not align then align=16 end
		local n=align-((write_addr)%align)
		if n==align then return end -- already aligned
		write_addr=write_addr+n
		write_datas[#write_datas+1]=string.rep("\0",n)
	end

-- first we need to pre calculate values, mostly the position in the output file of each structure

	mod.head.magick = "IMPM"

	mod.head.ordnum = #mod.orders
	mod.head.insnum = #mod.instruments
	mod.head.smpnum = #mod.samples
	mod.head.patnum = #mod.patterns

	mod.head.size = 192+mod.head.ordnum+(mod.head.insnum*4)+(mod.head.smpnum*4)+(mod.head.patnum*4)
	mod.head.addr = alloc( mod.head.size ) -- should always be 0 as it is the first allocation
	
	mod.head.u16_flags    = compact_bits( mod.head.flags )
	mod.head.u16_special  = compact_bits( mod.head.special )

	mod.head.u8s_name = pad_cstring( mod.head.name , 26 )
	mod.head.chanel_u8s_pan = fats.table_to_uint8s( mod.head.chanel_pan )
	mod.head.chanel_u8s_vol = fats.table_to_uint8s( mod.head.chanel_vol )
	
	for i,instrument in ipairs( mod.instruments ) do
		instrument.size = 0x226
		instrument.addr = alloc( instrument.size )

		instrument.magick = "IMPI"

		instrument.u8_nna=compact_bits( instrument.nna )
		instrument.u8_dct=compact_bits( instrument.dct )
		instrument.u8_dca=compact_bits( instrument.dca )

		instrument.u8s_filename = pad_cstring( instrument.filename , 13 )
		instrument.u8s_name = pad_cstring( instrument.name , 26 )
		instrument.u8s_keyboard = fats.table_to_uint8s( instrument.keyboard )

		local prepare_envelope=function(envelope)
			envelope.u8_flg=compact_bits( envelope.flg )
			envelope.u8s_points={}
			for i=1,25 do
				local node=envelope.nodes[i]
				if not node then node={0,0} end -- pad empty nodes
				envelope.u8s_points[#envelope.u8s_points+1]= fats.table_to_uint8s( { node[1] } )
				envelope.u8s_points[#envelope.u8s_points+1]= fats.table_to_uint16s( { node[2] } )
			end
			envelope.u8s_points=table.concat(envelope.u8s_points)
		end
		
		prepare_envelope( instrument.envelope_volume )
		prepare_envelope( instrument.envelope_panning )
		prepare_envelope( instrument.envelope_pitch )

	end

	for i,sample in ipairs( mod.samples ) do
		sample.size = 0x50
		sample.addr = alloc( sample.size )

		sample.magick = "IMPS"

		sample.u8_flg = compact_bits( sample.flg )
		sample.u8_cvt = compact_bits( sample.cvt )
		sample.u8_vit = compact_bits( sample.vit )

		sample.u8s_filename = pad_cstring( sample.filename , 13 )
		sample.u8s_name = pad_cstring( sample.name , 26 )
	end

	for i,pattern in ipairs( mod.patterns ) do -- just an empty pattern for now
		pattern.size = 8 + 32
		pattern.addr = alloc( pattern.size )
	end

	for i,sample in ipairs( mod.samples ) do -- put sample data last
		sample.data_size = #sample.data
		sample.data_addr = alloc( sample.data_size*2 )
	end

-- then we create a data string of all the data combined

	write( wpack.save( mod.head , bitsynth_tracker.format_module ) )
	clean_format( mod.head , bitsynth_tracker.format_module )

	write( fats.table_to_uint8s( mod.orders ) )
	for i,instrument in ipairs( mod.instruments ) do
		write( fats.table_to_uint32s( {instrument.addr} ) )
	end
	for i,sample in ipairs( mod.samples ) do
		write( fats.table_to_uint32s( {sample.addr} ) )
	end
	for i,pattern in ipairs( mod.patterns ) do
		write( fats.table_to_uint32s( {pattern.addr} ) )
	end


	for i,instrument in ipairs( mod.instruments ) do
		write_align()
		write( wpack.save( instrument , bitsynth_tracker.format_instrument ) )		
		clean_format( instrument , bitsynth_tracker.format_instrument )
	end

	for i,sample in ipairs( mod.samples ) do
		write_align()
		write( wpack.save( sample , bitsynth_tracker.format_sample ) )
		clean_format( sample , bitsynth_tracker.format_sample )
	end

	for i,pattern in ipairs( mod.patterns ) do
		write_align()
		-- shortest valid pattern
		write( fats.table_to_uint16s( { 32,32 } ) )
		-- padding
		write( fats.table_to_uint8s( { 0,0,0,0 } ) )
		-- blank pattern
		write( fats.table_to_uint8s( { 0,0,0,0,0,0,0,0 , 0,0,0,0,0,0,0,0 , 0,0,0,0,0,0,0,0 , 0,0,0,0,0,0,0,0 } ) )
	end

	for i,sample in ipairs( mod.samples ) do
		write_align()
		local d={}
		for i,f in ipairs(sample.data) do
			local n=math.floor(f*0x7fff)
			if n<-0x7fff then n=-0x7fff end
			if n> 0x7fff then n= 0x7fff end
			d[i]=n
		end
		write( fats.table_to_int16s( d ) )
	end

	write_align()
	return table.concat( write_datas )
end
