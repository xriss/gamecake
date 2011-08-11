--[[
	The XML parser.
--]]

local ti, sf = table.insert, string.format

-- ** State stack **

local Stack = {}
local StackMT = {__index = Stack}

function Stack.new()
	local self = {top = 0}
	setmetatable(self, StackMT)
	
	return self
end

function Stack:push(state)
	self.top = self.top + 1
	self[self.top] = state 
end

function Stack:peek()
	return self[self.top]
end

function Stack:pop()
	self.top = self.top - 1
	return self[self.top + 1]
end

-- ** XML parser ** --

local symbols = {}
local scSymbols = {}
local included = {}

-- Reuses the symbols table.
function parse(name, v1, v2)
	log('Parsing file ' .. name)
	local file = io.open(name)
	
	if not file then
		log('-> Couldn\'t open the file ' .. name) 
		return
	end
	
	local state = {}
	local stack = Stack.new()
	local defName, defVersion
	local classes = {}
	local structs = {}
	local enums = {}
	local functions = {}

	local TagHandler = {		
		['namespace'] = function(attr)
			defName = attr['name']
			defVersion = attr['version']
		end,
		
		-- parse the included files recursively
		['include'] = function(attr)
			local lname = attr['name']
			
			if not included[lname] then
				included[lname] = true
				local incname = sf([[%s%s-%s.gir]], getPath(name), lname, attr['version'])
				parse(incname, v1, v2)
			end
		end,
		
		['class'] = function(attr)
			local tp     = attr['c:type'] or attr['glib:type-name']
			local parent = attr['parent']
			local longn  = sf('%s.%s', defName, attr['name'])
			
			if parent and not parent:find('%.') then
				parent = sf('%s.%s', defName, parent)
			end
			
			local class = {
				['name']        = tp,
				['parent']      = parent,
				['methods']     = {},
				['properties']  = {},
				['implements']  = {},
			}
			
			state.class = class
			symbols[longn]          = tp
			symbols[longn .. '*']   = tp .. '*'
			symbols[longn .. '**']  = tp .. '**'
			scSymbols[tp .. '*']    = 'GObject*'
			scSymbols[tp .. '**']   = 'GObject**'
			ti(classes, class)
			
			local getType = {
				['name'] = attr['glib:get-type'],
				['class'] = class.name,
				['since'] = defVersion,
				['args'] = {},
				['ret'] = {['type'] = 'GType', ['ownership'] = 'none'},
			}
			
			ti(class.methods, getType)
			ti(functions, getType)
		end,
		
		['interface'] = function(attr)
			local tp = attr['c:type']
			local longn = sf('%s.%s', defName, attr['name'])
			
			local class = {
				['name'] = tp,
				['abstract'] = true,
				['methods'] = {},
				['properties'] = {},
				['implements'] = {}, 			-- Not used
				['parent'] = attr['parent'], 	-- Not used
			}
			
			state.class = class
			symbols[longn] = tp
			symbols[longn .. '*'] = tp .. '*'
			symbols[longn .. '**'] = tp .. '**'
			scSymbols[tp .. '*'] = 'GObject*'
			scSymbols[tp .. '**'] = 'GObject**'
			ti(classes, class)
		end,
		
		-- ignored
		['virtual-method'] = function(attr)
			local tag, class, ctype = stack:peek()
						
			if tag == 'class' or tag == 'interface' then
				class = state.class.name
			elseif tag == 'record' or tag == 'union' or 'glib:boxed' then
				class = state.struct.name
			end
						
			local method = {
				['args'] = {},
			}
			
			state.method = method
			state.args = method.args
		end,
		
		['record'] = function(attr)
			local tp = attr['c:type']
			local longn = sf('%s.%s', defName, attr['name'])
			
			local struct = {
				['name'] = tp,
				['fields'] = {}
			}
			
			ti(structs, struct)
			symbols[longn] = tp
			symbols[longn .. '*'] = tp .. '*'
			symbols[longn .. '**'] = tp .. '**'
			scSymbols[tp .. '*'] = 'struct*'
			scSymbols[tp .. '**'] = 'struct**'
			state.struct = struct
		end,
		
		['union'] = function(attr)
			local tp = attr['c:type']
			local longn = sf('%s.%s', defName, attr['name'])
			
			local struct = {
				['name'] = tp,
				['fields'] = {}
			}
			
			ti(structs, struct)
			symbols[longn] = tp
			symbols[longn .. '*'] = tp .. '*'
			symbols[longn .. '**'] = tp .. '**'
			scSymbols[tp .. '*'] = 'struct*'
			scSymbols[tp .. '**'] = 'struct**'
			state.struct = struct
		end,	
		
		['constructor'] = function(attr)
			local tag, class = stack:peek()
			
			if tag == 'class' then
				class = state.class.name
			elseif tag == 'record' or tag == 'union' then
				class = state.struct.name
			end
		
			local method = {
				['name'] = attr['c:identifier'],
				['class'] = class,
				['deprecated'] = attr['deprecated'] and (attr['deprecated-version'] or defVersion),
				['since'] = attr['version'] or defVersion,
				['constructor'] = true,
				['args'] = {},
				['throws'] = attr['throws'] == '1',
			}
				
			state.method = method
			state.args = method.args
			ti(functions, method)
				
		end,
		
		['method'] = function(attr)
			local tag, class, ctype = stack:peek()
						
			if tag == 'class' or tag == 'interface' then
				class = state.class.name
			elseif tag == 'record' or tag == 'union' or 'glib:boxed' then
				class = state.struct.name
			end
						
			local method = {
				['name'] = attr['c:identifier'],
				['class'] = class,
				['deprecated'] = attr['deprecated'] and (attr['deprecated-version'] or defVersion),
				['since'] = attr['version'] or defVersion,
				['args'] = {},
				['throws'] = attr['throws'] == '1',
			}
			
			ti(method.args, {
				['ownership'] = 'none',
				['type'] = class .. '*',
			})
			
			if tag == 'class' then
				ti(state.class.methods, method)
			end
			
			state.method = method
			state.args = method.args
			ti(functions, method)
		end,
		
		['function'] = function(attr)
			local method = {
				['name'] = attr['c:identifier'],
				['deprecated'] = attr['deprecated'] and (attr['deprecated-version'] or defVersion),
				['since'] = attr['version'] or defVersion,
				['args'] = {},
				['throws'] = attr['throws'] == '1',
			}
				
			state.method = method
			state.args = method.args
			ti(functions, method)
		end,
		
		['callback'] = function(attr)
			local tp = attr['c:type']
			
			local method = {
				['name'] = tp,
				['args'] = {},
			}
				
			state.method = method
			state.args   = method.args
            
            if tp then symbols[tp]  = 'callback' end
		end,
		
		['glib:signal'] = function(attr)
			local tp = attr['name']
			
			local method = {
				['name'] = tp,
				['args'] = {},
			}
				
			state.method = method
			state.args = method.args
		end,
		
		['return-value'] = function(attr)
			local tag = stack:peek()
			
			state.method.ret = {
				['ownership'] = attr['transfer-ownership'] or 'none'
			}
		end,
		
		['parameter'] = function(attr)
			local arg = {
				['ownership'] = attr['transfer-ownership'] or 'none',
				['type'] = 'unhandled'
			}
			
			state.arg = arg
			ti(state.args, arg)
		end,
		
		['property'] = function(attr)
			local name = attr['name']
			
			local property = {
				['name'] = name,
				['since'] = attr['version'] or defVersion,
			}
			
			state.property = property
			state.class.properties[name] = property
		end,
		
		['alias'] = function(attr)
            state.alias = attr['c:type'] or 'unhandled'
		end,
		
		['implements'] = function(attr)
			local name = attr['name']
			ti(state.class.implements, fullName(name, defName))
		end,
		
		['enumeration'] = function(attr)
			local tp = attr['c:type']
		
			local enum = {
				['name'] = tp,
				['members'] = {}
			}
			
			symbols[tp] = 'gint'
			symbols[tp .. '*'] = 'gint*'
			ti(enums, enum)
			state.enum = enum
		end,
		
		['bitfield'] = function(attr)
			local tp = attr['c:type']
		
			local enum = {
				['name'] = tp,
				['members'] = {}
			}
			
			symbols[tp] = 'gint'
			symbols[tp .. '*'] = 'gint*'
			ti(enums, enum)
			state.enum = enum
		end,
		
		['member'] = function(attr)
			local member = {
				['name'] = attr['c:identifier'],
				['value'] = attr['value'],
			}
			
			ti(state.enum.members, member)
		end,
		
		['field'] = function(attr)
			local tag = stack:peek()
			
			if tp == 'record' then
				local field = {
					['name'] = attr['name'],
					['writable'] = attr['writable'] == 1
				}
			
				ti(state.struct.fields, field)
				state.field = field
			end
		end,
		
		['type'] = function(attr)
			local tag = stack:peek()
			local tp = attr['c:type'] or attr['name']
			
			if tag == 'return-value' then
				state.method.ret.type = tp
			elseif tag == 'parameter' then
				state.arg.type = tp
			elseif tag == 'property' then
				state.property.type = tp
			elseif tag == 'alias' then
                local tp = state.alias
			    local tg = attr['name']
                symbols[tp]         = tg
			    symbols[tp .. '*']  = tg .. '*'
			    symbols[tp .. '**'] = tg .. '**'
            end
		end,
		
		['array'] = function(attr)
			local tag = stack:peek()
			local tp  = attr['c:type'] or 'unhandled'
		
			-- currently, only char arrays are accepted
			if not tp:match('char') or tp:match('%*%*') then return end
			
			if tag == 'return-value' then
				state.method.ret.type = tp
			elseif tag == 'parameter' then
				state.arg.type        = tp
			elseif tag == 'property' then
				state.property.type   = tp
			end
		end,
	}

	local callbacks = {
		StartElement = function(p, name, attr)
			if TagHandler[name] then
				TagHandler[name](attr)
			end
			
			stack:push(name)
		end,
		
		EndElement = function(p, name)
			stack:pop()
		end,
	}

	local p = lxp.new(callbacks)

	for l in file:lines() do
		p:parse(l)
		p:parse("\n")
	end

	p:parse()
	p:close()
	file:close()
	
	return defName, classes, structs, enums, functions, symbols, scSymbols
end
