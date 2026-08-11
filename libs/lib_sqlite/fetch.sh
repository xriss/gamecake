cd `dirname $0`

wget https://sqlite.org/2026/sqlite-amalgamation-3530400.zip
unzip sqlite-amalgamation-3530400.zip

rm -rf sqlite-amalgamation
mv sqlite-amalgamation-3530400 sqlite-amalgamation
rm sqlite-amalgamation-3530400.zip


