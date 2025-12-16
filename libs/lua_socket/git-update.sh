cd `dirname $0`


rm -rf git
git clone https://github.com/diegonehab/luasocket.git git
rm -rf git/.git

cp git/src/ltn12.lua   ../lua/
cp git/src/mime.lua    ../lua/
cp git/src/socket.lua  ../lua/

cp git/src/ftp.lua     ../lua/socket/
cp git/src/headers.lua ../lua/socket/
cp git/src/http.lua    ../lua/socket/
cp git/src/mbox.lua    ../lua/socket/
cp git/src/smtp.lua    ../lua/socket/
cp git/src/tp.lua      ../lua/socket/
cp git/src/url.lua     ../lua/socket/



