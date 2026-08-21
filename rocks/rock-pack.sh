
if [ -z "$ROCK_DIR" ]; then
	echo "Must be run via rock.sh to set env correctly."
	exit 20
fi
cd $ROCK_DIR


# check for special ./rock-pre-pack.sh actions
if [ -e "./rock-pre-pack.sh" ]; then

./rock-pre-pack.sh

fi


rm -f src.zip
zip -r src.zip src

luarocks pack $ROCK_BASENAME.rockspec
#rm -f $ROCK_BASENAME.rockspec

rm -f src.zip
