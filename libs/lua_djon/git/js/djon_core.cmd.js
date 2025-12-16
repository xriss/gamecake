#!/usr/bin/env node

import * as fs   from 'fs'

  const dat = fs.readFileSync("./djon_core.wasm")
  const bas = Buffer.from(dat).toString('base64')
  const url = `data:application/wasm;base64,${bas}`

  fs.writeFileSync("./djon_core.wasm.js",`

import { Environment, napi } from "napi-wasm"

const r = await fetch("${url}")
const b = await r.arrayBuffer()
const m = await WebAssembly.instantiate( b , {napi:napi } )
const e = new Environment(m.instance)

const djoncore=e.exports.djoncore
export { djoncore }

`,{encoding:"utf8"})


