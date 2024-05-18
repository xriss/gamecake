
package = "djon"

version = "HEAD-0"

source = {
	url = "git://github.com/xriss/djon",
	tag = "HEAD",
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
