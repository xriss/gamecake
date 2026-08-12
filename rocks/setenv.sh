cd `dirname $0`

export ROCK_ROOT=`pwd`

if [ -z "$1" ]; then

echo "Must provide rock directory, eg one of the following directoy names:"
ls -d */
exit 20

else

echo "Setting $1 environment"

fi

cd $1
source ./env.sh
cd ..

echo
printenv | grep ROCK_
echo
