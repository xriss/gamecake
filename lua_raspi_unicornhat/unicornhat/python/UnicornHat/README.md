Unicorn Hat Python Library
==========================

This library wraps the ws2812 python driver for UnicornHat, handling conversion of X/Y coordinates to pixel index
and exposing the basic methods you need to set pixels and update UnicornHat.


Installing
----------

**PIP**

    sudo pip install unicornhat

**GitHub**

    sudo ./setup.py install


Usage
-----

Just import unicornhat, it'll set up ws2812 for you!

Then all you need is:

* unicornhat.set_pixel( x, y, red, green, blue ) - Set a pixel in the buffer to the specified colour
* unicornhat.show - Update UnicornHat with the current buffer
* unicornhat.clear - Turn off all the pixels in the buffer and update UnicornHat

See the examples for more advanced usage.
