#!/bin/sh
cd `dirname $0`
cd test

echo "# Converting all input json ( note this file is forced 7bit ascii clean with # replacing the missing bytes )" >output.json

files=`find json -name "*.json" -type f -exec echo {} \; && find djon -name "*.djon" -type f -exec echo {} \;`

djonfiles=`find djon -name "*.djon" -type f -exec echo {} \;`

eval `luarocks --lua-version 5.1 path`

for fname in $files ; do

	echo Transforming ${fname}
	
	echo "" >>output.json
	echo "#$fname" >>output.json
	{ cat "$fname" | tr -c '[:print:]\t\n\r' '#' ; } &>>output.json

	echo "" >>output.json
	echo "#c.json" >>output.json
	{ ../c/djon --strict "${fname}" | tr -c '[:print:]\t\n\r' '#' ; } &>>output.json
	echo "#c.djon" >>output.json
	{ ../c/djon --djon "${fname}" | tr -c '[:print:]\t\n\r' '#' ; } &>>output.json

	../c/djon --djon "${fname}" >tmp1.djon 2>/dev/null
	../c/djon --djon tmp1.djon  >tmp2.djon 2>/dev/null
	if cmp --quiet tmp1.djon tmp2.djon ; then
		:
	else
		echo "#BAD reparsed djon" >>output.json
		diff tmp1.djon tmp2.djon >>output.json
	fi
	rm tmp1.djon
	rm tmp2.djon

	../c/djon --json "${fname}" >tmp1.json 2>/dev/null
	../c/djon --json tmp1.json  >tmp2.json 2>/dev/null
	if cmp --quiet tmp1.json tmp2.json ; then
		:
	else
		echo "#BAD reparsed json" >>output.json
		diff tmp1.json tmp2.json >>output.json
	fi
	rm tmp1.json
	rm tmp2.json

	echo "#lua.json" >>output.json
	{ luajit -- ../lua/djon.cmd.lua --strict "${fname}" 2>&1 | sed -n '/stack traceback:/q;p' | tr -c '[:print:]\t\n\r' '#' ; } &>>output.json
	echo "#lua.djon" >>output.json
	{ luajit -- ../lua/djon.cmd.lua --djon "${fname}" 2>&1 | sed -n '/stack traceback:/q;p' | tr -c '[:print:]\t\n\r' '#' ; } &>>output.json
	echo "#js.json" >>output.json
	{ node -- ../js/cmd.js --strict "${fname}" 2>&1 | sed -n '/at djon.check_error/q;p' | sed '/Error:/q' | tr -c '[:print:]\t\n\r' '#' ; } &>>output.json
	echo "#js.djon" >>output.json
	{ node -- ../js/cmd.js --djon "${fname}" 2>&1 | sed -n '/at djon.check_error/q;p' | sed '/Error:/q' | tr -c '[:print:]\t\n\r' '#' ; } &>>output.json


done

for fname in $djonfiles ; do

	echo Transforming ${fname}
	
	echo "" >>output.json
	echo "#$fname" >>output.json
	{ cat "$fname" | tr -c '[:print:]\t\n\r' '#' ; } &>>output.json

	echo "" >>output.json
	echo "#c.comments.json" >>output.json
	{ ../c/djon --comments "${fname}" | tr -c '[:print:]\t\n\r' '#' ; } &>>output.json
	echo "#c.comments.djon" >>output.json
	{ ../c/djon --comments "${fname}" | ../c/djon --djon --comments -- | tr -c '[:print:]\t\n\r' '#' ; } &>>output.json
	echo "#lua.comments.json" >>output.json
	{ luajit -- ../lua/djon.cmd.lua --comments "${fname}" 2>&1 | sed -n '/stack traceback:/q;p' | tr -c '[:print:]\t\n\r' '#' ; } &>>output.json
	echo "#lua.comments.djon" >>output.json
	{ luajit -- ../lua/djon.cmd.lua --comments "${fname}" 2>&1 | luajit -- ../lua/djon.cmd.lua --djon --comments 2>&1 | sed -n '/stack traceback:/q;p' | tr -c '[:print:]\t\n\r' '#' ; } &>>output.json
	echo "#js.comments.json" >>output.json
	{ node -- ../js/cmd.js --comments "${fname}" 2>&1 | sed -n '/at djon.check_error/q;p' | sed '/Error:/q' | tr -c '[:print:]\t\n\r' '#' ; } &>>output.json
	echo "#js.comments.djon" >>output.json
	{ node -- ../js/cmd.js --comments "${fname}" 2>&1 | node -- ../js/cmd.js --djon --comments 2>&1 | sed -n '/at djon.check_error/q;p' | sed '/Error:/q' | tr -c '[:print:]\t\n\r' '#' ; } &>>output.json

done
