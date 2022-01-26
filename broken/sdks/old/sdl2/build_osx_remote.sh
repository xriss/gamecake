#build on remote mac machine and grab the binary back to this system

ssh donald.local " bash --login -c \" cd hg/sdks/sdl2 ; hg fetch \""
ssh donald.local " bash --login -c \" cd hg/sdks/sdl2 ; ./wget.sh \""
ssh donald.local " bash --login -c \" cd hg/sdks/sdl2 ; ./build_osx.sh \""
scp -r donald.local:hg/sdks/sdl2/sdl2_osx sdl2_osx

