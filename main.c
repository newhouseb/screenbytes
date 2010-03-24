#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <jpeglib.h>
#include "image_functions.h"

/* This is a pretty simple example that

   1) Opens an RGB JPEG
   2) Displays said JPEG with X11
   3) Converts the JPEG to grayscale
   4) Displays said grayscale version with X11
   5) Writes the grayscal JPEG to a file
   5) Waits for a keypress and then quits

*/

int main(int argc, char * argv[]) {
    if(argc != 3) {
        printf("%s [input RGB JPEG] [output grayscale JPEG]\n\
\tTakes a RGB JPEG, displays it with X11 as well as displays a grayscale\n\
\tversion.  Finally it writes the grayscale-ified JPEG to the given file\n\
\tand waits for a user keystroke to quit\n", argv[0]);
        return 0;
    }

    /* Open a jpeg to a RGB bitmap */
    int width, height, bpp, colorspace;
    uint8_t * rgb = read_jpeg_file(argv[1], &width, &height, &bpp, &colorspace);
    assert(rgb);
    assert(colorspace == JCS_RGB);

    /* Display aforementioned bitmap with X11 */
    display_rgb(rgb, width, height);

    /* Make a grayscale version of the bitmap */
    uint8_t * gray = malloc(width*height);
    assert(gray);
    rgb_to_gray(rgb, width, height, gray);

    /* Display the grayscale image */
    display_gray(gray, width, height);    

    /* Write out the file to a JPG */
    int write_success = write_jpeg_file(argv[2], gray, width, height, 1, JCS_GRAYSCALE);
    assert(write_success == 0);

    /* Free up memory */
    free(gray);
    free(rgb); 
    
    /* Wait for a keypress to quit */
    char wait_for_char;
    scanf("%c", &wait_for_char);
}
