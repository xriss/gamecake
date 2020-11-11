
// one global function to call and assume we have $=jquery

gamecake_loader=function(opts)
{
	opts=opts || {};
	opts.div=opts.div || "#gamecake"; // where to put it
	opts.dir=opts.dir || "http://play.4lfa.com/gamecake/"; // where to load stuff from
	
	opts.cakefile=opts.cakefile || "game.cake"; // http location of cakefile
	
//	opts.force_emscripten=true; // Looks like VHS has won, again, so lets just focus on that.
	
	var gamecake={};
	
	gamecake.msg_hook=opts.msg_hook; // a function to handle msgs
	gamecake.loaded_hook=opts.loaded_hook; // a function to call after everything has loaded




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

	gamecake.msg=function(m,d) {

		if("string" == typeof m) // string to decode
		{
			m=gamecake.str_to_msg( m.replace(/^\s+|\s+$/g,'') );
		}

		if(m.cmd=="print") // basic print command (we cant do this from within nacl)
		{
			console.log(d);
		}
		else
		if(m.cmd=="loading") // loading progress
		{
			gamecake.progress_bar.attr("value",0.5+(0.5*Number(m.progress)/Number(m.total)));
			if(m.progress==m.total)
			{
				gamecake.progress_bar.hide();
				gamecake.progress_about.hide();
				if( gamecake.loaded_hook )
				{
					gamecake.loaded_hook();
				}
			}
		}
		else
		{
			if(gamecake.msg_hook) // pass on
			{
				gamecake.msg_hook(m,d);
			}
		}
	};

// Handle a message coming from the NaCl module.
	gamecake.nacl_msg=function(message_event) {
		if( typeof(message_event.data)=='string' )
		{
			var s=message_event.data;
			var sn=s.indexOf('\n');
			var msg=s.substring(0,sn);
			var dat=s.substring(sn+1);
//			console.log(msg);
			gamecake.msg(msg,dat);
		}
	}

	gamecake.post=function(a,b){
		if(gamecake.engine=="nacl")
		{
			if(b) { gamecake.post_nacl(a+b); } else { gamecake.post_nacl(a); }
		}
	};
	gamecake.post_nacl=function(s){
		return gamecake.canvas[0].postMessage(s);
	};
	
// wait for page load
$(function(){

	var webgl_support=function()
	{ 
		try {
			var canvas = document.createElement( 'canvas' ); 
			return !! window.WebGLRenderingContext && ( canvas.getContext( 'webgl' ) ||
			canvas.getContext( 'experimental-webgl' ) );
		} catch( e ) { return false; }
	};

	var warning="";
	
	if(!webgl_support())
	{
		warning+='<br/>Something is wrong with WebGL on your browser!<br/>Visit <a href="http://get.webgl.org/">http://get.webgl.org/</a> for help.<br/><br/>Maybe Chrome needs you to run with the command line option --disable-gpu-sandbox to work.<br/><br/>';
	}

	var readme='<br/>Please wait, the first load may be a little bit slow.<br/><br/>';
	if(warning!="") { readme=warning; }

	gamecake.div=$(opts.div); // where to put stuff	


	if( (opts.force_emscripten) || ( navigator.mimeTypes['application/x-pnacl'] == undefined) ) // use emscripten
	{
		gamecake.engine="emcc";
		
		gamecake.listener=$('<div style="width:100%;height:100%;position:relative; background-color:#000;"></div>'); // Main container of stuff
		gamecake.progress_bar=$('<progress value="0" style="width:100%; position:absolute;" title="Loading GameCake"></progress>');
		gamecake.progress_about=$(
			'<div style="font-family:sans-serif; font-size:2em; color:#fff; line-height:1.5em; text-align:center; width:66%; margin:auto; ">'+readme+'</div>');
		gamecake.canvas=$('<canvas style=" width:100%; height:100%; position:absolute; " oncontextmenu="event.preventDefault()"></canvas>');
		gamecake.listener.append(gamecake.progress_bar,gamecake.progress_about,gamecake.canvas);

		gamecake.div.empty();
		gamecake.div.append(gamecake.listener);

		if(warning!="") // hide canvas as it is not going to work.
		{
			gamecake.canvas.hide();
		}
	
		var gamecake_start=function() {

//define a callmelater function
				var requestAnimationFrame = (function(){
					return	function(callback,element){
						window.setTimeout(callback, 1000 / 60);
					};
			})();

//initialise lua
			gamecake.post('cmd=lua\n','require("wetgenes.win").emcc_start({})');

// create a pulse function and call it every frame
			var pulse;
			pulse=function() {
				requestAnimationFrame(pulse); // we need to always ask to be called again
				gamecake.post('cmd=lua\n','return gamecake_pulse()');
			};
			requestAnimationFrame(pulse); // start the updates
		}

		var show_progress=function(n)
		{
			window.show_progress_max=window.show_progress_max || 0;
			if(window.show_progress_max<n) { window.show_progress_max=n; }
			var pct=Math.floor(100*(1-(n/window.show_progress_max)));
			console.log("GameCake Loading "+pct+"%");
			gamecake.progress_bar.attr("value",pct/100);
			if(pct==100)
			{
				gamecake.progress_bar.hide();
				gamecake.progress_about.hide();
			}
		};
		var resize=function(){
			var e=gamecake.div[0];
			var w=parseFloat(window.getComputedStyle(e).width);
			var h=parseFloat(window.getComputedStyle(e).height);
			Module.setCanvasSize(w,h);
			gamecake.post('cmd=lua\n','require("wetgenes.win").hardcore.resize(nil,'+w+','+h+')');
		};
		window.addEventListener("resize",resize);
		Module={};
		Module.msg=gamecake.msg;
		Module.canvas=gamecake.canvas[0];
		Module.memoryInitializerPrefixURL=opts.dir;
		Module['_main'] = function() {
			gamecake.post = Module.cwrap('main_post', 'int', ['string','string']);
			gamecake_start();
			resize();
			if( gamecake.loaded_hook )
			{
				gamecake.loaded_hook();
			}
		};
		Module["preInit"] = function() {
			FS.createPreloadedFile('/', "gamecake.zip", opts.cakefile, true, false);
		};
		Module["monitorRunDependencies"]=show_progress;

		var first=document.getElementsByTagName('script')[0];
		var js=document.createElement('script');
		js.src=opts.dir+"gamecake.js";
		first.parentNode.insertBefore(js, first);

		gamecake.div.resize(resize);

	}
	else // use pnacl
	{
		gamecake.engine="nacl";
			
		var nacl_loaded=function(event) {
			gamecake.progress_bar.attr("value",0.5);
			gamecake.post(
				'cmd=lua\n',
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
				gamecake.post('cmd=lua\n',' return require("wetgenes.win").nacl_pulse() ');
			};

			requestAnimationFrame(update); // start the updates
		};
		var nacl_progress=function(event)
		{
			var loadPercent = -1.0;
			if (event.lengthComputable && event.total > 0)
			{
				loadPercent = event.loaded / event.total * 100.0;
			}
			gamecake.progress_bar.attr("value",0.5*(loadPercent/100));
		};
		
		gamecake.listener=$('<div style="width:100%;height:100%;position:relative; background-color:#000;"></div>'); // Main container of stuff
		gamecake.progress_bar=$('<progress value="0" style="width:100%; position:absolute;" title="Loading Cake"></progress>');
		gamecake.progress_about=$(
			'<div style="font-family:sans-serif; font-size:2em; color:#fff; line-height:1.5em; text-align:center; width:66%; margin:auto; ">'+readme+'</div>');
		gamecake.canvas=$('<embed name="nacl_module" id="naclmod" src="'+opts.dir+"gamecake.nmf"+
		'" type="application/x-pnacl" style=" width:100%; height:100%; position:absolute; "/>');
		gamecake.listener.append(gamecake.progress_bar,gamecake.progress_about,gamecake.canvas);

		gamecake.div.empty();
		gamecake.div.append(gamecake.listener);

		gamecake.listener[0].addEventListener('load', nacl_loaded,true);
		gamecake.listener[0].addEventListener('message', gamecake.nacl_msg,true);
		gamecake.listener[0].addEventListener('progress', nacl_progress,true);
		
		if(warning!="") // hide canvas as it is not going to work.
		{
			gamecake.canvas.hide();
		}
	}
	
});


	return gamecake;
}
