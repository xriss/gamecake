cd `dirname $0`

rm src.zip
zip -r src.zip src

luarocks pack gamecake-22-001.rockspec

echo " now you need to do sommthing like ... "
echo "  luarocks upload gamecake-22-001.rockspec --api-key=secrret --force "


