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
 *  |    R13    |    R14    |    R15    |    R16    |  *
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
 *  |    FCTX   |    DATA   |           |           |  *
 *  -------------------------------------------------  *
 *                                                     *
 *******************************************************/

.text
.globl _hoost_make_fcontext
_hoost_make_fcontext:
    ; save return address into R6
    mflr  r6

    ; first arg of hoost_make_fcontext() == top address of context-function
    ; shift address in R3 to lower 16 byte boundary
    clrrwi  r3, r3, 4

    ; reserve space for context-data on context-stack
    ; including 64 byte of linkage + parameter area (R1  16 == 0)
    subi  r3, r3, 240

    ; third arg of hoost_make_fcontext() == address of context-function
    stw  r5, 176(r3)

    ; set back-chain to zero
    li   r0, 0
    std  r0, 184(r3)

    ; compute address of returned transfer_t
    addi r0, r3, 224
    mr   r4, r0
    std  r4, 152(r3)

    ; load LR
    mflr  r0
    ; jump to label 1
    bl  l1
l1:
    ; load LR into R4
    mflr  r4
    ; compute abs address of label finish
    addi  r4, r4, lo16((finish - .) + 4)
    ; restore LR
    mtlr  r0
    ; save address of finish as return-address for context-function
    ; will be entered after context-function returns
    std  r4, 168(r3)

    ; restore return address from R6
    mtlr  r6

    blr  ; return pointer to context-data

finish:
    ; save return address into R0
    mflr  r0
    ; save return address on stack, set up stack frame
    stw  r0, 8(r1)
    ; allocate stack space, R1  16 == 0
    stwu  r1, -32(r1)

    ; set return value to zero
    li  r3, 0
    ; exit application
    bl  __exit
    nop
