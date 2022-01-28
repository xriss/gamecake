local function normalizePath(path)
  local value = string.gsub(path ,'\\', '/')
  value = string.gsub(value, '^/*', '/')
  value = string.gsub(value, '(/%.%.?)$', '%1/')
  value = string.gsub(value, '/%./', '/')
  value = string.gsub(value, '/+', '/')

  while true do
    local first, last = string.find(value, '/[^/]+/%.%./')
    if not first then break end
    value = string.sub(value, 1, first) .. string.sub(value, last + 1)
  end

  while true do
    local n
    value, n = string.gsub(value, '^/%.%.?/', '/')
    if n == 0 then break end
  end

  while true do
    local n
    value, n = string.gsub(value, '/%.%.?$', '/')
    if n == 0 then break end
  end

  return value
end

local Request = {}
Request.__index = Request
Request.PATTERN_METHOD = '^(.-)%s'
Request.PATTERN_PATH = '(%S+)%s*'
Request.PATTERN_PROTOCOL = '(HTTP%/%d%.%d)'
Request.PATTERN_REQUEST = (Request.PATTERN_METHOD ..
Request.PATTERN_PATH ..Request.PATTERN_PROTOCOL)
Request.PATTERN_QUERY_STRING = '([^=]*)=([^&]*)&?'
Request.PATTERN_HEADER = '([%w-]+): ([%w %p]+=?)'

function Request:new(port, client, server)
  local obj = {}
  obj.client = client
  obj.server = server
  obj.port = port
  obj.ip = client:getpeername()
  obj.querystring = {}
  obj._firstLine = nil
  obj._method = nil
  obj._path = nil
  obj._params = {}
  obj._headerParsed = false
  obj._headers = {}
  obj._contentDone = 0
  obj._contentLength = nil

  return setmetatable(obj, self)
end

function Request:parseFirstLine()
  if (self._firstLine ~= nil) then
    return
  end

  local status, partial
  self._firstLine, status, partial = self.client:receive()

  if (self._firstLine == nil or status == 'timeout' or partial == '' or status == 'closed') then
    return
  end

  -- Parse firstline http: METHOD PATH
  -- GET Makefile HTTP/1.1
  local method, path = string.match(self._firstLine, Request.PATTERN_REQUEST)

  if not method then
    self.client:close()
    self.server:close()

    return
  end

  print('Request for: ' .. path)

  local filename = ''
  local querystring = ''

  if #path then
    filename, querystring = string.match(path, '^([^#?]+)[#|?]?(.*)')
    filename = normalizePath(filename)
  end

  self._path = filename
  self._method = method
  self.querystring = self:parseUrlEncoded(querystring)
end

function Request:parseUrlEncoded(value)
  local output = {}

  if value then
    for k, v in  string.gmatch(value, Request.PATTERN_QUERY_STRING) do
        output[k] = v
    end
  end

  return output
end

function Request:post()
  if self:method() ~= 'POST' then return nil end
  local data = self:receiveBody()
  return self:parseUrlEncoded(data)
end

function Request:path()
  self:parseFirstLine()
  return self._path
end

function Request:method()
  self:parseFirstLine()
  return self._method
end

function Request:headers()
  if self._headerParsed then
    return self._headers
  end

  self:parseFirstLine()

  local data = self.client:receive()

  while (data ~= nil) and (data:len() > 0) do
    local key, value = string.match(data, Request.PATTERN_HEADER)

    if key and value then
      self._headers[key] = value
    end

    data = self.client:receive()
  end

  self._headerParsed = true
  self._contentLength = tonumber(self._headers["Content-Length"] or 0)

  return self._headers
end

function Request:receiveBody(size)
  size = size or self._contentLength

  -- do we have content?
  if (self._contentLength == nil) or (self._contentDone >= self._contentLength) then
    return false
  end

  -- fetch in chunks
  local fetch = math.min(self._contentLength - self._contentDone, size)
  local data, err, partial = self.client:receive(fetch)

  if err == 'timeout' then
    data = partial
  end

  self._contentDone = self._contentDone + #data

  return data
end

return Request
