
project "lib_freetype"
language "C"

defines { "FT2_BUILD_LIBRARY" , "DARWIN_NO_CARBON" }

includedirs { "git/include/" , "." }


files {

	"git/src/base/ftsystem.c",
	"git/src/base/ftinit.c",
	"git/src/base/ftdebug.c",

	"git/src/base/ftbase.c",

	"git/src/base/ftbbox.c",       -- recommended, see <freetype/ftbbox.h>
	"git/src/base/ftglyph.c",      -- recommended, see <freetype/ftglyph.h>

	"git/src/base/ftbdf.c",        -- optional, see <freetype/ftbdf.h>
	"git/src/base/ftbitmap.c",     -- optional, see <freetype/ftbitmap.h>
	"git/src/base/ftcid.c",        -- optional, see <freetype/ftcid.h>
	"git/src/base/ftfstype.c",     -- optional
	"git/src/base/ftgasp.c",       -- optional, see <freetype/ftgasp.h>
	"git/src/base/ftgxval.c",      -- optional, see <freetype/ftgxval.h>
	"git/src/base/ftlcdfil.c",     -- optional, see <freetype/ftlcdfil.h>
	"git/src/base/ftmm.c",         -- optional, see <freetype/ftmm.h>
	"git/src/base/ftotval.c",      -- optional, see <freetype/ftotval.h>
	"git/src/base/ftpatent.c",     -- optional
	"git/src/base/ftpfr.c",        -- optional, see <freetype/ftpfr.h>
	"git/src/base/ftstroke.c",     -- optional, see <freetype/ftstroke.h>
	"git/src/base/ftsynth.c",      -- optional, see <freetype/ftsynth.h>
	"git/src/base/fttype1.c",      -- optional, see <freetype/t1tables.h>
	"git/src/base/ftwinfnt.c",     -- optional, see <freetype/ftwinfnt.h>
--	"git/src/base/ftxf86.c",       -- optional, see <freetype/ftxf86.h>

	-- font drivers (optional; at least one is needed)

	"git/src/hvf/hvf.c",
	"git/src/svg/svg.c",

	"git/src/bdf/bdf.c",           -- BDF font driver
	"git/src/cff/cff.c",           -- CFF/OpenType font driver
	"git/src/cid/type1cid.c",      -- Type 1 CID-keyed font driver
	"git/src/pcf/pcf.c",           -- PCF font driver
	"git/src/pfr/pfr.c",           -- PFR/TrueDoc font driver
	"git/src/sfnt/sfnt.c",         -- SFNT files support

	--                                 (TrueType & OpenType)

	"git/src/truetype/truetype.c", -- TrueType font driver
	"git/src/type1/type1.c",       -- Type 1 font driver
	"git/src/type42/type42.c",     -- Type 42 font driver
	"git/src/winfonts/winfnt.c",   -- Windows FONT / FNT font driver

	-- rasterizers (optional; at least one is needed for vector
	-- formats)

	"git/src/raster/raster.c",     -- monochrome rasterizer
	"git/src/smooth/smooth.c",     -- anti-aliasing rasterizer
	"git/src/sdf/sdf.c",           -- Signed Distance Field driver

	-- auxiliary modules (optional)

	"git/src/autofit/autofit.c",   -- auto hinting module
	"git/src/cache/ftcache.c",     -- cache sub-system (in beta)
	"git/src/gzip/ftgzip.c",       -- support for compressed fonts (.gz)
	"git/src/lzw/ftlzw.c",         -- support for compressed fonts (.Z)
	"git/src/bzip2/ftbzip2.c",     -- support for compressed fonts (.bz2)
	"git/src/gxvalid/gxvalid.c",   -- TrueTypeGX/AAT table validation
	"git/src/otvalid/otvalid.c",   -- OpenType table validation
	"git/src/psaux/psaux.c",       -- PostScript Type 1 parsing
	"git/src/pshinter/pshinter.c", -- PS hinting module
	"git/src/psnames/psnames.c",   -- PostScript glyph names support


}

if EMCC then
--	buildlinkoptions{
--		"-Wno-error=pointer-bool-conversion",
--	}
end


KIND{}

