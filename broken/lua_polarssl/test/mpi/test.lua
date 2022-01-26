#!/usr/bin/env lua

local polarssl = require("polarssl")


local p = polarssl.mpi()
assert(p:gen_prime(100))
print("p", p:get(10))

local q = polarssl.mpi()
assert(q:gen_prime(250))
print("q", q:get(10))

local m = p * q
print("m", m:get(10))

local mx = (p - 1) * (q - 1)
print("mx", mx:get(10))

local e = polarssl.mpi()
assert(e:set("10001"))
print("e", e:get(10))

local d = polarssl.mpi()
assert(d:inv_mod(e, mx))
print("d", d:get(10))

assert(((e * d) % mx):get(10) == '1')

local t = polarssl.mpi()
local x = polarssl.mpi()
local y = polarssl.mpi()
assert(t:set(2))
assert(x:exp_mod(t, e, m))
assert(y:exp_mod(x, d, m))
assert(t == y)
print()

local message = "The quick brown fox jumps over the lazy dog"
print("message as text")
print(message)

local encoded = polarssl.mpi()
assert(encoded:set(message, 0))
print("encoded message")
print(encoded:get(10))
assert(encoded < m)
print(encoded:get(0))
assert(message == encoded:get(0))

local x = polarssl.mpi()
assert(x:exp_mod(encoded, e, m))
print("encrypted message")
print(x:get(10))

local y = polarssl.mpi()
assert(y:exp_mod(x, d, m))
print("decrypted message as number")
print(y:get(10))
assert(y == encoded)

y = y:get(0)
print("decrypted message as text")
print(y)
assert(y == message)

