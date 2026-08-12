
if [ -z "$ROCK_DIR" ]; then
	echo "Must be run via rock.sh to set env correctly."
	exit 20
fi
cd $ROCK_DIR

oldrev=$ROCK_REVISION

# if revisions need to be multiple ones on the same day then we fucked up :)

printf -v ROCK_REVISION '%(%y%m%d)T' -1

if [ "$oldrev" == "$ROCK_REVISION" ]; then
	echo "Revision already set to $ROCK_REVISION"
	exit 20
fi

echo "bumping revision from $oldrev to $ROCK_REVISION"

sed -i "s/^export ROCK_REVISION=.*\$/export ROCK_REVISION=${ROCK_REVISION}/" ./env.sh
