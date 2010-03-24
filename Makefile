# Notes:
#     lipjpeg can be a major pain to deal with because it checks at runtime (not
#     at compile time) to make sure that your libjpeg headers came from the
#     same version of libjpeg that you're linking with.  For some reason, I
#     have both libjpeg 8 and libjpeg 6 on my system and so I have to change
#
#       -ljpeg
#
#     to
#
#       /usr/local/lib/libjpeg.8.dylib
#
#     Alternatively, you can just compile your own libjpeg locally and link
#     against the regular old libjpeg.a.
#
#     If I was fancier, I would make a config script
#

all:
	gcc main.c image_functions.c `pkg-config --libs x11` -ljpeg -o main

clean:
	rm main
