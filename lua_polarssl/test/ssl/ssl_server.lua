#!/usr/bin/env lua

-- SSL server demonstration program

local polarssl = require"polarssl"
local sys = require"sys"
local sock = require"sys.sock"


local SERVER_PORT = 4433

local DEBUG_LEVEL = 0

-- Sorted by order of preference
local my_ciphersuites = {
  "TLS-RSA-WITH-RC4-128-MD5",
  "TLS-RSA-WITH-RC4-128-SHA",
  "TLS-RSA-WITH-3DES-EDE-CBC-SHA",
  "TLS-DHE-RSA-WITH-3DES-EDE-CBC-SHA",
  "TLS-RSA-WITH-AES-128-CBC-SHA",
  "TLS-DHE-RSA-WITH-AES-128-CBC-SHA",
  "TLS-RSA-WITH-AES-256-CBC-SHA",
  "TLS-DHE-RSA-WITH-AES-256-CBC-SHA",
  "TLS-RSA-WITH-AES-128-CBC-SHA256",
  "TLS-RSA-WITH-AES-256-CBC-SHA256",
  "TLS-DHE-RSA-WITH-AES-128-CBC-SHA256",
  "TLS-DHE-RSA-WITH-AES-256-CBC-SHA256",
  "TLS-RSA-WITH-AES-128-GCM-SHA256",
  "TLS-RSA-WITH-AES-256-GCM-SHA384",
  "TLS-DHE-RSA-WITH-AES-128-GCM-SHA256",
  "TLS-DHE-RSA-WITH-AES-256-GCM-SHA384",
  "TLS-RSA-WITH-CAMELLIA-128-CBC-SHA",
  "TLS-DHE-RSA-WITH-CAMELLIA-128-CBC-SHA",
  "TLS-RSA-WITH-CAMELLIA-256-CBC-SHA",
  "TLS-DHE-RSA-WITH-CAMELLIA-256-CBC-SHA",
  "TLS-RSA-WITH-CAMELLIA-128-CBC-SHA256",
  "TLS-DHE-RSA-WITH-CAMELLIA-128-CBC-SHA256",
  "TLS-RSA-WITH-CAMELLIA-256-CBC-SHA256",
  "TLS-DHE-RSA-WITH-CAMELLIA-256-CBC-SHA256",
  "TLS-RSA-WITH-NULL-MD5",
  "TLS-RSA-WITH-NULL-SHA",
  "TLS-RSA-WITH-NULL-SHA256",
  "TLS-RSA-WITH-DES-CBC-SHA",
  "TLS-DHE-RSA-WITH-DES-CBC-SHA"
}

local listen_fd, client_fd
local srvcert, ssl


local function net_bind(host, port)
  local fd = sock.handle()
  assert(fd:socket())
  assert(fd:sockopt("reuseaddr", 1))
  local saddr = sock.addr()
  assert(saddr:inet(port, sock.inet_pton(host)))
  assert(fd:bind(saddr))
  assert(fd:listen(10))
  return fd
end

local function net_close(fd)
  fd:shutdown()
  fd:close()
end


local get_session, set_session
do
  local sessions = setmetatable({}, {__mode = "v"})

  get_session = function(ssl, sid)
    return sessions[sid]
  end

  set_session = function(ssl, sid)
    local ssn = sessions[sid]
    if not ssn then
      ssn = polarssl.session()
      sessions[sid] = ssn
    end
    return ssn
  end
end


local function accept()
  net_close(client_fd)
  ssl:close()

  print("  . Waiting for a remote connection ...")

  if not listen_fd:accept(client_fd) then return false end

  print(" ok")

  --
  -- 4. Setup stuff
  --
  print("  . Setting up the RNG and SSL data....")

  if not ssl:init() then return false end

  print(" ok")

  ssl:set_endpoint("server")
  ssl:set_authmode("none")

  ssl:set_dbg(io.stdout):dbg_level(DEBUG_LEVEL)

  ssl:set_bio(client_fd.read, client_fd, client_fd.write, client_fd)

  ssl:set_session_cache(get_session, set_session)

  ssl:set_ciphersuites(my_ciphersuites)

  ssl:set_ca_cert(srvcert, 1)
  ssl:set_own_cert(srvcert)
  ssl:set_rsa_keytest("server")

  --
  -- 5. Handshake
  --
  print("  . Performing the SSL/TLS handshake...")

  if not ssl:handshake() then return false end

  print(" ok")

  --
  -- 6. Read the HTTP Request
  --
  print("  < Read from client:")

  do
    local data = ssl:read(1024)

    if data then
      print(" " .. #data .. " bytes read\n")
      print(data)
    end
  end

  --
  -- 7. Write the 200 Response
  --
  print("  > Write to client:")

  do
    local data = "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n"
      .. "<h2><p><center>Successful connection using: "
      .. ssl:get_ciphersuite() .. "\r\n"

    local res, len = ssl:write(data)
    if res == nil then return false end

    print(" " .. len .. " bytes written\n")
    print(data)
  end

  ssl:close_notify()

  return true
end

local function main()
  --
  -- 1. Load the certificates and private RSA key
  --
  print("\n  . Loading the server cert. and key...")

  srvcert = polarssl.x509_cert()
  assert(srvcert:parse_test"server")
  assert(srvcert:parse_test"ca")

  print("ok")

  --
  -- 2. Setup the listening TCP socket
  --
  print("  . Bind on https://localhost:" .. SERVER_PORT .. "/ ...")

  listen_fd = net_bind("127.0.0.1", SERVER_PORT)

  print("ok")

  --
  -- 3. Wait until a client connects
  --
  if sys.win32 then
    sys.run("https://localhost:" .. SERVER_PORT .. "/")
  end

  client_fd = sock.handle()

  ssl = polarssl.ssl()

  while true do
    if not accept() then
      print("ERROR:", SYS_ERR)
    end
  end
end

sys.exit(main() and 0 or 1)
