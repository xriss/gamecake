
ngx.say("Hello oldworld")

ngx.say(ngx.var.request_uri)

for i,v in pairs(ngx.req) do

ngx.say(tostring(i).." = "..tostring(v))

end

