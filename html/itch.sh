cd `dirname $0`

DATE=`date '+%y%m%d'`

cp ../lua/fun/poopeepanda.fun.lua poopeepanda/poopeepanda.fun.lua

cd poopeepanda



ZIPNAME=poopeepanda-$DATE.web.zip
rm  ../$ZIPNAME
zip -r ../$ZIPNAME js
zip -r ../$ZIPNAME exe/gamecake.wasm
zip -r ../$ZIPNAME exe/gamecake.js
zip -r ../$ZIPNAME index.html
zip -r ../$ZIPNAME poopeepanda.fun.lua



ZIPNAME=poopeepanda-$DATE.win.zip
rm  ../$ZIPNAME
zip -r ../$ZIPNAME exe/gamecake.exe
zip -r ../$ZIPNAME poopeepanda.fun.lua
zip -r ../$ZIPNAME poopeepanda.bat

