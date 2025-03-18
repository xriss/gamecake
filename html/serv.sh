cd `dirname $0`

#xdg-open http://localhost:12211/exe/index.html?terminal=1\&cakefile=../out/swanky.zip &

exe/gamecake.http server --policy=wasm --location=.

