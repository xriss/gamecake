
project "lib_luajit"
language "C"

files {
	"src/*.h",

	"src/ljamalg.c",

--[[	
	"src/lj_vmmath.c",
	"src/lj_vmevent.c",
	"src/lj_udata.c",
	"src/lj_trace.c",
	"src/lj_tab.c",
	"src/lj_str.c",
	"src/lj_state.c",
	"src/lj_snap.c",
	"src/lj_record.c",
	"src/lj_parse.c",
	"src/lj_opt_split.c",
	"src/lj_opt_narrow.c",
	"src/lj_opt_mem.c",
	"src/lj_opt_loop.c",
	"src/lj_opt_fold.c",
	"src/lj_opt_dce.c",
	"src/lj_obj.c",
	"src/lj_meta.c",
	"src/lj_mcode.c",
	"src/lj_lib.c",
	"src/lj_lex.c",
	"src/lj_ir.c",
	"src/lj_gdbjit.c",
	"src/lj_gc.c",
	"src/lj_func.c",
	"src/lj_ffrecord.c",
	"src/lj_err.c",
	"src/lj_dispatch.c",
	"src/lj_debug.c",
	"src/lj_ctype.c",
	"src/lj_crecord.c",
	"src/lj_cparse.c",
	"src/lj_clib.c",
	"src/lj_char.c",
	"src/lj_cdata.c",
	"src/lj_cconv.c",
	"src/lj_ccallback.c",
	"src/lj_ccall.c",
	"src/lj_carith.c",
	"src/lj_bcwrite.c",
	"src/lj_bcread.c",
	"src/lj_bc.c",
	"src/lj_asm.c",
	"src/lj_api.c",
	"src/lj_alloc.c",
]]
--	"src/lj_buf.c",
--	"src/lj_strfmt.c",
--[[
	"src/lj_load.c",
	"src/lj_strscan.c",
	"src/lj_opt_sink.c",
]]
--[[
	"src/lib_table.c",
	"src/lib_string.c",
	"src/lib_package.c",
	"src/lib_os.c",
	"src/lib_math.c",
	"src/lib_jit.c",
	"src/lib_io.c",
	"src/lib_init.c",
	"src/lib_ffi.c",
	"src/lib_debug.c",
	"src/lib_bit.c",
	"src/lib_base.c",
	"src/lib_aux.c",
]]
	}


--[[

asm.lua builds cached code in the asm dir, needs to be run if we bump the code

]]




if RASPI then -- hardfloat for raspbian

	includedirs { "asm/armhf" }
	files { "asm/armhf/lj_vm.s" }

elseif ANDROID then

	includedirs { "asm/arm" }
	files { "asm/arm/lj_vm.s" }

else

	includedirs { "asm/x86" }
	files { "asm/x86/lj_vm.s" }
	
end

includedirs { "src" }

defines("LUA_PRELOADLIBS=lua_preloadlibs")

KIND() -- set defaults

