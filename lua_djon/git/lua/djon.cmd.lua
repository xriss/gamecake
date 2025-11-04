

local djon=require("djon")

local opts={}

for _,v in ipairs({...}) do

	local vp=v
	if opts.skip_opts then vp=nil end -- skip all opts

	if     vp=="--"         then		opts.skip_opts=true
	elseif vp=="--djon"     then		opts.djon=true
	elseif vp=="--json"     then		opts.djon=false
	elseif vp=="--compact"  then		opts.compact=true
	elseif vp=="--strict"   then		opts.strict=true
	elseif vp=="--comments" then		opts.comments=true
	elseif vp=="--help"     then		opts.help=true
	else
		if vp and vp:sub(1,2)=="--" then
			print( "unknown option "..v )
			os.exit(20)
		elseif not opts.fname1 then opts.fname1=v
		elseif not opts.fname2 then opts.fname2=v
		else
			print( "unknown option "..v )
			os.exit(20)
		end
	end

end

if opts.help then

print([[

lua/djon.sh input.filename output.filename

	If no output.filename then write to stdout
	If no input.filename then read from stdin
	Input is always parsed as djon (which can be valid json)

Possible options are:

	--djon     : output djon format
	--json     : output json format (default)
	--compact  : compact output format
	--strict   : require input to be valid json ( bitch mode )
	--comments : comments array to djon or djon to comments array
	--         : stop parsing options

The comments flag implies that you are converting between a
json array format [value,comment,...] values and djon.
So it applies to input if writing djon and output if writing json.

]])

os.exit(0)

end


local data_input=""

if opts.fname1 then
	local fp=assert(io.open(opts.fname1,"rb"))
	data_input=fp:read("*all")
	fp:close()
else
	data_input=io.read("*all")
end

local flags={}
if opts.strict then flags[#flags+1]="strict" end
if ( not opts.djon and opts.comments ) or not opts.comments then flags[#flags+1]="comments" end
local data_tab=djon.load(data_input,unpack(flags))


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
			io.write( tostring(k) .. " = " )
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
--dump(data_tab)


local flags={}
if opts.djon then flags[#flags+1]="djon" end
if opts.strict then flags[#flags+1]="strict" end
if opts.compact then flags[#flags+1]="compact" end
if ( opts.djon and opts.comments ) or not opts.comments then flags[#flags+1]="comments" end
local data_output=djon.save(data_tab,unpack(flags))

if opts.fname2 then
	local fp=assert(io.open(opts.fname2,"wb"))
	data_input=fp:write(data_output)
	fp:close()
else
	data_input=io.write(data_output)
end

os.exit(0)
