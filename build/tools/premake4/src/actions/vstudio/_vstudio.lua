--
-- _vstudio.lua
-- Define the Visual Studio 200x actions.
-- Copyright (c) 2008-2011 Jason Perkins and the Premake project
--

	premake.vstudio = { }
	local vstudio = premake.vstudio


--
-- Map Premake platform identifiers to the Visual Studio versions. Adds the Visual
-- Studio specific "any" and "mixed" to make solution generation easier.
--

	vstudio.platforms = { 
		any     = "Any CPU", 
		mixed   = "Mixed Platforms", 
		Native  = "Win32",
		x86     = "x86",
		x32     = "Win32", 
		x64     = "x64",
		PS3     = "PS3",
		Xbox360 = "Xbox 360",
	}

	

--
-- Returns the architecture identifier for a project.
-- Used by the solutions.
--

	function vstudio.arch(prj)
		if (prj.language == "C#") then
			if (_ACTION < "vs2005") then
				return ".NET"
			else
				return "Any CPU"
			end
		else
			return "Win32"
		end
	end
	
	

--
-- Process the solution's list of configurations and platforms, creates a list
-- of build configuration/platform pairs in a Visual Studio compatible format.
--

	function vstudio.buildconfigs(sln)
		local cfgs = { }
		
		local platforms = premake.filterplatforms(sln, vstudio.platforms, "Native")

		-- Figure out what's in this solution
		local hascpp    = premake.hascppproject(sln)
		local hasdotnet = premake.hasdotnetproject(sln)

		-- "Mixed Platform" solutions are generally those containing both
		-- C/C++ and .NET projects. Starting in VS2010, all .NET solutions
		-- also contain the Mixed Platform option.
		if hasdotnet and (_ACTION > "vs2008" or hascpp) then
			table.insert(platforms, 1, "mixed")
		end
		
		-- "Any CPU" is added to solutions with .NET projects. Starting in
		-- VS2010, only pure .NET solutions get this option.
		if hasdotnet and (_ACTION < "vs2010" or not hascpp) then
			table.insert(platforms, 1, "any")
		end

		-- In Visual Studio 2010, pure .NET solutions replace the Win32 platform
		-- with x86. In mixed mode solution, x86 is used in addition to Win32.
		if _ACTION > "vs2008" then
			local platforms2010 = { }
			for _, platform in ipairs(platforms) do
				if vstudio.platforms[platform] == "Win32" then
					if hascpp then
						table.insert(platforms2010, platform)
					end
					if hasdotnet then
						table.insert(platforms2010, "x86")
					end
				else
					table.insert(platforms2010, platform)
				end
			end
			platforms = platforms2010
		end


		for _, buildcfg in ipairs(sln.configurations) do
			for _, platform in ipairs(platforms) do
				local entry = { }
				entry.src_buildcfg = buildcfg
				entry.src_platform = platform
				
				-- PS3 is funky and needs special handling; it's more of a build
				-- configuration than a platform from Visual Studio's point of view				
				if platform ~= "PS3" then
					entry.buildcfg = buildcfg
					entry.platform = vstudio.platforms[platform]
				else
					entry.buildcfg = platform .. " " .. buildcfg
					entry.platform = "Win32"
				end

				-- create a name the way VS likes it
				entry.name = entry.buildcfg .. "|" .. entry.platform
				
				-- flag the "fake" platforms added for .NET
				entry.isreal = (platform ~= "any" and platform ~= "mixed")
								
				table.insert(cfgs, entry)
			end
		end
		
		return cfgs
	end
	


--
-- Clean Visual Studio files
--

	function vstudio.cleansolution(sln)
		premake.clean.file(sln, "%%.sln")
		premake.clean.file(sln, "%%.suo")
		premake.clean.file(sln, "%%.ncb")
		-- MonoDevelop files
		premake.clean.file(sln, "%%.userprefs")
		premake.clean.file(sln, "%%.usertasks")
	end
	
	function vstudio.cleanproject(prj)
		local fname = premake.project.getfilename(prj, "%%")

		os.remove(fname .. ".vcproj")
		os.remove(fname .. ".vcproj.user")

		os.remove(fname .. ".vcxproj")
		os.remove(fname .. ".vcxproj.user")
		os.remove(fname .. ".vcxproj.filters")

		os.remove(fname .. ".csproj")
		os.remove(fname .. ".csproj.user")

		os.remove(fname .. ".pidb")
		os.remove(fname .. ".sdf")
	end

	function vstudio.cleantarget(name)
		os.remove(name .. ".pdb")
		os.remove(name .. ".idb")
		os.remove(name .. ".ilk")
		os.remove(name .. ".vshost.exe")
		os.remove(name .. ".exe.manifest")
	end
	
	

--
-- Assemble the project file name.
--

	function vstudio.projectfile(prj)
		local extension
		if prj.language == "C#" then
			extension = ".csproj"
		else
			extension = iif(_ACTION > "vs2008", ".vcxproj", ".vcproj")
		end

		local fname = path.join(prj.location, prj.name)
		return fname..extension
	end
	

--
-- Returns the Visual Studio tool ID for a given project type.
--

	function vstudio.tool(prj)
		if (prj.language == "C#") then
			return "FAE04EC0-301F-11D3-BF4B-00C04F79EFBC"
		else
			return "8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942"
		end
	end


--
-- Register Visual Studio 2002
--

	newaction {
		trigger         = "vs2002",
		shortname       = "Visual Studio 2002",
		description     = "Generate Microsoft Visual Studio 2002 project files",
		os              = "windows",
		
		valid_kinds     = { "ConsoleApp", "WindowedApp", "StaticLib", "SharedLib" },
		
		valid_languages = { "C", "C++", "C#" },
		
		valid_tools     = {
			cc     = { "msc"   },
			dotnet = { "msnet" },
		},

		onsolution = function(sln)
			premake.generate(sln, "%%.sln", vstudio.sln2002.generate)
		end,
		
		onproject = function(prj)
			if premake.isdotnetproject(prj) then
				premake.generate(prj, "%%.csproj", vstudio.cs2002.generate)
				premake.generate(prj, "%%.csproj.user", vstudio.cs2002.generate_user)
			else
				premake.generate(prj, "%%.vcproj", vstudio.vc200x.generate)
				premake.generate(prj, "%%.vcproj.user", vstudio.vc200x.generate_user)
			end
		end,
		
		oncleansolution = premake.vstudio.cleansolution,
		oncleanproject  = premake.vstudio.cleanproject,
		oncleantarget   = premake.vstudio.cleantarget
	}


--
-- Register Visual Studio 2003
--

	newaction {
		trigger         = "vs2003",
		shortname       = "Visual Studio 2003",
		description     = "Generate Microsoft Visual Studio 2003 project files",
		os              = "windows",

		valid_kinds     = { "ConsoleApp", "WindowedApp", "StaticLib", "SharedLib" },
		
		valid_languages = { "C", "C++", "C#" },
		
		valid_tools     = {
			cc     = { "msc"   },
			dotnet = { "msnet" },
		},

		onsolution = function(sln)
			premake.generate(sln, "%%.sln", vstudio.sln2003.generate)
		end,
		
		onproject = function(prj)
			if premake.isdotnetproject(prj) then
				premake.generate(prj, "%%.csproj", vstudio.cs2002.generate)
				premake.generate(prj, "%%.csproj.user", vstudio.cs2002.generate_user)
			else
				premake.generate(prj, "%%.vcproj", vstudio.vc200x.generate)
				premake.generate(prj, "%%.vcproj.user", vstudio.vc200x.generate_user)
			end
		end,
		
		oncleansolution = premake.vstudio.cleansolution,
		oncleanproject  = premake.vstudio.cleanproject,
		oncleantarget   = premake.vstudio.cleantarget
	}


--
-- Register Visual Studio 2005
--

	newaction {
		trigger         = "vs2005",
		shortname       = "Visual Studio 2005",
		description     = "Generate Microsoft Visual Studio 2005 project files",
		os              = "windows",

		valid_kinds     = { "ConsoleApp", "WindowedApp", "StaticLib", "SharedLib" },
		
		valid_languages = { "C", "C++", "C#" },
		
		valid_tools     = {
			cc     = { "msc"   },
			dotnet = { "msnet" },
		},

		onsolution = function(sln)
			premake.generate(sln, "%%.sln", vstudio.sln2005.generate)
		end,
		
		onproject = function(prj)
			if premake.isdotnetproject(prj) then
				premake.generate(prj, "%%.csproj", vstudio.cs2005.generate)
				premake.generate(prj, "%%.csproj.user", vstudio.cs2005.generate_user)
			else
				premake.generate(prj, "%%.vcproj", vstudio.vc200x.generate)
				premake.generate(prj, "%%.vcproj.user", vstudio.vc200x.generate_user)
			end
		end,
		
		oncleansolution = vstudio.cleansolution,
		oncleanproject  = vstudio.cleanproject,
		oncleantarget   = vstudio.cleantarget
	}


--
-- Register Visual Studio 2008
--

	newaction {
		trigger         = "vs2008",
		shortname       = "Visual Studio 2008",
		description     = "Generate Microsoft Visual Studio 2008 project files",
		os              = "windows",

		valid_kinds     = { "ConsoleApp", "WindowedApp", "StaticLib", "SharedLib" },
		
		valid_languages = { "C", "C++", "C#" },
		
		valid_tools     = {
			cc     = { "msc"   },
			dotnet = { "msnet" },
		},

		onsolution = function(sln)
			premake.generate(sln, "%%.sln", vstudio.sln2005.generate)
		end,
		
		onproject = function(prj)
			if premake.isdotnetproject(prj) then
				premake.generate(prj, "%%.csproj", vstudio.cs2005.generate)
				premake.generate(prj, "%%.csproj.user", vstudio.cs2005.generate_user)
			else
				premake.generate(prj, "%%.vcproj", vstudio.vc200x.generate)
				premake.generate(prj, "%%.vcproj.user", vstudio.vc200x.generate_user)
			end
		end,
		
		oncleansolution = vstudio.cleansolution,
		oncleanproject  = vstudio.cleanproject,
		oncleantarget   = vstudio.cleantarget
	}

		
--
-- Register Visual Studio 2010
--

	newaction 
	{
		trigger         = "vs2010",
		shortname       = "Visual Studio 2010",
		description     = "Generate Visual Studio 2010 project files",
		os              = "windows",

		valid_kinds     = { "ConsoleApp", "WindowedApp", "StaticLib", "SharedLib" },
		
		valid_languages = { "C", "C++", "C#"},
		
		valid_tools     = {
			cc     = { "msc"   },
			dotnet = { "msnet" },
		},

		onsolution = function(sln)
			premake.generate(sln, "%%.sln", vstudio.sln2005.generate)
		end,
		
		onproject = function(prj)
			if premake.isdotnetproject(prj) then
				premake.generate(prj, "%%.csproj", vstudio.cs2005.generate)
				premake.generate(prj, "%%.csproj.user", vstudio.cs2005.generate_user)
			else
			premake.generate(prj, "%%.vcxproj", premake.vs2010_vcxproj)
			premake.generate(prj, "%%.vcxproj.user", premake.vs2010_vcxproj_user)
			premake.generate(prj, "%%.vcxproj.filters", vstudio.vc2010.generate_filters)
			end
		end,
		

		oncleansolution = premake.vstudio.cleansolution,
		oncleanproject  = premake.vstudio.cleanproject,
		oncleantarget   = premake.vstudio.cleantarget
	}