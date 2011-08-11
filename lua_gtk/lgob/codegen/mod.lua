MODULE = 'codegen'
init()

shell( es'$INSTD $DEST/$SHARED' )

shell( es'$INSTD $DEST/bin' )
LOCAL_BIN = 'bin'
shell( es'$INST $LOCAL_BIN/generator.lua $DEST/bin/lgob-generator' )
shell( es'$INST $LOCAL_BIN/gir-parser.lua $DEST/bin/lgob-gir-parser' )
shell( es'$INST $LOCAL_BIN/update-version.lua $DEST/bin/lgob-update-def' )
shell( es'$CHMOD $DEST/bin/lgob-generator' )
shell( es'$CHMOD $DEST/bin/lgob-gir-parser' )
shell( es'$CHMOD $DEST/bin/lgob-update-def' )

shell( es'$INST src/*.lua $DEST/$SHARED' )
