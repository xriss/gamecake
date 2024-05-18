Javascript json parser.

Currently plan on binding to the C core until I get around to writing a native version.

Probably faster anyway, and since js strings are 16bit it will probably use less memory and work better with utf8.

Still, we will need a version to run in the browser, prefrably without resorting to wasm.


