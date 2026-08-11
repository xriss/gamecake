cd `dirname $0`

wget https://fossies.org/linux/misc/giflib-6.1.3.tar.gz
tar -xf giflib-6.1.3.tar.gz

rm -rf giflib
mv giflib-6.1.3 giflib
rm giflib-6.1.3.tar.gz

