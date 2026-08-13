
if [ -z "$ROCK_DIR" ]; then
	echo "Must be run via rock.sh to set env correctly."
	exit 20
fi
cd $ROCK_DIR

rm -f src.zip

zip -r src.zip src

luarocks pack $ROCK_FILENAME

rm -f src.zip

