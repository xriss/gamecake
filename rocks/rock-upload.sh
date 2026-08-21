
if [ -z "$ROCK_DIR" ]; then
	echo "Must be run via rock.sh to set env correctly."
	exit 20
fi
cd $ROCK_DIR

luarocks upload --api-key=$ROCK_API_KEY $ROCK_BASENAME.rockspec $ROCK_BASENAME.src.rock
