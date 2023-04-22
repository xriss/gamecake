
local ipa_arpabet={

--	IPA				ARPABET			CMUDICT

	["ɑ"]	=	{	"a",	"AA",	"vowel",		},	--		arm			father		balm		bot
	["æ"]	=	{	"@",	"AE",	"vowel",		},	--		cat			black		bat
	["aɪ"]	=	{	"Y",	"AY",	"vowel",		},	--		five		eye			bite
	["ə"]	=	{	"x",	"AX",	false,			},	--		away		cinema		comma
	["ɚ"]	=	{	"",		"AXR"	false,			},	--		letter
		                            
	["b"]	=	{	"b",	"B",	"stop",			},	--		bad			lab			buy
		                            
	["tʃ"]	=	{	"C",	"CH",	"affricate",	},	--		check		church		china
		                            
	["d"]	=	{	"d",	"D",	"stop",			},	--		did			lady		die
	["ð"]	=	{	"D",	"DH",	"fricative",	},	--		this		mother		thy
		                            
	["eɪ"]	=	{	"e",	"EY",	"vowel",		},	--		bait
	["ɛ"]	=	{	"E",	"EH",	"vowel",		},	--		met			bed			bet
	["ɝ"]	=	{	"R",	"ER",	"vowel",		},	--		bird
		                            
	["f"]	=	{	"f",	"F",	"fricative",	},	--		find		if			fight
	["θ"]	=	{	"T",	"TH",	"fricative",	},	--		think		both		thigh
		                            
	["g"]	=	{	"g",	"G",	"stop",			},	--		give		flag		guy
	["ŋ"]	=	{	"G",	"NG",	"nasal",		},	--		sing

	["h"]	=	{	"h",	"HH",	"aspirate",		},	--		how			hello		high
		                            
	["i"]	=	{	"i",	"IY",	"vowel",		},	--		see			heat		beat
	["ɪ"]	=	{	"I",	"IH",	"vowel",		},	--		hit			sitting		bit
	["ɨ"]	=	{	"X",	"IX",	false,			},	--		roses		rabbit
		                            
	["dʒ"]	=	{	"J",	"JH",	"affricate",	},	--		just		large		jive
				                    
	["k"]	=	{	"k",	"K",	"stop",			},	--		cat			back		kite
		                            
	["l"]	=	{	"l",	"L",	"liquid",		},	--		leg			little		lie
	["l̩"]	=	{	"L",	"EL",	false,			},	--		bottle
		                            
	["m"]	=	{	"m",	"M",	"nasal",		},	--		man			lemon		my
	["m̩"]	=	{	"M",	"EM",	"",				},	--		rhythm
		                            
	["n"]	=	{	"n",	"N",	"nasal",		},	--		no			ten			nigh
	["ŋ"]	=	{	"N",	"EN",	false,			},	--		sing		finger		button
	["ɾ̃"]	=	{	"",		"NX",	false,			},	--		winner
		                            
	["oʊ"]	=	{	"o",	"OW",	"vowel",		},	--		go			home		boat
	["ɔɪ"]	=	{	"O",	"OY",	"vowel",		},	--		boy			join
	["ɔ"]	=	{	"c",	"AO",	"vowel",		},	--		four		story
	["aʊ"]	=	{	"W",	"AW",	"vowel",		},	--		now			out			bout
		                            
	["p"]	=	{	"p",	"P",	"stop",			},	--		pet			map			pie
		                            
	["ʔ"]	=	{	"Q",	"Q",	false,			},	--		uh-oh
		                            
	["r"]	=	{	"r",	"R",	"liquid",		},	--		red			try			rye
		                            
	["s"]	=	{	"s",	"S",	"fricative",	},	--		sun			miss		sigh
	["ʃ"]	=	{	"S",	"SH",	"fricative",	},	--		she			crash		shy
		                            
	["t"]	=	{	"t",	"T",	"stop",			},	--		tea			getting		tie
	["ɾ"]	=	{	"F",	"DX",	false,			},	--		butter

	["u"]	=	{	"u",	"UW",	"vowel",		},	--		blue		food		boot
	["ʊ"]	=	{	"U",	"UH",	"vowel",		},	--		put			could		book
	["ʌ"]	=	{	"A",	"AH"	"vowel",		},	--		cup			luck		butt
	["ʉ"]	=	{	"",		"UX",	false,			},	--		dude
		                            
	["v"]	=	{	"v",	"V",	"fricative",	},	--		voice		five		vie
		                            
	["w"]	=	{	"w",	"W",	"semivowel",	},	--		wet			window		wise
	["ʍ"]	=	{	"H",	"WH",	false,			},	--		why

	["j"]	=	{	"y",	"Y",	"semivowel",	},	--		yes			yellow		yacht
				                    
	["z"]	=	{	"z",	"Z",	"fricative",	},	--		zoo			lazy
	["ʒ"]	=	{	"Z",	"ZH",	"fricative",	},	--		pleasure	vision

}


local ipa_id={

--	[#]	=	"IPA",	"Branner",	"M&O",	"PHONASCII","Praat",	"UPSID",	"Usenet",	"Worldbet",	"X-SAMPA",	"Value"

	[101]={	"p",	"p",		"p",	"p",		"p",		"p",		"p",		"p",		"p",		"Voiceless bilabial stop"},
	[102]={	"b",	"b",		"b",	"b",		"b",		"b",		"b",		"b",		"b",		"Voiced bilabial stop"},
	[103]={	"t",	"t",		"t",	"t",		"t",		"t",		"t",		"t",		"t",		"Voiceless alveolar stop"},
	[104]={	"d",	"d",		"d",	"d",		"d",		"d",		"d",		"d",		"d",		"Voiced alveolar stop"},
	[105]={	"ʈ",	"tr)",		"t(",	"tr",		"\\t.",		"t.",		"t.",		"tr",		"t`",		"Voiceless retroflex stop"},
	[106]={	"ɖ",	"dr)",		"d(",	"dr",		"\\d.",		"d.",		"d.",		"dr",		"d`",		"Voiced retroflex stop"},
	[107]={	"c",	"c",		"c",	"c",		"c",		"c",		"c",		"c",		"c",		"Voiceless palatal stop"},
	[108]={	"ɟ",	"j-",		"J",	"J",		"\\j-",		"dj",		"J^",		"J",		"J\\",		"Voiced palatal stop"},
	[109]={	"k",	"k",		"k",	"k",		"k",		"k",		"k",		"k",		"k",		"Voiceless velar stop"},
	[110]={	"ɡ",	"g",		"g",	"g",		"\\gs",		"g",		"g",		"g",		"g",		"Voiced velar stop"},
	[111]={	"q",	"q",		"q",	"q",		"q",		"q",		"q",		"q",		"q",		"Voiceless uvular stop"},
	[112]={	"ɢ",	"G",		"G",	"G",		"\\gc",		"G",		"G",		"Q",		"G",		"Voiced uvular stop"},
	[113]={	"ʔ",	"?",		"?",	"?",		"\\?g",		"?",		"?",		"?",		"?",		"Glottal stop"},
	[114]={	"m",	"m",		"m",	"m",		"m",		"m",		"m"	,		"m",		"m",		"Voiced bilabial nasal"},
	[115]={	"ɱ",	"m\"",		"m>",	"mv",		"\\mj",		"mD",		"M",		"M",		"F",		"Voiced labiodental nasal"},
	[116]={	"n",	"n",		"n",	"n",		"n",		"n",		"n",		"n",		"n",		"Voiced alveolar nasal"},
	[117]={	"ɳ",	"nr)",		"n(",	"nr",		"\\n.",		"n.",		"n.",		"nr",		"n`",		"Voiced retroflex nasal"},
	[118]={	"ɲ",	"nj)",		"n)",	"nj",		"\\nj",		"nj",		"n^",		"n~",		"J",		"Voiced palatal nasal"},
	[119]={	"ŋ",	"ng)",		"g~",	"ng",		"\\ng",		"N",		"N",		"N",		"N",		"Voiced velar nasal"},
	[120]={	"ɴ",	"N",		"N",	"N",		"\\nc",		"nU",		"n\"",		"Nq[a]",	"N\\",		"Voiced uvular nasal"},
	[121]={	"ʙ",	"B",		"",		"bb",		"\\bc",		"",			"b<trl>",	"B",		"B\\",		"Voiced bilabial trill"},
	[122]={	"r",	"r",		"r",	"rr",		"r",		"r",		"r<trl>",	"r",		"r",		"Voiced alveolar trill"},
	[123]={	"ʀ",	"R",		"R",	"RR",		"\\rc",		"R",		"r\\\"",	"R",		"R\\",		"Voiced uvular trill"},
	[124]={	"ɾ",	"r\"",		"r*",	"dt",		"\\fh",		"r[",		"*",		"d(",		"4",		"Voiced alveolar tap"},
	[125]={	"ɽ",	"rr)",		"r(*",	"rt",		"\\f.",		"r.[",		"*.",		"rr",		"r`",		"Voiced retroflex flap"},
	[126]={	"ɸ",	"P\"",		"F",	"F",		"\\ff",		"P",		"P",		"F",		"p\\",		"Voiceless bilabial fricative"},
	[127]={	"β",	"B\"",		"B",	"V",		"\\bf",		"B",		"B",		"V",		"B",		"Voiced bilabial fricative"},
	[128]={	"f",	"f",		"f",	"f",		"f",		"f",		"f",		"f",		"f",		"Voiceless labiodental fricative"},
	[129]={	"v",	"v",		"v",	"v",		"v",		"v",		"v",		"v",		"v",		"Voiced labiodental fricative"},
	[130]={	"θ",	"O-",		"s[]",	"sd",		"\\tf",		"0D",		"T",		"T",		"T",		"Voiceless dental fricative"},
	[131]={	"ð",	"d-",		"z[]",	"zd",		"\\dh",		"6D",		"D",		"D",		"D",		"Voiced dental fricative"},
	[132]={	"s",	"s",		"s",	"s",		"s",		"s",		"s",		"s",		"s",		"Voiceless alveolar fricative"},
	[133]={	"z",	"z",		"z",	"z",		"z",		"z",		"z",		"z",		"z",		"Voiced alveolar fricative"},
	[134]={	"ʃ",	"S",		"sV",	"S",		"\\sh",		"S",		"S",		"S",		"S",		"Voiceless palato-alveolar fricative"},
	[135]={	"ʒ",	"3\"",		"zV",	"Z",		"\\zh",		"Z",		"Z",		"Z",		"Z",		"Voiced palato-alveolar fricative"},
	[136]={	"ʂ",	"sr)",		"s(",	"sr",		"\\s.",		"s.",		"s.",		"sr",		"S`",		"Voiceless retroflex fricative"},
	[137]={	"ʐ",	"zr)",		"z(",	"zr",		"\\z.",		"z.",		"z.",		"zr",		"Z`",		"Voiced retroflex fricative"},
	[138]={	"ç",	"c\"",		"c$",	"c\\",		"\\c,",		"C",		"C",		"C",		"C",		"Voiceless palatal fricative"},
	[139]={	"ʝ",	"j\"",		"j$",	"J\\",		"\\jc",		"jF",		"C<vcd>",	"j^[a]",	"J\\",		"Voiced palatal fricative"},
	[140]={	"x",	"x",		"x",	"x",		"x",		"x",		"x",		"x",		"x",		"Voiceless velar fricative"},
	[141]={	"ɣ",	"g\"",		"r=<",	"g\\",		"\\gf",		"gF",		"Q",		"G",		"G",		"Voiced velar fricative"},
	[142]={	"χ",	"X",		"X",	"X",		"\\cf",		"X",		"X",		"X",		"X",		"Voiceless uvular fricative"},
	[143]={	"ʁ",	"R%",		"R=",	"G\\[b]",	"\\ri",		"RF",		"g\"",		"K",		"R",		"Voiced uvular fricative"},
	[144]={	"ħ",	"h-",		"h<",	"H",		"\\h-",		"H",		"H",		"H",		"X\\",		"Voiceless pharyngeal fricative"},
	[145]={	"ʕ",	"?&",		"6<",	"Hv",		"\\9e",		"9",		"H<vcd>",	"!",		"?\\",		"Voiced pharyngeal fricative"},
	[146]={	"h",	"h",		"h",	"h",		"h",		"h",		"h",		"h",		"h",		"Voiceless glottal fricative"},
	[147]={	"ɦ",	"h\"",		"6",	"hv",		"\\h^",		"hh",		"h<?>",		"hv[a]",	"h\\",		"Voiced glottal fricative"},
	[148]={	"ɬ",	"l-",		"l%$",	"ls",		"\\l-",		"hlF",		"s<lat>",	"hl[a]",	"K",		"Voiceless alveolar lateral fricative"},
	[149]={	"ɮ",	"l3\")",	"l$",	"lz",		"\\lz",		"lF",		"z<lat>",	"Zl[a]",	"K\\",		"Voiced alveolar lateral fricative"},
	[150]={	"ʋ",	"v\"",		"v>",	"",			"\\vs",		"vA",		"r<lbd>",	"V[[a]",	"P",		"Voiced labiodental approximant"},
	[151]={	"ɹ",	"r&",		"r=",	"r",		"\\rt",		"rA",		"r",		"9",		"r\\",		"Voiced alveolar approximant"},
	[152]={	"ɻ",	"jr)",		"r=(",	"",			"\\r.",		"r.A",		"r.",		"9r",		"r\\`",		"Voiced retroflex approximant"},
	[153]={	"j",	"j",		"j",	"j",		"j",		"j",		"j",		"j",		"j",		"Voiced palatal approximant"},
	[154]={	"ɰ",	"m&\"",		"",		"Rg",		"\\ml",		"RA",		"j<vel>",	"4)[a]",	"M\\",		"Voiced velar approximant"},
	[155]={	"l",	"l",		"l",	"l",		"l",		"l",		"l",		"l",		"l",		"Voiced alveolar lateral approximant"},
	[156]={	"ɭ",	"lr)",		"l(",	"lr",		"\\l.",		"l.",		"l.",		"lr",		"l`",		"Voiced retroflex lateral approximant"},
	[157]={	"ʎ",	"y&",		"l)",	"lj",		"\\yt",		"lj",		"l^",		"L",		"L",		"Voiced palatal lateral approximant"},
	[158]={	"ʟ",	"L",		"",		"",			"\\lc",		"L",		"L[c]",		"Lg[a]",	"L\\",		"Voiced velar lateral approximant"},
	[160]={	"ɓ",	"b$",		"b,,",	"b?",		"\\b^",		"b<",		"b`",		"b<",		"b_<",		"Voiced bilabial implosive"},
	[162]={	"ɗ",	"d$",		"d,,",	"d?",		"\\d^",		"d<",		"d`",		"d<",		"d_<",		"Voiced alveolar implosive"},
	[164]={	"ʄ",	"j$",		"J,,",	"J?",		"\\j^",		"dj<",		"J`",		"J<",		"J\\_<",	"Voiced palatal implosive"},
	[166]={	"ɠ",	"g$",		"g,,",	"g?",		"\\g^",		"g<",		"g`",		"g<",		"g_<",		"Voiced velar implosive"},
	[168]={	"ʛ",	"G$",		"G,,",	"G?",		"\\G^",		"G<",		"G`",		"Q<",		"G\\_<",	"Voiced uvular implosive"},
	[169]={	"ʍ",	"w&",		"",		"",			"\\wt",		"hw",		"v<vls>",	"W",		"W",		"Voiceless labial–velar fricative"},
	[170]={	"w",	"w",		"w",	"w",		"w",		"w",		"w",		"w",		"w",		"Voiced labial–velar approximant"},
	[171]={	"ɥ",	"h&",		"w.",	"",			"\\ht",		"wj",		"j<rnd>",	"jw[a]",	"H",		"Voiced labial–palatal approximant"},
	[172]={	"ʜ",	"H",		"",		"",			"\\hc",		"",			"",			"",			"H\\",		"Voiceless epiglottal trill"},
	[173]={	"ʡ",	"?-",		"",		"",			"\\?-",		"99",		"",			"",			">\\",		"Epiglottal stop"},
	[174]={	"ʢ",	"?\"",		"",		"",			"\\9-",		"",			"",			"",			"<\\",		"Voiced epiglottal trill"},
	[175]={	"ɧ",	"Sx)",		"",		"",			"\\hj",		"",			"",			"",			"x\\",		"Sj-sound"},
	[176]={	"ʘ",	"p!",		"p*",	"",			"\\O.",		"",			"p!",		"p|",		"O\\",		"Bilabial click"},
	[177]={	"ǀ",	"t!",		"t*",	"t!",		"\\|1",		"/",		"t!",		"|",		"|\\",		"Dental click"},
	[178]={	"ǃ",	"r!",		"",		"",			"!",		"!",		"c![d]",	"",			"!\\",		"Alveolar click"},
	[179]={	"ǂ",	"c!",		"c*",	"c!",		"\\|-",		"/=",		"c![d]",	"c|",		"=\\",		"Palatal click"},
	[180]={	"ǁ",	"l!",		"l*",	"l!",		"\\|2",		"#",		"l!",		"||",		"|\\|\\",	"Alveolar lateral click"},
	[181]={	"ɺ",	"l\"",		"",		"lt",		"\\rl",		"l[",		"*<lat>",	"l)",		"l\\",		"Voiced alveolar lateral flap"},
	[182]={	"ɕ",	"ci)",		"sV>",	"ss",		"\\cc",		"SJ",		"",			"c}",		"s\\",		"Voiceless alveolo-palatal fricative"},
	[183]={	"ʑ",	"zi)",		"zV>",	"zz",		"\\zc",		"ZJ",		"",			"z}",		"z\\",		"Voiced alveolo-palatal fricative"},
	[184]={	"ⱱ",	"",			"",		"",			"",			"v[",		"",			"",			"",			"Voiced labiodental flap"},
	[209]={	"ɫ",	"l~)",		"l-",	"",			"\\l~",		"l-",		"L[c]",		"",			"5",		"Velarized alveolar lateral approximant"},
	[301]={	"i",	"i",		"i",	"i",		"i",		"i",		"i",		"i",		"i",		"Close front unrounded vowel"},
	[302]={	"e",	"e",		"e",	"e",		"e",		"e",		"e",		"e",		"e",		"Close-mid front unrounded vowel"},
	[303]={	"ɛ",	"E",		"E",	"E",		"\\ef",		"E",		"E",		"E",		"E",		"Open-mid front unrounded vowel"},
	[304]={	"a",	"a",		"a",	"a",		"a",		"a",		"a",		"a",		"a",		"Open front unrounded vowel"},
	[305]={	"ɑ",	"a\"",		"A",	"aa",		"\\as",		"a_",		"A",		"A",		"A",		"Open back unrounded vowel"},
	[306]={	"ɔ",	"c&",		"O",	"O",		"\\ct",		"O",		"O",		">",		"O",		"Open-mid back rounded vowel"},
	[307]={	"o",	"o",		"o",	"o",		"o",		"o",		"o",		"o",		"o",		"Close-mid back rounded vowel"},
	[308]={	"u",	"u",		"u",	"u",		"u",		"u",		"u",		"u",		"u",		"Close back rounded vowel"},
	[309]={	"y",	"y",		"y",	"y",		"y",		"y",		"y",		"y",		"y",		"Close front rounded vowel"},
	[310]={	"ø",	"o/)",		"0",	"oe",		"\\o/",		"o/",		"Y",		"7",		"2",		"Close-mid front rounded vowel"},
	[311]={	"œ",	"oe)",		"E!",	"oE",		"\\oe",		"E)",		"W",		"8",		"9",		"Open-mid front rounded vowel"},
	[312]={	"ɶ",	"OE)",		"a!",	"OE",		"\\Oe",		"",			"&.",		"6",		"&",		"Open front rounded vowel"},
	[313]={	"ɒ",	"a\"&",		"A=",	"ao",		"\\ab",		"a_)",		"A.",		"5",		"Q",		"Open back rounded vowel"},
	[314]={	"ʌ",	"v&",		"^",	"A",		"\\vt",		"^",		"V",		"^",		"V",		"Open-mid back unrounded vowel"},
	[315]={	"ɤ",	"U\"",		"o!",	"oo",		"\\rh",		"o(",		"o-",		"2",		"7",		"Close-mid back unrounded vowel"},
	[316]={	"ɯ",	"m&",		"m=",	"uu",		"\\mt",		"uu",		"u-",		"4",		"M",		"Close back unrounded vowel"},
	[317]={	"ɨ",	"i-",		"i\"",	"i-",		"\\i-",		"i_",		"i\"",		"ix",		"1",		"Close central unrounded vowel"},
	[318]={	"ʉ",	"u-",		"u\"",	"u-",		"\\u-",		"u+",		"u\"",		"ux",		"}",		"Close central rounded vowel"},
	[319]={	"ɪ",	"I",		"I",	"I",		"\\ic",		"I",		"I",		"I",		"I",		"Near-close front unrounded vowel"},
	[320]={	"ʏ",	"Y",		"Y",	"Y",		"\\yc",		"Y",		"U.",		"Y",		"Y",		"Near-close front rounded vowel"},
	[321]={	"ʊ",	"U",		"U",	"U",		"\\hs",		"U",		"U",		"U",		"U",		"Near-close back rounded vowel"},
	[322]={	"ə",	"@",		"e=",	"6",		"\\sw",		"\"@",		"@",		"&",		"@",		"Mid central vowel"},
	[323]={	"ɵ",	"o-",		"o\"",	"",			"\\o-",		"@)",		"@.",		"ox",		"8",		"Close-mid central rounded vowel"},
	[324]={	"ɐ",	"a&",		"",		"",			"\\at",		"4",		"",			"ax",		"6",		"Near-open central vowel"},
	[325]={	"æ",	"ae)",		"@",	"ae",		"\\ae",		"aa",		"&",		"@",		"{",		"Near-open front unrounded vowel"},
	[326]={	"ɜ",	"E&",		"E\"",	"3",		"\\er",		"3",		"V\"",		"3",		"3",		"Open-mid central unrounded vowel"},
	[327]={	"ɚ",	"xr^",		"",		"3r",		"\\sr",		"\"@.",		"R",		"",			"@`",		"R-coloured mid central vowel"},
	[395]={	"ɞ",	"E\"",		"O\"",	"",			"\\kb",		"3)",		"O\"",		"",			"3\\",		"Open-mid central rounded vowel"},
	[397]={	"ɘ",	"e&",		"e\"",	"6",		"\\e-",		"@",		"@<umd>",	"",			"@\\",		"Close-mid central unrounded vowel"},
	[401]={	"◌ʼ",	"`",		"",		"?",		"\\ap",		"'",		"`",		">",		"_>",		"Ejective"},
	[402]={	"◌̥",	"V)",		"%",	",-v",		"\\0v",		"",			"<o>",		"0",		"_0",		"Voiceless"},
	[403]={	"◌̬",	"v)",		"",		",+v",		"",			"",			"<vcd>",	"v",		"_v",		"Voiced"},
	[404]={	"ʰ",	"h^",		"HH",	",h",		"\\^h",		"h",		"<h>",		"h",		"_h",		"Aspirated"},
	[405]={	"◌̤",	"h\")",		"",		",hv",		"\\:v",		"h",		"<?>",		"Hv",		"_t",		"Breathy voiced"},
	[406]={	"◌̰",	"~",		"",		",?v",		"\\~v",		"*",		"",			"?",		"_k",		"Creaky voiced"},
	[407]={	"◌̼",	"{",		"",		"",			"",			"",			"",			"{",		"_N",		"Linguolabial"},
	[408]={	"◌̪",	"[",		"[",	",d",		"\\Nv",		"",			"[",		"[",		"_d",		"Dental"},
	[409]={	"◌̺",	"]",		"",		",ap",		"\\Uv",		"",			"",			"]",		"_a",		"Apical"},
	[410]={	"◌̻",	"[]",		"",		",lm",		"\\Dv",		"",			"",			"}",		"_m",		"Laminal"},
	[411]={	"◌̹",	"u)",		"}",	",+w",		"\\3v",		"",			".",		"(w)[a]",	"_O",		"More rounded"},
	[412]={	"◌̜",	"U)",		"{",	"",			"\\cv",		"",			"-",		"",			"_c",		"Less rounded"},
	[413]={	"◌̟",	"+",		"+",	",<",		"\\+v",		"",			"",			"+",		"_+",		"Advanced"},
	[414]={	"◌̠",	"_",		"-[e]",	",>",		"\\-v",		"",			"",			"-",		"_-",		"Retracted"},
	[415]={	"◌̈",	"\"^",		"\"",	"",			"\\:^",		"",			"\"",		"",			"_\"",		"Centralized"},
	[416]={	"◌̽",	"x^",		"",		"",			"",			"",			"",			"",			"_x",		"Mid-centralized"},
	[417]={	"◌̘",	"<",		"",		"",			"\\T(",		"",			"",			"¿[a][f]",	"_A",		"Advanced tongue root"},
	[418]={	"◌̙",	">",		"",		"",			"\\T)",		"",			"",			"¡[a][f]",	"_q",		"Retracted tongue root"},
	[419]={	"◌˞",	"r^",		"",		"",			"\\hr",		".",		"<r>",		"",			"`",		"Rhoticity"},
	[420]={	"ʷ",	"w^",		"",		",V\\",		"\\^w",		"W",		"<w>",		"w",		"_w",		"Labialized"},
	[421]={	"ʲ",	"j^",		".",	",j",		"\\^j",		"J",		"<pzd>",	"j",		"'",		"Palatalized"},
	[422]={	"ˠ",	"g^",		"",		",g",		"\\^G",		"-",		"<vzd>",	"2",		"_G",		"Velarized"},
	[423]={	"ˤ",	"&g^",		"",		",H",		"\\^9",		"9",		"<H>",		"!",		"_?\\",		"Pharyngealized"},
	[424]={	"◌̃",	"~^",		"~",	",+n",		"\\~^",		"~",		"~",		"~",		"~",		"Nasalized"},
	[425]={	"ⁿ",	"n^",		"",		",n-",		"\\^n",		"n",		"",			"n",		"_n",		"Nasal release"},
	[426]={	"ˡ",	"l^",		"",		",l-",		"\\^l",		"L",		"",			"l",		"_l",		"Lateral release"},
	[427]={	"◌̚",	".)",		"",		",=",		"\\cn",		"",			"<unx>",	"c",		"_}",		"No audible release"},
	[428]={	"◌̴",	"~)",		"-[e]",	"",			"",			"",			"",			"",			"_e",		"Velarized or pharyngealized"},
	[429]={	"◌̝",	"=",		".[g]",	",/",		"\\T^",		"",			"",			"",			"_r",		"Raised"},
	[430]={	"◌̞",	"=\"",		"(",	",\\",		"\\Tv",		"",			"",			"",			"_o",		"Lowered"},
	[431]={	"◌̩",	",)",		"\\",	",$",		"\\|v",		"",			"-",		"=",		"=",		"Syllabic"},
	[432]={	"◌̯",	"(",		"*[h]",	",gl",		"\\nv",		"",			"",			"(",		"_^",		"Non-syllabic"},
	[433]={	"◌͡◌",	"))[i]",	"",		"_",		"\\lip",	"",			"",			"",			"_",		"Affricate or double articulation"},
	[501]={	"ˈ",	"'",		"'",	"$S5",		"\\'1",		"",			"'",		"`",		"\"",		"Primary stress"},
	[502]={	"ˌ",	",",		"`",	"$S3",		"\\'2",		"",			",",		"'",		"%",		"Secondary stress"},
	[503]={	"ː",	":",		"|",	",:",		"\\:f",		":",		":",		":",		":",		"Long"},
	[504]={	"ˑ",	";",		":",	",.",		"\\.f",		"",			"",			";",		":\\",		"Half-long"},
	[505]={	"◌̆",	"(^",		"*[h]",	",--",		"\\N^",		"S",		"",			"(",		"_X",		"Extra-short"},
	[506]={	".",	".",		"",		"$",		".",		"",			"#",		".",		".",		"Syllable break"},
	[507]={	"|",	"|",		"",		"",			"|",		"",			"",			"",			"|",		"Minor (foot) group"},
	[508]={	"‖",	"||",		"",		"",			"||",		"",			"",			"",			"||",		"Major (intonation) group"},
	[509]={	"‿",	"=)",		"",		"",			"\\_ub",	"",			"",			"",			"-\\",		"Linking (absence of a break)"},
	[510]={	"↗",	"/",		"",		"",			"",			"",			"",			"",			"</>",		"Global rise"},
	[511]={	"↘",	"\\",		"",		"",			"",			"",			"",			"",			"<\\>",		"Global fall"},
	[512]={	"◌̋",	"5",		"5",	"$T5-",		"",			"",			"",			"_9",		"_T",		"Extra-high"},
	[513]={	"◌́",	"4",		"4",	"$T4-",		"\\'^",		"",			"",			"_7",		"_H",		"High"},
	[514]={	"◌̄",	"3",		"3",	"$T3-",		"\\-^",		"",			"",			"_5",		"_M",		"Mid"},
	[515]={	"◌̀",	"2",		"2",	"$T2-",		"\\`^",		"",			"",			"_3",		"_L",		"Low"},
	[516]={	"◌̏",	"1",		"1",	"$T1-",		"",			"",			"",			"_1",		"_B",		"Extra-low"},
	[517]={	"ꜛ",	"/)",		"",		"",			"",			"",			"",			"",			"^",		"Upstep"},
	[518]={	"ꜜ",	"\\)",		"",		"",			"",			"",			"",			"",			"!",		"Downstep"},
	[519]={	"˥",	"5",		"5",	"$T5-",		"",			"",			"",			"",			"<T>",		"Extra-high"},
	[520]={	"˦",	"4",		"4",	"$T4-",		"",			"",			"",			"",			"<H>",		"High"},
	[521]={	"˧",	"3",		"3",	"$T3-",		"",			"",			"",			"",			"<M>",		"Mid"},
	[522]={	"˨",	"2",		"2",	"$T2-",		"",			"",			"",			"",			"<L>",		"Low"},
	[523]={	"˩",	"1",		"1",	"$T1-",		"",			"",			"",			"",			"<B>",		"Extra-low"},
	[524]={	"◌̌",	"15",		"15",	"$1/",		"\\v^",		"",			"",			"",			"_L_H",		"Rising"},
	[525]={	"◌̂",	"51",		"51",	"$5\\",		"\\^^",		"",			"",			"",			"_H_L",		"Falling"},
	[526]={	"◌᷄",	"35",		"35",	"$3/",		"",			"",			"",			"",			"_H_T",		"High-rising"},
	[527]={	"◌᷅",	"13",		"13",	"$3\\",		"",			"",			"",			"",			"_B_L",		"Low-rising"},
	[528]={	"◌᷈",	"342",		"342",	"$T3^",		"",			"",			"",			"",			"_M_H_L",	"Rising–falling"},
}
