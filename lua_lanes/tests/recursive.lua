--
-- RECURSIVE.LUA
--
-- Test program for Lua Lanes
--

--
-- This uses an upvalue that refers to itself
--
if false then
    require "lanes"

    local func
    func =
    function( depth )
        print("depth = " .. depth)
        if depth > 10 then
            return "done!"
        end
    
        local lgen= lanes.gen("*", func)
        local lane= lgen( depth+1 )
        return lane[1]
    end
    
    local v= func(0)
    print(v)    -- "done!"
    
    assert(v=="done!")
end


--
-- Same without upvalue but using 'require' within the lanes (does not work, why: TBD)
--
if false then
    local function f( depth )
        print("depth = " .. depth)
        if depth > 10 then
            return "done!"
        end
    
        assert( require )
        require "lanes"     -- gives: "'package.loaders' must be a table"

        local lgen= lanes.gen("*", debug.getinfo(1,"f").func)
        local lane= lgen( depth+1 )
        return lane[1]
    end
    
    local v= f(0)
    print(v)    -- "done!"
    
    assert(v=="done!")
end


--
-- Same without upvalue, and with 'require' only at the main level 
-- ('lanes' is passed onto others as upvalue table)
--
do
    local lanes= require "lanes"
    assert(lanes)

    local function f( depth )
        print("depth = " .. depth)
        if depth > 10 then
            return "done!"
        end
    
        local lgen= lanes.gen("*", debug.getinfo(1,"f").func)
        local lane= lgen( depth+1 )
        return lane[1]
    end
    
    local v= f(0)
    print(v)    -- "done!"
    
    assert(v=="done!")
end
