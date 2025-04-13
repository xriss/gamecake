#!/usr/local/bin/gamecake

-- copy all globals into locals, some locals are prefixed with a G to reduce name clashes
local coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,Gload,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require=coroutine,package,string,table,math,io,os,debug,assert,dofile,error,_G,getfenv,getmetatable,ipairs,load,loadfile,loadstring,next,pairs,pcall,print,rawequal,rawget,rawset,select,setfenv,setmetatable,tonumber,tostring,type,unpack,_VERSION,xpcall,module,require

-- setup some default search paths,
local apps=require("apps")
apps.default_paths()

local pack=require("wetgenes.pack")
local wwin=require("wetgenes.win")
local wxox=require("wetgenes.xox")
local wstr=require("wetgenes.string")
local tardis=require("wetgenes.tardis")	-- matrix/vector math
local wpath=require("wetgenes.path")

local function dprint(a) print(wstr.dump(a)) end

--module
local M={ modname=(...) } ; package.loaded[M.modname]=M

M.bake=function(oven,main)
	main=main or {}
	main.modname=M.modname
	
	oven.modname="swanky.edit"

	local gl=oven.gl
	local cake=oven.cake
	local sheets=cake.sheets
	local opts=oven.opts
	local canvas=cake.canvas
	local views=cake.views
	local font=canvas.font
	local flat=canvas.flat

	local view=views.create({
		parent=views.get(),
		mode="full",
		vx=oven.win.width,
		vy=oven.win.height,
		vz=8192,
		fov=0,
	})

--	local skeys=oven.rebake("wetgenes.gamecake.spew.keys")
--	local srecaps=oven.rebake("wetgenes.gamecake.spew.recaps")
--	skeys.setup({max_up=1}) -- also calls srecaps.setup
--	skeys.set_opts("yestyping",true) -- disable widget moves with keys/joystick



--	local beep=oven.rebake(oven.modname..".beep")
	local gui=oven.rebake(oven.modname..".gui")
	local cmd=oven.rebake(oven.modname..".cmd")
	local docs=oven.rebake(oven.modname..".docs")
	local finds=oven.rebake(oven.modname..".finds")

	local show=oven.rebake(oven.modname..".show")

	main.loads=function()

		oven.cake.fonts.loads({1,4}) -- load 1st builtin font, a basic 8x8 font

		gui.loads()

	end
			
	main.setup=function()

		main.loads(oven)
		
		show.setup()

		gui.setup()
		
local ipsum=[[

But I must explain to you how all this mistaken idea of denouncing 
pleasure and praising pain was born and I will give you a complete 
account of the system, and expound the actual teachings of the great 
explorer of the truth, the master-builder of human happiness. No one 
rejects, dislikes, or avoids pleasure itself, because it is pleasure, 
but because those who do not know how to pursue pleasure rationally 
encounter consequences that are extremely painful. Nor again is there 
anyone who loves or pursues or desires to obtain pain of itself, 
because it is pain, but because occasionally circumstances occur in 
which toil and pain can procure him some great pleasure. To take a 
trivial example, which of us ever undertakes laborious physical 
exercise, except to obtain some advantage from it? But who has any 
right to find fault with a man who chooses to enjoy a pleasure that has 
no annoying consequences, or one who avoids a pain that produces no 
resultant pleasure?

Pero debo explicarte cómo toda esta idea equivocada de denunciar nació 
el placer y el elogio del dolor y te daré un completo cuenta del 
sistema, y exponga las enseñanzas reales de los grandes explorador de 
la verdad, el maestro constructor de la felicidad humana. Ninguno 
rechaza, no le gusta o evita el placer mismo, porque es placer, sino 
porque aquellos que no saben buscar el placer racionalmente encontrar 
consecuencias que son extremadamente dolorosas. Tampoco hay de nuevo 
cualquiera que ama o persigue o desea obtener dolor de sí mismo, porque 
es dolor, pero porque ocasionalmente ocurren circunstancias en que 
trabajo y dolor pueden proporcionarle un gran placer. Para tomar un 
ejemplo trivial, ¿quién de nosotros emprende alguna vez un trabajo 
físico laborioso? ejercicio, excepto para obtener alguna ventaja de él? 
Pero quien tiene derecho a encontrar fallas en un hombre que elige 
disfrutar de un placer que tiene sin consecuencias molestas, o alguien 
que evita un dolor que no produce placer resultante?


Mais je dois vous expliquer comment toute cette idée erronée de 
dénonciation le plaisir et la louange de la douleur était né et je vais 
vous donner une complète compte du système, et exposer les 
enseignements actuels du grand explorateur de la vérité, maître d'œuvre 
du bonheur humain. Personne rejette, n'aime pas ou évite le plaisir 
lui-même, parce que c'est un plaisir, mais parce que ceux qui ne savent 
pas rechercher le plaisir rationnellement faire face à des conséquences 
extrêmement douloureuses. Ni encore est-il toute personne qui aime ou 
poursuit ou désire obtenir la douleur d'elle-même, parce que c'est la 
douleur, mais parce que parfois des circonstances surviennent dans le 
travail et la douleur peuvent lui procurer un grand plaisir. Prendre un 
exemple trivial, lequel d'entre nous entreprend jamais physique 
laborieuse exercice, sauf pour en tirer un avantage? Mais qui a tout le 
droit de trouver à redire à un homme qui choisit de jouir d'un plaisir 
qui a pas de conséquences ennuyeuses, ou qui évite une douleur qui ne 
produit pas plaisir résultant?


Aber ich muss Ihnen erklären, wie all diese falsche Vorstellung, etwas 
anzuprangern Vergnügen und lobender Schmerz wurden geboren und ich 
werde dir ein vollständiges geben Rechenschaft über das System ab und 
erläutern Sie die tatsächlichen Lehren der Großen Erforscher der 
Wahrheit, der Baumeister des menschlichen Glücks. Niemand lehnt ab, 
lehnt ab oder vermeidet das Vergnügen selbst, weil es das Vergnügen 
ist, sondern weil diejenigen, die nicht wissen, wie man Vergnügen 
rational verfolgt Konsequenzen haben, die äußerst schmerzhaft sind. 
Noch ist wieder da wer liebt oder verfolgt oder Schmerz von sich selbst 
erlangen will, weil es schmerzen sind, aber weil gelegentlich umstände 
in Welche Mühe und welcher Schmerz können ihm ein großes Vergnügen 
bereiten? Etwas nehmen triviales Beispiel, das von uns jemals mühsame 
körperliche unternimmt Übung, außer um einen Vorteil daraus zu ziehen? 
Aber wer hat welche? Recht, einen Mann zu bemängeln, der sich dafür 
entscheidet, ein Vergnügen zu genießen, das er hat Keine lästigen 
Konsequenzen oder einer, der einen Schmerz vermeidet, der keine erzeugt 
resultierendes Vergnügen?

]]
		
		cmd.start()
		
		local loaded=false
		if cmd.args then

			local fname=cmd.args.data[1]
			if fname then
				local path=wpath.resolve(fname)
				docs.manifest(path):show()
				loaded=true
			end

		end
		if not loaded then
			docs.manifest():show()
--			gui.master.ids.texteditor.txt.set_text(string.rep(ipsum,4),"")
--			gui.master.ids.texteditor.txt.set_text("\n","")
--			gui.master.ids.texteditor.txt.set_lexer()
		end

		
		gui.master.set_focus( gui.master.ids.texteditor.scroll_widget.pan )
		gui.refresh_tree()

	end


	main.clean=function()

	end

	main.msg=function(m)

		view.msg(m) -- fix mouse coords

--		if skeys.msg(m) then m.skeys=true end -- flag this msg as handled by skeys

		gui.msg(m)

--dprint(m)

	end



	main.update=function()

--		srecaps.step()
		
		docs.update()

		gui.update()
		
		show.update()
		

		if not gui.master.focus  then -- auto focus text editor when no other focus
			gui.master.set_focus( gui.master.ids.texteditor.scroll_widget.pan )
		end
		gui.master.ids.runfbo:set_dirty()


	end

	main.draw=function()
	
		
--  we want the main view to track the window size

		oven.win:info()
		view.vx=oven.win.width
		view.vy=oven.win.height
		view.vz=8192
		
		views.push_and_apply(view)
		canvas.gl_default() -- reset gl state
			
		gl.ClearColor(pack.argb4_pmf4(0xf000))
		gl.Clear(gl.COLOR_BUFFER_BIT+gl.DEPTH_BUFFER_BIT)

		gl.PushMatrix()
		
		font.set(cake.fonts.get(4)) -- default font
		font.set_size(16,0)
		
		canvas.gl_default() -- reset gl state

		
		gl.Translate(view.vx*0.5,view.vy*0.9,view.vz*0.0) -- top left corner is origin

--[[
		local ss=view.vy*0.125*gui.datas.get_value("zoom")*0.01
		gl.Scale(ss,ss,ss)
		gl.Rotate(90,1,0,0)
		gl.Rotate( -gui.datas.get_value("rotate") ,0,0,1)
]]		

--		beep.draw()

		gl.PopMatrix()

		gui.draw()

		views.pop_and_apply()
		
	end

	return main
end

