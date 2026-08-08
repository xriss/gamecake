cd `dirname $0`


rm -rf git
#git clone --branch v0.9 git@github.com:brunoos/luasec.git git
git clone git@github.com:brunoos/luasec.git git
rm -rf git/.git


cp git/src/ssl.lua   ../../lua/
cp git/src/https.lua   ../../lua/ssl/

