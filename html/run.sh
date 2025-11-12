cd `dirname $0`

emrun --kill-start --kill-exit --browser=firefox dbg/gamecake.html -- $*
