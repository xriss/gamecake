local http = require("socket.http")
local ltn12 = require("ltn12")

local base_url = "https://httpbin.org/"

--Helper for priniting nested table
function deep_print(tbl)
    for i, v in pairs(tbl) do
        if type(v) == "table" then 
            deep_print(v)
        else 
            print(i, v) 
        end
    end
end


function http_request( args )
--http.request(url [, body])
--http.request{
--  url = string,
--  [sink = LTN12 sink,]
--  [method = string,]
--  [headers = header-table,]
--  [source = LTN12 source],
--  [step = LTN12 pump step,]
--  [proxy = string,]
--  [redirect = boolean,]
--  [create = function]
--}
--
--
    local resp, r = {}, {}
    if args.endpoint then
        local params = ""
        if args.method == nil or args.method == "GET" then
            -- prepare query parameters like http://xyz.com?q=23&a=2
            if args.params then
                for i, v in pairs(args.params) do
                    params = params .. i .. "=" .. v .. "&"
                end
            end
        end
        params = string.sub(params, 1, -2)
        local url = ""
        if params then url = base_url .. args.endpoint .. "?" .. params else url = base_url .. args.endpoint end
        client, code, headers, status = http.request{url=url, sink=ltn12.sink.table(resp),
                                                method=args.method or "GET", headers=args.headers, source=args.source,
                                                step=args.step,     proxy=args.proxy, redirect=args.redirect, create=args.create }
        r['code'], r['headers'], r['status'], r['response'] = code, headers, status, resp
    else
        error("endpoint is missing")
    end
    return r
end

function main()
    -- Normal GET request
    endpoint = "/user-agent"
    print(endpoint)
    deep_print(http_request{endpoint=endpoint})
    -- GET request with parameters
    endpoint = "/get"
    print(endpoint)
    deep_print(http_request{endpoint=endpoint, params={age=23, name    ="kracekumar"}})
    -- POST request with form
    endpoint = "/post"
    print(endpoint)
    local req_body = "a=2"
    local headers = {
    ["Content-Type"] = "application/x-www-form-urlencoded";
    ["Content-Length"] = #req_body;
    }
    deep_print(http_request{endpoint=endpoint, method="POST", source=ltn12.source.string(req_body), headers=headers})
    -- PATCH Method
    endpoint = "/patch"
    print(endpoint)
    deep_print(http_request{endpoint=endpoint, method="PATCH"})
    -- PUT Method
    endpoint = "/put"
    print(endpoint)
    deep_print(http_request{endpoint=endpoint, method="PUT", source    =ltn12.source.string("a=23")})
    -- Delete method
    endpoint = "/delete"
    print(endpoint)
    deep_print(http_request{endpoint=endpoint, method="DELETE",     source=ltn12.source.string("a=23")})

end

main()

