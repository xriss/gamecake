#!/usr/bin/env lua

-- Diffie-Hellman-Merkle key exchange (prime generation)

local polarssl = require"polarssl"


--
-- Note: G = 4 is always a quadratic residue mod P,
-- so it is a generator of order Q (with P = 2*Q+1).
--
local DH_P_SIZE = 1024
local GENERATOR = "4"


local function main()
  local G, P, Q = polarssl.mpi(), polarssl.mpi(), polarssl.mpi()
  local hs = polarssl.ssl()

  assert(G.gen_prime, "  ! Prime-number generation is not available.")

  G:set(GENERATOR, 10)

  print("  . Seeding the random number generator...")

  assert(hs:init())

  print(" ok\n  . Generating the modulus, please wait...")

  --
  -- This can take a long time...
  --
  assert(P:gen_prime(DH_P_SIZE, true, hs))

  print(" ok\n  . Verifying that Q = (P-1)/2 is prime...")

  assert(Q:sub(P, 1))
  assert(Q:div(Q, 2))
  assert(Q:is_prime(hs))

  print(" ok\n  . Exporting the value in dh_prime.txt...")

  local fout = assert(io.open("dh_prime.txt", "wb+"))

  assert(P:write_file(fout, "P = ", 16))
  assert(G:write_file(fout, "G = ", 16))

  fout:close()
  print(" ok")

  return true
end

os.exit(main() and 0 or 1)
