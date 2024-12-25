#include <inc/mmu.h>
.set PROT_MODE_CSEG,0x8		# code segment selector
.set PROT_MODE_DSEG,0x10        # data segment selector
.set CR0_PE_ON,0x1		# protected mode enable flag

########################################################################################
# Here's a bit of a backstory for what is this whole stage 2 thing
# Basically, for basic bootloaders working in legacy BIOS
# the boot file needs to fit fully in the boot sector
# which means the whole boot file had to be at max sector size
# where a sector is 512 bytes (510 for the boot code and 2 for the magic)
# and this came in as an issue when attempting to work with VESA video modes
# since the vesa mode info block is 256 bytes, it won't fit with the original boot file
# so to bypass this limitaion, the idea is to go for a multi-staged bootloader
# which works by having only the first stage loaded in the boot sector
# and then stage 1 would jump to stage 2 (loaded on a different sector)
# and in case we need more stages then stage 2 would call stage 3 and so on
# but for now we only need 2 stages, thus there is only stage 2
########################################################################################
	
.globl start2					# Entry point	
start2:		.code16				# This runs in real mode

# Switch from real to protected mode:
#   Up until now, there's been no protection, so we've gotten along perfectly
#   well without explicitly telling the processor how to translate addresses.
#   When we switch to protected mode, this is no longer true!
#   We need at least to set up some "segments" that tell the processor it's
#   OK to run code at any address, or write to any address.
#   The 'gdt' and 'gdtdesc' tables below define these segments.
#   This code loads them into the processor.
#   We need this setup to ensure the transition to protected mode is smooth.

real_to_prot:	cli			# Don't allow interrupts: mandatory,
					# since we didn't set up an interrupt
					# descriptor table for handling them
		lgdt	gdtdesc		# load GDT: mandatory in protected mode
		movl	%cr0, %eax	# Turn on protected mode
		orl	$CR0_PE_ON, %eax
		movl	%eax, %cr0

	        # CPU magic: jump to relocation, flush prefetch queue, and
		# reload %cs.  Has the effect of just jmp to the next
		# instruction, but simultaneously loads CS with
		# $PROT_MODE_CSEG.
		ljmp	$PROT_MODE_CSEG, $protcseg
	
		# we've switched to 32-bit protected mode; tell the assembler
		# to generate code for that mode
protcseg:	.code32
		# Set up the protected-mode data segment registers
		movw	$PROT_MODE_DSEG, %ax	# Our data segment selector
		movw	%ax, %ds		# -> DS: Data Segment
		movw	%ax, %es		# -> ES: Extra Segment
		movw	%ax, %fs		# -> FS
		movw	%ax, %gs		# -> GS
		movw	%ax, %ss		# -> SS: Stack Segment

		# making sure we pass ebs (which contains the mode info block address) to the cmain
		push	%ebx
		call	cmain			# finish the boot load from C.
						# cmain() should not return
spin:		jmp spin			# ..but in case it does, spin
	
		.p2align 2			# force 4 byte alignment
gdt:		SEG_NULL				# null seg
		SEG(STA_X|STA_R, 0x0, 0xffffffff)	# code seg
		SEG(STA_W, 0x0, 0xffffffff)	        # data seg
	
gdtdesc:	.word	0x17			# sizeof(gdt) - 1
		.long	gdt			# address gdt