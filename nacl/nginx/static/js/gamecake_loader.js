
// one global function to call and assume we have $=jquery

gamecake_loader=function(opts)
{
	opts=opts || {};
	opts.div=opts.div || "#gamecake"; // where to put it
	opts.nmf=opts.nmf || "http://play.4lfa.com/gamecake/gamecake.nmf"; // location of nmf file
	
	opts.cakefile=opts.cakefile || "/game.cake"; // http location of cakefile

	var gamecake={};

// Indicate success when the NaCl module has loaded.
	gamecake.loaded=function(event) {
		gamecake.progress_salt.attr("value",1);
		gamecake.luasetup();
	}


	gamecake.progress=function(event)
	{
		var loadPercent = -1.0;
		if (event.lengthComputable && event.total > 0)
		{
			loadPercent = event.loaded / event.total * 100.0;
		}
		gamecake.progress_salt.attr("value",loadPercent/100);
	}


	gamecake.msg_dec=function(s)
	{
		var s2="";
		if(s)
		{
			try
			{
				s2=s.split("%26").join("&").split("%3d").join("=").split("%25").join("%");
				return s2;
			}
			catch(e)
			{
				return "";
			}
		}
		return "";
	};


	gamecake.str_to_msg=function(s) // split a query like string
	{
	var i;
		var msg={};
		
		var aa=s.split("&");
		for(i in aa)
		{
		var v=aa[i];
			var va=v.split("=");
			msg[gamecake.msg_dec(va[0])]=gamecake.msg_dec(va[1]);
		}
		
		return msg;
	};

// Handle a message coming from the NaCl module.
	gamecake.msg=function(message_event) {
		if( typeof(message_event.data)=='string' )
		{
			var s=message_event.data;
			var sn=s.indexOf('\n');
			var msg=s.substring(0,sn);
			var dat=s.substring(sn+1);
//			console.log(msg);
			var m=gamecake.str_to_msg(msg);
			if(m.cmd=="print") // basic print command
			{
				console.log(dat);
			}
			else
			if(m.cmd=="loading") // loading progress
			{
				gamecake.progress_cake.attr("value",Number(m.progress)/Number(m.total));
				if(m.progress==m.total)
				{
					gamecake.progress_salt.hide();
					gamecake.progress_cake.hide();
				}
			}
			else
			{
				console.log(s);
			}
		}
	}


	gamecake.luasetup=function() {
		gamecake.object.postMessage(
			'cmd=lua\n'+
			'local win=require("wetgenes.win")\n'+
			'return win.\n'+
			'nacl_start({\n'+
			'zips={"'+opts.cakefile+'"},progress=function(t,p)\n'+
			'win.js_post("cmd=loading&total="..t.."&progress="..p.."\\n") end\n'+
			'})\n');

		var requestAnimationFrame = function(callback,element){
			window.setTimeout(callback, 1000 / 60);
		};

		var update; update=function() {
			requestAnimationFrame(update); // we need to always ask to be called again
			gamecake.object.postMessage('cmd=lua\n return require("wetgenes.win").nacl_pulse() ');
		};

		requestAnimationFrame(update); // start the updates
	}

// wait for page load
$(function(){

	gamecake.div=$(opts.div); // where to put stuff

	gamecake.listener=$('<div style="width:100%;height:100%;"></div>'); // Main container of stuff
	gamecake.progress_salt=$('<progress value="0" style="width:50%" title="Loading Salt"></progress>');
	gamecake.progress_cake=$('<progress value="0" style="width:50%" title="Loading Cake"></progress>');
	gamecake.salt=$('<embed name="nacl_module" id="naclmod" src="'+opts.nmf+
	'" type="application/x-pnacl" style=" width:100%; height:100%; position:relative; "/>');
	gamecake.listener.append(gamecake.progress_salt,gamecake.progress_cake,gamecake.salt);

	gamecake.div.empty();
	gamecake.div.append(gamecake.listener);

	gamecake.object = gamecake.salt[0];  // Global application object.

	gamecake.listener[0].addEventListener('load', gamecake.loaded,true);
	gamecake.listener[0].addEventListener('message', gamecake.msg,true);
	gamecake.listener[0].addEventListener('progress', gamecake.progress,true);
	
});


	return gamecake;
}
