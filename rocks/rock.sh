cd `dirname $0`

# $1 must be dir
source ./rock-env.sh $1 || exit 0


if [ -z "$2" ]; then

	cat <<EOF

./rock.sh rockdir command

Available commands are :

bump
	set latest revision to todays date

spec
	generate spec file for latest verison/revision

pack
	zip up the src and pack it into the src.rock

install
	make and install locally ( home directory ) from src.rock

make
	runs all the spec and pack and install steps

EOF

exit 0

fi


case $2 in

	"bump")
		./rock-bump.sh || exit 20
	;;

	"make")
		./rock-spec.sh || exit 20
		./rock-pack.sh || exit 20
		./rock-install.sh || exit 20
	;;

	"spec")
		./rock-spec.sh || exit 20
	;;

	"pack")
		./rock-pack.sh || exit 20
	;;

	"install")
		./rock-install.sh || exit 20
	;;

	*)
		echo "unknown command"
		exit 20
	;;

esac
