cd `dirname $0`


rm itch.zip


zip -r itch.zip js
zip -r itch.zip exe/gamecake.wasm
zip -r itch.zip exe/gamecake.js
zip -r itch.zip index.html


cp ../lua/fun/poopeepanda.fun.lua start.fun.lua
zip -r itch.zip start.fun.lua


