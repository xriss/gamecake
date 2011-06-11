cd `dirname $0`
cd ../../bin/dbg
gdb --args ./lua ../lua/apps.lua wettest $*
