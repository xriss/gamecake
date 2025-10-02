cd `dirname $0`


rm test.zip
( cd ../test && zip -r ../html/test.zip . )


xdg-open "http://localhost:12211/?dir=./exe/&zipfile=cake.zip&args=-lcmd,swed,--logs" &

exe/gamecake.http server --policy=wasm --location=.

