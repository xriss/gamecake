cd `dirname $0`
../bin/dbg/nginx -pnginx -sstop
../bin/dbg/nginx -pnginx $*
echo "http://127.0.0.1:8888/"
