#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <jpeglib.h>
#include "image_functions.h"

/* The following two functions were borrowed from Junaed Sattar at
   http://www.cim.mcgill.ca/~junaed/libjpeg.php
   and stripped of all the "add one to i" style comments :) 

   My changes are largely the extra arguments that allow you to
   capture various image properties like width and height */ 
uint8_t * read_jpeg_file( char * filename, int * width, int * height, 
                          int * bytes_per_pixel, int * color_space )
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	JSAMPROW row_pointer[1];
	
	FILE *infile = fopen( filename, "rb" );
	unsigned long location = 0;
	int i = 0;
	
	if ( !infile )
	{
		printf("Error opening jpeg file %s\n!", filename );
		return NULL;
	}
	cinfo.err = jpeg_std_error( &jerr );
	jpeg_create_decompress( &cinfo );
	jpeg_stdio_src( &cinfo, infile );
	jpeg_read_header( &cinfo, TRUE );

	jpeg_start_decompress( &cinfo );

    if(width) *width = cinfo.output_width;
    if(height) *height = cinfo.output_height;
	if(bytes_per_pixel) *bytes_per_pixel = cinfo.num_components;
	if(color_space) *color_space = cinfo.out_color_space;

	uint8_t * raw_image = (uint8_t*)malloc( cinfo.output_width*cinfo.output_height*cinfo.num_components );
	row_pointer[0] = (uint8_t *)malloc( cinfo.output_width*cinfo.num_components );
	while( cinfo.output_scanline < cinfo.image_height )
	{
		jpeg_read_scanlines( &cinfo, row_pointer, 1 );
		for( i=0; i<cinfo.image_width*cinfo.num_components;i++) 
			raw_image[location++] = row_pointer[0][i];
	}

	jpeg_finish_decompress( &cinfo );
	jpeg_destroy_decompress( &cinfo );

	free( row_pointer[0] );
	fclose( infile );

    return raw_image;
}

int write_jpeg_file( char *filename, uint8_t * raw_image, int width, int height, int bytes_per_pixel, int color_space )
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	
	JSAMPROW row_pointer[1];
	FILE *outfile = fopen( filename, "wb" );
	
	if ( !outfile )
	{
		printf("Error opening output jpeg file %s\n!", filename );
		return -1;
	}
	cinfo.err = jpeg_std_error( &jerr );
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, outfile);

	cinfo.image_width = width;	
	cinfo.image_height = height;
	cinfo.input_components = bytes_per_pixel;
	cinfo.in_color_space = (J_COLOR_SPACE) color_space;
	jpeg_set_defaults( &cinfo );
	jpeg_start_compress( &cinfo, TRUE );
	while( cinfo.next_scanline < cinfo.image_height )
	{
		row_pointer[0] = &raw_image[ cinfo.next_scanline * cinfo.image_width *  cinfo.input_components];
		jpeg_write_scanlines( &cinfo, row_pointer, 1 );
	}
	jpeg_finish_compress( &cinfo );
	jpeg_destroy_compress( &cinfo );
	fclose( outfile );
	return 0;
}

void rgb_to_gray( uint8_t * src, int width, int height, uint8_t * dst ) {
    int i;
    for(i = 0; i < width*height*3; i += 3) {
        /* This is one of a thousand ways to do grayscale, this one was
           proposed by a one "Craig Markwart" on the IDL newsgroup */
        dst[i/3] = src[i]*0.3 + src[i + 1]*0.59 + src[i+2]*0.11;
    }
}

/* Displays a new X11 window and blits the specified data to it. This was
   adapted from:

    http://tronche.com/gui/x/xlib-tutorial/

   Efficiency-wise this isn't particularly hot, it's preferable to manually
   making XImages and things since that requires a lot of magic parameter
   initializations (which are hardware specific).  This basically says 
   "give me an RGB framebuffer corresponding to a simple white window on
   this current platform" and draws in each pixel one at a time. Inefficient 
   yes, but dead simple and fast enough for my purposes */

void display( uint8_t * data, int width, int height, int is_rgb ) {
    Display *dpy = XOpenDisplay(NULL);

    int blackColor = BlackPixel(dpy, DefaultScreen(dpy));
    int whiteColor = WhitePixel(dpy, DefaultScreen(dpy));
    Window w = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0, 
				     width, height, 0, blackColor, blackColor);
    XSelectInput(dpy, w, StructureNotifyMask);
    XMapWindow(dpy, w);
    /* GC is graphics context, not garbage collector */
    GC gc = XCreateGC(dpy, w, 0, NULL);
    XSetForeground(dpy, gc, whiteColor);

    /* Wait for confirmation from the X server that this window has been placed */
    for(;;) {
      XEvent e;
      XNextEvent(dpy, &e);
	    if (e.type == MapNotify)
		  break;
      }

    /* After we've created a window, suck out the image (which takes care of
       us having to figure out hardware specific parameters for the image) */
    XImage * image = XGetImage( dpy, w, 0, 0, width, height, AllPlanes, XYPixmap );

    int x,y,i;

    /* TODO: Remove this branch, in a perfect world GCC would optimize this away
             automagically, but I don't think it actually does - maybe with -O3?
             (ie. with super-duper inlining where it can see is_rgb is a constant
             within display_gray and display_rgb respectively).  I have at least
             moved the guard out of the tight loop (at the cost of some
             redundancy). Could do something fancy with macros too... */
    if(is_rgb)
    for(y = 0; y < height; y++) {
      for(x = 0; x < width; x++) {
        i = y*width + x; 
        XPutPixel(image, x, y, data[i*3] << 16 | data[i*3 + 1] << 8 | data[i*3+2]);
      }
    }
    else
    for(y = 0; y < height; y++) {
      for(x = 0; x < width; x++) {
        i = y*width + x; 
        XPutPixel(image, x, y, data[i] << 16 | data[i] << 8 | data[i]);
      }
    }

    XPutImage( dpy, w, gc, image, 0, 0, 0, 0, width, height );
    XFlush(dpy);
}

void display_gray( uint8_t * data, int width, int height ) {
    display( data, width, height, 0 );
}

void display_rgb( uint8_t * data, int width, int height ) {
    display( data, width, height, 1 );
}
