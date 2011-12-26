# vim:set ft= ts=4 sw=4 et fdm=marker:

use lib 'lib';
use Test::Nginx::Socket;

#worker_connections(1014);
#master_on();
log_level('debug');

#repeat_each(120);
repeat_each(3);

plan tests => repeat_each() * (blocks() * 2 + 2);

our $HtmlDir = html_dir;
#warn $html_dir;

$ENV{LUA_CPATH} = "/home/lz/luax/?.so;;";

#no_diff();
#no_long_string();

$ENV{TEST_NGINX_MEMCACHED_PORT} ||= 11211;

run_tests();

__DATA__

=== TEST 1: sanity
--- http_config eval
    "lua_package_path '$::HtmlDir/?.lua;./?.lua';"
--- config
    location /load {
        content_by_lua '
            package.loaded.foo = nil;
            local foo = require "foo";
            foo.hi()
        ';
    }
--- request
GET /load
--- user_files
>>> foo.lua
module(..., package.seeall);

function foo () 
    return 1
    return 2
end
--- error_code: 500
--- response_body_like: 500 Internal Server Error



=== TEST 2: sanity
--- http_config
lua_package_path '/home/agentz/rpm/BUILD/lua-yajl-1.1/build/?.so;/home/lz/luax/?.so;./?.so';
--- config
    location = '/report/listBidwordPrices4lzExtra.htm' {
        content_by_lua '
            local yajl = require "yajl"
            local w = ngx.var.arg_words
            w = ngx.unescape_uri(w)
            local r = {}
            print("start for")
            for id in string.gmatch(w, "%d+") do
                 r[id] = -1
            end
            print("end for, start yajl")
            ngx.print(yajl.to_string(r))
            print("end yajl")
        ';
    }
--- request
GET /report/listBidwordPrices4lzExtra.htm?words=123,156,2532
--- response_body
--- SKIP



=== TEST 3: sanity
--- config
    location = /memc {
        #set $memc_value 'hello';
        set $memc_value $arg_v;
        set $memc_cmd $arg_c;
        set $memc_key $arg_k;
        #set $memc_value hello;

        memc_pass 127.0.0.1:$TEST_NGINX_MEMCACHED_PORT;
        #echo $memc_value;
    }
    location = /echo {
        echo_location '/memc?c=get&k=foo';
        echo_location '/memc?c=set&k=foo&v=hello';
        echo_location '/memc?c=get&k=foo';
    }
    location = /main {
        content_by_lua '
            res = ngx.location.capture("/memc?c=get&k=foo&v=")
            ngx.say("1: ", res.body)

            res = ngx.location.capture("/memc?c=set&k=foo&v=bar");
            ngx.say("2: ", res.body);

            res = ngx.location.capture("/memc?c=get&k=foo")
            ngx.say("3: ", res.body);
        ';
    }
--- request
GET /main
--- response_body_like: 3: bar$



=== TEST 4: capture works for subrequests with internal redirects
--- config
    location /lua {
        content_by_lua '
            local res = ngx.location.capture("/")
            ngx.say(res.status)
            ngx.print(res.body)
        ';
    }
--- request
    GET /lua
--- response_body_like chop
200
.*It works
--- SKIP



=== TEST 5: disk file bufs not working
--- config
    location /lua {
        content_by_lua '
            local res = ngx.location.capture("/test.lua")
            ngx.say(res.status)
            ngx.print(res.body)
        ';
    }
--- user_files
>>> test.lua
print("Hello, world")
--- request
    GET /lua
--- response_body
200
print("Hello, world")



=== TEST 6: print lua empty strings
--- config
    location /lua {
        content_by_lua 'ngx.print("") ngx.flush() ngx.print("Hi")';
    }
--- request
GET /lua
--- response_body chop
Hi



=== TEST 7: say lua empty strings
--- config
    location /lua {
        content_by_lua 'ngx.say("") ngx.flush() ngx.print("Hi")';
    }
--- request
GET /lua
--- response_body eval
"
Hi"



=== TEST 8: github issue 37: header bug
https://github.com/chaoslawful/lua-nginx-module/issues/37
--- config
    location /sub {
        content_by_lua '
            ngx.header["Set-Cookie"] = {"TestCookie1=foo", "TestCookie2=bar"};
            ngx.say("Hello")
        ';
    }
    location /lua {
        content_by_lua '
            -- local yajl = require "yajl"
            ngx.header["Set-Cookie"] = {}
            res = ngx.location.capture("/sub")

            for i,j in pairs(res.header) do
                ngx.header[i] = j
            end

            -- ngx.say("set-cookie: ", yajl.to_string(res.header["Set-Cookie"]))

            ngx.send_headers()
            ngx.print("body: ", res.body)
        ';
    }
--- request
GET /lua
--- raw_response_headers_like eval
".*Set-Cookie: TestCookie1=foo\r
Set-Cookie: TestCookie2=bar.*"



=== TEST 9: memory leak
--- config
    location /foo {
        content_by_lua_file 'html/foo.lua';
    }
--- user_files
>>> foo.lua
res = {}
res = {'good 1', 'good 2', 'good 3'}
return ngx.redirect("/somedir/" .. ngx.escape_uri(res[math.random(1,#res)]))
--- request
    GET /foo
--- response_body
--- SKIP



=== TEST 10: capturing locations with internal redirects (no lua redirect)
--- config
    location /bar {
        echo Bar;
    }
    location /foo {
        #content_by_lua '
        #ngx.exec("/bar")
        #';
        echo_exec /bar;
    }
    location /main {
        content_by_lua '
            local res = ngx.location.capture("/foo")
            ngx.print(res.body)
        ';
    }
--- request
    GET /main
--- response_body
Bar



=== TEST 11: capturing locations with internal redirects (lua redirect)
--- config
    location /bar {
        content_by_lua 'ngx.say("Bar")';
    }
    location /foo {
        content_by_lua '
            ngx.exec("/bar")
        ';
    }
    location /main {
        content_by_lua '
            local res = ngx.location.capture("/foo")
            ngx.print(res.body)
        ';
    }
--- request
    GET /main
--- response_body
Bar



=== TEST 12: capturing locations with internal redirects (simple index)
--- config
    location /main {
        content_by_lua '
            local res = ngx.location.capture("/")
            ngx.print(res.body)
        ';
    }
--- request
    GET /main
--- response_body chop
<html><head><title>It works!</title></head><body>It works!</body></html>



=== TEST 13: capturing locations with internal redirects (more lua statements)
--- config
    location /bar {
        content_by_lua '
            ngx.say("hello")
            ngx.say("world")
        ';
    }
    location /foo {
        #content_by_lua '
        #ngx.exec("/bar")
        #';
        echo_exec /bar;
    }
    location /main {
        content_by_lua '
            local res = ngx.location.capture("/foo")
            ngx.print(res.body)
        ';
    }
--- request
    GET /main
--- response_body
hello
world



=== TEST 14: capturing locations with internal redirects (post subrequest with internal redirect)
--- config
    location /bar {
        lua_need_request_body on;
        client_body_in_single_buffer on;

        content_by_lua '
            ngx.say(ngx.var.request_body)
        ';
    }
    location /foo {
        #content_by_lua '
        #ngx.exec("/bar")
        #';
        echo_exec /bar;
    }
    location /main {
        content_by_lua '
            local res = ngx.location.capture("/foo", { method = ngx.HTTP_POST, body = "hello" })
            ngx.print(res.body)
        ';
    }
--- request
    GET /main
--- response_body
hello



=== TEST 15: nginx rewrite works in subrequests
--- config
    rewrite /foo /foo/ permanent;
    location = /foo/ {
        echo hello;
    }
    location /main {
        content_by_lua '
            local res = ngx.location.capture("/foo")
            ngx.say("status = ", res.status)
            ngx.say("Location: ", res.header["Location"] or "nil")
        ';
    }
--- request
    GET /main
--- response_body
status = 301
Location: /foo/



=== TEST 16: nginx rewrite works in subrequests
--- config
    access_by_lua '
        local res = ngx.location.capture(ngx.var.uri)
        ngx.say("status = ", res.status)
        ngx.say("Location: ", res.header["Location"] or "nil")
        ngx.exit(200)
    ';
--- request
    GET /foo
--- user_files
>>> foo/index.html
It works!
--- response_body
status = 301
Location: /foo/



=== TEST 17: set content-type header with charset
--- config
    location /lua {
        charset GBK;
        content_by_lua '
            ngx.header.content_type = "text/xml; charset=UTF-8"
            ngx.say("hi")
        ';
    }
--- request
    GET /lua
--- response_body
hi
--- response_headers
Content-Type: text/xml; charset=UTF-8



=== TEST 18: set response header content-type with charset
--- config
    location /lua {
        charset GBK;
        content_by_lua '
            ngx.header.content_type = "text/xml"
            ngx.say("hi")
        ';
    }
--- request
    GET /lua
--- response_body
hi
--- response_headers
Content-Type: text/xml; charset=GBK



=== TEST 19: get by-position capturing variables
--- config
    location ~ '^/lua/(.*)' {
        content_by_lua '
            ngx.say(ngx.var[1] or "nil")
        ';
    }
--- request
    GET /lua/hello
--- response_body
hello



=== TEST 20: get by-position capturing variables ($0)
--- config
    location ~ '^/lua/(.*)' {
        content_by_lua '
            ngx.say(ngx.var[0] or "nil")
        ';
    }
--- request
    GET /lua/hello
--- response_body
nil



=== TEST 21: get by-position capturing variables (exceeding captures)
--- config
    location ~ '^/lua/(.*)' {
        content_by_lua '
            ngx.say(ngx.var[2] or "nil")
        ';
    }
--- request
    GET /lua/hello
--- response_body
nil



=== TEST 22: get by-position capturing variables ($1, $2)
--- config
    location ~ '^/lua/(.*)/(.*)' {
        content_by_lua '
            ngx.say(ngx.var[-1] or "nil")
            ngx.say(ngx.var[0] or "nil")
            ngx.say(ngx.var[1] or "nil")
            ngx.say(ngx.var[2] or "nil")
            ngx.say(ngx.var[3] or "nil")
            ngx.say(ngx.var[4] or "nil")
        ';
    }
--- request
    GET /lua/hello/world
--- response_body
nil
nil
hello
world
nil
nil



=== TEST 23: set special variables
--- config
    location /main {
        #set_unescape_uri $cookie_a "hello";
        set $http_a "hello";
        content_by_lua '
            ngx.say(ngx.var.http_a)
        ';
    }
--- request
    GET /main
--- response_body
hello
--- SKIP



=== TEST 24: set special variables
--- config
    location /main {
        content_by_lua '
            dofile(ngx.var.realpath_root .. "/a.lua")
        ';
    }
    location /echo {
        echo hi;
    }
--- request
    GET /main
--- user_files
>>> a.lua
ngx.location.capture("/echo")
--- response_body
--- SKIP

