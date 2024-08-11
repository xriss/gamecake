BASEDIR=$( cd "$( dirname "$0" )" && pwd )

cd $BASEDIR/../..

exe/gamecake lua/fun/notwork.fun.lua --win-hx=800 --win-hy=450 --win-px=800 --win-py=0   --win-borderless --quickexit --host=2342 &\
sleep 1 ; exe/gamecake lua/fun/notwork.fun.lua --win-hx=800 --win-hy=450 --win-px=800 --win-py=450 --win-borderless --quickexit --host=2342 --join=[::1]:2342 &\
wait


