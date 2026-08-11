cd `dirname $0`

echo "ERRORROROROROROOROROR NO WORKING "

wget https://www.ijg.org/files/jpegsrc.v10.tar.gz
tar -xf jpegsrc.v10.tar.gz

rm -rf jpeg
mv jpeg-10 jpeg
rm jpegsrc.v10.tar.gz

