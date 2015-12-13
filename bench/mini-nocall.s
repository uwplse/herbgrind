	.file	"mini.c"
	.text
.Ltext0:
	.type	VALGRIND_PRINTF, @function
VALGRIND_PRINTF:
.LFB0:
	.file 1 "../valgrind/herbgrind/../include/valgrind.h"
	.loc 1 6760 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$168, %rsp
	movq	%rdi, -280(%rbp)
	movq	%rsi, -168(%rbp)
	movq	%rdx, -160(%rbp)
	movq	%rcx, -152(%rbp)
	movq	%r8, -144(%rbp)
	movq	%r9, -136(%rbp)
	testb	%al, %al
	je	.L4
	movaps	%xmm0, -128(%rbp)
	movaps	%xmm1, -112(%rbp)
	movaps	%xmm2, -96(%rbp)
	movaps	%xmm3, -80(%rbp)
	movaps	%xmm4, -64(%rbp)
	movaps	%xmm5, -48(%rbp)
	movaps	%xmm6, -32(%rbp)
	movaps	%xmm7, -16(%rbp)
.L4:
	.loc 1 6770 0
	movl	$8, -208(%rbp)
	movl	$48, -204(%rbp)
	leaq	16(%rbp), %rax
	movq	%rax, -200(%rbp)
	leaq	-176(%rbp), %rax
	movq	%rax, -192(%rbp)
.LBB2:
	.loc 1 6778 0
	movq	$5123, -272(%rbp)
	movq	-280(%rbp), %rax
	movq	%rax, -264(%rbp)
	leaq	-208(%rbp), %rax
	movq	%rax, -256(%rbp)
	movq	$0, -248(%rbp)
	movq	$0, -240(%rbp)
	movq	$0, -232(%rbp)
	leaq	-272(%rbp), %rax
	movl	$0, %ecx
	movl	%ecx, %edx
#APP
# 6778 "../valgrind/herbgrind/../include/valgrind.h" 1
	rolq $3,  %rdi ; rolq $13, %rdi
	rolq $61, %rdi ; rolq $51, %rdi
	xchgq %rbx,%rbx
# 0 "" 2
#NO_APP
	movq	%rdx, %rax
	movq	%rax, -216(%rbp)
	movq	-216(%rbp), %rax
.LBE2:
	movq	%rax, -184(%rbp)
	.loc 1 6785 0
	movq	-184(%rbp), %rax
	.loc 1 6787 0
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	VALGRIND_PRINTF, .-VALGRIND_PRINTF
	.type	VALGRIND_PRINTF_BACKTRACE, @function
VALGRIND_PRINTF_BACKTRACE:
.LFB1:
	.loc 1 6798 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$168, %rsp
	movq	%rdi, -280(%rbp)
	movq	%rsi, -168(%rbp)
	movq	%rdx, -160(%rbp)
	movq	%rcx, -152(%rbp)
	movq	%r8, -144(%rbp)
	movq	%r9, -136(%rbp)
	testb	%al, %al
	je	.L8
	movaps	%xmm0, -128(%rbp)
	movaps	%xmm1, -112(%rbp)
	movaps	%xmm2, -96(%rbp)
	movaps	%xmm3, -80(%rbp)
	movaps	%xmm4, -64(%rbp)
	movaps	%xmm5, -48(%rbp)
	movaps	%xmm6, -32(%rbp)
	movaps	%xmm7, -16(%rbp)
.L8:
	.loc 1 6808 0
	movl	$8, -208(%rbp)
	movl	$48, -204(%rbp)
	leaq	16(%rbp), %rax
	movq	%rax, -200(%rbp)
	leaq	-176(%rbp), %rax
	movq	%rax, -192(%rbp)
.LBB3:
	.loc 1 6816 0
	movq	$5124, -272(%rbp)
	movq	-280(%rbp), %rax
	movq	%rax, -264(%rbp)
	leaq	-208(%rbp), %rax
	movq	%rax, -256(%rbp)
	movq	$0, -248(%rbp)
	movq	$0, -240(%rbp)
	movq	$0, -232(%rbp)
	leaq	-272(%rbp), %rax
	movl	$0, %ecx
	movl	%ecx, %edx
#APP
# 6816 "../valgrind/herbgrind/../include/valgrind.h" 1
	rolq $3,  %rdi ; rolq $13, %rdi
	rolq $61, %rdi ; rolq $51, %rdi
	xchgq %rbx,%rbx
# 0 "" 2
#NO_APP
	movq	%rdx, %rax
	movq	%rax, -216(%rbp)
	movq	-216(%rbp), %rax
.LBE3:
	movq	%rax, -184(%rbp)
	.loc 1 6823 0
	movq	-184(%rbp), %rax
	.loc 1 6825 0
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1:
	.size	VALGRIND_PRINTF_BACKTRACE, .-VALGRIND_PRINTF_BACKTRACE
	.section	.rodata
.LC2:
	.string	"%e\n"
	.text
	.globl	main
	.type	main, @function
main:
.LFB2:
	.file 2 "mini.c"
	.loc 2 5 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$176, %rsp
	movl	%edi, -148(%rbp)
	movq	%rsi, -160(%rbp)
.LBB4:
.LBB5:
	.loc 2 7 0
	movq	$0, -144(%rbp)
	movq	$0, -136(%rbp)
	movq	$0, -128(%rbp)
	movq	$0, -120(%rbp)
	movq	$0, -112(%rbp)
	movq	$0, -104(%rbp)
	leaq	-144(%rbp), %rax
	movl	$0, %ecx
	movl	%ecx, %edx
#APP
# 7 "mini.c" 1
	rolq $3,  %rdi ; rolq $13, %rdi
	rolq $61, %rdi ; rolq $51, %rdi
	xchgq %rbx,%rbx
# 0 "" 2
#NO_APP
.LBE5:
.LBE4:
	.loc 2 8 0
	movsd	.LC0(%rip), %xmm1
	.loc 2 9 0
	movsd	.LC1(%rip), %xmm0
	addsd	%xmm1, %xmm0
	sqrtsd	%xmm0, %xmm0
	movq	%xmm0, -24(%rbp)
.LBB6:
.LBB7:
	.loc 2 10 0
	movq	$1, -96(%rbp)
	movq	$0, -88(%rbp)
	movq	$0, -80(%rbp)
	movq	$0, -72(%rbp)
	movq	$0, -64(%rbp)
	movq	$0, -56(%rbp)
	leaq	-96(%rbp), %rax
	movl	$0, %esi
	movl	%esi, %edx
#APP
# 10 "mini.c" 1
	rolq $3,  %rdi ; rolq $13, %rdi
	rolq $61, %rdi ; rolq $51, %rdi
	xchgq %rbx,%rbx
# 0 "" 2
#NO_APP
	movq	%rdx, %rax
	movq	%rax, -48(%rbp)
	movq	-48(%rbp), %rax
.LBE7:
	movq	%rax, -32(%rbp)
.LBE6:
	.loc 2 11 0
	movq	-24(%rbp), %rax
	movq	%rax, -168(%rbp)
	movsd	-168(%rbp), %xmm0
	movl	$.LC2, %edi
	movl	$1, %eax
	call	printf
	.loc 2 12 0
	movl	$0, %eax
	.loc 2 13 0
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2:
	.size	main, .-main
	.section	.rodata
	.align 8
.LC0:
	.long	0
	.long	1074266112
	.align 8
.LC1:
	.long	0
	.long	1072693248
	.text
.Letext0:
	.file 3 "<built-in>"
	.file 4 "/usr/lib/gcc/x86_64-unknown-linux-gnu/5.2.0/include/stdarg.h"
	.file 5 "../valgrind/herbgrind/herbgrind.h"
	.section	.debug_info,"",@progbits
.Ldebug_info0:
	.long	0x422
	.value	0x4
	.long	.Ldebug_abbrev0
	.byte	0x8
	.uleb128 0x1
	.long	.LASF58
	.byte	0xc
	.long	.LASF59
	.long	.LASF60
	.quad	.Ltext0
	.quad	.Letext0-.Ltext0
	.long	.Ldebug_line0
	.uleb128 0x2
	.byte	0x8
	.byte	0x7
	.long	.LASF0
	.uleb128 0x2
	.byte	0x1
	.byte	0x8
	.long	.LASF1
	.uleb128 0x2
	.byte	0x2
	.byte	0x7
	.long	.LASF2
	.uleb128 0x2
	.byte	0x4
	.byte	0x7
	.long	.LASF3
	.uleb128 0x2
	.byte	0x1
	.byte	0x6
	.long	.LASF4
	.uleb128 0x2
	.byte	0x2
	.byte	0x5
	.long	.LASF5
	.uleb128 0x3
	.byte	0x4
	.byte	0x5
	.string	"int"
	.uleb128 0x2
	.byte	0x8
	.byte	0x5
	.long	.LASF6
	.uleb128 0x2
	.byte	0x8
	.byte	0x7
	.long	.LASF7
	.uleb128 0x4
	.byte	0x8
	.uleb128 0x5
	.byte	0x8
	.long	0x74
	.uleb128 0x2
	.byte	0x1
	.byte	0x6
	.long	.LASF8
	.uleb128 0x6
	.long	.LASF15
	.byte	0x4
	.byte	0x28
	.long	0x86
	.uleb128 0x7
	.long	0x96
	.long	0x96
	.uleb128 0x8
	.long	0x65
	.byte	0
	.byte	0
	.uleb128 0x9
	.long	.LASF61
	.byte	0x18
	.byte	0x3
	.byte	0
	.long	0xd3
	.uleb128 0xa
	.long	.LASF9
	.byte	0x3
	.byte	0
	.long	0x42
	.byte	0
	.uleb128 0xa
	.long	.LASF10
	.byte	0x3
	.byte	0
	.long	0x42
	.byte	0x4
	.uleb128 0xa
	.long	.LASF11
	.byte	0x3
	.byte	0
	.long	0x6c
	.byte	0x8
	.uleb128 0xa
	.long	.LASF12
	.byte	0x3
	.byte	0
	.long	0x6c
	.byte	0x10
	.byte	0
	.uleb128 0x5
	.byte	0x8
	.long	0xd9
	.uleb128 0xb
	.long	0x74
	.uleb128 0x2
	.byte	0x4
	.byte	0x4
	.long	.LASF13
	.uleb128 0x2
	.byte	0x8
	.byte	0x4
	.long	.LASF14
	.uleb128 0x6
	.long	.LASF16
	.byte	0x4
	.byte	0x62
	.long	0x7b
	.uleb128 0xc
	.byte	0x4
	.long	0x42
	.byte	0x1
	.value	0x19f8
	.long	0x1d7
	.uleb128 0xd
	.long	.LASF17
	.value	0x1001
	.uleb128 0xd
	.long	.LASF18
	.value	0x1002
	.uleb128 0xd
	.long	.LASF19
	.value	0x1101
	.uleb128 0xd
	.long	.LASF20
	.value	0x1102
	.uleb128 0xd
	.long	.LASF21
	.value	0x1103
	.uleb128 0xd
	.long	.LASF22
	.value	0x1104
	.uleb128 0xd
	.long	.LASF23
	.value	0x1201
	.uleb128 0xd
	.long	.LASF24
	.value	0x1202
	.uleb128 0xd
	.long	.LASF25
	.value	0x1301
	.uleb128 0xd
	.long	.LASF26
	.value	0x130b
	.uleb128 0xd
	.long	.LASF27
	.value	0x1302
	.uleb128 0xd
	.long	.LASF28
	.value	0x1303
	.uleb128 0xd
	.long	.LASF29
	.value	0x1304
	.uleb128 0xd
	.long	.LASF30
	.value	0x1305
	.uleb128 0xd
	.long	.LASF31
	.value	0x1306
	.uleb128 0xd
	.long	.LASF32
	.value	0x1307
	.uleb128 0xd
	.long	.LASF33
	.value	0x1308
	.uleb128 0xd
	.long	.LASF34
	.value	0x1309
	.uleb128 0xd
	.long	.LASF35
	.value	0x130a
	.uleb128 0xd
	.long	.LASF36
	.value	0x1401
	.uleb128 0xd
	.long	.LASF37
	.value	0x1402
	.uleb128 0xd
	.long	.LASF38
	.value	0x1403
	.uleb128 0xd
	.long	.LASF39
	.value	0x1404
	.uleb128 0xd
	.long	.LASF40
	.value	0x1501
	.uleb128 0xd
	.long	.LASF41
	.value	0x1502
	.uleb128 0xd
	.long	.LASF42
	.value	0x1503
	.uleb128 0xd
	.long	.LASF43
	.value	0x1601
	.uleb128 0xd
	.long	.LASF44
	.value	0x1701
	.uleb128 0xd
	.long	.LASF45
	.value	0x1801
	.uleb128 0xd
	.long	.LASF46
	.value	0x1901
	.byte	0
	.uleb128 0xe
	.byte	0x4
	.long	0x42
	.byte	0x5
	.byte	0x7
	.long	0x1f0
	.uleb128 0xf
	.long	.LASF47
	.byte	0
	.uleb128 0xf
	.long	.LASF48
	.byte	0x1
	.byte	0
	.uleb128 0x10
	.long	.LASF53
	.byte	0x1
	.value	0x1a67
	.long	0x57
	.quad	.LFB0
	.quad	.LFE0-.LFB0
	.uleb128 0x1
	.byte	0x9c
	.long	0x276
	.uleb128 0x11
	.long	.LASF55
	.byte	0x1
	.value	0x1a67
	.long	0xd3
	.uleb128 0x3
	.byte	0x91
	.sleb128 -296
	.uleb128 0x12
	.uleb128 0x13
	.long	.LASF49
	.byte	0x1
	.value	0x1a6f
	.long	0x2d
	.uleb128 0x3
	.byte	0x91
	.sleb128 -200
	.uleb128 0x13
	.long	.LASF50
	.byte	0x1
	.value	0x1a71
	.long	0xec
	.uleb128 0x3
	.byte	0x91
	.sleb128 -224
	.uleb128 0x14
	.quad	.LBB2
	.quad	.LBE2-.LBB2
	.uleb128 0x13
	.long	.LASF51
	.byte	0x1
	.value	0x1a7a
	.long	0x286
	.uleb128 0x3
	.byte	0x91
	.sleb128 -288
	.uleb128 0x13
	.long	.LASF52
	.byte	0x1
	.value	0x1a7a
	.long	0x28b
	.uleb128 0x3
	.byte	0x91
	.sleb128 -232
	.byte	0
	.byte	0
	.uleb128 0x7
	.long	0x2d
	.long	0x286
	.uleb128 0x8
	.long	0x65
	.byte	0x5
	.byte	0
	.uleb128 0x15
	.long	0x276
	.uleb128 0x15
	.long	0x2d
	.uleb128 0x10
	.long	.LASF54
	.byte	0x1
	.value	0x1a8d
	.long	0x57
	.quad	.LFB1
	.quad	.LFE1-.LFB1
	.uleb128 0x1
	.byte	0x9c
	.long	0x316
	.uleb128 0x11
	.long	.LASF55
	.byte	0x1
	.value	0x1a8d
	.long	0xd3
	.uleb128 0x3
	.byte	0x91
	.sleb128 -296
	.uleb128 0x12
	.uleb128 0x13
	.long	.LASF49
	.byte	0x1
	.value	0x1a95
	.long	0x2d
	.uleb128 0x3
	.byte	0x91
	.sleb128 -200
	.uleb128 0x13
	.long	.LASF50
	.byte	0x1
	.value	0x1a97
	.long	0xec
	.uleb128 0x3
	.byte	0x91
	.sleb128 -224
	.uleb128 0x14
	.quad	.LBB3
	.quad	.LBE3-.LBB3
	.uleb128 0x13
	.long	.LASF51
	.byte	0x1
	.value	0x1aa0
	.long	0x316
	.uleb128 0x3
	.byte	0x91
	.sleb128 -288
	.uleb128 0x13
	.long	.LASF52
	.byte	0x1
	.value	0x1aa0
	.long	0x28b
	.uleb128 0x3
	.byte	0x91
	.sleb128 -232
	.byte	0
	.byte	0
	.uleb128 0x15
	.long	0x276
	.uleb128 0x16
	.long	.LASF62
	.byte	0x2
	.byte	0x5
	.long	0x57
	.quad	.LFB2
	.quad	.LFE2-.LFB2
	.uleb128 0x1
	.byte	0x9c
	.long	0x415
	.uleb128 0x17
	.long	.LASF56
	.byte	0x2
	.byte	0x5
	.long	0x57
	.uleb128 0x3
	.byte	0x91
	.sleb128 -164
	.uleb128 0x17
	.long	.LASF57
	.byte	0x2
	.byte	0x5
	.long	0x415
	.uleb128 0x3
	.byte	0x91
	.sleb128 -176
	.uleb128 0x18
	.string	"x"
	.byte	0x2
	.byte	0x6
	.long	0xe5
	.uleb128 0x2
	.byte	0x91
	.sleb128 -32
	.uleb128 0x18
	.string	"y"
	.byte	0x2
	.byte	0x6
	.long	0xe5
	.uleb128 0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0x19
	.quad	.LBB4
	.quad	.LBE4-.LBB4
	.long	0x3c5
	.uleb128 0x1a
	.long	.LASF49
	.byte	0x2
	.byte	0x7
	.long	0x2d
	.uleb128 0x2
	.byte	0x91
	.sleb128 -24
	.uleb128 0x14
	.quad	.LBB5
	.quad	.LBE5-.LBB5
	.uleb128 0x1a
	.long	.LASF51
	.byte	0x2
	.byte	0x7
	.long	0x41b
	.uleb128 0x3
	.byte	0x91
	.sleb128 -160
	.uleb128 0x1a
	.long	.LASF52
	.byte	0x2
	.byte	0x7
	.long	0x28b
	.uleb128 0x2
	.byte	0x91
	.sleb128 -56
	.byte	0
	.byte	0
	.uleb128 0x14
	.quad	.LBB6
	.quad	.LBE6-.LBB6
	.uleb128 0x1a
	.long	.LASF49
	.byte	0x2
	.byte	0xa
	.long	0x2d
	.uleb128 0x2
	.byte	0x91
	.sleb128 -48
	.uleb128 0x14
	.quad	.LBB7
	.quad	.LBE7-.LBB7
	.uleb128 0x1a
	.long	.LASF51
	.byte	0x2
	.byte	0xa
	.long	0x420
	.uleb128 0x3
	.byte	0x91
	.sleb128 -112
	.uleb128 0x1a
	.long	.LASF52
	.byte	0x2
	.byte	0xa
	.long	0x28b
	.uleb128 0x2
	.byte	0x91
	.sleb128 -64
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0x5
	.byte	0x8
	.long	0x6e
	.uleb128 0x15
	.long	0x276
	.uleb128 0x15
	.long	0x276
	.byte	0
	.section	.debug_abbrev,"",@progbits
.Ldebug_abbrev0:
	.uleb128 0x1
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x25
	.uleb128 0xe
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1b
	.uleb128 0xe
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x10
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0x2
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.byte	0
	.byte	0
	.uleb128 0x3
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x8
	.byte	0
	.byte	0
	.uleb128 0x4
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x5
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x6
	.uleb128 0x16
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x7
	.uleb128 0x1
	.byte	0x1
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x8
	.uleb128 0x21
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2f
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x9
	.uleb128 0x13
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xa
	.uleb128 0xd
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0xb
	.uleb128 0x26
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xc
	.uleb128 0x4
	.byte	0x1
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xd
	.uleb128 0x28
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1c
	.uleb128 0x5
	.byte	0
	.byte	0
	.uleb128 0xe
	.uleb128 0x4
	.byte	0x1
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xf
	.uleb128 0x28
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1c
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x10
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x2117
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x11
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x12
	.uleb128 0x18
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0x13
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x14
	.uleb128 0xb
	.byte	0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x7
	.byte	0
	.byte	0
	.uleb128 0x15
	.uleb128 0x35
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x16
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x2116
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x17
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x18
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x19
	.uleb128 0xb
	.byte	0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x1a
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_aranges,"",@progbits
	.long	0x2c
	.value	0x2
	.long	.Ldebug_info0
	.byte	0x8
	.byte	0
	.value	0
	.value	0
	.quad	.Ltext0
	.quad	.Letext0-.Ltext0
	.quad	0
	.quad	0
	.section	.debug_line,"",@progbits
.Ldebug_line0:
	.section	.debug_str,"MS",@progbits,1
.LASF9:
	.string	"gp_offset"
.LASF16:
	.string	"va_list"
.LASF54:
	.string	"VALGRIND_PRINTF_BACKTRACE"
.LASF28:
	.string	"VG_USERREQ__CREATE_MEMPOOL"
.LASF40:
	.string	"VG_USERREQ__STACK_REGISTER"
.LASF4:
	.string	"signed char"
.LASF58:
	.string	"GNU C11 5.2.0 -mtune=generic -march=x86-64 -g -std=c11"
.LASF36:
	.string	"VG_USERREQ__PRINTF"
.LASF27:
	.string	"VG_USERREQ__FREELIKE_BLOCK"
.LASF19:
	.string	"VG_USERREQ__CLIENT_CALL0"
.LASF21:
	.string	"VG_USERREQ__CLIENT_CALL2"
.LASF6:
	.string	"long int"
.LASF14:
	.string	"double"
.LASF3:
	.string	"unsigned int"
.LASF24:
	.string	"VG_USERREQ__GDB_MONITOR_COMMAND"
.LASF11:
	.string	"overflow_arg_area"
.LASF20:
	.string	"VG_USERREQ__CLIENT_CALL1"
.LASF0:
	.string	"long unsigned int"
.LASF22:
	.string	"VG_USERREQ__CLIENT_CALL3"
.LASF34:
	.string	"VG_USERREQ__MEMPOOL_CHANGE"
.LASF2:
	.string	"short unsigned int"
.LASF31:
	.string	"VG_USERREQ__MEMPOOL_FREE"
.LASF32:
	.string	"VG_USERREQ__MEMPOOL_TRIM"
.LASF50:
	.string	"vargs"
.LASF30:
	.string	"VG_USERREQ__MEMPOOL_ALLOC"
.LASF49:
	.string	"_qzz_res"
.LASF42:
	.string	"VG_USERREQ__STACK_CHANGE"
.LASF46:
	.string	"VG_USERREQ__VEX_INIT_FOR_IRI"
.LASF51:
	.string	"_zzq_args"
.LASF43:
	.string	"VG_USERREQ__LOAD_PDB_DEBUGINFO"
.LASF7:
	.string	"sizetype"
.LASF39:
	.string	"VG_USERREQ__PRINTF_BACKTRACE_VALIST_BY_REF"
.LASF15:
	.string	"__gnuc_va_list"
.LASF44:
	.string	"VG_USERREQ__MAP_IP_TO_SRCLOC"
.LASF45:
	.string	"VG_USERREQ__CHANGE_ERR_DISABLEMENT"
.LASF37:
	.string	"VG_USERREQ__PRINTF_BACKTRACE"
.LASF13:
	.string	"float"
.LASF29:
	.string	"VG_USERREQ__DESTROY_MEMPOOL"
.LASF33:
	.string	"VG_USERREQ__MOVE_MEMPOOL"
.LASF52:
	.string	"_zzq_result"
.LASF1:
	.string	"unsigned char"
.LASF47:
	.string	"VG_USERREQ__BEGIN"
.LASF5:
	.string	"short int"
.LASF12:
	.string	"reg_save_area"
.LASF55:
	.string	"format"
.LASF53:
	.string	"VALGRIND_PRINTF"
.LASF8:
	.string	"char"
.LASF18:
	.string	"VG_USERREQ__DISCARD_TRANSLATIONS"
.LASF59:
	.string	"mini.c"
.LASF23:
	.string	"VG_USERREQ__COUNT_ERRORS"
.LASF25:
	.string	"VG_USERREQ__MALLOCLIKE_BLOCK"
.LASF60:
	.string	"/home/alex/herbie-all/herbgrind/bench"
.LASF17:
	.string	"VG_USERREQ__RUNNING_ON_VALGRIND"
.LASF57:
	.string	"argv"
.LASF38:
	.string	"VG_USERREQ__PRINTF_VALIST_BY_REF"
.LASF10:
	.string	"fp_offset"
.LASF41:
	.string	"VG_USERREQ__STACK_DEREGISTER"
.LASF35:
	.string	"VG_USERREQ__MEMPOOL_EXISTS"
.LASF56:
	.string	"argc"
.LASF62:
	.string	"main"
.LASF48:
	.string	"VG_USERREQ__END"
.LASF26:
	.string	"VG_USERREQ__RESIZEINPLACE_BLOCK"
.LASF61:
	.string	"__va_list_tag"
	.ident	"GCC: (GNU) 5.2.0"
	.section	.note.GNU-stack,"",@progbits
