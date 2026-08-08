cd `dirname $0`


rm -rf git
git clone git@github.com:brunoos/luasec.git git
rm -rf git/.git


cp git/src/ssl.lua   ../../lua/
cp git/src/https.lua   ../../lua/ssl/

