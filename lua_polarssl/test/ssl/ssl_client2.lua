#!/usr/bin/env lua

-- SSL client with certificate authentication

local polarssl = require"polarssl"
local sys = require"sys"
local sock = require"sys.sock"


local SERVER_PORT = 4433
local SERVER_NAME = "localhost"

local DEBUG_LEVEL = 0

local server_fd
local cacert, clicert, ssl


local function net_connect(host, port)
  local fd = sock.handle()
  assert(fd:socket())
  local addr = assert(sock.getaddrinfo(host))
  assert(fd:connect(sock.addr():inet(port, addr)))
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


local function connect()
  --
  -- 2. Start the connection
  --
  print("  . Connecting to https://" .. SERVER_NAME
    .. ":" .. SERVER_PORT .. "/ ...")

  server_fd = net_connect(SERVER_NAME, SERVER_PORT)

  print(" ok")

  --
  -- 3. Setup stuff
  --
  print("  . Setting up the SSL/TLS structure...")

  ssl = polarssl.ssl()
  if not ssl:init() then return false end

  print(" ok")

  ssl:set_endpoint("client")
  ssl:set_authmode("optional")

  ssl:set_dbg(io.stdout):dbg_level(DEBUG_LEVEL)

  ssl:set_bio(server_fd.read, server_fd, server_fd.write, server_fd)

  ssl:set_ca_cert(cacert)
  ssl:set_peer_cn(SERVER_NAME)
  ssl:set_own_cert(clicert)
  ssl:set_rsa_keytest("client")

  ssl:set_hostname(SERVER_NAME)

  --
  -- 4. Handshake
  --
  print("  . Performing the SSL/TLS handshake...")

  if not ssl:handshake() then return false end

  print(" ok")
  print("  [ Ciphersuite is " .. ssl:get_ciphersuite() .. " ]")

  --
  -- 5. Verify the server certificate
  --
  print("  . Verifying peer X.509 certificate...")

  do
    local expired, revoked, cn_mismatch, not_trusted
      = ssl:get_verify_result()

    if expired then
      print(" failed\n  ! server certificate has expired\n")
    elseif revoked then
      print(" failed\n  ! server certificate has been revoked\n")
    elseif cn_mismatch then
      print(" failed\n  ! CN mismatch (expected CN=" .. SERVER_NAME .. ")\n")
    elseif not_trusted then
      print(" failed\n  ! self-signed or not signed by a trusted CA\n")
    else
      print(" ok")
    end
  end

  print("  . Peer certificate information ...")
  do
    local peer_cert = polarssl.x509_cert()
    ssl:get_peer_cert(peer_cert)
    print(peer_cert:info("    "))
    peer_cert:close(true)
  end

  --
  -- 6. Write the GET request
  --
  print("  > Write to server:")

  do
    local data = "GET / HTTP/1.0\r\n\r\n"

    local res, len = ssl:write(data)
    if res == nil then return false end

    print(" " .. len .. " bytes written\n")
    print(data)
  end

  --
  -- 7. Read the HTTP response
  --
  print("  < Read from server:")

  do
    local data = ssl:read(1024)

    if data then
      print(" " .. #data .. " bytes read\n")
      print(data)
    end
  end

  ssl:close_notify()

  return true
end

local function main()
  --
  -- 1.1. Load the trusted CA
  --
  print("  . Loading the CA root certificate ...")

  cacert = polarssl.x509_cert()
  assert(cacert:parse_test"ca")

  print("ok")

  --
  -- 1.2. Load own certificate
  --
  -- (can be skipped if client authentication is not required)
  --
  print("  . Loading the client cert. ...")

  clicert = polarssl.x509_cert()
  assert(clicert:parse_test"client")

  print("ok")


  if not connect() then
    print("ERROR:", SYS_ERR)
    return false
  end
  return true
end

sys.exit(main() and 0 or 1)
