
// one global function to call

gamecake_loader=function(opts)
{
	opts=opts || {};
	opts.div=opts.div || "#gamecake"; // where to put it
	opts.term=opts.term || "#terminal"; // where to log
	opts.dir=opts.dir || "./"; // where to load stuff from
	
//	opts.cakefile=opts.cakefile || "game.cake"; // http location of cakefile
	
	var gamecake={};
	
	gamecake.msg_hook=opts.msg_hook; // a function to handle msgs
	gamecake.loaded_hook=opts.loaded_hook; // a function to call after everything has loaded

	var template_element=function(html)
	{
		var template = document.createElement('template');
		template.innerHTML = html.trim();
		return template.content.firstChild;
	}

	var log=(function () {
		return function (...args) {
			console.log(...args)
			if( opts.terminal )
			{
				var logger = document.querySelector( opts.term );
				if( logger )
				{
					for (var i = 0; i < arguments.length; i++) {
						if (typeof arguments[i] == 'object') {
							s=(JSON && JSON.stringify ? JSON.stringify(arguments[i], undefined, 2) : arguments[i]) + "\n";
						} else {
							s=arguments[i] + "\n";
						}
					}
					
					var sticky=( logger.scrollTop >= (logger.scrollHeight-logger.clientHeight) );
					
					var t=logger.innerHTML || "";
					if(t.length>4096) { t=t.slice(-4096); } // limit
					logger.innerHTML=t+s;

					if(sticky)
					{
						logger.scrollTop=(logger.scrollHeight-logger.clientHeight);
					}
				}
			}
		}
	})();

	gamecake.post=function(a,b){}

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
			log(d);
		}
		else
		if(m.cmd=="loading") // loading progress
		{
			gamecake.progress_bar.setAttribute("value",0.5+(0.5*Number(m.progress)/Number(m.total)));
			if(m.progress==m.total)
			{
				gamecake.progress_bar.style.display = "none";
				gamecake.progress_about.style.display = "none";
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

	var webgl_support=function()
	{ 
		try {
			var canvas = document.createElement( 'canvas' ); 
			return !! window.WebGLRenderingContext && ( canvas.getContext( 'webgl' ) ||
			canvas.getContext( 'experimental-webgl' ) );
		} catch( e ) { return false; }
	};


	var sharedarraybuffer_support=function()
	{
		try {
			return SharedArrayBuffer && true;
		} catch( e ) { return false; }
	};


	var warning="";
	
	if(!sharedarraybuffer_support())
	{
		warning+='<br/>SharedArrayBuffer is missing! <br/> Make sure you are using a https url. <br/> <a href="https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/SharedArrayBuffer/Planned_changes">Would you like to know more?</a><br/><br/>';
	}

	if(!webgl_support())
	{
		warning+='<br/>WebGL is missing!<br/>Visit <a href="http://get.webgl.org/">http://get.webgl.org/</a> for help.<br/><br/>Maybe Chrome needs you to run with the command line option --disable-gpu-sandbox to work.<br/><br/>';
	}

	var readme='<br/>Please wait, the first load may be a little bit slow.<br/><br/>';
	if(warning!="") { readme=warning; }

	gamecake.div=document.querySelector(opts.div); // where to put stuff	


	gamecake.engine="emcc";
	
	gamecake.listener=template_element('<div id="gamecake_listener" style="width:100%;height:100%;position:relative; background-color:#000;"></div>'); // Main container of stuff
	gamecake.progress_bar=template_element('<progress value="0" style="width:100%; position:absolute;" title="Loading GameCake"></progress>');
	gamecake.progress_about=template_element(
		'<div style="font-family:sans-serif; font-size:2em; color:#fff; line-height:1.5em; text-align:center; width:66%; margin:auto; ">'+readme+'</div>');
	gamecake.canvas=template_element('<canvas onclick="this.focus();" tabindex="-1" id="canvas" style=" width:100%; height:100%; position:absolute; " oncontextmenu="event.preventDefault()"></canvas>');

	gamecake.listener.appendChild(gamecake.progress_bar)
	gamecake.listener.appendChild(gamecake.progress_about)
	gamecake.listener.appendChild(gamecake.canvas)

	gamecake.div.innerHTML=""
	gamecake.div.appendChild(gamecake.listener);

	if(warning!="") // hide canvas as it is not going to work.
	{
		gamecake.canvas.style.display = "none";
	}

	var gamecake_start=function() {

//initialise lua
		gamecake.post('cmd=lua\n','require("wetgenes.win").emcc_start({"--logs"})');

// create a pulse function and call it every frame
		gamecake.pulse=function() {
			if(!gamecake.stop) // the loop erred?
			{
				requestAnimationFrame(gamecake.pulse); // we need to always ask to be called again
				gamecake.post('cmd=lua\n','return gamecake_pulse()');
			}
		};
		requestAnimationFrame(gamecake.pulse); // start the updates
	}

	var show_progress=function(n)
	{
		window.show_progress_max=window.show_progress_max || 0;
		if(window.show_progress_max<n) { window.show_progress_max=n; }
		var pct=Math.floor(100*(1-(n/window.show_progress_max)));
		log("GameCake Loading "+pct+"%");
		gamecake.progress_bar.setAttribute("value",pct/100);
		if(pct==100)
		{
			gamecake.progress_bar.style.display = "none";
			gamecake.progress_about.style.display = "none";
		}
	};
	gamecake.resize=function(){
		var e=gamecake.div;

		var isFullscreen = !((document.fullScreenElement !== undefined && document.fullScreenElement === null) || 
		 (document.msFullscreenElement !== undefined && document.msFullscreenElement === null) || 
		 (document.mozFullScreen !== undefined && !document.mozFullScreen) || 
		 (document.webkitIsFullScreen !== undefined && !document.webkitIsFullScreen));

		if(isFullscreen)
		{
			e=gamecake.canvas; // fullscreen does funky stuff, try not to disturb it.
		}

		var w=parseFloat(window.getComputedStyle(e).width);
		var h=parseFloat(window.getComputedStyle(e).height);

console.log(w,h)
		
		if(!isFullscreen) // it seems better not to call this when fullscreen
		{
			if(Module.setCanvasSize)
			{
				Module.setCanvasSize(w,h);
			}
		}

		if(gamecake.post)
		{
			gamecake.post('cmd=lua\n','require("wetgenes.win").hardcore.resize(nil,'+w+','+h+')'); // but this might help?
		}
	};
	window.addEventListener("resize",gamecake.resize);
	Module={};
	gamecake.Module=Module
	Module.msg=gamecake.msg;
	Module.canvas=gamecake.canvas;
	Module.noInitialRun=true
	Module.memoryInitializerPrefixURL=opts.dir;

	Module.monitorRunDependencies=show_progress;

	Module.preInit = function() {
		gamecake.status="init"
		gamecake.FS=FS
//		ENV.SDL_EMSCRIPTEN_KEYBOARD_ELEMENT = "#gamecake_listener";

		gamecake.canvas.focus()
		
/*
		var BFS = new BrowserFS.EmscriptenFS();
  		FS.mkdir('/files');
		FS.mount(BFS, {root: '/'}, '/files');
*/

  		FS.mkdir('/files');
		FS.mount(IDBFS, {}, '/files');

		FS.syncfs(true, function (err) {
			if(err) { console.log(err); }

			let syncfs ; syncfs=function(){
//				console.log("Syncing IDBFS")
				FS.syncfs(false, function (err) {
					if(err) { console.log(err); }
				});
				setTimeout(syncfs, 10000);
			} ; syncfs()

		}); // this probably works but is all rather squiffy for now


		if(opts.cakefile)
		{
			log("fetching "+opts.cakefile);
			FS.createPreloadedFile('/', "gamecake.zip", opts.cakefile, true, false);
		}
		if( gamecake.loading_hook )
		{
			gamecake.loading_hook();
		}
	};

	Module.onRuntimeInitialized = function() {
		gamecake.status="start"
		gamecake.post = Module.cwrap('main_post', 'int', ['string','string']);
		gamecake_start();
		gamecake.resize();
		if( gamecake.loaded_hook )
		{
			gamecake.loaded_hook();
		}
	};

	Module.print=function(...args)
	{
		log(...args)
	}

	Module.printErr=function(...args)
	{
		gamecake.stop=true
		log(...args)
	}

	Module.window_event_resize=function(){setTimeout(gamecake.resize,500);}
	Module.window_event_fullscreenchange=function(){setTimeout(gamecake.resize,500);}
	Module.window_event_orientationchange=function(){setTimeout(gamecake.resize,500);}

	window.addEventListener("resize",Module.window_event_resize);
	window.addEventListener("fullscreenchange",Module.window_event_fullscreenchange);
	window.addEventListener("orientationchange",Module.window_event_orientationchange);


	gamecake.load_gamecakejs=function()
	{
		gamecake.status="load"
		var first=document.getElementsByTagName('script')[0];
		var js=document.createElement('script');
		js.src=opts.dir+"gamecake.js";
		first.parentNode.insertBefore(js, first);
	}

	gamecake.status="setup"
	if(!opts.no_load_gamecakejs) // delay this final load action that sets everything in motion
	{
		gamecake.load_gamecakejs()
	}

	return gamecake;
}
