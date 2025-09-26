cd `dirname $0`


#rm test.zip
#( cd ../test && zip -r ../html/test.zip . )


emrun --kill-start --kill-exit --browser=firefox dbg/gamecake.html -- -lcmd swed --logs

