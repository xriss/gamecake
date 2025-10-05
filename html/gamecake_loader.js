
// one global function to call

gamecake_loader=async function(opts)
{
/* cors fetch test
let fr=await fetch("https://cors-anywhere.herokuapp.com/https://gist.githubusercontent.com/xriss/e6965fb7b34b9353fd744e223b0f6af6/raw/d70f94d57a0bbe7b81213b6ae22dbdebd4034cdb/palette.fun.lua")
let tr=await fr.text()
console.log(tr)
*/
  

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
	if(!Array.isArray(opts.args)){opts.args=[opts.args]} // force args to array

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
	
	// empty and insert elements into main container
	gamecake.div.innerHTML=""
	gamecake.progress_bar=template_element('<progress value="0" style="width:100%; position:absolute;" title="Loading GameCake"></progress>');
	gamecake.div.appendChild(gamecake.progress_bar)
	gamecake.progress_about=template_element('<div style="font-family:sans-serif; font-size:2em; color:#fff; line-height:1.5em; text-align:center; width:66%; margin:auto; ">'+readme+'</div>');
	gamecake.div.appendChild(gamecake.progress_about)
	gamecake.canvas=template_element('<canvas onclick="this.focus();" tabindex="-1" id="canvas" style="position:absolute; width:100%; height:100%;" oncontextmenu="event.preventDefault()"></canvas>');
	gamecake.div.appendChild(gamecake.canvas)

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

// simple sync clipboard get/set and refresh at startup or when we gain focus
	gamecake.clipboard_data=""
	gamecake.set_clipboard=function(data)
	{
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

	Module={}; // this is a global
	gamecake.Module=Module
	Module.gamecake=gamecake
	Module.noExitRuntime=true
	Module.canvas=gamecake.canvas;
//	Module.noInitialRun=true

	Module.monitorRunDependencies=show_progress;

	Module.arguments=opts.args

	if(opts.zipfile) // possibly multiple zips
	{
		let zips=[opts.zipfile]
		if(Array.isArray(opts.zipfile)){zips=opts.zipfile}
		for(zip of zips)
		{
			console.log("fetching "+zip);

			Module.zfiles={}
			let z = await unzipit.unzip(zip)
			for( n in z.entries )
			{
				let e=z.entries[n]
				let b=await e.blob()
				Module.zfiles[n]=new Uint8Array( await b.arrayBuffer() )
			}
		}
	}
	
	Module.preInit = function() {
		gamecake.status="init"
		gamecake.FS=FS

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

	Module.onRuntimeInitialized = function() {

		// export functions
		gamecake.main_close=Module.cwrap("main_close")

		gamecake.status="start"
		if( gamecake.loaded_hook )
		{
			gamecake.loaded_hook();
		}
	};

// the clipboard api is useless so we just read when we gain focus and hope that is enough
	Module.window_event_focus=function(){setTimeout(gamecake.refresh_clipboard,500);}
	window.addEventListener('focus', Module.window_event_focus);

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
