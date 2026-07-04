
package = "djon"

version = "1.260512-1"

source = {
	url = "git://github.com/xriss/djon",
	tag = "v1.260512.1"
}

description = {
	homepage = "https://github.com/xriss/djon",
	license = "MIT",
}

dependencies = {
}

external_dependencies = {
}

build = {
	type = "builtin",
	modules = {
		["djon.core"] = {
			sources = {
				"lua/djon.core.c",
			},
			incdirs = {
				"c",
				"lua",
			},
		},
		["djon"] = "lua/djon.lua",
	},
}
