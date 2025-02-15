// C implementations of image processing functions

#include <stdint.h>
// #include <stdlib.h>
#include <assert.h>
#include "imgproc.h"
// TODO: define your helper functions here

// Get the r values within the input pixel
//
// Parameters:
//   pixel - a uint32_t value contains rgba information
//
// Returns:
//   the r value contained in the pixel
uint32_t get_r( uint32_t pixel){ 
  uint32_t red = (pixel & 0xFF000000) >> 24;
  return red;
}

// Get the g values within the input pixel
//
// Parameters:
//   pixel - a uint32_t value contains rgba information
//
// Returns:
//   the g value contained in the pixel
uint32_t get_g( uint32_t pixel){
  uint32_t green = (pixel & 0xFF0000) >> 16;
  return green;
}

// Get the b values within the input pixel
//
// Parameters:
//   pixel - a uint32_t value contains rgba information
//
// Returns:
//   the b value contained in the pixel
uint32_t get_b( uint32_t pixel){
  uint32_t blue = (pixel & 0xFF00) >> 8;
  return blue;
}

// Get the a values within the input pixel
//
// Parameters:
//   pixel - a uint32_t value contains rgba information
//
// Returns:
//   the a value contained in the pixel
uint32_t get_a( uint32_t pixel){
  uint32_t alpha = (pixel & 0xFF);
  return alpha;
}

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
uint32_t make_pixel(uint32_t r, uint32_t g, uint32_t b, uint32_t a){
  uint32_t pixel = ((r << 24) | (g << 16) | (b << 8) | a);
  return pixel;
}

// Convert input pixel to grayscale.
//
// Parameters:
//   pixel - one single pixel contains rgba information
//
// Returns:
//   a pixel in grayscale
uint32_t to_grayscale( uint32_t pixel ) {
  uint32_t red = get_r(pixel);
  uint32_t green = get_g(pixel);
  uint32_t blue = get_b(pixel);
  uint32_t alpha = get_a(pixel);
  uint32_t gray = (79 * red + 128 * green + 49 * blue) / 256;
  uint32_t new_pixel = make_pixel(gray, gray, gray, alpha);
  return new_pixel;
}

// Convert input pixels to grayscale.
// This transformation always succeeds.
//
// Parameters:
//   input_img  - pointer to the input Image
//   output_img - pointer to the output Image (in which the transformed
//                pixels should be stored)
void imgproc_grayscale( struct Image *input_img, struct Image *output_img ) {
  output_img->width = input_img->width;
  output_img->height = input_img->height;
  for (int i = 0; i < input_img->width * input_img->height; i++){
    uint32_t pixel = input_img->data[i];
    uint32_t new_pixel = to_grayscale(pixel);
    output_img->data[i] = new_pixel; 
  }
}

// Return the relative index of a pixel in the array that represent the image
// 
// Parameters:
//   img - pointer to the image which contains data
//   col - the column index in the 2D image of the pixel
//   row - the row index in the 2D image of the pixel
// 
// Returns:
//   the pixel's index in an array, converting from the 2D image
int32_t compute_index( struct Image *img, int32_t col, int32_t row ){
  int32_t index = row * img->width + col;
  return index;
}

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
void imgproc_rgb( struct Image *input_img, struct Image *output_img ) {
  // The output image has twice the width/height of the input image
  output_img->height = input_img->height * 2;
  output_img->width = input_img->width * 2;

  for (int row = 0; row < input_img->height; row++){
    for (int col = 0; col < input_img->width; col++){
      uint32_t pixel = input_img->data[compute_index(input_img, col, row)];
      uint32_t r = get_r(pixel);
      uint32_t g = get_g(pixel);
      uint32_t b = get_b(pixel);
      uint32_t a = get_a(pixel);
      uint32_t pixel_r = make_pixel(r, 0, 0, a);
      uint32_t pixel_g = make_pixel(0, g, 0, a);
      uint32_t pixel_b = make_pixel(0, 0, b, a);
      
      // replicate original image A
      output_img->data[compute_index(output_img, col, row)] = pixel;

      // B shows only the red color component
      output_img->data[compute_index(output_img, col + input_img->width, row)] = pixel_r;

      // C shows only the green color component
      output_img->data[compute_index(output_img, col, row + input_img->height)] = pixel_g;

      // D shows only the blue color component
      output_img->data[compute_index(output_img, col + input_img->width, row + input_img->height)] = pixel_b;
    }
  }
}

// Calculate the gradient for the input row/column coordinate
// 
// Parameters:
//   x - input row/column coordinate
//   max - number of pixels in that row or column
// 
// Return:
//   the calculated gradient for the input row/column coordinate
int64_t gradient( int64_t x, int64_t max ){
  int64_t gradient = ((2000000000 * x)/(1000000 * max) - 1000) * ((2000000000 * x)/(1000000 * max) - 1000);
  return 1000000 - gradient;
}


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
void imgproc_fade( struct Image *input_img, struct Image *output_img ) {
  output_img->width = input_img->width;
  output_img->height = input_img->height;

  for (int i = 0; i < input_img->height * input_img->width; i++){
    int row = i / input_img->width;
    int col = i % input_img->width;
    uint32_t pixel = input_img->data[i];
    uint32_t r = get_r(pixel);
    uint32_t g = get_g(pixel);
    uint32_t b = get_b(pixel);
    uint32_t a = get_a(pixel);

    // Calculate the gradient for row/column coordiante respectively
    int64_t tr = gradient(row, input_img->height);
    int64_t tc = gradient(col, input_img->width);

    uint64_t DENOM = 1000000000000;
    uint32_t fadeR = (tr * tc * r) / DENOM;
    uint32_t fadeG = (tr * tc * g) / DENOM;
    uint32_t fadeB = (tr * tc * b) / DENOM;

    uint32_t fadePixel = make_pixel(fadeR, fadeG, fadeB, a);
    output_img->data[i] = fadePixel;
  }
}

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
int imgproc_kaleidoscope( struct Image *input_img, struct Image *output_img ) {
  if (input_img->width != input_img->height){
    return 0;
  }
  output_img->width = input_img->width;
  output_img->height = input_img->height;
  int32_t width = input_img->width;
  int32_t in_row, in_col, in_idx, fake_width;
  for(int i = 0; i < width * width; i++){
    in_row = i / width;
    in_col = i % width;
    fake_width = width;
    if (width % 2 != 0){
     fake_width = width + 1;
    }
    if (in_row >= fake_width / 2){
      in_row = fake_width - in_row - 1;
    }
    if (in_col >= fake_width / 2){
      in_col = fake_width - in_col - 1;
    }
    if (in_row > in_col){
      int32_t temp = in_row;
      in_row = in_col;
      in_col = temp;
    }
    in_idx = in_row * width + in_col;
    output_img->data[i] = input_img->data[in_idx];
  }
  return 1;
}
