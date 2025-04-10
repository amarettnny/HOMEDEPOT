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

/*
 * int64_t gradient( int64_t x, int64_t max )
 * 
 * Calculate the gradient for the input row/column coordinate
 * 
 * Parameters:
 *   %rdi - input row/column coordinate
 *	 %rsi - number of pixels in that row or column
 * 	 
 * Returns:
 *   %rax - store the calculated gradient of the input row/column coordinate
 */
 		.globl gradient
gradient:
	imulq $2000, %rdi					    /* (2000000000 * x) / (1000000 * n) = 2000 * x / n*/
	movq %rdi, %rax							/* move the multiplication to rax for divisor */

	movq %rsi, %rcx							/* move the multiplication to rcx for dividend*/
	cqto									/* convert %rax to %rdx:%rax */
	idivq %rcx								/* do division: divide %rdx:rax by %rcx */

	subq $1000, %rax						/* subtract 1000 from (2000000000 * x)/(1000000 * max) */
	movq %rax, %rdx
	imulq  %rdx, %rax						/* square the result */

	movq $1000000, %rcx						/* return 1000000 - squared result */
	subq %rax, %rcx
	movq %rcx, %rax
	ret


/*
 * int32_t compute_index( struct Image *img, int32_t col, int32_t row )
 * 
 * Return the relative index of a pixel in the array that represent the image.
 * 
 * Parameters:
 *   %rdi - pointer to input Image sturct
 *	 %rsi - column index
 *	 %rdx - row index
 * 	 
 * Returns:
 *   %rax - store the value of output grayscale pixel 
 */
		.globl compute_index
compute_index:
	movl IMAGE_WIDTH_OFFSET(%rdi), %eax		/* Load the image width to eax */
	imull %edx, %eax						/* index = row * img->width + col */
	addl %esi, %eax
	cltq									/* signed extension to 64-bits */
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
	/* prologue to create ABI-compliant stack frame */
    pushq %rbp
    movq %rsp, %rbp
    subq $64, %rsp                      /* Reserve 64 bytes for local variables */
    pushq %r12							/* row counter */
    pushq %r13							/* column counter */
    pushq %r14							/* pointer to input Image struct */
    pushq %r15							/* pointer to output Image struct */

	movq %rdi, %r14
	movq %rsi, %r15

	/* Update output image dimensions */
	movl IMAGE_HEIGHT_OFFSET(%r14), %eax	/* store input image height */
	addl %eax, %eax							/* output height = input height * 2 */
	movl %eax, IMAGE_HEIGHT_OFFSET(%r15)
	movl IMAGE_WIDTH_OFFSET(%r14), %eax		/* store input image width */
	addl %eax, %eax							/* output width = input width * 2 */
	movl %eax, IMAGE_WIDTH_OFFSET(%r15)

	/* Find loop limitations */
    movl IMAGE_WIDTH_OFFSET(%r14), %r8d    	/* input width */
    movl IMAGE_HEIGHT_OFFSET(%r14), %r9d   	/* input height */
	movl $0, %r12d
.Louter:
    movl $0, %r13d     						/* Initialize column counter when iterating new row */        	
.Linner:
    /* Load pixel in the input image based on current row/column index */
    movq %r14, %rdi                        /* Pointer to input image */
    movl %r13d, %esi                       /* column index */
    movl %r12d, %edx                       /* row index */
    call compute_index          
	movq IMAGE_DATA_OFFSET(%r14), %rcx
    movl (%rcx,%rax,4), %ebx               /* Load pixel to ebx */

	/* Extract r+g+b+alpha component individually */
    movl %ebx, %edi                        
    call get_r
    movl %eax, -4(%rbp)                    /* Store red component in -4 offsets */
    movl %ebx, %edi
    call get_g
    movl %eax, -8(%rbp)                    /* Store green component in -8 offsets */
    movl %ebx, %edi
    call get_b
    movl %eax, -12(%rbp)                   /* Store blue component in -12 offsets */
    movl %ebx, %edi
    call get_a
    movl %eax, -16(%rbp)                   /* Store alpha component in -16 offsets */

	/* Create r pixel */
	movl -4(%rbp), %edi                   
    movl $0, %esi                       
    movl $0, %edx                        
    movl -16(%rbp), %ecx                 
    call make_pixel
    movl %eax, -20(%rbp)					/* Store red pixel in -20 offsets */
    /* Create g pixel */
    movl $0, %edi                         
    movl -8(%rbp), %esi                   
    movl $0, %edx                    
    movl -16(%rbp), %ecx                   
    call make_pixel
    movl %eax, -24(%rbp)					/* Store green pixel in -24 offsets */
    /* Create b pixel */
    movl $0, %edi                     
    movl $0, %esi                     
    movl -12(%rbp), %edx                  
    movl -16(%rbp), %ecx                 
    call make_pixel
    movl %eax, -28(%rbp)					/* Store blue pixel in -28 offsets */

	/* Update 4 quadrants */
    /* Quadrant A */
    movq %r15, %rdi                        
    movl %r13d, %esi                       
    movl %r12d, %edx                       
    call compute_index						/* Quadrant A index = current index */
    movq IMAGE_DATA_OFFSET(%r15), %rcx
    movl %ebx, (%rcx,%rax,4)

    /* Quadrant B */
	movq %r15, %rdi
    movl %r13d, %esi
    addl %r8d, %esi                        	/* Quadrant B column index = current index + input image width */
	movq IMAGE_DATA_OFFSET(%r15), %rcx
    call compute_index
	movl -20(%rbp), %edx   					/* Load red pixel value to edx */
	movl %edx, (%rcx,%rax,4)				/* to prevent mismatch in dimensions */

    /* Quadrant C */
    movq %r15, %rdi
	movl %r13d, %esi
    movl %r12d, %edx
    addl %r9d, %edx                        /* Quadrant C row index = current index + input image height */
    call compute_index
    movq IMAGE_DATA_OFFSET(%r15), %rcx
    movl -24(%rbp), %edx
	movl %edx, (%rcx,%rax,4)

    /* Quadrant D */
    movq %r15, %rdi
	movl %r13d, %esi
    addl %r8d, %esi                        /* Quadrant D column index = current index + input image width */
    movl %r12d, %edx
    addl %r9d, %edx                        /* Quadrant D row index = current index + input image height */
    call compute_index
    movq IMAGE_DATA_OFFSET(%r15), %rcx
    movl -28(%rbp), %edx
	movl %edx, (%rcx,%rax,4)

    incl %r13d								/* Increment column counter */
    cmpl %r8d, %r13d
    jl .Linner								/* < input image width */
    incl %r12d								/* Increment row counter */
    cmpl %r9d, %r12d
    jl .Louter								/* < input image height */

	/* epilogue */
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    addq $64, %rsp
    popq %rbp
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
    /* prologue to create ABI-compliant stack frame */
    pushq %rbp
    movq %rsp, %rbp
    pushq %rbx               
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    subq $64, %rsp          

    /*  Copy width and height from input to output */
    movl IMAGE_WIDTH_OFFSET(%rdi), %eax   
    movl %eax, IMAGE_WIDTH_OFFSET(%rsi)   
    movl IMAGE_HEIGHT_OFFSET(%rdi), %eax  
    movl %eax, IMAGE_HEIGHT_OFFSET(%rsi)  

    /*  Compute total number of pixels */
    movl IMAGE_WIDTH_OFFSET(%rdi), %r8d                  /*  r8d = width */
    movl IMAGE_HEIGHT_OFFSET(%rdi), %r9d                 /*  r9d = height */
    movl %r8d, %r12d                     
    imull %r9d, %r12d                                     /*  r12d = width * height */

    movq IMAGE_DATA_OFFSET(%rdi), %r13                   /*  r13 = input image data pointer */
    movq IMAGE_DATA_OFFSET(%rsi), %r14                   /*  r14 = output image data pointer */

    /*  Load constant DENOM = 1000000000000 to r15 */
    movq $1000000000000, %r15

    movq $0, %rbx                                      /*  loop index i = 0 */
.Lfade:
    cmpq %r12, %rbx                                  /*  continue loop until total pixels >= i */
    jge .Lfade_epilogue

    /*  Compute row and column index */
    movl %ebx, %eax      
    cltd                                             /*  sign extend eax into edx:eax for division */
    idivl %r8d            
    movl %eax, %r10d                                 /*  row_index = i / input_img width */
    movl %edx, %r11d                                 /*  column_index = i % input_img width */

    /*  Load current pixel */
    movl (%r13, %rbx, 4), %edi  
    movl %edi, -4(%rbp)                              /*  store pixel at -4(%rbp) */

    /*  Extract r, g, b colors */
    movl -4(%rbp), %edi
    call get_r
    movl %eax, -8(%rbp)                              /*  store red at -8(%rbp) */

    movl -4(%rbp), %edi
    call get_g
    movl %eax, -12(%rbp)                             /*  store green at -12(%rbp) */

    movl -4(%rbp), %edi
    call get_b
    movl %eax, -16(%rbp)                             /*  store blue at -16(%rbp) */

    movl -4(%rbp), %edi
    call get_a
    movl %eax, -20(%rbp)                             /*  store alpha at -20(%rbp) */

    /*  Compute row gradient tr */
    movl %r10d, %edi          
    movl %r9d, %esi           
    call gradient
    movq %rax, -32(%rbp)                             /*  store tr at -32(%rbp) */

    /*  Compute column gradient tc */
    movl %r11d, %edi                     
    movl %r8d, %esi          
    call gradient
    movq %rax, %rcx                                  /*  store tc in rcx */

    movq -32(%rbp), %rdx                             /*  load tr into rdx */
    imulq %rcx, %rdx                                 /*  rdx = tr * tc (fade factor) */
    movq %rdx, -56(%rbp)                             /*  save fade factor at -56(%rbp) */

    /*  fadeR = (fade factor * red) / DENOM */
    movq -56(%rbp), %rax     
    movslq -8(%rbp), %rsi      
    imulq %rsi, %rax                                  /*  rax = fade factor * red */
    cqto                                              /*  sign extend rax into rdx:rax */
    idivq %r15                                        /*  divide by DENOM */
    movl  %eax, -40(%rbp)                             /*  store faded red at -40(%rbp) */

    /*  fadeG = (fade factor * green) / DENOM */
    movq -56(%rbp), %rax                                
    movslq -12(%rbp), %rsi     
    imulq %rsi, %rax         
    cqto
    idivq %r15
    movl %eax, -44(%rbp)                              /*  store faded green at -44(%rbp) */

    /*  fadeB = (fade factor * blue) / DENOM */
    movq    -56(%rbp), %rax     
    movslq  -16(%rbp), %rsi     
    imulq   %rsi, %rax         
    cqto
    idivq   %r15
    movl    %eax, -48(%rbp)                           /*  store faded blue at -48(%rbp) */

    /*  create output pixel using faded components */
    movl    -40(%rbp), %edi    
    movl    -44(%rbp), %esi    
    movl    -48(%rbp), %edx    
    movl    -20(%rbp), %ecx    
    call    make_pixel                                /*  returns new pixel in eax */

    /*  store the new pixel in output image */
    movl    %eax, (%r14, %rbx, 4)                 

    incq    %rbx                                      /*  loop index i++ */
    jmp     .Lfade
.Lfade_epilogue:
    addq    $64, %rsp
    popq    %rbx
    popq    %r12
    popq    %r13
    popq    %r14
    popq    %r15
    popq    %rbp
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
 *   %rdi  - pointer to the input Image
 *   %rsi  - pointer to the output Image (in which the transformed
 *                pixels should be stored)
 *
 * Returns:
 *   1 if successful, 0 if the transformation fails because the
 *   width and height of input_img are not the same.
 */
	.globl imgproc_kaleidoscope
imgproc_kaleidoscope:
	movl (%rdi), %r8d // move width to output
	movl %r8d, (%rsi)
	movl 4(%rdi), %r9d // move height to output
	movl %r9d, 4(%rsi) 
	movl $0, %eax  //prepare for transwformation fail output
	cmpl %r8d, %r9d // compare width and height
	jne .Lreturn  // if not equal, return 
	movq 8(%rdi), %r14 // %r14 is data[] for input image
	movq 8(%rsi), %r15 // %r15 is data[] for output image
	imulq %r8, %r9 //%r9 stores width * width
	movl %r8d, %r10d // %r10 is the "fake width" for width is odd case
	shr $1, %r10d 
	shl $1, %r10d
	cmpl %r8d, %r10d //check if width is even
	je .Lcontinue  
	movl %r8d, %r10d  // if no even %r10 = %r8 + 1
	addl $1, %r10d  
.Lcontinue:
	movl %r10d, %r11d 
	shr $1, %r11d //%r11 is %r10/2 	
	movl $0, %r13d // r13 is the index of the loop
.Lloop:
	cmpl %r9d, %r13d // compare index with width*width
	jge .Lsuccess
	movl %r13d, %eax // %eax now stores the index
	cltd   // extension, ready for division
	idivl %r8d // divide index by width to determine row_idx and col_idx 
	cmpl %r11d, %eax // compre row_idx and width/2
	jl .Lrowfinish
	movl %eax, %r12d // if row_idx >= width/2, map it up
	movl %r10d, %eax
	subl %r12d, %eax
	subl $1, %eax	
.Lrowfinish:
	cmpl %r11d, %edx // compare col_idx and width/2
	jl .Lcolfinish
	movl %edx, %r12d // if col_idx >= width/2, map it left
	movl %r10d, %edx
	subl %r12d, %edx
	subl $1, %edx		
.Lcolfinish:
	cmpl %edx, %eax // compare row_idx and col_idx
	jle .Lindexfinish
	movl %eax, %r12d // if row_idx > col_idx, map it through the diagnal
	movl %edx, %eax
	movl %r12d, %edx
.Lindexfinish:
	imull %r8d, %eax // calculate the corresponding idx in input image
	addl %edx, %eax
	imull $4, %eax  // time 4 for memory address edition
	addq %rax, %r14  // get data[i] in input image
	movl (%r14), %ebx
	subq %rax, %r14
	movl %ebx, (%r15) // store it to output image data
	addq $4, %r15 
	inc %r13d  //update loop index
	jmp .Lloop	
.Lsuccess:
	movl $1, %eax // prepare output for successful transformation
.Lreturn:
	ret

	/* This avoids linker warning about executable stack */
.section .note.GNU-stack,"",@progbits

/*
vim:ft=gas:
*/
