cd `dirname $0`


rm -rf git
git clone --depth 1 git@github.com:freetype/freetype.git git
rm -rf git/.git
rm -rf git/.gitignore



