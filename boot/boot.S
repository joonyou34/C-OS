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

		# initiate video mode switch
		movw	$0x4f02, %ax
        movw	$0x4118, %bx
        int	$0x10

		# in case the mode set was successful, spin
		jc      spin
		
		# retrieve mode info for mode 0x118
		lea     VesaModeInfoBlock, %di  
		movw    %cs, %ax           
		movw    %ax, %es           
		movw    $0x4f01, %ax       
		movw    $0x0118, %cx       
		int     $0x10              

		# in case the mode info query was successful, spin
		jc      spin           

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

	mov $0x02, %ah
	mov	$1,	   %al 		  # number of sectors to read
	mov $0x2, %cl        # Set sector to read (2nd sector)
	mov $0x00, %ch        # Set cylinder (0)
	mov $0x00, %dh        # Set head (0, for the first platter)
	mov $0x80, %dl        # Drive number (0x80 for the first hard disk)

	xor %bx, %bx          # Clear BX register (segment = 0)
	mov %bx, %es
	mov $0x8000, %bx      # Set memory location (0x8000)

	int $0x13             # BIOS interrupt to read the disk

	jc spin

	lea     VesaModeInfoBlock, %eax    # Load the address of Mode Info Block

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