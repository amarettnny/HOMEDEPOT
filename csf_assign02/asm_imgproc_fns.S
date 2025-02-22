/*
 * x86-64 assembly language implementations of functions
 */

	.section .text

/* Offsets of struct Image fields */
#define IMAGE_WIDTH_OFFSET   0
#define IMAGE_HEIGHT_OFFSET  4
#define IMAGE_DATA_OFFSET    8

/*
 * TODO: define your helper functions here.
 * Don't forget to use the .globl directive to make
 * them visible to the test code.
 */

/*
 * uint32_t get_r( uint32_t pixel);
 *
 * Get r value from the given pixel
 *
 * Parameters:
 *   %rdi - store the value of input pixel
 * Returns
 *   %rax - store the return value r
 */
        .globl get_r
get_r:
	movl %edi, %eax //copy input pixel
	shrl $24, %eax // pixel >> 24 to get r
        ret
/*
 * uint32_t get_g( uint32_t pixel);
 *
 * Get r value from the given pixel
 *
 * Parameters:
 *   %rdi - store the value of input pixel
 * Returns
 *   %rax - store the return value g
 */
        .globl get_g
get_g:
	movl %edi, %eax  //copy input pixel
	andl $0xFF0000, %eax // get bits for g
	shrl $16, %eax // pixel >> 16 to get g
        ret
/*
 * uint32_t get_b( uint32_t pixel);
 *
 * Get r value from the given pixel
 *
 * Parameters:
 *   %rdi - store the value of input pixel
 * Returns
 *   %rax - store the return value b
 */
        .globl get_b
get_b:
	movl %edi, %eax  //copy input pixel
	andl $0xFF00, %eax // get bits for b
	shrl $8, %eax // pixel >> 8 to get g
        ret
/*
 * uint32_t get_a( uint32_t pixel);
 *
 * Get r value from the given pixel
 *
 * Parameters:
 *   %rdi - store the value of input pixel
 * Returns
 *   %rax - store the return value a
 */
        .globl get_a
get_a:
	movl %edi, %eax //copy input pixel
	andl $0xFF, %eax // get a
        ret
/* 
 * uint32_t make_pixel(uint32_t r, uint32_t g, uint32_t b, uint32_t a)
 * 
 * Make a pixel given its rgba values
 * 
 * Parameters:
 *   %rdi - store the value of input r
 *   %rsi - store the value of input g
 *   %rdx - store the value of input b
 *   %rcx - store the value of input a
 * Returns:
 *   %rax - store the value of output pixel
 *
 */
        .globl make_pixel
make_pixel:
	shll $24, %edi // r << 24
	shll $16, %esi // g << 16
	shll $8, %edx  // b << 8
	movl %ecx, %eax // add a to the 0-7 bits
	orl %edx, %eax  // add b to the 8-15 bits
	orl %esi, %eax  // add g to the 16-23 bits
	orl %edi, %eax  // add r to the 24-31 bits
        ret
/*
 * uint32_t to_grayscale( uint32_t pixel )
 * 
 * Turn the given pixel into grayscale
 * 
 * Parameters:
 *   %rdi - store the value of input pixel
 * 
 * Returns:
 *   %rax - store the value of output grayscale pixel 
 */
	.globl to_grayscale
to_grayscale:
	call get_r   // get r value to %eax
	imulq $79, %rax // r * 79
	movq %rax, %r8 // move to %r8
	call get_g  // get g value to %eax
	imulq $128, %rax // g * 128
	addq %rax, %r8 // add g * 128 to %r8
	call get_b // get b value to %eax
	imulq $49, %rax // b * 49
	addq %rax, %r8 // add b * 49 to %r8
	shrq $8, %r8 // devide by 256
	call get_a  // get a value to %eax
	movq %rax, %rcx //move rgba arguments to the corresponding register
	movq %r8, %rdi
	movq %r8, %rsi
	movq %r8, %rdx
	call make_pixel // call make_pixel and store output in %eax
        ret
		.globl gradient
gradient:
		ret
		.globl compute_index
compute_index:
		ret

/*
 * Implementations of API functions
 */

/*
 * Render an output image containing 4 copies of the original image,
 * refered to as A, B, C, and D in the following diagram:
 *
 *   +---+---+
 *   | A | B |
 *   +---+---+
 *   | C | D |
 *   +---+---+
 *
 * The width and height of the output image are (respectively) twice
 * the width and height of the input image.
 *
 * A is an exact copy of the original input image. B has only the
 * red color component values of the input image, C has only the
 * green color component values, and D has only the blue color component
 * values.
 *
 * Each of the copies (A-D) should use the same alpha values as the
 * original image.
 *
 * This transformation always succeeds.
 *
 * Parameters:
 *   %rdi - pointer to the input Image
 *   %rsi - pointer to the output Image (which will have
 *          width and height twice the width/height of the
 *          input image)
 */
	.globl imgproc_rgb
imgproc_rgb:
	/* TODO: implement */
	ret

/*
 * void imgproc_grayscale( struct Image *input_img, struct Image *output_img );
 *
 * Transform image by converting each pixel to grayscale.
 * This transformation always succeeds.
 *
 * Parameters:
 *   %rdi - pointer to original struct Image
 *   %rsi - pointer to output struct Image
 */
	.globl imgproc_grayscale
imgproc_grayscale:
	movl (%rdi), %r8d // move width to output
	movl %r8d, (%rsi)
	movl 4(%rdi), %r9d // move height to output
	movl %r9d, 4(%rsi) 
	movq 8(%rdi), %r10 // store the input image pixel pointer
	movq 8(%rsi), %r12 // store the output image pixel pointer
	imulq %r8, %r9  // width * height in %r9
	movl $0, %r13d  // %13 is the loop index
.Lloopstart:
	cmpl %r9d, %r13d // compare index with width * height
	jge .Lloopfinish // if index >= width * height, finish loop
	movl (%r10), %edi // get the pixel value to %edi
	call to_grayscale // get the grayscale pixel
	movl %eax, (%r12) // put the gray pixel to output image
	addq $4, %r12 // update the memory address
	addq $4, %r10 // update the memory address
	inc %r13d  // index += 1
	jmp .Lloopstart 
.Lloopfinish:
			
	ret

/*
 * Render a "faded" version of the input image.
 *
 * See the assignment description for an explanation of how this transformation
 * should work.
 *
 * This transformation always succeeds.
 *
 * Parameters:
 *   %rdi - pointer to the input Image
 *   %rsi - pointer to the output Image
 */
	.globl imgproc_fade
imgproc_fade:
	/* TODO: implement */
	ret

/*
 * Render a "kaleidoscope" transformation of input_img in output_img.
 * The input_img must be square, i.e., the width and height must be
 * the same. Assume that the input image is divided into 8 "wedges"
 * like this:
 *
 *    +----+----+
 *    |\ A |   /|
 *    | \  |  / |
 *    |  \ | /  |
 *    | B \|/   |
 *    +----+----+
 *    |   /|\   |
 *    |  / | \  |
 *    | /  |  \ |
 *    |/   |   \|
 *    +----+----+
 *
 * The wedge labeled "A" is the one that will be replicated 8 times
 * in the output image. In the output image,
 *
 *    Wedge A will have exactly the same pixels as wedge A in
 *    the input image, in the same positions.
 *
 *    Wedge B will have exactly the same pixels as wedge A, but the
 *    column and row are transposed. (I.e., it is a reflection across
 *    the diagonal.)
 *
 *    All other wedges are mirrored versions of A such that the overall
 *    image is symmetrical across all four lines dividing the image
 *    (vertical split, horizontal split, and the two diagonal splits.)
 *
 * Parameters:
 *   input_img  - pointer to the input Image
 *   output_img - pointer to the output Image (in which the transformed
 *                pixels should be stored)
 *
 * Returns:
 *   1 if successful, 0 if the transformation fails because the
 *   width and height of input_img are not the same.
 */
	.globl imgproc_kaleidoscope
imgproc_kaleidoscope:
	/* TODO: implement */
	ret

	/* This avoids linker warning about executable stack */
.section .note.GNU-stack,"",@progbits

/*
vim:ft=gas:
*/
