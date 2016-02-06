
project "lib_freetype"
language "C"

defines { "FT2_BUILD_LIBRARY" , "DARWIN_NO_CARBON" }

includedirs { "../lib_freetype/freetype/include/" , "." }


files {

	"freetype/src/base/ftsystem.c",
	"freetype/src/base/ftinit.c",
	"freetype/src/base/ftdebug.c",

	"freetype/src/base/ftbase.c",

	"freetype/src/base/ftbbox.c",       -- recommended, see <freetype/ftbbox.h>
	"freetype/src/base/ftglyph.c",      -- recommended, see <freetype/ftglyph.h>

	"freetype/src/base/ftbdf.c",        -- optional, see <freetype/ftbdf.h>
	"freetype/src/base/ftbitmap.c",     -- optional, see <freetype/ftbitmap.h>
	"freetype/src/base/ftcid.c",        -- optional, see <freetype/ftcid.h>
	"freetype/src/base/ftfstype.c",     -- optional
	"freetype/src/base/ftgasp.c",       -- optional, see <freetype/ftgasp.h>
	"freetype/src/base/ftgxval.c",      -- optional, see <freetype/ftgxval.h>
	"freetype/src/base/ftlcdfil.c",     -- optional, see <freetype/ftlcdfil.h>
	"freetype/src/base/ftmm.c",         -- optional, see <freetype/ftmm.h>
	"freetype/src/base/ftotval.c",      -- optional, see <freetype/ftotval.h>
	"freetype/src/base/ftpatent.c",     -- optional
	"freetype/src/base/ftpfr.c",        -- optional, see <freetype/ftpfr.h>
	"freetype/src/base/ftstroke.c",     -- optional, see <freetype/ftstroke.h>
	"freetype/src/base/ftsynth.c",      -- optional, see <freetype/ftsynth.h>
	"freetype/src/base/fttype1.c",      -- optional, see <freetype/t1tables.h>
	"freetype/src/base/ftwinfnt.c",     -- optional, see <freetype/ftwinfnt.h>
--	"freetype/src/base/ftxf86.c",       -- optional, see <freetype/ftxf86.h>

	-- font drivers (optional; at least one is needed)

	"freetype/src/bdf/bdf.c",           -- BDF font driver
	"freetype/src/cff/cff.c",           -- CFF/OpenType font driver
	"freetype/src/cid/type1cid.c",      -- Type 1 CID-keyed font driver
	"freetype/src/pcf/pcf.c",           -- PCF font driver
	"freetype/src/pfr/pfr.c",           -- PFR/TrueDoc font driver
	"freetype/src/sfnt/sfnt.c",         -- SFNT files support

	--                                 (TrueType & OpenType)

	"freetype/src/truetype/truetype.c", -- TrueType font driver
	"freetype/src/type1/type1.c",       -- Type 1 font driver
	"freetype/src/type42/type42.c",     -- Type 42 font driver
	"freetype/src/winfonts/winfnt.c",   -- Windows FONT / FNT font driver

	-- rasterizers (optional; at least one is needed for vector
	-- formats)

	"freetype/src/raster/raster.c",     -- monochrome rasterizer
	"freetype/src/smooth/smooth.c",     -- anti-aliasing rasterizer

	-- auxiliary modules (optional)

	"freetype/src/autofit/autofit.c",   -- auto hinting module
	"freetype/src/cache/ftcache.c",     -- cache sub-system (in beta)
	"freetype/src/gzip/ftgzip.c",       -- support for compressed fonts (.gz)
	"freetype/src/lzw/ftlzw.c",         -- support for compressed fonts (.Z)
	"freetype/src/bzip2/ftbzip2.c",     -- support for compressed fonts (.bz2)
	"freetype/src/gxvalid/gxvalid.c",   -- TrueTypeGX/AAT table validation
	"freetype/src/otvalid/otvalid.c",   -- OpenType table validation
	"freetype/src/psaux/psaux.c",       -- PostScript Type 1 parsing
	"freetype/src/pshinter/pshinter.c", -- PS hinting module
	"freetype/src/psnames/psnames.c",   -- PostScript glyph names support


}

if EMCC then
--	buildlinkoptions{
--		"-Wno-error=pointer-bool-conversion",
--	}
end


KIND{}

