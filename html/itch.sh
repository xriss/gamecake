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
zip -r ../$ZIPNAME poopeepanda.win.bat

ZIPNAME=poopeepanda-$DATE.linux.zip
rm  ../$ZIPNAME
zip -r ../$ZIPNAME exe/gamecake.x64
zip -r ../$ZIPNAME exe/gamecake.a64
zip -r ../$ZIPNAME poopeepanda.fun.lua
zip -r ../$ZIPNAME poopeepanda.linux.bat

cd ..
ZIPNAME=poopeepanda-$DATE.apk
../android/make ../lua/fun/poopeepanda.fun.lua fun
cp ../android/gamecake/build/outputs/apk/release/gamecake-release.apk $ZIPNAME 
