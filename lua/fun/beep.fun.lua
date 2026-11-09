--
-- This is fun64 code, you can copy paste it into https://xriss.github.io/fun64/pad/ to run it.
--

-- be careful not to include exploding locals as these small functions will get
-- moved to another thread for rendering and try and take everything with them
--local note2freq = require("wetgenes.gamecake.fun.bitsynth").note2freq
--local fwav = require("wetgenes.gamecake.fun.bitsynth").fwav


--[=[

local midinote_to_freq=function(m)
	return (2^((m-69)/12))*440
end
local note_to_freq=function(s)
	local lookup={ ["C"]=0,["C#"]=1,["D"]=2,["D#"]=3,["E"]=4,["F"]=5,["F#"]=6,["G"]=7,["G#"]=8,["A"]=9,["A#"]=10,["B"]=11}
	local o=tonumber(s:sub(-1)) or 0
	local n=lookup[ s:sub(1,-2):upper() ] or 0
	return midinote_to_freq( (o+2)*12 + (n-1) )
end

beeps={}
beeps["1"]={
fwav="sine",
duty=0.5,
adsr={
	1,
	0,0,0.4,0.1
},
}

beeps["2"]={
fwav="triangle",
adsr={
	1,
	0,0,0.4,0.1
},
}

beeps["3"]={
fwav="sawtooth",
duty=0.125,
adsr={
	1,
	0,0,0.4,0.1
},
}

beeps["4"]={
fwav="toothsaw",
duty=0.125,
adsr={
	1,
	0,0,0.4,0.1
},
}

beeps["5"]={
fwav="square",
duty=0.5,
adsr={
	1,
	0,0,0.4,0.1
},
}

beeps["6"]={
fwav="whitenoise",
frequency="C2",
adsr={
	1,
	0,0,0.4,0.1
},
}
beeps["7"]={
fwav="whitenoise",
frequency="C4",
adsr={
	1,
	0,0,0.4,0.1
},
}
beeps["8"]={
fwav="whitenoise",
frequency="C6",
adsr={
	1,
	0,0,0.4,0.1
},
}

beeps["0"]={
fwav=function(t)
	local t2=t/8
	local n=math.sin( (t%1) * math.pi*2  ) * math.sin( ((t2)%1) * math.pi*2  )
	return n
end,
adsr={
	1,
	0,0,0.4,0.1
},
}

beeps["q"]={
fwav="sine",
adsr={
	0.5,
	0.0, 0.0, 2.0, 0.1
},
fm={
	frequency=8,
--	    fwav="toothsaw",
	fwav="sawtooth",
--	    fwav="square",
	ffreq=function(it)
	local f1=note_to_freq("C4")
	local f2=note_to_freq("D4")
	local f3=note_to_freq("E4")
	local t1=0
	local f=function(m,t)
		if m<0 then return f2+((f1-f2)*-m)+t*t1 end
		return f2+((f3-f2)*m)+t*t1
	end
	return f
	end,
}
}

--[[
beeps["w"]={
fwav="sine",
adsr={
	1,
	0,0,1,0.1
},
fm={
	frequency=1024,
	fwav="sine",
	ffreq=function(it)
	local f1=note2freq("C3")
	local f2=note2freq("D3")
	local f3=note2freq("E3")
	local t1=64
	local f=function(m,t)
		if m<0 then return f2+((f1-f2)*-m)+fwav.square(t*4)*t1 end
		return f2+((f3-f2)*m)+fwav.square(t*4)*t1
	end
	return f
	end,
}
}
]]

beeps["e"]={
fwav="sine",
adsr={
	1,
	0,0,0.4,0.1
},
fm={
	frequency=16,
	fwav="sawtooth",
	ffreq=function(it)
	local f1=note_to_freq("C3")
	local f2=note_to_freq("C4")
	local f3=note_to_freq("C5")
	local t1=0
	local f=function(m,t)
		if m<0 then return f2+((f1-f2)*-m)+t*t1 end
		return f2+((f3-f2)*m)+t*t1
	end
	return f
	end,
}
}

beeps["r"]={
fwav="sine",
adsr={
	1,
	0,0,0.4,0.1
},
fm={
	frequency=16,
	fwav="triangle",
	ffreq=function(it)
	local f1=note_to_freq("C3")
	local f2=note_to_freq("C4")
	local f3=note_to_freq("C5")
	local t1=0
	local f=function(m,t)
		if m<0 then return f2+((f1-f2)*-m)+t*t1 end
		return f2+((f3-f2)*m)+t*t1
	end
	return f
	end,
}
}

beeps["t"]={
fwav="sawtooth",
adsr={
	1,
	0,0,0.4,0.5
},
fm={
	frequency=128,
	fwav="square",
	ffreq=function(it)
	local f1=note_to_freq("C3")
	local f2=note_to_freq("C4")
	local f3=note_to_freq("C5")
	local t1=0
	local f=function(m,t)
		if m<0 then return f2+((f1-f2)*-m)+t*t1 end
		return f2+((f3-f2)*m)+t*t1
	end
	return f
	end,
}
}

beeps["a"]={
fwav="sawtooth",
adsr={
	0.75,
	0.0,0.1,0.4,0.3
},
fm={
	frequency=16,
	fwav="sine",
	duty=0.5,
	ffreq=function(it)
	local f1=note_to_freq("C3")
	local f2=note_to_freq("D3")
	local f3=note_to_freq("E3")
	local t1=0
	local f=function(m,t)
--			return f2+t*t1
		local t2=math.pow(t+2,4)*t1
		if m<0 then return f2+((f1-f2)*-m)+t2 end
		return f2+((f3-f2)*m)+t2
	end
	return f
	end,
},
fread=function(it)
	return function(t)
	it.fm_gwav.set_frequency(2*(t+1))
	end
end
}

beeps["z"]={
fwav="triangle",
frequency="D4",
duty=0.25,
adsr={
	0.7,
	0.2,0.2,0.8,0.2
},
--[[
fm={
	frequency=16,
	fwav="sine",
	duty=0.5,
	ffreq=function(it)
	local f1=bitsynth.C3
	local f2=bitsynth.D3
	local f3=bitsynth.E3
	local t1=0
	local f=function(m,t)
--			return f2+t*t1
--			local t2=math.pow(t+2,4)*t1
--			if m<0 then return f2+((f1-f2)*-m)+t2 end
--			return f2+((f3-f2)*m)+t2
		return bitsynth.D3
	end
	return f
	end,
},
]]
--	fread=function(it)
--	    return function(t)
--		it.fm_gwav.set_frequency(2*(t+1))
--	    end
--	end
}


beeps["x"]={
fwav="sine",
frequency="C5",
volume=1.0,
duty=0.5,
adsr={
	0.7,
	0.2,0.2,0.8,0.2
},
}
beeps["c"]={
fwav="triangle",
frequency="C5",
volume=1.0,
duty=0.5,
adsr={
	0.7,
	0.2,0.2,0.8,0.2
},
}
beeps["v"]={
fwav="sawtooth",
frequency="C5",
volume=0.5,
duty=0.5,
adsr={
	0.7,
	0.2,0.2,0.8,0.2
},
}
beeps["b"]={
fwav="toothsaw",
frequency="C5",
volume=0.5,
duty=0.5,
adsr={
	0.7,
	0.2,0.2,0.8,0.2
},
}
beeps["n"]={
fwav="square",
frequency="C5",
volume=0.5,
duty=0.5,
adsr={
	0.7,
	0.2,0.2,0.8,0.2
},
}

beeps["m"]={
fwav="whitenoise",
frequency="C1",
volume=1.0,
duty=0.5,
adsr={
	0.7,
	0.2,0.2,0.8,0.2
},
}
beeps["k"]={
fwav="square",
frequency="C8",
volume=1.0,
duty=0.5,
adsr={
	0.7,
	0.2,0.2,1.8,0.2
},
fm={
	frequency=2,
	fwav="sine",
	duty=0.25,
	ffreq=function(it)
	local f1=note_to_freq("C3")
	local f2=note_to_freq("C4")
	local f3=note_to_freq("E3")
	local t1=0
	local f=function(m,t)
--			return f2+t*t1
		local t2=t*t1
		if m<0 then return f2+((f1-f2)*-m)+t2 end
		return f2+((f3-f2)*m)+t2
--			return bitsynth.D3
	end
	return f
	end,
},
}

beeps["p"]={
fwav="whitenoise",
frequency="C5",
volume=1.0,
duty=0.5,
adsr={
	1.0,
	0.0,0.0,0.0,0.5
},
fm={
	frequency=16,
	fwav="square",
	duty=0.5,
	ffreq=function(it)
	local t1=-35*35
	return function(m,t)
		return note_to_freq("c5")+t*t1
	end
	end,
},
}


40 45 50 55 59 64

]=]


beeps={}

beeps["1"]={

	monophonic=1,

	lua=[=[

note = NOTE or 48
attack_length=ATTACK or 0.5
decay_length=DECAY or 10
sustain_length=SUSTAIN or 2
release_length=RELEASE or 0.5
filter_length=FILTER or 0.01

adsr,sustain_off=env_adsr(env)

filter=gene_filter( filter_length , function(get,box)
	return	( get( 0                     ) * 1.0  ) +
			( get( 0.5   * filter_length ) * 0.5  ) +
			( get( 1.0   * filter_length ) * 0.25 )
end)

sin=gene_sin(note)
square=gene_square(note)
triangle=gene_triangle(note)
sawtooth=gene_sawtooth(note)
toothsaw=gene_toothsaw(note)
whitenoise=gene_whitenoise(note,256)

wave_base = function() return whitenoise(time,sin(time,vib_note_sweep(12,-36,1))*3) end

vib_note_sweep= function( a , b , len )
	return b + ( (a-b) * (gene_release( 0 , len ))(time) )
end

time=0 -- reset time
sample_next = function()
	time=time+sample_time

	local ret = 0.5 * adsr(time) *
		(
			( wave_base() * 1.0 )
		)
	
	return filter(ret)

end

	]=],
}

beeps["2"]={

	monophonic=1,

	lua=[=[

note = NOTE or 72
attack_length=ATTACK or 0.5
decay_length=DECAY or 10
sustain_length=SUSTAIN or 2
release_length=RELEASE or 0.1
filter_length=FILTER or 0.001

adsr,sustain_off=env_adsr(env)

filter=gene_filter( filter_length , function(get,box)
	return	(box(filter_length)+get(0))/2
end)

sin=gene_sin(note)
square=gene_square(note)
triangle=gene_triangle(note)
sawtooth=gene_sawtooth(note)
toothsaw=gene_toothsaw(note)
whitenoise=gene_whitenoise(note,256)

wave_decay=gene_decay(1)
wave_base = function(n,s)
	return wave_decay( time*s ) *
	(
		(
			sin( time , n+note+gene_decay(0.125)(time)*4 )
		) * 1
	) 
end

time=0 -- reset time
sample_next = function()
	time=time+sample_time

	local ret = 0.5 * adsr(time) *
		(
			  wave_base(  0 , 1 )
--			+ wave_base( 12 , 2 )
--			+ wave_base( 19 , 3 )
--			+ wave_base( 24 , 4 )
--			+ wave_base( 28 , 5 )
--			+ wave_base( 31 , 6 )
--			+ wave_base( 34 , 7 )
--			+ wave_base( 36 , 8 )
		)
	
	return filter(ret)

end

	]=],
}

beeps["3"]={

	monophonic=1,

	lua=[=[

note = NOTE or 48
attack_length=ATTACK or 0.1
decay_length=DECAY or 10
sustain_length=SUSTAIN or 2
release_length=RELEASE or 0.2
filter_length=FILTER or 0.25

adsr,sustain_off=env_adsr(env)

filter=gene_filter( filter_length , function(get,box)
	return	( get( 0                     ) * 1.0  ) +
			( get( 0.5   * filter_length ) * 0.5  ) +
			( get( 1.0   * filter_length ) * 0.25 )
end)

sin=gene_sin(note)
square=gene_square(note)
triangle=gene_triangle(note)
sawtooth=gene_sawtooth(note)
toothsaw=gene_toothsaw(note)
whitenoise=gene_whitenoise(note,256)

wave_decay=gene_decay(1.0)
wave_base = function()
	return
	  wave_decay( time*1 ) * sin( time*1 , note )
	+ wave_decay( time*2 ) * sin( time*2 , note )
	+ wave_decay( time*3 ) * sin( time*3 , note )
	+ wave_decay( time*4 ) * sin( time*4 , note )
	+ wave_decay( time*5 ) * sin( time*5 , note )
	+ wave_decay( time*6 ) * sin( time*6 , note )
	+ wave_decay( time*7 ) * sin( time*7 , note )
	+ wave_decay( time*8 ) * sin( time*8 , note )
--	+ wave_decay( time*16 ) * square( time*16 , note )
end

time=0 -- reset time
sample_next = function()
	time=time+sample_time
	return ( 0.5 * adsr(time) * wave_base() )
end

	]=],
}

oven.opts.fun="" -- back to menu on reset
hardware,main=system.configurator({
	mode="fun64", -- select the standard 320x240 screen using the swanky32 palette.
	update=function() update() end, -- called repeatedly to update+draw
	msg=function(m) msg(m) end, -- handle msgs
})

local wstr=require("wetgenes.string")


-- we will call this once in the update function
setup=function()

--    system.components.screen.bloom=0
--    system.components.screen.filter=nil
--    system.components.screen.shadow=nil

    local csfx=system.components.sfx

    for n,v in pairs(beeps) do 
		v.name=n
--		csfx.render(v)
    end

    print("Setup complete!")

end


-- handle raw key press
msg=function(m)
    if m.class=="key" then
--	print(m.keyname,m.action,m.ascii)
	if m.action==1 then
	    local csfx=system.components.sfx
	    local s=beeps[m.keyname]
	    if s then
PRINT(s.name)
			csfx.stream_sound(s)
	    end
	end
    end
end

-- updates are run at 60fps
update=function()
    
    if setup then setup() setup=nil end

    local cmap=system.components.map
    local ctext=system.components.text
    local bg=9
    local fg=31

    cmap.text_clear(0x01000000*bg) -- clear text forcing a background color
	

    local tx=wstr.trim([[

Hit a key to play a sound!

]])

    local tl=wstr.smart_wrap(tx,cmap.text_hx-2)
    for i=1,#tl do
	    local t=tl[i]
	    cmap.text_print(t,1,16+i,fg,bg)
    end


end
