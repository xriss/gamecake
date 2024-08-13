BASEDIR=$( cd "$( dirname "$0" )" && pwd )

cd $BASEDIR/../..

HX=$( xdpyinfo | grep dimensions: | awk '{print $2}' | cut -d 'x' -f1 )
HY=$( xdpyinfo | grep dimensions: | awk '{print $2}' | cut -d 'x' -f2 )

            exe/gamecake lua/fun/notwork.fun.lua --win-hx=$((HX/2)) --win-hy=$((HY/2)) --win-px=$((HX/2)) --win-py=0         --win-borderless --quickexit --host=2342 &\
sleep 0.5 ; exe/gamecake lua/fun/notwork.fun.lua --win-hx=$((HX/2)) --win-hy=$((HY/2)) --win-px=$((HX/2)) --win-py=$((HY/2)) --win-borderless --quickexit --host=2342 --join=[::1]:2342 &\
wait


