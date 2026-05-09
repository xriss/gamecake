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

local wpack=require("wetgenes.pack")
local fats=require("wetgenes.fats")

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M
local bitsynth_tracker=M

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

	mod.head.message=extract_cstring(data,mod.head.message_offset+1,mod.head.message_length)

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

		instrument.name=extract_cstring(dat,33,58)

		local t1=fats.uint8s_to_table( dat:sub(59,62) )
		local t2=fats.uint8s_to_table( dat:sub(63,64) )

		instrument.ifc=t1[1]
		instrument.ifr=t1[2]
		instrument.mch=t1[3]
		instrument.mpr=t1[4]

		instrument.midibnk=t2[1]
		
		instrument.keyboard=fats.uint8s_to_table( dat:sub(64+1,64+240) )
		
		local parse_envelope=function(t)
			envelope={}

			envelope.flg=extract_8bits(t[1])
			envelope.num=t[2]
			envelope.lpb=t[3]
			envelope.lpe=t[4]
			envelope.slb=t[5]
			envelope.sle=t[6]
			
			envelope.nodes={}
			if envelope.num>25 then envelope.num=25 end --clamp
			for i=1,envelope.num do
				envelope.nodes[i]={ t[ 7+((i-1)*3)+1 ] , t[ 7+((i-1)*3)+2 ] + t[ 7+((i-1)*3)+3 ]*256 }
			end

			return envelope
		end

		instrument.envelope_volume  = parse_envelope( fats.uint8s_to_table( data:sub(0x130+1,0x130+0x52) ) )
		instrument.envelope_panning = parse_envelope( fats.uint8s_to_table( data:sub(0x182+1,0x182+0x52) ) )
		instrument.envelope_pitch   = parse_envelope( fats.uint8s_to_table( data:sub(0x1d4+1,0x1d4+0x52) ) )
                   
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


	return data
end
