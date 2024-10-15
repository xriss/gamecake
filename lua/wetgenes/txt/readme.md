

---
			
The following text is automatically extracted from other files in this 
directory and should not be edited here.

---




## lua.wetgenes.txt.diff


(C) 2020 Kriss Blank and released under the MIT license, see 
http://opensource.org/licenses/MIT for full license text.

	local wtxtdiff=require("wetgenes.txt.diff")
  


## lua.wetgenes.txt.diff.find


Given two tables of strings, return the length , starta , startb of the longest 
common subsequence in table indexes or nil if not similar.




## lua.wetgenes.txt.diff.match


Given two tables of strings, return two tables of strings of 
the same length where as many strings as possible match.



## lua.wetgenes.txt.diff.split


Use the delimiter to split a string into a table of strings such that 
each string ends in the delimiter (except for possibly the final string) and a 
table.concat on the result will recreate the input string exactly.

	table = wtxtdiff.split(string,delimiter)

String is the string to split and delimiter is a lua pattern so any 
special chars should be escaped.

for example

	st = wtxtdiff.split(s) -- split on newline (default)
	st = wtxtdiff.split(s,"\n") -- split on newline (explicit)

	st - wtxtdiff.split(s,"%s+") -- split on white space



## lua.wetgenes.txt.diff.trim


Given two tables of strings, return the length at the start and at the 
end that are the same. This tends to be a good first step when 
comparing two chunks of text.




## lua.wetgenes.txt.edit


(C) 2023 Kriss Blank under the https://opensource.org/licenses/MIT

Generic text modifying functions.
  


## lua.wetgenes.txt.lex


(C) 2020 Kriss Blank under the https://opensource.org/licenses/MIT



Some useful lex files for other editors to be used as starting points 
and checking we did not miss anything.

	https://github.com/vim/vim/tree/master/runtime/syntax
	https://github.com/sublimehq/Packages




## lua.wetgenes.txt.lex_js


(C) 2020 Kriss Blank under the https://opensource.org/licenses/MIT



## lua.wetgenes.txt.lex_lua


(C) 2020 Kriss Blank under the https://opensource.org/licenses/MIT



## lua.wetgenes.txt.undo


(C) 2020 Kriss Blank under the https://opensource.org/licenses/MIT

undo / redo code for a text editor with persistence to disk

persistance to disk is in tsv format filename.txt.undo files where a 
.undo is added to the end of the file.

see https://pypi.org/project/linear-tsv/1.0.0/ for tsv format the first 
most column is always a command and the other columns are data needed to 
apply/reverse this command in theory a .undo file is a total history 
and as we should only be appending data (lines at a time even) then a 
file can be recovered from it and it should have limited corruption 
possibilities when things go wrong.

For security reasons this file may have undos removed as a separate 
step. That is to say things that where done / pasted accidentality then 
removed instantly will be purged from its history when performing a 
file save.

You can be save in the knowledge that any information you undo will not 
be saved except as temporary crash safe buffers.

The following need to be escaped with a \ when used in each column.

	\n for newline,
	\t for tab,
	\r for carriage return,
	\\ for backslash.
  


## lua.wetgenes.txt.utf


(C) 2020 Kriss Blank under the https://opensource.org/licenses/MIT

	local wutf = require("wetgenes.txt.utf")

helper functions to help manage a string as a stream of utf8 tokens.



## lua.wetgenes.txt.utf.char


	string = wutf.char(number)

convert a single unicode value to a utf8 string of 1-4 bytes



## lua.wetgenes.txt.utf.charpattern


	string:gmatch(wutf.charpattern)

lua pattern to match each utf8 character in a string



## lua.wetgenes.txt.utf.chars


	string = wutf.chars(number,number,...)
	string = wutf.chars({number,number,...})

convert one or more unicode values into a utf8 string



## lua.wetgenes.txt.utf.length


	unicode = wutf.ncode(string,index)

get the utf8 value at the given code index.

Note that this is slower than wutf.code as we must search the string to 
find the byte index of the code. 



## lua.wetgenes.txt.utf.map_latin0_to_unicode


	unicode = wutf.map_latin0_to_unicode[latin0] or latin0



## lua.wetgenes.txt.utf.map_unicode_to_latin0


	latin0 = wutf.map_unicode_to_latin0[unicode] or unicode

I prefer the coverage of latin0 (ISO/IEC 8859-15) for font layout as it 
is just a few small differences for western european languages to get 
most needed glyphs into the first 256 codes.



## lua.wetgenes.txt.utf.size


	size = wutf.size(string,index)

get the size in bytes of the utf8 value at the given byte index.

	size = wutf.size(string)

get the size in bytes of the utf8 value at the start of this string

The return value will be 1-4 as 4 is the biggest utf8 code size.



## lua.wetgenes.txt.utf.string


	unicode = wutf.code(string,index)

get the utf8 value at the given byte index.


	unicode = wutf.code(string)

get the utf8 value at the start of this string



## lua.wetgenes.txt.words


(C) 2023 Kriss Blank and released under the MIT license, see 
http://opensource.org/licenses/MIT for full license text.

	local wtxtwords=require("wetgenes.txt.words")

See https://github.com/xriss/engrish for source of words and possible 
alternative licenses.
  


## lua.wetgenes.txt.words.load


	yes = wtxtwords.check(word)

This is a fast check if the word exists.

May call wtxtwords.load() to auto load data.



## lua.wetgenes.txt.words.transform


	list = wtxtwords.transform(word,count,addletters,subletters)

Returns a table of upto count correctly spelled words that you may have 
miss spelt given the input word ordered by probability.

If the input word is spelled correctly then it will probably be the 
first word in this list but that is not guaranteed.

addletters is the maximum number of additive transforms, the higher 
this number the slower this function and it defaults to 4.

subletters is the maximum number of subtractive transforms and will not 
have much impact on speed, this defaults to the same value as 
addletters.

We run subletters subtractive transforms on our starting word and then 
we scan all possible words and perform addletters number of subtractive 
transforms on them and see if they match any of the transforms we built 
from our starting word. A match then means we can add up the number of 
transforms on both sides and that is how many steps it would take to 
get from one word to another by adding and subtracting letters.
