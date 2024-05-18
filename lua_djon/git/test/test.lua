

local djon=require("djon")

local dump;dump=function(it,idnt)
	idnt=idnt or ""
	local first=true
	local write_idnt=function()
		if first then first=false return end
		io.write(idnt)
	end
	if type(it)=="table" then
		write_idnt()
		io.write("{\n")
		for k,v in pairs(it) do
			write_idnt()
			io.write(" ")
			io.write( k .. " = " )
			dump(v,idnt.." ")
		end
		write_idnt()
		io.write("}\n")
	else
		write_idnt()
		io.write( tostring(it) )
		io.write("\n")
	end
end


local tab=djon.load([[ { top:"43" , not:null , boo:true , aaa:[1,2,3,4] , bob:"bob" } ]])

print()
print( djon.save({"OK"}) )
print()

dump( tab )
print()
print( djon.save(tab) )
print()
print( djon.save(tab,"djon") )
print()
print( djon.save(tab,"compact") )
print()
print( djon.save(tab,"djon","compact") )
print()
