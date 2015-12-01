from neopixel import *
import atexit
import colorsys

# LED strip configuration:
LED_COUNT      = 64      # Number of LED pixels.
LED_PIN        = 18      # GPIO pin connected to the pixels (must support PWM!).
LED_FREQ_HZ    = 800000  # LED signal frequency in hertz (usually 800khz)
LED_DMA        = 5       # DMA channel to use for generating signal (try 5)
LED_BRIGHTNESS = 255     # Set to 0 for darkest and 255 for brightest
LED_INVERT     = False   # True to invert the signal (when using NPN transistor level shift)

ws2812 = Adafruit_NeoPixel(LED_COUNT, LED_PIN, LED_FREQ_HZ, LED_DMA, LED_INVERT, LED_BRIGHTNESS)
ws2812.begin()

"""
Store the rotation of UnicornHat, defaults to
0 which places 0,0 on the top left with the B+
HDMI port facing downwards
"""
_rotation = 0


"""
Store a map of pixel indexes for
translating x, y coordinates.
"""
map = [
    [7 , 6 , 5 , 4 , 3 , 2 , 1 , 0 ],
    [8 , 9 , 10, 11, 12, 13, 14, 15],
    [23, 22, 21, 20, 19, 18, 17, 16],
    [24, 25, 26, 27, 28, 29, 30, 31],
    [39, 38, 37, 36, 35, 34, 33, 32],
    [40, 41, 42, 43, 44, 45, 46, 47],
    [55, 54, 53, 52, 51, 50, 49, 48],
    [56, 57, 58, 59, 60, 61, 62, 63]
]


def _clean_shutdown():
    """Registered at exit to ensure ws2812 cleans up after itself
    and all pixels are turned off.
    """
    off()


def rotation(r=0):
    """Set the display rotation

    Valid values:
    0
    90
    180
    270"""

    global _rotation
    if r in [0, 90, 180, 270]:
        _rotation = r
        return True
    else:
        raise ValueError('Rotation must be 0, 90, 180 or 270 degrees')


def brightness(b=0.2):
    """Set the display brightness between 0.0 and 1.0

    0.2 is highly recommended, UnicornHat can get painfully bright!"""

    if b > 1 or b < 0:
        raise ValueError('Brightness must be between 0.0 and 1.0')

    ws2812.setBrightness(int(b*255.0))


def get_brightness():
    """Get the display brightness value

    Returns a float between 0.0 and 1.0
    """
    return 0#ws2812.getBrightness()


def clear():
    """Clear the buffer"""
    for x in range(64):
        ws2812.setPixelColorRGB(x, 0, 0, 0)


def off():
    """Clear the buffer and immediately update UnicornHat

    Turns off all pixels."""
    clear()
    show()


def get_index_from_xy(x, y):
    """Convert an x, y value to an index on the display"""
    if x > 7 or x < 0:
        raise ValueError('X position must be between 0 and 7')
    if y > 7 or y < 0:
        raise ValueError('Y position must be between 0 and 7')

    y = 7-y

    if _rotation == 90:
        x, y = y, 7-x
    elif _rotation == 180:
        x, y = 7-x, 7-y
    elif _rotation == 270:
        x, y = 7-y, x

    return map[x][y]


def set_pixel_hsv(x, y, h, s, v):
    """Set a single pixel to a colour using HSV"""
    index = get_index_from_xy(x, y)
    if index is not None:
        r, g, b = [int(n*255) for n in colorsys.hsv_to_rgb(h, s, v)]
        ws2812.setPixelColorRGB(index, r, g, b)


def set_pixel(x, y, r, g, b):
    """Set a single pixel to RGB colour"""
    index = get_index_from_xy(x, y)
    if index is not None:
        ws2812.setPixelColorRGB(index, r, g, b)


def get_pixel(x, y):
    """Get the RGB value of a single pixel"""
    index = get_index_from_xy(x, y)
    if index is not None:
        pixel = ws2812.getPixelColorRGB(index)
        return int(pixel.r), int(pixel.g), int(pixel.b)


def set_pixels(pixels):
    for x in range(8):
        for y in range(8):
            r, g, b = pixels[y][x]
            set_pixel(x, y, r, g, b)


def get_pixels():
    """Get the RGB value of all pixels in a 7x7x3 2d array of tuples"""
    return [[get_pixel(x, y) for x in range(8)] for y in range(8)]


def show():
    """Update UnicornHat with the contents of the display buffer"""
    ws2812.show()


atexit.register(_clean_shutdown)
