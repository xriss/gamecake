

local djon=require("djon")


local tab=djon.load([[ { top:"43" , not:null , boo:true , aaa:[1,2,3,4] , bob:"bob" } ]])

print( djon.save(tab) )
