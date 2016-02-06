
rm base -rf
mkdir base

cd base
tar xvfJ ../libpng-1.6.16.tar.xz

cd libpng-1.6.16
patch -p1 < ../../libpng-1.6.16-apng.patch

cd ../..

