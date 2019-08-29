#! /usr/bin/lua
require 'DataDumper'   -- http://lua-users.org/wiki/DataDumper
------------------------ test infrastructure --------------------
local function warn(str)
    io.stderr:write(str,'\n')
end
local function die(str)
    io.stderr:write(str,'\n')
    os.exit(1)
end
local function equals(t1,t2)
    if DataDumper(t1) == DataDumper(t2) then return true else return false end
end
local function readOnly(t)  -- Programming in Lua, page 127
    local proxy = {}
    local mt = {
        __index = t,
        __newindex = function (t, k, v)
            die("attempt to update a read-only table")
        end
    }
    setmetatable(proxy, mt)
    return proxy
end
local function deepcopy(object)  -- http://lua-users.org/wiki/CopyTable
    local lookup_table = {}
    local function _copy(object)
        if type(object) ~= "table" then
            return object
        elseif lookup_table[object] then
            return lookup_table[object]
        end
        local new_table = {}
        lookup_table[object] = new_table
        for index, value in pairs(object) do
            new_table[_copy(index)] = _copy(value)
        end
        return setmetatable(new_table, getmetatable(object))
    end
    return _copy(object)
end

local Test = 24 ; local i_test = 0; local Failed = 0;
function ok(b,s)
    i_test = i_test + 1
    if b then
        io.write('ok '..i_test..' - '..s.."\n")
    else
        io.write('not ok '..i_test..' - '..s.."\n")
        Failed = Failed + 1
    end
	return b
end
-------------------------------------------------------------
ALSA = require 'midialsa'
-- print("ALSA="..DataDumper(ALSA))
-- for index, value in pairs(ALSA) do   --- 5.2
-- 	print("index="..index.."  "..type(value))
-- end

function fwrite (fmt, ...)
	return io.write(string.format(fmt, ...))
end

warn('# This test script is not very portable: it depends')
warn('# on virmidi ports on 20:0 and 21:0, for example...')

rc = ALSA.inputpending()
ok(not rc, "inputpending() with no client returned "..tostring(rc))

cl,po = ALSA.parse_address('97:3');
ok((cl==97) and (po==3), "parse_address('97:3') with no client returned 97,3");

local my_name='test_a_lua'
rc = ALSA.client(my_name,2,2,1)
ok(rc, "client('"..my_name.."',2,2,1)")

-- my ($seconds, $microseconds) = Time::HiRes::gettimeofday;
-- my $start_time = $seconds + 1.0E-6 * $microseconds;
-- No builtin sub-second time; could usedate +%s.%N
-- As a cheapo, use 2.5

id = ALSA.id()
ok(id > 0, 'id() returns '..id)

cl,po = ALSA.parse_address(my_name)
if not ok(cl==id, "parse_address('"..my_name.."') returns "..id..","..po) then
	print('# it returned instead: '..cl..','..po)
end

cl,po = ALSA.parse_address('test_a')
if not ok(cl==id, "parse_address('test_a') returns "..id..","..po) then
	print('# it returned instead: '..cl..','..po)
end

local vm  = 20  -- you may have to change this on your system...
rc = ALSA.connectfrom(1,vm,0)
ok(rc, 'connectfrom(1,'..vm..',0)')

rc = ALSA.connectfrom(1,133,0)
ok(not rc, 'connectfrom(1,133,0) correctly reported failure')

rc = ALSA.connectto(2,21,0)
ok(rc, 'connectto(2,21,0)')

rc = ALSA.connectto(1,133,0)
ok(not rc, 'connectto(1,133,0) correctly reported failure')

rc = ALSA.start()
ok(rc, 'start()')

local qid = ALSA.queue_id()
if not ok((qid >= 0 and qid ~= ALSA.SND_SEQ_QUEUE_DIRECT),
  "queue_id is not negative and not SND_SEQ_QUEUE_DIRECT") then
	print('# queue_id() returned '..qid)
end

fd = ALSA.fd()
ok(fd > 0, 'fd()')

num2name = ALSA.listclients();
ok(num2name[id] == my_name, "listclients()");

num2nports = ALSA.listnumports();
ok(num2nports[id] == 4, "listnumports()");

local inp = assert(io.open('/dev/snd/midiC1D0','wb'))  -- client 20
local oup = assert(io.open('/dev/snd/midiC1D1','rb'))  -- client 21

warn('# feeding ourselves a patch_change event...')
assert(inp:write(string.char(12*16, 99))) -- {'patch_change',0,0,99}
assert(inp:flush())
rc =  ALSA.inputpending()
ok(rc > 0, 'inputpending() returns '..rc)
local alsaevent  = ALSA.input()
correct = {11, 1, 0, 1, 300, {vm,0}, {id,1}, {0, 0, 0, 0, 0, 99} }
alsaevent[4] = 1
alsaevent[5] = 300
if not ok(equals(alsaevent, correct),
 'input() returns {11,1,0,1,300,{vm,0},{id,1},{0,0,0,0,0,99}}') then
	print("# alsaevent="..DataDumper(alsaevent));
	print("# correct="..DataDumper(correct));
end
local e = ALSA.alsa2scoreevent(alsaevent)
correct = {'patch_change',300000,0,99}
ok(equals(e, correct),
 'alsa2scoreevent() returns {"patch_change",300000,0,99}')

warn('# feeding ourselves a control_change event...')
assert(inp:write(string.char(11*16+2,10,103))) -- {'control_change',3,2,10,103}
assert(inp:flush())
rc =  ALSA.inputpending()
local alsaevent  = ALSA.input()
correct = {10, 1, 0, 1, 300, {vm,0}, {id,1}, {2, 0, 0, 0,10,103} }
alsaevent[5] = 300
if not ok(equals(alsaevent, correct),
 'input() returns {10,1,0,1,300,{vm,0},{id,1},{2,0,0,0,10,103}}') then
	print("# alsaevent="..DataDumper(alsaevent));
	print("# correct="..DataDumper(correct));
end
local e = ALSA.alsa2scoreevent(alsaevent)
correct = {'control_change',300000,2,10,103}
ok(equals(e, correct),
 'alsa2scoreevent() returns {"control_change",300000,2,10,103}')

warn('# feeding ourselves a note_on event...')
assert(inp:write(string.char(9*16, 60,101))) -- {'note_on',0,60,101}
assert(inp:flush())
local alsaevent  = ALSA.input()
local save_time = alsaevent[5]
correct = { 6, 1, 0, 1, 300, { vm, 0 }, { id, 1 }, { 0, 60, 101, 0, 0 } }
alsaevent[5] = 300
alsaevent[8][5] = 0
if not ok(equals(alsaevent, correct),
 'input() returns {6,1,0,1,300,{vm,0},{id,1},{0,60,101,0,0}}') then
	print("# alsaevent="..DataDumper(alsaevent));
	print("# correct="..DataDumper(correct));
end
local scoreevent = ALSA.alsa2scoreevent(alsaevent)

warn('# feeding ourselves a note_off event...')
assert(inp:write(string.char(8*16, 60,101))) -- {'note_off',0,60,101}
assert(inp:flush())
rc =  ALSA.inputpending()
local alsaevent  = ALSA.input()
local save_time = alsaevent[5]
correct = { 7, 1, 0, 1, 301, { vm, 0 }, { id, 1 }, { 0, 60, 101, 0, 0 } }
alsaevent[5] = 301
alsaevent[8][5] = 0
ok(equals(alsaevent, correct),
 'input() returns {7,1,0,1,301,{vm,0},{id,1},{0,60,101,0,0}}')
scoreevent = ALSA.alsa2scoreevent(alsaevent)
-- print("scoreevent="..DataDumper(scoreevent))
scoreevent[2] = 300000
correct = {'note',300000,1000,0,60,101}
if not ok(equals(scoreevent, correct),
 'alsa2scoreevent() returns {"note",300000,1000,0,60,101}') then
	print("# alsaevent="..DataDumper(alsaevent));
	print("# correct="..DataDumper(correct));
end

warn("# feeding ourselves a sysex_f0 event...")
assert(inp:write("\240}hello world\247"))
assert(inp:flush())
alsaevent  = ALSA.input();
save_time = alsaevent[5];
correct = {130, 5, 0, 1, 300, {vm,0}, {id,1},
     {"\240}hello world\247",nil,nil,nil,0} }
alsaevent[5] = 300;
alsaevent[8][5] = 0;
if not ok(equals(alsaevent, correct),
 'input() returns {130,5,0,1,300,{vm,0},{id,1},{"\\240}hello world\\247"}}') then
	print("# alsaevent="..DataDumper(alsaevent));
	print("# correct="..DataDumper(correct));
end
scoreevent = ALSA.alsa2scoreevent(alsaevent);
scoreevent[2] = 300000;
correct = {'sysex_f0',300000,"}hello world\247"}
ok(equals(scoreevent, correct),
 'alsa2scoreevent() returns {"sysex_f0",300000,"}hello world\\247"}');

to = ALSA.listconnectedto()
correct = {{2,21,0},}
--print('to='..DataDumper(to))
ok(equals(to, correct), "listconnectedto() returns {{2,21,0}}")
from = ALSA.listconnectedfrom()
correct = {{1,vm,0},}
--print('from='..DataDumper(from))
ok(equals(from, correct), "listconnectedfrom() returns {{1,vm,0}}")


warn('# outputting a patch_change event...')
alsaevent = {11, 1, 0, 1, 0.5, {id,1}, {21,0}, {0, 0, 0, 0, 0, 99} }
rc =  ALSA.output(alsaevent)
bytes = assert(oup:read(2))
-- warn('# bytes = '..string.format('%d %d',string.byte(bytes),string.byte(bytes,2)))
ok(equals(bytes, string.char(12*16, 99)), 'patch_change event detected')

warn('# outputting a control_change event...')
alsaevent = {10, 1, 0, 1, 1.5, {id,1}, {21,0}, {2, 0, 0, 0,10,103} }
rc =  ALSA.output(alsaevent)
bytes = assert(oup:read(3))
ok(equals(bytes, string.char(11*16+2,10,103)), 'control_change event detected')

warn('# outputting a note_on event...')
alsaevent = { 6, 1, 0, 1, 2.0, {id,1}, {21,0}, { 0, 60, 101, 0, 0 } }
rc =  ALSA.output(alsaevent)
bytes = assert(oup:read(3))
ok(equals(bytes, string.char(9*16, 60,101)), 'note_on event detected')

warn('# outputting a note_off event...')
alsaevent = { 7, 1, 0, 1, 2.5, {id,1}, {21,0}, { 0, 60, 101, 0, 0 } }
rc =  ALSA.output(alsaevent)
bytes = assert(oup:read(3))
ok(equals(bytes, string.char(8*16, 60,101)), 'note_off event detected')

warn('# running  aconnect -d '..vm..' '..id..':1 ...')
os.execute('aconnect -d '..vm..' '..id..':1')
for i=1,5 do  -- 1.17
	rc =  ALSA.inputpending()
	alsaevent  = ALSA.input()
	if alsaevent[1] ~= ALSA.SND_SEQ_EVENT_SENSING then break end
	local cl = table.concat(alsaevent[6],":")
	print ("# discarding a SND_SEQ_EVENT_SENSING event from "..cl)
end
if not ok(alsaevent[1] == ALSA.SND_SEQ_EVENT_PORT_UNSUBSCRIBED,
 'SND_SEQ_EVENT_PORT_UNSUBSCRIBED event received') then
	print("# inputpending returned "..rc)
	print("# alsaevent="..DataDumper(alsaevent));
end

rc = ALSA.disconnectto(2,21,0)
ok(rc, "disconnectto(2,21,0)")

rc = ALSA.connectto(2, my_name..":1")
ok(rc, "connectto(2,'"..my_name..":1') connected to myself by name")

correct = {11, 1, 0, qid, 2.8, {id,2}, {id,1}, {0, 0, 0, 0, 0, 99} }
rc =  ALSA.output(correct)
for i=1,5 do  -- 1.17
	rc =  ALSA.inputpending()
	alsaevent  = ALSA.input()
	if alsaevent[1] ~= ALSA.SND_SEQ_EVENT_SENSING then break end
	local cl = table.concat(alsaevent[6],":")
	print ("# discarding a SND_SEQ_EVENT_SENSING event from "..cl)
end

latency = math.floor(0.5 + 1000000 * (alsaevent[5]-correct[5]))
-- alsaevent[4] = 1  -- sometimes it's 0
alsaevent[5] = correct[5]
if not ok(equals(alsaevent, correct), "received an event from myself") then
	print("# alsaevent="..DataDumper(alsaevent));
	print("# correct="..DataDumper(correct));
end
ok(latency < 10000, "latency was "..latency.." microsec")

rc =  ALSA.disconnectfrom(1,id,2)
ok(rc, "disconnectfrom(1,"..id..",2)")

a={}
a = ALSA.status();
ok(a[1], 'status() reports running');
abs_error = 2.8 - a[2]
if abs_error < 0 then abs_error = 0-abs_error end
ok(abs_error < 0.1, "status() reports time = "..a[2].." not 2.8");

os.execute('sleep 1');
a = ALSA.status();
abs_error = 3.8 - a[2]
if abs_error < 0 then abs_error = 0-abs_error end
ok(abs_error < 0.1, "status() reports time = "..a[2].." not 3.8");

rc = ALSA.stop()
ok(rc,'stop() returns success')

alsaevent = ALSA.noteonevent(15, 72, 100, 2.7)
correct = {6,1,0,qid,2.7,{0,0},{0,0},{15,72,100,0,0}}
-- print("alsaevent="..DataDumper(alsaevent))
-- print(" correct ="..DataDumper(correct))
ok(equals(alsaevent, correct), 'noteonevent()')

alsaevent = ALSA.noteoffevent(15, 72, 100, 2.7)
correct = {7,1,0,qid,2.7,{0,0},{0,0},{15,72,100,100,0}}
-- print("alsaevent="..DataDumper(alsaevent))
-- print(" correct ="..DataDumper(correct))
ok(equals(alsaevent, correct), 'noteoffevent()')

alsaevent  = ALSA.noteevent(15, 72, 100, 2.7, 3.1)
scoreevent = ALSA.alsa2scoreevent(alsaevent)
correct = {'note',2700,3100,15,72,100}
ok(equals(scoreevent, correct), 'noteevent()')

alsaevent = ALSA.pgmchangeevent(11, 98, 2.7)
scoreevent = ALSA.alsa2scoreevent(alsaevent)
correct = {'patch_change',2700,11,98}
ok(equals(scoreevent, correct), 'pgmchangeevent() with time>=0')

alsaevent = ALSA.pgmchangeevent(11, 98)
scoreevent = ALSA.alsa2scoreevent(alsaevent)
correct = {'patch_change',0,11,98}
ok(equals(scoreevent, correct), 'pgmchangeevent() with time undefined')

alsaevent = ALSA.pitchbendevent(11, 98, 2.7)
scoreevent = ALSA.alsa2scoreevent(alsaevent)
correct = {'pitch_wheel_change',2700,11,98}
ok(equals(scoreevent, correct), 'pitchbendevent() with time>=0')

alsaevent = ALSA.pitchbendevent(11, 98)
scoreevent = ALSA.alsa2scoreevent(alsaevent)
correct = {'pitch_wheel_change',0,11,98}
ok(equals(scoreevent, correct), 'pitchbendevent() with time undefined')

alsaevent = ALSA.chanpress(11, 98, 2.7)
scoreevent = ALSA.alsa2scoreevent(alsaevent)
correct = {'channel_after_touch',2700,11,98}
ok(equals(scoreevent, correct), 'chanpress() with time>=0')

alsaevent = ALSA.chanpress(11, 98)
scoreevent = ALSA.alsa2scoreevent(alsaevent)
correct = {'channel_after_touch',0,11,98}
ok(equals(scoreevent, correct), 'chanpress() with time undefined')

correct = {'note',0,1000,15,72,100}
alsaevent = ALSA.scoreevent2alsa(correct)
scoreevent = ALSA.alsa2scoreevent(alsaevent);
ok(equals(scoreevent, correct), 'scoreevent2alsa({"note"...})');

correct = {'control_change',10,15,72,100}
alsaevent = ALSA.scoreevent2alsa(correct);
scoreevent = ALSA.alsa2scoreevent(alsaevent);
ok(equals(scoreevent, correct), 'scoreevent2alsa({"control_change"...})');

correct = {'patch_change',10,15,72}
alsaevent = ALSA.scoreevent2alsa(correct);
scoreevent = ALSA.alsa2scoreevent(alsaevent);
ok(equals(scoreevent, correct), 'scoreevent2alsa({"patch_change"...})');

correct = {'pitch_wheel_change',10,15,3232}
alsaevent = ALSA.scoreevent2alsa(correct);
scoreevent = ALSA.alsa2scoreevent(alsaevent);
ok(equals(scoreevent, correct), 'scoreevent2alsa({"pitch_wheel_change"...})');

correct = {'channel_after_touch',10,15,123}
alsaevent = ALSA.scoreevent2alsa(correct);
scoreevent = ALSA.alsa2scoreevent(alsaevent);
ok(equals(scoreevent, correct), 'scoreevent2alsa({"channel_after_touch"...})');

correct = {'sysex_f0',2,"}hello world\247"}
alsaevent = ALSA.scoreevent2alsa(correct);
scoreevent = ALSA.alsa2scoreevent(alsaevent);
ok(equals(scoreevent, correct), 'scoreevent2alsa({"sysex_f0"...})');

correct = {'sysex_f7',2,"that's all folks\xF7"}
alsaevent = ALSA.scoreevent2alsa(correct);
-- print "alsaevent=",Dumper(alsaevent);
scoreevent = ALSA.alsa2scoreevent(alsaevent);
-- print "scoreevent=",Dumper(scoreevent),"correct=",Dumper(correct);
ok(equals(scoreevent, correct), 'scoreevent2alsa({"sysex_f7"...})');
