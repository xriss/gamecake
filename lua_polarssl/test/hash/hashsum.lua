#!/usr/bin/env lua

-- Hashsum demonstration program

local polarssl = require"polarssl"

local stdout, stderr = io.stdout, io.stderr
local Match = string.match


local hash_type

local function hash_wrapper(filename)
  local sum, err = polarssl.hash_file(hash_type, filename)

  if not sum then
    stderr:write(err, ": ", filename, "\n")
  end
  return sum
end

local function hash_print(filename)
  local sum = hash_wrapper(filename)
  if not sum then return false end

  stdout:write(sum, "  ", filename, "\n")
  return true
end

local function hash_check(filename)
  local f, err = io.open(filename, "r")
  if not f then
    stderr:write(err, ": ", filename, "\n")
    return false
  end

  local nb_err1, nb_err2 = 0, 0
  local nb_tot1, nb_tot2 = 0, 0

  for line in f:lines() do
    local filesum, filename = Match(line, "^(%w+)  ([^\r\n]+)")

    if filesum and filename then
      nb_tot1 = nb_tot1 + 1

      local sum = hash_wrapper(filename)
      if not sum then
        nb_err1 = nb_err1 + 1
      else
        nb_tot2 = nb_tot2 + 1

        if sum ~= filesum then
          nb_err2 = nb_err2 + 1
          stderr:write("wrong checksum: ", filename, "\n")
        end
      end
    end
  end

  if nb_err1 ~= 0 then
    stdout:write("WARNING: ", nb_err1, " (out of ",
      nb_tot1, ") input files could not be read\n")
  end

  if nb_err2 ~= 0 then
    stdout:write("WARNING: ", nb_err2, " (out of ",
      nb_tot2, ") computed checksums did not match\n")
  end

  f:close()
  return (nb_err1 == 0 and nb_err2 == 0)
end

local function main(arg)
  local narg = #arg

  if narg == 0 then
    print"print mode:  hashsum <type> <file> <file> ..."
    print"check mode:  hashsum <type> -c <checksum file>"
    print"  type:  MD2 MD4 MD5 SHA1 SHA224 SHA256 SHA384 SHA512"
    return false
  end

  hash_type = arg[1]

  if narg == 3 and arg[2] == "-c" then
    return hash_check(arg[3])
  end

  local res = true
  for i = 2, narg do
    if hash_print(arg[i]) ~= 0 then
      res = false
    end
  end
  return res
end

os.exit(main{...} and 0 or 1)
