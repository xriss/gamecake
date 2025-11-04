
import util from "util";
import { promises as pfs } from 'fs';

import djon from "./djon.js";

let opts={}
for( let v of (process.argv.slice(2)) )
{
	let vp=v
	if(opts.skip_opts) { vp=null } // skip all opts

	if      (vp=="--")         {		opts.skip_opts=true		}
	else if (vp=="--djon")     {		opts.djon=true			}
	else if (vp=="--json")     {		opts.djon=false			}
	else if (vp=="--compact")  {		opts.compact=true		}
	else if (vp=="--strict")   {		opts.strict=true		}
	else if (vp=="--comments") {		opts.comments=true		}
	else if (vp=="--help")     {		opts.help=true			}
	else
	{
		if( vp && vp.slice(0,2)=="--" ) 
		{
			console.log( "unknown option "+v )
			process.exit(20)
		}
		else if(!opts.fname1) { opts.fname1=v }
		else if(!opts.fname2) { opts.fname2=v }
		else
		{
			console.log( "unknown option "+v )
			process.exit(20)
		}
	}
}

if(opts.help)
{
console.log(`

js/djon.sh input.filename output.filename

	If no output.filename then write to stdout
	If no input.filename then read from stdin
	Input is always parsed as djon (which can be valid json)

Possible options are:

	--djon     : output djon format
	--json     : output json format
	--compact  : compact output format
	--strict   : require input to be valid json ( bitch mode )
	--comments : comments array to djon or djon to comments array
	--         : stop parsing options

The comments flag implies that you are converting between a
json array format [value,comment,...] values and djon.
So it applies to input if writing djon and output if writing json.

`)

process.exit(0)

}

let data_input 
if(opts.fname1)
{
	data_input = await pfs.readFile( opts.fname1 , 'utf-8' )
}
else
{
	let readstdin = async function()
	{
		const chunks = [];
		for await (const chunk of process.stdin) { chunks.push(chunk); }
		return Buffer.concat(chunks).toString('utf8');
	}
	data_input=await readstdin()
}

let flag_djon=opts.djon?"djon":""
let flag_compact=opts.compact?"compact":""
let flag_strict=opts.strict?"strict":""

let flag_comments="comments"
if( opts.djon && opts.comments ) // input is in special array comments format
{
	flag_comments=""
}
let tree=djon.load(data_input,flag_strict,flag_comments)
if( !opts.djon && opts.comments ) // output in special array comments format
{
	flag_comments=""
}
if( opts.djon && opts.comments ) // input is in special array comments format
{
	flag_comments="comments"
}
let data_output=djon.save(tree,flag_djon,flag_compact,flag_strict,flag_comments)

if( opts.fname2 )
{
	await pfs.writeFile( opts.fname2 , data_output , 'utf8' )
}
else
{
	await process.stdout.write( data_output , 'utf8' )
}

//process.exit(0)
