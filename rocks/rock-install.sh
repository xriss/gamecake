
if [ -z "$ROCK_DIR" ]; then
	echo "Must be run via rock.sh to set env correctly."
	exit 20
fi
cd $ROCK_DIR

luarocks install --local --force $ROCK_DIR/$ROCK_BASENAME.src.rock
