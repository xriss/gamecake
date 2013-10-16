# vim:set ft= ts=4 sw=4 et fdm=marker:
use lib 'lib';
use t::TestNginxLua;

#worker_connections(1014);
#master_process_enabled(1);
#log_level('warn');

repeat_each(2);

plan tests => repeat_each() * (blocks() * 2 + 2);

#no_diff();
#no_long_string();
#master_on();
#workers(2);
run_tests();

__DATA__

=== TEST 1: set indexed variables to nil
--- config
    location = /test {
        set $var 32;
        content_by_lua '
            ngx.say("old: ", ngx.var.var)
            ngx.var.var = nil
            ngx.say("new: ", ngx.var.var)
        ';
    }
--- request
GET /test
--- response_body
old: 32
new: nil



=== TEST 2: set variables with set_handler to nil
--- config
    location = /test {
        content_by_lua '
            ngx.say("old: ", ngx.var.args)
            ngx.var.args = nil
            ngx.say("new: ", ngx.var.args)
        ';
    }
--- request
GET /test?hello=world
--- response_body
old: hello=world
new: nil



=== TEST 3: reference nonexistent variable
--- config
    location = /test {
        set $var 32;
        content_by_lua '
            ngx.say("value: ", ngx.var.notfound)
        ';
    }
--- request
GET /test
--- response_body
value: nil



=== TEST 4: no-hash variables
--- config
    location = /test {
        proxy_pass http://127.0.0.1:$server_port/foo;
        header_filter_by_lua '
            ngx.header["X-My-Host"] = ngx.var.proxy_host
        ';
    }

    location = /foo {
        echo foo;
    }
--- request
GET /test
--- response_headers
X-My-Host: foo
--- response_body
foo
--- SKIP



=== TEST 5: variable name is caseless
--- config
    location = /test {
        set $Var 32;
        content_by_lua '
            ngx.say("value: ", ngx.var.VAR)
        ';
    }
--- request
GET /test
--- response_body
value: 32



=== TEST 6: true $invalid_referer variable value in Lua
github issue #239
--- config
    location = /t {
        valid_referers www.foo.com;
        content_by_lua '
            ngx.say("invalid referer: ", ngx.var.invalid_referer)
            ngx.exit(200)
        ';
        #echo $invalid_referer;
    }

--- request
GET /t
--- more_headers
Referer: http://www.foo.com/

--- response_body
invalid referer: 

--- no_error_log
[error]



=== TEST 7: false $invalid_referer variable value in Lua
github issue #239
--- config
    location = /t {
        valid_referers www.foo.com;
        content_by_lua '
            ngx.say("invalid referer: ", ngx.var.invalid_referer)
            ngx.exit(200)
        ';
        #echo $invalid_referer;
    }

--- request
GET /t
--- more_headers
Referer: http://www.bar.com

--- response_body
invalid referer: 1

--- no_error_log
[error]

