
local require=require

local ipairs=ipairs

module(...)

function build(tab)

	local bake=require("wetgenes.bake")
	local bakejs=require("wetgenes.bake.js")

	-- where we are building from
	bake.cd_base	=	bake.cd_base or bake.get_cd()
	-- where we are building to
	bake.cd_out		=	bake.cd_out or 'out'

	bake.files_min_js=bake.files_min_js or {}
	bake.files_min_js[ #bake.files_min_js + 1 ]="gamecake"

	local files_gamecake={}
	local r=bake.findfiles{basedir="../gamecake/src",dir="",filter="%.js$"}
	for i,v in ipairs(r.ret) do
		files_gamecake[#files_gamecake+1]=v
		bake.create_dir_for_file(bake.cd_out.."/js/gamecake/"..v)
	end

	bake.files_pp=bake.files_pp or {}
	for i,v in ipairs(files_gamecake) do
		bake.files_pp[ #bake.files_pp +1]={ "../../js/gamecake/src"..v , bake.cd_out..'/js/gamecake'..v }
	end

-- the main action happens here
	bakejs.build(tab)

end
