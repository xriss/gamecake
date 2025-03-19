cd `dirname $0`


rm test.zip
( cd ../test && zip -r ../html/test.zip . )


xdg-open "http://localhost:12211/?dir=./dbg/&args=--,lua/unit.lua&zipfile=test.zip" &

exe/gamecake.http server --policy=wasm --location=.

