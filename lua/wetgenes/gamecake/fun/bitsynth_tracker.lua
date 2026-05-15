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
  
bitsynth_tracker.format_module_1E={
	"u16",	"philigt",
	"u16",	"ordnum",
	"u16",	"insnum",
	"u16",	"smpnum",
	"u16",	"patnum",
	"u16",	"cwtv",
	"u16",	"cmwt",
	"u16",	"flags",
	"u16",	"special",
	"u8",	"gv",
	"u8",	"mv",
	"u8",	"is",
	"u8",	"it",
	"u8",	"sep",
	"u8",	"pwd",
	"u16",	"message_length",
	"u32",	"message_offset",
	"u32",	"reserved",
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

bitsynth_tracker.format_instrument_11={
	"u8",	"nna",
	"u8",	"dct",
	"u8",	"dca",
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
}

bitsynth_tracker.format_instrument_3A={
	"u8",	"ifc",
	"u8",	"ifr",
	"u8",	"mch",
	"u8",	"mpr",
	"u16",	"midibnk",
}

--[[

		ENVELOPE
        0   1   2   3   4   5   6.......
      ┌───┬───┬───┬───┬───┬───┬───────────────────────────────────┬───┐
xxxx: │Flg│Num│LpB│LpE│SLB│SLE│ Node points, 25 sets, 75 bytes....│ x │
      ├───┼───┼───┼───┼───┼───┼───┬───┬───┬───┬───┬───┬───┬───┬───┼───┤

]]

bitsynth_tracker.format_envelope_00={
	"u8",	"flg",
	"u8",	"num",
	"u8",	"lpb",
	"u8",	"lpe",
	"u8",	"slb",
	"u8",	"sle",
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

bitsynth_tracker.format_sample_11={
	"u8",	"gvl",
	"u8",	"flg",
	"u8",	"vol",
}

bitsynth_tracker.format_sample_2E={
	"u8",	"cvt",
	"u8",	"dfp",
	"u32",	"length",
	"u32",	"loop_begin",
	"u32",	"loop_end",
	"u32",	"c5speed",
	"u32",	"susloop_begin",
	"u32",	"susloop_end",
	"u32",	"pointer",
	"u8",	"vis",
	"u8",	"vid",
	"u8",	"vir",
	"u8",	"vit",
}

--[[

		PATTERN
        0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
      ┌───────┬───────┬───┬───┬───┬───┬───────────────────────────────┐
0000: │Length │ Rows  │ x │ x │ x │ x │ Packed data................   │
      ├───┬───┼───┬───┼───┼───┼───┼───┼───┬───┬───┬───┬───┬───┬───┬───┤

]]

bitsynth_tracker.format_pattern_00={
	"u16",	"length",
	"u16",	"rows",
}



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

-- table of true/false for bits 0-7 in the given unsigned byte number 0-255
local extract_8bits=function(num)
	local bits={}
	for i=1,8 do
		if num%2==1 then bits[i]=true else bits[i]=false end
		num=math.floor(num/2)
	end
	return bits
end

bitsynth_tracker.IT_to_mod=function(data)

	local mod={}
	mod.head={}
	
	mod.head.magick=data:sub(1,4)
	assert( mod.head.magick == "IMPM" )

	mod.head.name=extract_cstring(data,5,26)

--[[
	local t=fats.uint16s_to_table( data:sub(31,48) )
	
	mod.head.philigt  = t[1]
	mod.head.ordnum   = t[2]
	mod.head.insnum   = t[3]
	mod.head.smpnum   = t[4]
	mod.head.patnum   = t[5]
	mod.head.cwt      = t[6]
	mod.head.cmwt     = t[7]
	mod.head.flags    = extract_8bits(t[8])
	mod.head.special  = extract_8bits(t[9])

	local t=fats.uint8s_to_table( data:sub(49,54) )

	mod.head.gv   = t[1]
	mod.head.mv   = t[2]
	mod.head.is   = t[3]
	mod.head.it   = t[4]
	mod.head.sep  = t[5]
	mod.head.pwd  = t[6]

	local t=fats.uint16s_to_table( data:sub(55,56) )

	mod.head.message_length = t[1]

	local t=fats.uint32s_to_table( data:sub(57,60) )

	mod.head.message_offset = t[1]
]]

	wpack.load( data , bitsynth_tracker.format_module_1E , 0x1e , mod.head )

	mod.head.flags    = extract_8bits( mod.head.flags )

	mod.head.special  = extract_8bits( mod.head.special )

	mod.head.message=extract_cstring( data , mod.head.message_offset+1 , mod.head.message_length )

	mod.chanel_pan=fats.uint8s_to_table( data:sub(65,128) )

	mod.chanel_vol=fats.uint8s_to_table( data:sub(129,192) )

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
		local dat=data:sub( mod.instruments_offsets[idx]+1 , mod.instruments_offsets[idx]+1024 )
		
		instrument.magick=dat:sub(1,4)
		assert( instrument.magick == "IMPI" )
		instrument.filename=extract_cstring(dat,5,17)
		
--[[
		local t1=fats.uint8s_to_table( dat:sub(17,32) )
		local t2=fats.uint16s_to_table( dat:sub(17,32) )

		instrument.nna=extract_8bits(t1[2])
		instrument.dct=extract_8bits(t1[3])
		instrument.dca=extract_8bits(t1[4])

		instrument.fadeout=t2[3]

		instrument.pps=t1[7]
		instrument.ppc=t1[8]
		instrument.gbv=t1[9]
		instrument.dfp=t1[10]
		instrument.rv=t1[11]
		instrument.rp=t1[12]

		instrument.trkvers=t2[7]

		instrument.nos=t1[15]

		local t1=fats.uint8s_to_table( dat:sub(59,62) )
		local t2=fats.uint8s_to_table( dat:sub(63,64) )

		instrument.ifc=t1[1]
		instrument.ifr=t1[2]
		instrument.mch=t1[3]
		instrument.mpr=t1[4]

		instrument.midibnk=t2[1]
]]
		wpack.load( dat , bitsynth_tracker.format_instrument_11 , 0x11 , instrument )

		instrument.nna=extract_8bits( instrument.nna )
		instrument.dct=extract_8bits( instrument.dct )
		instrument.dca=extract_8bits( instrument.dca )

		instrument.name=extract_cstring(dat,33,58)
		
		instrument.keyboard=fats.uint8s_to_table( dat:sub(64+1,64+240) )
		
		local parse_envelope=function(data,start)
			local t=fats.uint8s_to_table( data:sub(start+1,start+0x52) )

			envelope={}

--[[
			envelope.flg=extract_8bits(t[1])
			envelope.num=t[2]
			envelope.lpb=t[3]
			envelope.lpe=t[4]
			envelope.slb=t[5]
			envelope.sle=t[6]
]]


			wpack.load(  data , bitsynth_tracker.format_envelope_00, start , envelope )
			envelope.flg=extract_8bits( envelope.flg )
			
			envelope.nodes={}
			if envelope.num>25 then envelope.num=25 end --clamp
			for i=1,envelope.num do
				envelope.nodes[i]={ t[ 7+((i-1)*3)+1 ] , t[ 7+((i-1)*3)+2 ] + t[ 7+((i-1)*3)+3 ]*256 }
			end

			return envelope
		end

		instrument.envelope_volume  = parse_envelope( data , 0x130 ) -- fats.uint8s_to_table( data:sub(0x130+1,0x130+0x52) ) )
		instrument.envelope_panning = parse_envelope( data , 0x182 ) --  fats.uint8s_to_table( data:sub(0x182+1,0x182+0x52) ) )
		instrument.envelope_pitch   = parse_envelope( data , 0x1d4 ) --  fats.uint8s_to_table( data:sub(0x1d4+1,0x1d4+0x52) ) )
                   
	end
	mod.instruments_offsets=nil
	
	mod.samples={}
	for idx=1,mod.head.smpnum do
		local sample={}
		mod.samples[idx]=sample
		local dat=data:sub( mod.samples_offsets[idx]+1 , mod.samples_offsets[idx]+1024 )
		
		sample.magick=dat:sub(1,4)
		assert( sample.magick == "IMPS" )
		sample.filename=extract_cstring(dat,5,17)

--[[
		local t1=fats.uint8s_to_table( dat:sub(16+1,16+4) )
		
		sample.gvl = t1[2]
		sample.flg = extract_8bits(t1[3])
		sample.vol = t1[4]

		sample.name=extract_cstring(dat,20+1,20+26)

		local t1=fats.uint8s_to_table( dat:sub(0x2e+1,0x2e+2) )

		sample.cvt = extract_8bits(t1[1])
		sample.dfp = t1[2]

		local t4=fats.uint32s_to_table( dat:sub(0x30+1,0x30+28) )

		sample.length        = t4[1]
		sample.loop_begin    = t4[2]
		sample.loop_end      = t4[3]
		sample.c5speed       = t4[4]
		sample.susloop_begin = t4[5]
		sample.susloop_end   = t4[6]
		sample.pointer       = t4[7]

		local t1=fats.uint8s_to_table( dat:sub(0x4c+1,0x4c+4) )

		sample.vis = t1[1]
		sample.vid = t1[2]
		sample.vir = t1[3]
		sample.vit = extract_8bits(t1[4])
]]

		wpack.load( dat , bitsynth_tracker.format_sample_11, 0x11 , sample )
		wpack.load( dat , bitsynth_tracker.format_sample_2E, 0x2e , sample )

		sample.flg = extract_8bits( sample.flg )
		sample.cvt = extract_8bits( sample.cvt )
		sample.vit = extract_8bits( sample.vit )

		sample.name=extract_cstring(dat,20+1,20+26)

		
		if sample.flg[1] then -- got a sample
			local maxs=0
			local adds=0
			if sample.flg[2] then -- 16 bit
				sample.data=fats.int16s_to_table( data:sub( sample.pointer+1 , sample.pointer+(sample.length*2) ) )
				maxs=0x8000
				if sample.cvt[1] then -- signed
					adds=0
				else
					adds=-0x8000
				end
			else
				sample.data=fats.int8s_to_table( data:sub( sample.pointer+1 , sample.pointer+(sample.length*1) ) )
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
			if not states[idx] then states[idx]={0,0,0,0,0,mask=extract_8bits(0)} end
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
					s.mask=extract_8bits(pull())
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
		local ret=alloc_addr		
		size=math.ceil(size/16)*16 -- round up length to 16 bytes
		alloc_addr=alloc_addr+size
		return ret
	end

	local write_datas={}
	local write=function(data)
		write_datas[#write_datas+1]=data
	end

-- first we need to pre calculate values, mostly the position in the output file of each structure

	mod.head.size = 192+mod.head.ordnum+(mod.head.insnum*4)+(mod.head.smpnum*4)+(mod.head.patnum*4)
	mod.head.addr = alloc( mod.head.size ) -- should always be 0 as it is the first allocation
	
	for i,instrument in ipairs( mod.instruments ) do
		instrument.size = 0x226
		instrument.addr = alloc( instrument.size )
	end

	for i,sample in ipairs( mod.samples ) do
		sample.size = 0x60
		sample.addr = alloc( sample.size )

		sample.data_size = 2*#sample.data
		sample.data_addr = alloc( sample.data_size )
	end

	for i,pattern in ipairs( mod.patterns ) do -- just an empty pattern for now
		pattern.size = 8 + 32
		pattern.addr = alloc( pattern.size )
	end

-- then we create a data string of all the data combined

	for i,instrument in ipairs( mod.instruments ) do
	end

	for i,sample in ipairs( mod.samples ) do
	end

	for i,pattern in ipairs( mod.patterns ) do -- just empty pattern for now
	end



	return table.concat( write_datas )
end
