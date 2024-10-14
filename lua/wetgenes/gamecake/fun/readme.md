Source for https://github.com/xriss/fun64


---
			
The following text is automatically extracted from other files in this 
directory and should not be edited here.

---




## lua.wetgenes.gamecake.fun.chatdown.chat.get_tag


	tag_value = chat:get_tag(tag_name)

The same as chats:get_tag but the subject of this chat is the 
default root.



## lua.wetgenes.gamecake.fun.chatdown.chat.replace_tags


	output = chat:replace_tags(input)

The same as chats:replace_tags but the subject of this chat is the 
default root.



## lua.wetgenes.gamecake.fun.chatdown.chat.set_tag


	tag_value = chat:set_tag(tag_name,tag_value)

The same as chats:set_tag but the subject of this chat is the 
default root.



## lua.wetgenes.gamecake.fun.chatdown.chat.set_tags


	chat:set_tags(tags)

Set all the values in the given table of {tag_name=tag_value} pairs.



## lua.wetgenes.gamecake.fun.chatdown.chat.set_topic


	chat:set_topic(topic_name)

Set the current topic for this chat object, information about this 
topic and its gotos are built from and stored in this chat object.

	chat.topic_name
	
Will be set to the given topic name.

We merge all gotos found in this topic and all topic parents by 
iterating over the dotnames. We only use add each topic once and each 
topic may have a bit of logic that decides if it should be displayed.

	<topic_name?logic_test

So this would goto the topic_name if the logic_test passes. The logic 
test is written simmilar to a url query after the question mark comes a 
number of tests that are then & or | together in left to right order 
(no operator precedence).

	<topic_name?count<10&seen==false

So this would display the goto option if count is less than 10 and seen 
is set to false. These variables we are testing are tag_names and 
default to the current subject chat but could reference other subjects 
by explicitly including a root.

Available logic tests are

	name==value
	name=value

True if the tag is set to this value.

	name!=value

True if the tag is not set to this value.

	name<value

True if the tag is less than this value (numeric test).

	name>value

True if the tag is more than this value (numeric test).

	name<=value

True if the tag is less than or equal to this value (numeric test).

	name>=value

True if the tag is more than or equal to  this value (numeric test).

All of these tests can be preceded by a ! to negate them so

	name!=vale
	!name==value

Are both a test for inequality.

Multiple tests can be joined together by & (and) or | (or) this logic 
will be applied to a running total moving from left to right as the 
values are evaluated with the final value deciding if this goto will be 
displayed.




## lua.wetgenes.gamecake.fun.chatdown.chats.changes


	chats.changes(chat,change,...)

	chats.changes(chat,"subject")
	chats.changes(chat,"topic",topic)
	chats.changes(chat,"goto",goto)
	chats.changes(chat,"tag",tag_name,tag_value)

This is a callback hook, replace to be notified of changes and possibly 
alter then, by default we print debuging information. Replace this 
function with an empty function to prevent this eg

	chats.changes=function()end



## lua.wetgenes.gamecake.fun.chatdown.chats.get_subject


	chat = chats:get_subject(subject_name)
	chat = chats:get_subject()

Get the chat for the given subject or the chat for the last subject 
selected with set_subject if no subject_name is given.



## lua.wetgenes.gamecake.fun.chatdown.chats.get_tag


	tag_value = chats:get_tag(tag_name,subject_name)

Get the tag_value for the given tag_name which can either be 
"tag_root/tag_name" or "tag_name". The subject_name is the default root 
to use if no tag_root is given in the tag_name.



## lua.wetgenes.gamecake.fun.chatdown.chats.replace_tags


	output = chats:replace_tags(input,subject_name)

Tags in the input text can be wrapped in {tag_name} and will be 
replaced with the appropriate tag_value. This is done recursively so 
tag_values can contain references to other tags. If a tag does not 
exist then it will not expand and {tag_name} will remain in the output 
text.

Again if any tag name does not contain an explicit root then 
subject_name will be used as the default chat subject.



## lua.wetgenes.gamecake.fun.chatdown.chats.set_subject


	chat = chats:set_subject(subject_name)

Set the current subject for this chats object, this subject becomes the 
chat that you will get if you call get_subject with no arguments.




## lua.wetgenes.gamecake.fun.chatdown.chats.set_tag


	tag_value = chats:set_tag(tag_name,tag_value,subject_name)

Alter the value of the given tag_name. If the value string begins with 
a "+" or a "-" Then the values will be treated as numbers and added or 
subtracted from the current value. This allows for simple incremental 
flag values.

Again if the tag name does not contain an explicit  root then 
subject_name will be used as the default chat subject.



## lua.wetgenes.gamecake.fun.chatdown.dotnames


	for part_name in chatdown.dotnames(full_name) do
		print(part_name)
	end

Iterate all dotnames so if given "aa.bb.cc" we would iterate through 
"aa.bb.cc" , "aa.bb" and "aa". This is used to inherit data using just 
a naming convention.



## lua.wetgenes.gamecake.fun.chatdown.parse


	rawsubjects = chatdown.parse(text)

Parse text from flat text chatdown format into heirachical chat data, 
which we refer to as rawsubjects, something that can be output easily 
as json.

This gives us a readonly rawsubjects structure that can be used to control 
what text is displayed during a chat session.

This is intended to be descriptive and logic less, any real logic 
should be added using a real language that operates on this rawsubjects and 
gets triggered by the names used. EG, filter out gotos unless certain 
complicated conditions are met or change topics to redirect to an 
alternative.

A self documented example of chatdown formated text can be found in 
lua.wetgenes.gamecake.fun.chatdown.text



## lua.wetgenes.gamecake.fun.chatdown.setup


	chats = chatdown.setup_chats(chat_text,changes)

parse and initialise state data for every subjects chunk creating a 
global chats with a chat object for each subject.



## lua.wetgenes.gamecake.fun.chatdown.setup_chat


	chat = chatdown.setup_chat(chats,subject_name)

Setup the initial chat state for a subject. This is called 
automatically by chatdown.setup and probably should not be used 
elsewhere.



## lua.wetgenes.gamecake.fun.chatdown.text


Here is some example chatdown formatted text full of useful 
information, it it is intended to be a self documented example.

```

- This is a single line comment
-- This is the start of a multi-line comment

All lines are now comment lines until we see a line that begins with a 
control character leading white space is ignored. If for some reason 
you need to start a text line with a special character then it can be 
escaped by preceding it with a #

What follows is a list of these characters and a brief description 
about the parser state they switch to.

	1. - (text to ignore)
	
		A single line comment that does not change parser state and 
		only this line will be ignored so it can be inserted inside 
		other chunks without losing our place.

	2. -- (text to ignore)
	
		Begin a comment chunk, this line and all lines that follow this 
		line will be considered comments and ignored until we switch to 
		a new parser state.

	3. #subject_name

		Begin a new subject chunk, all future topic,goto or tag chunks will 
		belong to this subject.
		
		The text that follows this until the next chunk is the long 
		description intended for when you examine the character.
		
		Although it makes sense for a subject chunk to belong to one 
		character it could also a group conversation with tags being 
		set to change the current talker as the conversation flows.
		
		subject names have simple cascading inheritance according to their 
		name with each level separated by a dot. A chunk named a.b.c 
		will inherit data from any chunks defined for a.b and a in that 
		order of priority.

	4. >topic_name
	
		Begin a topic chunk, all future goto or tag chunks will belong 
		to this topic, the lines that follow are how the character 
		responds when questioned about this topic followed by one or 
		more gotos as possible responses that will lead you onto 
		another topic.
		
		Topics can be broken into parts, to create a pause, by using an 
		unnamed goto followed by an unnamed topic which will both 
		automatically be given the same generated name and hence linked 
		together.
		
	5. <goto_topic_name
	
		Begin a goto chunk, all future tag chunks will belong to this 
		goto, this is probably best thought of as a question that will 
		get a reply from the character. This is a choice made by the 
		player that causes a logical jump to another topic.
		
		Essentially this means GOTO another topic and there can be 
		multiple GOTO options associated with each topic which the 
		reader is expected to choose between.
		
	6. =set_tag_name to this value
	
		If there is any text on the rest of this line then it will be 
		assigned to the tag without changing the current parse state so 
		it can be used in the middle of another chunk without losing 
		our place.
		
		This single line tag assignment is usually all you need. 
		Alternatively, if there is no text on the rest of this first 
		line, only white space, then the parse state will change and 
		text on all following lines will be assigned to the named tag.
		
		This assignment can happen at various places, for instance if 
		it is part of the subject then it will be the starting 
		value for a tag but if it is linked to a topic or goto then the 
		value will be a change as the conversation happens. In all 
		cases the tags are set in a single batch as the state changes 
		so the placement and order makes no difference.
		
		Tags can be used inside text chunks or even GOTO labels by 
		tightly wrapping with {} eg {name} would be replaced with the 
		value of name. Tags from other subjects can be referenced by 
		prefixing the tag name with the subject name followed by a forward 
		slash like so {subject/tag}
				

The hierarchy of these chunks can be expressed by indentation as all 
white space is ignored and combined into a single space. Each SUBJECT will 
have multiple TOPICs associated with it and each TOPIC will have 
multiple GOTOs as options to jump between TOPICs. TAGs can be 
associated with any of these 3 levels and will be evaluated as the 
conversation flows through these points.

So the chunk hierarchy expressed using indentation to denote children 
of.

	SUBJECT
		TAG
		GOTO
			TAG
		TOPIC
			TAG
			GOTO
				TAG

The GOTO chunks in the root SUBJECT chunk are used as prototypes so if a 
GOTO is used in multiple topics its text can be placed within a GOTO 
inside the main SUBJECT chunk rather than repeated constantly. This will 
then be inherited by a GOTO with no text. An alternative to this 
shorthand is to assign an oft-used piece of text to a tag and reference 
that in each topic instead.

SUBJECTs and TOPICs also have simple inheritance based on their names this 
enables the building of a prototype which is later expanded. Each 
inheritance level is separated by a dot so aa.bb.cc will inherit from 
aa.bb and aa with the data in the longer names having precedence. This 
inheritance is additive only so for instance a TAG can not be unset and 
would have to be changed to another value by aa.bb.cc if it existed in 
aa.bb or aa.

In practise this means


```