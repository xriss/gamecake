cd `dirname $0`

# $1 must be dir
./setenv.sh $1 || exit 0


if [ -z "$2" ]; then

	cat <<EOF

./rock.sh rockdir command

Available commands are :

bump
	set latest revision to todays date

spec
	generate spec file for latest verison/revision

EOF

exit 0
fi


case $2 in

	"bump")
		echo "bumping"
	;;

	"spec")
		echo "specing"
	;;

	*)
		echo "unknown command"
		exit 20
	;;

esac
