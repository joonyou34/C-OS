# 0 for text, 1 for graphics
#define USE_GRAPHICS 1
###############################################################################
# ENTRY POINT	
#   This code should be stored in the first sector of the hard disk.
#   After the BIOS initializes the hardware on startup or system reset,
#   it loads this code at physical address 0x7c00 - 0x7d00 (512 bytes).
#   Then the BIOS jumps to the beginning of it, address 0x7c00,
#   while running in 16-bit real-mode (8086 compatibility mode).
#   The Code Segment register (CS) is initially zero on entry.
#	
# This code switches into 32-bit protected mode so that all of
# memory can accessed, then calls into C.
###############################################################################
	
.globl start					# Entry point	
start:		.code16				# This runs in real mode
		cli				# Disable interrupts
		cld				# String operations increment

		# Set up the important data segment registers (DS, ES, SS).
		xorw	%ax,%ax			# Segment number zero
		movw	%ax,%ds			# -> Data Segment
		movw	%ax,%es			# -> Extra Segment
		movw	%ax,%ss			# -> Stack Segment

		# Set up the stack pointer, growing downward from 0x7c00.
		movw	$start,%sp         	# Stack Pointer

		# this code snippet basically does if(USE_GRAPHICS) goto skip
		# which is used to skip the section where the graphics mode is initiaited
		mov 	$USE_GRAPHICS, %ax
		xor		%bx, %bx
		cmp 	%ax, %bx
		je	skip
		clc
		
		# initiate video mode switch to switch to mode 0x0118, which is a 24-bit color 1024x768 graphics mode
		# reference: https://en.wikipedia.org/wiki/VESA_BIOS_Extensions
		movw	$0x4f02, %ax			# this is the function code for setting the vesa video mode
        movw	$0x0118, %bx
        int	$0x10						# interrupt 0x10 is the bios interrupt for video mode, reference: https://en.wikipedia.org/wiki/INT_10H

		# in case the mode set was successful, spin
		jc      spin
		
		# retrieve mode info for mode 0x0118
		lea     VesaModeInfoBlock, %di  # struct definition at the bottom
		movw    %cs, %ax           		# the interrupt requires a pointer to the target struct in es:di registers
		movw    %ax, %es           
		movw    $0x4f01, %ax       		# this is the function code for retrieving the vesa mode info
		movw    $0x0118, %cx       
		int     $0x10

		# in case the mode info query was successful, spin
		jc      spin

skip:
# Enable A20:
#   For fascinating historical reasons (related to the fact that
#   the earliest 8086-based PCs could only address 1MB of physical memory
#   and subsequent 80286-based PCs wanted to retain maximum compatibility),
#   physical address line 20 is tied to low when the machine boots.
#   Obviously this a bit of a drag for us, especially when trying to
#   address memory above 1MB.  This code undoes this.
	
seta20.1:	inb	$0x64,%al		# Get status
		testb	$0x2,%al		# Busy?
		jnz	seta20.1		# Yes
		movb	$0xd1,%al		# Command: Write
		outb	%al,$0x64		#  output port
seta20.2:	inb	$0x64,%al		# Get status
		testb	$0x2,%al		# Busy?
		jnz	seta20.2		# Yes
		movb	$0xdf,%al		# Enable
		outb	%al,$0x60		#  A20

	# retrieve stage 2 from sector 2 (1-based) and put it in memory location 0x8000
	mov $0x02, %ah		  # function code to read sector(s) from drive, reference: https://en.wikipedia.org/wiki/INT_13H
	mov	$1,	   %al 		  # number of sectors to read
	mov $0x2,  %cl        # Set sector to read (2nd sector)
	mov $0x00, %ch        # Set cylinder (0)
	mov $0x00, %dh        # Set head (0, for the first platter)
	mov $0x80, %dl        # Drive number (0x80 for the first hard disk)

	xor %bx, %bx  	 	  # memory location to load into from sector is stored in es:bx
	mov %bx, %es 
	mov $0x8000, %bx

	int $0x13			  # as seen in the previously provided reference above, interrupt 0x13 is a bios interrupt for disk

	# in case loading from disk fails, spin
	jc spin

	# making sure ebx is 0, we use ebx to pass the video mode info struct address to stage 2
	# so in case we DON'T want to use graphics mode, we will send a 0 pointer (which will be used to indicate no graphics mode)
	xor 	%ebx, %ebx

	# same as above, skip the following part in case we don't want to use graphics
	mov 	$USE_GRAPHICS, %ax
	xor		%bx, %bx
	cmp 	%ax, %bx
	je	skip2
	clc

	# the pointer to the vesa mode info block will go through the following path:
	# boot.S -> stage2.S (through ebx register)
	# stage2.S -> main.c (as a paramater through memory)
	# main.c -> entry.S (as a paramater through memory)
	# entry.S -> init.c (as a paramater through memory)
	lea     VesaModeInfoBlock, %ebx    # Load the address of Mode Info Block

# jump to the address where stage2 is loaded
skip2:
	jmp 0x8000

spin:
	jmp spin

# the mode info block struct, reference: https://wiki.osdev.org/VESA_Video_Modes
.align 4
VesaModeInfoBlock:
    ModeAttributes:        .space 2         
    FirstWindowAttributes: .space 1         
    SecondWindowAttributes:.space 1         
    WindowGranularity:     .space 2         
    WindowSize:            .space 2         
    FirstWindowSegment:    .space 2         
    SecondWindowSegment:   .space 2         
    WindowFunctionPtr:     .space 4         
    BytesPerScanLine:      .space 2         

    Width:                 .space 2         
    Height:                .space 2         
    CharWidth:             .space 1         
    CharHeight:            .space 1         
    PlanesCount:           .space 1         
    BitsPerPixel:          .space 1         
    BanksCount:            .space 1         
    MemoryModel:           .space 1         
    BankSize:              .space 1         
    ImagePagesCount:       .space 1         
    Reserved1:             .space 1         

    RedMaskSize:           .space 1         
    RedFieldPosition:      .space 1         
    GreenMaskSize:         .space 1         
    GreenFieldPosition:    .space 1         
    BlueMaskSize:          .space 1         
    BlueFieldPosition:     .space 1         
    ReservedMaskSize:      .space 1         
    ReservedMaskPosition:  .space 1         
    DirectColorModeInfo:   .space 1         

    LFBAddress:            .space 4         
    OffscreenMemoryOffset: .space 4         
    OffscreenMemorySize:   .space 2         
    Reserved2:             .space 206 