/*
            Copyright Oliver Kowalke 2009.
   Distributed under the Boost Software License, Version 1.0.
      (See accompanying file LICENSE_1_0.txt or copy at
          http://www.boost.org/LICENSE_1_0.txt)
*/

/*******************************************************
 *                                                     *
 *  -------------------------------------------------  *
 *  |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  *
 *  -------------------------------------------------  *
 *  |  0  |  4  |  8  |  12 |  16 |  20 |  24 |  28 |  *
 *  -------------------------------------------------  *
 *  |    TOC    |    R14    |    R15    |    R16    |  *
 *  -------------------------------------------------  *
 *  -------------------------------------------------  *
 *  |  8  |  9  |  10 |  11 |  12 |  13 |  14 |  15 |  *
 *  -------------------------------------------------  *
 *  |  32 |  36 |  40 |  44 |  48 |  52 |  56 |  60 |  *
 *  -------------------------------------------------  *
 *  |    R17    |    R18    |     R19   |    R20    |  *
 *  -------------------------------------------------  *
 *  -------------------------------------------------  *
 *  |  16 |  17 |  18 |  19 |  20 |  21 |  22 |  23 |  *
 *  -------------------------------------------------  *
 *  |  64 |  68 |  72 |  76 |  80 |  84 |  88 |  92 |  *
 *  -------------------------------------------------  *
 *  |    R21    |    R22    |    R23    |    R24    |  *
 *  -------------------------------------------------  *
 *  -------------------------------------------------  *
 *  |  24 |  25 |  26 |  27 |  28 |  29 |  30 |  31 |  *
 *  -------------------------------------------------  *
 *  |  96 | 100 | 104 | 108 | 112 | 116 | 120 | 124 |  *
 *  -------------------------------------------------  *
 *  |    R25    |    R26    |    R27    |    R28    |  *
 *  -------------------------------------------------  *
 *  -------------------------------------------------  *
 *  |  32 |  33 |  34 |  35 |  36 |  37 |  38 |  39 |  *
 *  -------------------------------------------------  *
 *  | 128 | 132 | 136 | 140 | 144 | 148 | 152 | 156 |  *
 *  -------------------------------------------------  *
 *  |    R29    |    R30    |    R31    |   hidden  |  *
 *  -------------------------------------------------  *
 *  -------------------------------------------------  *
 *  |  40 |  41 |  42 |  43 |  44 |  45 |  46 |  47 |  *
 *  -------------------------------------------------  *
 *  | 160 | 164 | 168 | 172 | 176 | 180 | 184 | 188 |  *
 *  -------------------------------------------------  *
 *  |     CR    |     LR    |     PC    | back-chain|  *
 *  -------------------------------------------------  *
 *  -------------------------------------------------  *
 *  |  48 |  49 |  50 |  51 |  52 |  53 |  54 |  55 |  *
 *  -------------------------------------------------  *
 *  | 192 | 196 | 200 | 204 | 208 | 212 | 216 | 220 |  *
 *  -------------------------------------------------  *
 *  |  cr saved |  lr saved |  compiler |   linker  |  *
 *  -------------------------------------------------  *
 *  -------------------------------------------------  *
 *  |  56 |  57 |  58 |  59 |  60 |  61 |  62 |  63 |  *
 *  -------------------------------------------------  *
 *  | 224 | 228 | 232 | 236 | 240 | 244 | 248 | 252 |  *
 *  -------------------------------------------------  *
 *  | TOC saved |    FCTX   |    DATA   |           |  *
 *  -------------------------------------------------  *
 *                                                     *
 *******************************************************/

    .file "ontop_ppc64_sysv_xcoff_gas.S"
    .toc
    .csect .text[PR], 5
    .align 2
    .globl  hoost_ontop_fcontext[DS]
    .globl .hoost_ontop_fcontext
    .csect  hoost_ontop_fcontext[DS], 3
hoost_ontop_fcontext:
    .llong .hoost_ontop_fcontext[PR], TOC[tc0], 0
    .csect .text[PR], 5
.hoost_ontop_fcontext:
    # reserve space on stack
    subi  1, 1, 184

    std  2, 0(1)  # save TOC
    std  14, 8(1)  # save R14
    std  15, 16(1)  # save R15
    std  16, 24(1)  # save R16
    std  17, 32(1)  # save R17
    std  18, 40(1)  # save R18
    std  19, 48(1)  # save R19
    std  20, 56(1)  # save R20
    std  21, 64(1)  # save R21
    std  22, 72(1)  # save R22
    std  23, 80(1)  # save R23
    std  24, 88(1)  # save R24
    std  25, 96(1)  # save R25
    std  26, 104(1)  # save R26
    std  27, 112(1)  # save R27
    std  28, 120(1)  # save R28
    std  29, 128(1)  # save R29
    std  30, 136(1)  # save R30
    std  31, 144(1)  # save R31
    std  3,  152(1)  # save hidden

    # save CR
    mfcr  0
    std  0, 160(1)
    # save LR
    mflr  0
    std  0, 168(1)
    # save LR as PC
    std  0, 176(1)

    # store RSP (pointing to context-data) in R7
    mr  7, 1

    # restore RSP (pointing to context-data) from R4
    mr  1, 4

    ld  14, 8(1)  # restore R14
    ld  15, 16(1)  # restore R15
    ld  16, 24(1)  # restore R16
    ld  17, 32(1)  # restore R17
    ld  18, 40(1)  # restore R18
    ld  19, 48(1)  # restore R19
    ld  20, 56(1)  # restore R20
    ld  21, 64(1)  # restore R21
    ld  22, 72(1)  # restore R22
    ld  23, 80(1)  # restore R23
    ld  24, 88(1)  # restore R24
    ld  25, 96(1)  # restore R25
    ld  26, 104(1)  # restore R26
    ld  27, 112(1)  # restore R27
    ld  28, 120(1)  # restore R28
    ld  29, 128(1)  # restore R29
    ld  30, 136(1)  # restore R30
    ld  31, 144(1)  # restore R31
    ld  3,  152(1)  # restore hidden

    # restore CR
    ld  0, 160(1)
    mtcr  0

    # copy transfer_t into ontop_fn arg registers
    mr  4, 7
    # arg pointer already in r5
    # hidden arg already in r3

    # restore CTR
    ld   7, 0(6)
    mtctr  7
    # restore TOC
    ld   2, 8(6)

    # zero in r3 indicates first jump to context-function
    cmpdi  3, 0
    beq  use_entry_arg

return_to_ctx:
    # restore LR
    ld  0, 168(1)
    mtlr  0

    # adjust stack
    addi  1, 1, 184

    # jump to context
    bctr

use_entry_arg:
    # compute return-value struct address
    # (passed has hidden arg to ontop_fn)
    addi  3, 1, 8

    # jump to context and update LR
    bctrl

    # restore CTR
    ld   7, 176(1)
    mtctr  7
    # restore TOC
    ld   2, 0(1)

    # copy returned transfer_t into entry_fn arg registers
    ld  3, 8(1)
    ld  4, 16(1)

    b  return_to_ctx
