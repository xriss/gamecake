cd `dirname $0`

wget https://www.sqlite.org/2025/sqlite-amalgamation-3490100.zip
unzip sqlite-amalgamation-3490100.zip

rm -rf sqlite-amalgamation
mv sqlite-amalgamation-3490100 sqlite-amalgamation
rm sqlite-amalgamation-3490100.zip


