-- this is a special loader function, it is loaded and injected into the package loaders automagically
-- just before the search for internal strings (so we can overload builtin modules)
-- it should return a module function if it can find one within the registered zip files
-- this entire file is the function so when called ... contains the arguments passed into require


local wzips=package.loaded["wetgenes.zips"] -- must be required and setup for this to work
if type(wzips)=="table" and wzips.loader then
	return wzips.loader(...) -- real code lives here
end

