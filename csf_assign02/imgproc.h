// Header for image processing API functions (imgproc_mirror_h, etc.)
// as well as any helper functions they rely on.

#ifndef IMGPROC_H
#define IMGPROC_H

#include "image.h" // for struct Image and related functions

// Convert input pixels to grayscale.
// This transformation always succeeds.
//
// Parameters:
//   input_img  - pointer to the input Image
//   output_img - pointer to the output Image (in which the transformed
//                pixels should be stored)
void imgproc_grayscale( struct Image *input_img, struct Image *output_img );

// Render an output image containing 4 replicas of the original image,
// refered to as A, B, C, and D in the following diagram:
//
//   +---+---+
//   | A | B |
//   +---+---+
//   | C | D |
//   +---+---+
//
// The width and height of the output image are (respectively) twice
// the width and height of the input image.
//
// A is an exact copy of the original input image. B has only the
// red color component values of the input image, C has only the
// green color component values, and D has only the blue color component
// values.
//
// Each of the copies (A-D) should use the same alpha values as the
// original image.
//
// This transformation always succeeds.
//
// Parameters:
//   input_img - pointer to the input Image
//   output_img - pointer to the output Image (which will have
//                width and height twice the width/height of the
//                input image)
void imgproc_rgb( struct Image *input_img, struct Image *output_img );

// Render a "faded" version of the input image.
//
// See the assignment description for an explanation of how this transformation
// should work.
//
// This transformation always succeeds.
//
// Parameters:
//   input_img - pointer to the input Image
//   output_img - pointer to the output Image
void imgproc_fade( struct Image *input_img, struct Image *output_img );

// Render a "kaleidoscope" transformation of input_img in output_img.
// The input_img must be square, i.e., the width and height must be
// the same. Assume that the input image is divided into 8 "wedges"
// like this:
//
//    +----+----+
//    |\ A |   /|
//    | \  |  / |
//    |  \ | /  |
//    | B \|/   |
//    +----+----+
//    |   /|\   |
//    |  / | \  |
//    | /  |  \ |
//    |/   |   \|
//    +----+----+
//
// The wedge labeled "A" is the one that will be replicated 8 times
// in the output image. In the output image,
//
//    Wedge A will have exactly the same pixels as wedge A in
//    the input image, in the same positions.
//
//    Wedge B will have exactly the same pixels as wedge A, but the
//    column and row are transposed. (I.e., it is a reflection across
//    the diagonal.)
//
//    All other wedges are mirrored versions of A such that the overall
//    image is symmetrical across all four lines dividing the image
//    (vertical split, horizontal split, and the two diagonal splits.)
//
// Parameters:
//   input_img  - pointer to the input Image
//   output_img - pointer to the output Image (in which the transformed
//                pixels should be stored)
//
// Returns:
//   1 if successful, 0 if the transformation fails because the
//   width and height of input_img are not the same.
int imgproc_kaleidoscope( struct Image *input_img, struct Image *output_img );

// TODO: add prototypes for your helper functions
// Get the r values within the input pixel
//
// Parameters:
//   pixel - a uint32_t value contains rgba information
//
// Returns:
//   the r value contained in the pixel
uint32_t get_r( uint32_t pixel);

// Get the g values within the input pixel
//
// Parameters:
//   pixel - a uint32_t value contains rgba information
//
// Returns:
//   the g value contained in the pixel
uint32_t get_g( uint32_t pixel);

// Get the b values within the input pixel
//
// Parameters:
//   pixel - a uint32_t value contains rgba information
//
// Returns:
//   the b value contained in the pixel
uint32_t get_b( uint32_t pixel);

// Get the a values within the input pixel
//
// Parameters:
//   pixel - a uint32_t value contains rgba information
//
// Returns:
//   the a value contained in the pixel
uint32_t get_a( uint32_t pixel);

// Make a pixel given its rgba values
//
// Parameters:
//  r : red value
//  g : green value
//  b : blue value
//  a : alpha value
//
// Returns:
//  a pixel with given rgba value
uint32_t make_pixel(uint32_t r, uint32_t g, uint32_t b, uint32_t a);

// Convert input pixel to grayscale.
//
// Parameters:
//   pixel - one single pixel contains rgba information
//
// Returns:
//   a pixel in grayscale
uint32_t to_grayscale( uint32_t pixel );

// Return the relative index of a pixel in the array that represent the image
// 
// Parameters:
//   img - pointer to the image which contains data
//   col - the column index in the 2D image of the pixel
//   row - the row index in the 2D image of the pixel
// 
// Returns:
//   the pixel's index in an array, converting from the 2D image
int32_t compute_index( struct Image *img, int32_t col, int32_t row );

// Calculate the gradient for the input row/column coordinate
// 
// Parameters:
//   x - input row/column coordinate
//   max - number of pixels in that row or column
// 
// Return:
//   the calculated gradient for the input row/column coordinate
int64_t gradient( int64_t x, int64_t max );

#endif // IMGPROC_H
