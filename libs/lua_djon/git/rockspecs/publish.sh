
cd `dirname $0`
cd ..

. ~/keys.env

LUAROCKS_SPEC=`echo rockspecs/djon-1*`

luarocks upload $LUAROCKS_SPEC --api-key=$LUAROCKS_KEY

