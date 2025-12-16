
local djon=require("djon")

print("roundtrip data example")

local ls=function(it)
	local f;f=function(it)
		if type(it)=="table" then
			local o={}
			o[#o+1]="{\n"
			for n,v in pairs(it) do
				o[#o+1]=n.."="..f(v)
			end
			o[#o+1]="}\n"
			return table.concat(o)
		else
			return tostring(it).."\n"
		end
	end
	print(f(it))
end

-- normal json data structure but no comments
local data=djon.load_file("test.djon")

-- can modify data here
data.this_is_new="not old"
data.show="old comments are kept even if we change data type"

-- save djon with comments ( loaded from output file )
djon.save_comments("test.djon",data)
