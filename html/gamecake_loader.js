
// one global function to call

gamecake_loader=async function(opts)
{
	opts=opts || {};

	let msg_dec=function(s)
	{
		let s2="";
		if(s)
		{
			try
			{
				s2=s.split("%26").join("&").split("%3d").join("=").split("%25").join("%").split("%2c").join(",");
				return s2;
			}
			catch(e)
			{
				return "";
			}
		}
		return "";
	};
	let str_to_msg=function(s,msg) // split a query like string
	{
		msg=msg || {};
		
		let aa=s.split("&");
		for(let i in aa)
		{
			let v=aa[i];
			let va=v.split("=");
			if( va.length==2 )
			{
				let key=msg_dec(va[0])
				let val=[]
				for(let a of ( va[1].split(",") ) ) { val.push( msg_dec(a) ) }
				if(val.length==1) { val=val[0] }
				if(key!=="")
				{
					msg[key]=val;
				}
			}
		}
		
		return msg;
	};


	if(opts.urlopts) // read opts from url
	{
		str_to_msg( window.location.search.substring(1) , opts )
		str_to_msg( window.location.hash.substring(1) , opts )
	}

	opts.div=opts.div || "#gamecake"; // div to display within
	opts.dir=opts.dir || "./exe/"; // where to load stuff from
	opts.args=opts.args || ["--","gamecake.zip","--logs"]

console.log(opts)


	let template_element=function(html)
	{
		let template = document.createElement('template');
		template.innerHTML = html.trim();
		return template.content.firstChild;
	}


	let webgl_support=function()
	{ 
		try {
			let canvas = document.createElement( 'canvas' ); 
			return !! window.WebGLRenderingContext && ( canvas.getContext( 'webgl' ) ||
			canvas.getContext( 'experimental-webgl' ) );
		} catch( e ) { return false; }
	};


	let sharedarraybuffer_support=function()
	{
		try {
			return SharedArrayBuffer && true;
		} catch( e ) { return false; }
	};


	let warning="";
	
	if(!sharedarraybuffer_support())
	{
		warning+='<br/>SharedArrayBuffer is missing! <br/> Make sure you are using a https url. <br/> <a href="https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/SharedArrayBuffer/Planned_changes">Would you like to know more?</a><br/><br/>';
	}

	if(!webgl_support())
	{
		warning+='<br/>WebGL is missing!<br/>Visit <a href="http://get.webgl.org/">http://get.webgl.org/</a> for help.<br/><br/>Maybe Chrome needs you to run with the command line option --disable-gpu-sandbox to work.<br/><br/>';
	}

	let readme='<br/>Please wait, the first load may be a little bit slow.<br/><br/>';
	if(warning!="") { readme=warning; }



// setup gamecake

	let gamecake={};
	gamecake.div=document.querySelector(opts.div); // where to put stuff	


	gamecake.engine="emcc";
	
	gamecake.listener=template_element('<div id="gamecake_listener" style="width:100%;height:100%;position:relative; background-color:#000;"></div>'); // Main container of stuff
	gamecake.progress_bar=template_element('<progress value="0" style="width:100%; position:absolute;" title="Loading GameCake"></progress>');
	gamecake.progress_about=template_element(
		'<div style="font-family:sans-serif; font-size:2em; color:#fff; line-height:1.5em; text-align:center; width:66%; margin:auto; ">'+readme+'</div>');
	gamecake.canvas=template_element('<canvas onclick="this.focus();" tabindex="-1" id="canvas" style="position:absolute; " oncontextmenu="event.preventDefault()"></canvas>');

	gamecake.listener.appendChild(gamecake.progress_bar)
	gamecake.listener.appendChild(gamecake.progress_about)
	gamecake.listener.appendChild(gamecake.canvas)

	gamecake.div.innerHTML=""
	gamecake.div.appendChild(gamecake.listener);

	if(warning!="") // hide canvas as it is not going to work.
	{
		gamecake.canvas.style.display = "none";
	}

	let show_progress=function(n)
	{
		window.show_progress_max=window.show_progress_max || 0;
		if(window.show_progress_max<n) { window.show_progress_max=n; }
		let pct=Math.floor(100*(1-(n/window.show_progress_max)));
		console.log("GameCake Loading "+pct+"%");
		gamecake.progress_bar.setAttribute("value",pct/100);
		if(pct==100)
		{
			gamecake.progress_bar.style.display = "none";
			gamecake.progress_about.style.display = "none";
		}
	};
	gamecake.resize=function(){
		let e=document.documentElement;

		let isFullscreen = !((document.fullScreenElement !== undefined && document.fullScreenElement === null) || 
		 (document.msFullscreenElement !== undefined && document.msFullscreenElement === null) || 
		 (document.mozFullScreen !== undefined && !document.mozFullScreen) || 
		 (document.webkitIsFullScreen !== undefined && !document.webkitIsFullScreen));

		if(isFullscreen)
		{
			e=gamecake.canvas; // fullscreen does funky stuff, try not to disturb it.
		}

		let w=e.getBoundingClientRect().width;
		let h=e.getBoundingClientRect().height;

//console.log(w,h)
		
		if(!isFullscreen) // it seems better not to call this when fullscreen
		{
			console.log("size",w,h)
			gamecake.canvas.style.width = w+"px"
			gamecake.canvas.style.height = h+"px"
			gamecake.canvas.width = w
			gamecake.canvas.height = h
			if(Module.setCanvasSize)
			{
				setTimeout(function(){Module.setCanvasSize(w,h)},500);
			}
		}

	};

// simple sync clipboard get/set and refresh at startup or when we gain focus
	gamecake.clipboard_data=""
	gamecake.set_clipboard=function(data)
	{
console.log("set clipboard",data)
		try{
			gamecake.clipboard_data=data;
			if(navigator.clipboard.writeText)
			{
				navigator.clipboard.writeText(data);
			}
		}catch(e){}
	};
	gamecake.get_clipboard=function()
	{
		return gamecake.clipboard_data;
	};
	gamecake.refresh_clipboard=async function()
	{
		try{
			if( navigator.clipboard.readText )
			{
				let data=await navigator.clipboard.readText()
				gamecake.set_clipboard(data)
			}
		}catch(e){}
	};
	gamecake.refresh_clipboard()

//	window.addEventListener("resize",gamecake.resize);
	Module={}; // this is a global
	gamecake.Module=Module
	Module.gamecake=gamecake
	Module.noExitRuntime=true
	Module.canvas=gamecake.canvas;
//	Module.noInitialRun=true

	Module.monitorRunDependencies=show_progress;

	Module.arguments=opts.args

	if(opts.zipfile)
	{
		console.log("fetching "+opts.zipfile);

		Module.zfiles={}
		console.log(unzipit)
		let z = await unzipit.unzip(opts.zipfile)
        for( n in z.entries )
        {
			let e=z.entries[n]
			let b=await e.blob()
			Module.zfiles[n]=new Uint8Array( await b.arrayBuffer() )
        }
	}
	
	Module.preInit = function() {
		gamecake.status="init"
		gamecake.FS=FS
//		ENV.SDL_EMSCRIPTEN_KEYBOARD_ELEMENT = "#gamecake_listener";

		gamecake.canvas.focus()
		
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
			console.log("fetching "+opts.cakefile);
			FS.createPreloadedFile('/', "gamecake.zip", opts.cakefile, true, false);
		}
		if(Module.zfiles)
		{
			for(let n in Module.zfiles)
			{
				let b=Module.zfiles[n]
				let na=n.split("/")
				if(na.length>1)
				{
					let dn=""
					for(let i=0 ; i<na.length-1 ; i++)
					{
						dn=dn+na[i]+"/"
						try{ FS.mkdir( dn ) }catch(e){}
					}
				}
				if(na[na.length-1]!="")
				{
					console.log(n,b.length)
					FS.writeFile(n, b);
				}
			}
		}
		if( gamecake.loading_hook )
		{
			gamecake.loading_hook();
		}
	};

/*
	gamecake.animation_frame= function() {
		requestAnimationFrame(gamecake.animation_frame)
		if( gamecake.main_update )
		{
			gamecake.main_update()
		}
	}
*/

	Module.onRuntimeInitialized = function() {

		// export functions
//		gamecake.main_update=Module.cwrap("main_update")
		gamecake.main_close=Module.cwrap("main_close")

		gamecake.status="start"
		gamecake.resize();
		if( gamecake.loaded_hook )
		{
			gamecake.loaded_hook();
		}
//		requestAnimationFrame(gamecake.animation_frame)
	};

// the clipboard api is useless so we just read when we gain focus and hope that is enough
	Module.window_event_focus=function(){setTimeout(gamecake.refresh_clipboard,500);}

	Module.window_event_resize=function(){setTimeout(gamecake.resize,500);}
	Module.window_event_fullscreenchange=function(){setTimeout(gamecake.resize,500);}
	Module.window_event_orientationchange=function(){setTimeout(gamecake.resize,500);}

	window.addEventListener('focus', Module.window_event_focus);
	window.addEventListener("resize",Module.window_event_resize);
	window.addEventListener("fullscreenchange",Module.window_event_fullscreenchange);
	window.addEventListener("orientationchange",Module.window_event_orientationchange);


	gamecake.load_gamecakejs=function()
	{
		gamecake.status="load"
		let first=document.getElementsByTagName('script')[0];
		let js=document.createElement('script');
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
