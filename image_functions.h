#ifndef __IMAGE_FUNCTIONS_H__
#define __IMAGE_FUNCTIONS_H__
#include <stdint.h>
#include <X11/Xlib.h> 
#include <X11/Xutil.h> 

/* Opens a JPEG file and allocates the needed memory to hold the bitmap,
   so you should free the returned pointer with free() when you no longer
   need it.

   Returns a pointer to the image bitmap
   Returns NULL if a failure occurs (such as the file doesn't exist)

   Args:
        char * filename - Filename to read from
        int * width - Pointer to an integer to fill with width (can be NULL)
        int * height - Pointer to an integer to fill with height (can be NULL)
        int * bytes_per_pixel - Pointer to an integer to fill with the bytes
            per pixel (usually 3 for 24bit RGB or 1 for Grayscale) (can be NULL)
        int * color_space - Pointer to and integer to fill with color space
            enumerator as defined by libjpeg (usually either JCS_RGB or 
            JCS_GRAYSCALE) (can be NULL);

*/
uint8_t * read_jpeg_file( char *filename, int * width, int * height, int * bytes_per_pixel, int * color_space );

/* Writes a buffer specified by raw_image to the specified filename

   Returns -1 if failture, 0 if success

   Arguments are the same as above, except you supply the parameters instead
   of reading them back out */
int write_jpeg_file( char *filename, uint8_t * raw_image, int width, int height, int bpp, int colorspace );

/* Converts a 3 byte-per-pixel bitmap to a 1 byte-per-pixel bitmap.  You
   should either malloc the dst, point dst to a global variable, or point
   dst to a stack variable (and likely completely smash your stack) */
void rgb_to_gray( uint8_t * src, int width, int height, uint8_t * dst );

/* Displays a 3 byte-per-pixel RGB bitmap in a new X11 window of the
   specified width and height */
void display_rgb( uint8_t * data, int width, int height );

/* Displays a 1 byte-per-pixel grayscale bitmap in a new X11 window of
   the specified width and height */
void display_gray( uint8_t * data, int width, int height );

#endif
