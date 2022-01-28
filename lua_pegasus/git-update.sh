cd `dirname $0`

rm -rf git
git clone git@github.com:xriss/pegasus.lua.git git
rm -rf git/.git

cp git/LICENSE .
