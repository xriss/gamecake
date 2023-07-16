

local chatdown=require("wetgenes.gamecake.fun.chatdown")

oven.opts.fun="" -- back to menu on reset
hardware,main=system.configurator({
	mode="swordstone", -- select the standard 320x240 screen using the swanky32 palette.
	update=function() update() end, -- called repeatedly to update+draw
})

-- debug text dump
local ls=function(t) print(require("wetgenes.string").dump(t)) end



local chat_text=[[

#example

	=title Example Fun64 programs.


<welcome

	>cat_hello

		Hello World!
		
	>cat_game
	
		Games

	>cat_toy
	
		Toys

	>cat_shader
	
		GLES 2.0 Shaders

<cat_hello

	>welcome
		
		..

	>fun_picish
	
		=run hellopicish
	
		Picish hello.


<cat_game

	>welcome
		
		..

<cat_toy

	>welcome
		
		..

<cat_shader

	>welcome
		
		..


]]


-----------------------------------------------------------------------------
--[[#setup_menu

	menu = setup_menu()

Create a displayable and controllable menu system that can be fed chat 
data for user display.

After setup, provide it with menu items to display using 
menu.show(items) then call update and draw each frame.


]]
-----------------------------------------------------------------------------
function setup_menu(chats)

	if type(chats)=="string" then chats=chatdown.setup_chats(chats) end
	
	local wstr=require("wetgenes.string")

	local menu={}

	menu.stack={}

	menu.width=(system.components.screen.hx/4)
	menu.cursor=0
	menu.cx=math.floor(((system.components.screen.hx/4)-menu.width)/2)
	menu.cy=0
	
	function menu.show(items,subject_name,topic_name)
	
		if subject_name and topic_name then

			local chat=chats:get_subject(subject_name)
			chat:set_topic(topic_name)
			items=menu.chat_to_menu_items(chat)

		elseif subject_name then

			local chat=chats:get_subject(subject_name)
			items=menu.chat_to_menu_items(chat)

		end

		if items.call then items.call(items,menu) end -- refresh
		
		menu.items=items
		menu.cursor=items.cursor or 1
		
		menu.lines={}
		for idx=1,#items do
			local item=items[idx]
			local text=item.text
			if text then
				local ls=wstr.smart_wrap(text,menu.width-8)
				if #ls==0 then ls={""} end -- blank line
				for i=1,#ls do
					local prefix=""--(i>1 and " " or "")
					if item.cursor then prefix=" " end -- indent decisions
					menu.lines[#menu.lines+1]={s=prefix..ls[i],idx=idx,item=item,cursor=item.cursor,color=item.color}
				end
			end
		end

	end


	
	menu.update=function()
	
		if not menu.items then return end

		local bfire,bup,bdown,bleft,bright
		
		for i=0,5 do -- any player, press a button, to control menu
			local up=ups(i)
			if up then
				bfire =bfire  or up.button("fire_clr")
				bup   =bup    or up.button("up_set")
				bdown =bdown  or up.button("down_set")
				bleft =bleft  or up.button("left_set")
				bright=bright or up.button("right_set")
			end
		end
		

		if bfire then

			for i,item in ipairs(menu.items) do
			
				if item.cursor==menu.cursor then
			
					if item.call then -- do this
					
						item.call( item , menu )
											
					end
					
					break
				end
			end
		end
		
		if bleft or bup then
		
			menu.cursor=menu.cursor-1
			if menu.cursor<1 then menu.cursor=menu.items.cursor_max end

		end
		
		if bright or bdown then
			
			menu.cursor=menu.cursor+1
			if menu.cursor>menu.items.cursor_max then menu.cursor=1 end
		
		end
		
		local run=chats:get_tag("run")
		
		if run and run~="" then
			oven.opts.fun="lua/fun/"..run
			oven.next="wetgenes.gamecake.fun.main"
		end
	
	end
	
	menu.chat_to_menu_items=function(chat)
		local items={cursor=1,cursor_max=0}
		
		items.title=chat:get_tag("title")
		items.portrait=chat:get_tag("portrait")
		
		local ss=chat.topic and chat.topic.text or {} if type(ss)=="string" then ss={ss} end
		for i,v in ipairs(ss) do
			if i>1 then
				items[#items+1]={text="",chat=chat} -- blank line
			end
			items[#items+1]={text=chat:replace_tags(v)or"",chat=chat}
		end

		for i,v in ipairs(chat.gotos or {}) do

			items[#items+1]={text="",chat=chat} -- blank line before each goto

			local ss=v and v.text or {} if type(ss)=="string" then ss={ss} end

			local color=30
			if chat.viewed[v.name] then color=28 end -- we have already seen the response to this goto
			
			local f=function(item,menu)

				if item.topic and item.topic.name then

					chats.changes(chat,"topic",item.topic)

					chat:set_topic(item.topic.name)

					chat:set_tags(item.topic.tags)

					menu.show(menu.chat_to_menu_items(chat))

				end
			end
			
			items[#items+1]={text=chat:replace_tags(ss[1])or"",chat=chat,topic=v,cursor=i,call=f,color=color} -- only show first line
			items.cursor_max=i
		end

		return items
	end

	menu.draw=function()
	
		local fg1=30
		local fg0=31
		local bg1=8
		local bg0=9

		local tprint=system.components.text.text_print
		local tgrd=system.components.text.tilemap_grd

		if not menu.lines then return end
		
		menu.cy=math.floor(((system.components.screen.hy/8)-(#menu.lines+4))/2)
		
		tgrd:clip(menu.cx,menu.cy,0,menu.width,#menu.lines+4,1):clear(0x01000000*bg1)
		tgrd:clip(menu.cx+2,menu.cy+1,0,menu.width-4,#menu.lines+4-2,1):clear(0x01000000*bg0)
		
		if menu.items.title then
			local title=" "..(menu.items.title).." "
			local wo2=math.floor(#title/2)
			tprint(title,menu.cx+(menu.width/2)-wo2,menu.cy+0,fg0,bg1)
		end
		
		for i,v in ipairs(menu.lines) do
			tprint(v.s,menu.cx+4,menu.cy+i+1,v.color or fg0,bg0)
		end
		
		local it=nil
		for i=1,#menu.lines do
			if it~=menu.lines[i].item then -- first line only
				it=menu.lines[i].item
				if it.cursor == menu.cursor then
					tprint(">",menu.cx+4,menu.cy+i+1,fg0,bg0)
				end
			end
		end

		system.components.text.dirty(true)

	end
	
	return menu
end


-----------------------------------------------------------------------------
--[[#update

	update()

Update and draw loop, called every frame.

]]
-----------------------------------------------------------------------------
update=function()

	if not setup_done then
		menu=setup_menu( chat_text )
		menu.show(nil,"example","welcome")
		setup_done=true
	end
	
	menu.update()
	menu.draw()
	
end
