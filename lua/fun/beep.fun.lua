--
-- This is fun64 code, you can copy paste it into https://xriss.github.io/fun64/pad/ to run it.
--
oven.opts.fun="" -- back to menu on reset
hardware,main=system.configurator({
	mode="fun64", -- select the standard 320x240 screen using the swanky32 palette.
	update=function() update() end, -- called repeatedly to update+draw
	msg=function(m) msg(m) end, -- handle msgs
})

local wstr=require("wetgenes.string")

beeps={}

-- we will call this once in the update function
setup=function()

--    system.components.screen.bloom=0
--    system.components.screen.filter=nil
--    system.components.screen.shadow=nil
    
    
    local scsfx=system.components.sfx
    local bitsynth=scsfx.bitsynth
    
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
		local f1=bitsynth.note2freq("C4")
		local f2=bitsynth.note2freq("D4")
		local f3=bitsynth.note2freq("E4")
		local t1=0
		local f=function(m,t)
			if m<0 then return f2+((f1-f2)*-m)+t*t1 end
			return f2+((f3-f2)*m)+t*t1
		end
		return f
	    end,
	}
    }

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
		local f1=bitsynth.note2freq("C3")
		local f2=bitsynth.note2freq("D3")
		local f3=bitsynth.note2freq("E3")
		local t1=64
		local f=function(m,t)
			if m<0 then return f2+((f1-f2)*-m)+bitsynth.fwav.square(t*4)*t1 end
			return f2+((f3-f2)*m)+bitsynth.fwav.square(t*4)*t1
		end
		return f
	    end,
	}
    }

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
		local f1=bitsynth.note2freq("C3")
		local f2=bitsynth.note2freq("C4")
		local f3=bitsynth.note2freq("C5")
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
		local f1=bitsynth.note2freq("C3")
		local f2=bitsynth.note2freq("C4")
		local f3=bitsynth.note2freq("C5")
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
		local f1=bitsynth.note2freq("C3")
		local f2=bitsynth.note2freq("C4")
		local f3=bitsynth.note2freq("C5")
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
		local f1=bitsynth.C3
		local f2=bitsynth.D3
		local f3=bitsynth.E3
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
		local f1=bitsynth.C3
		local f2=bitsynth.C4
		local f3=bitsynth.E3
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
		    return bitsynth.c5+t*t1
		end
	    end,
	},
    }


    for n,v in pairs(beeps) do v.name=n scsfx.render(v) end

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
		csfx.play(s.name,1,1.0)
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
