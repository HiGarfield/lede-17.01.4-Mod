;  n2e_d.ash -- NRV2E decompression in 68000 assembly
;
;  This file is part of the UCL data compression library.
;
;  Copyright (C) 1996-2004 Markus Franz Xaver Johannes Oberhumer
;  All Rights Reserved.
;
;  The UCL library is free software; you can redistribute it and/or
;  modify it under the terms of the GNU General Public License as
;  published by the Free Software Foundation; either version 2 of
;  the License, or (at your option) any later version.
;
;  The UCL library is distributed in the hope that it will be useful,
;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;  GNU General Public License for more details.
;
;  You should have received a copy of the GNU General Public License
;  along with the UCL library; see the file COPYING.
;  If not, write to the Free Software Foundation, Inc.,
;  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
;
;  Markus F.X.J. Oberhumer
;  <markus@oberhumer.com>
;  http://www.oberhumer.com/opensource/ucl/
;


; ------------- DECOMPRESSION -------------

; decompress from a0 to a1
;   note: must preserve d4 and a5-a7

;
; On entry:
;   a0  src pointer
;   a1  dest pointer
;
; On exit:
;   d1.l = 0x00008000
;   d2.l = 0
;
; Register usage:
;   a3  m_pos
;
;   d0  bit buffer
;   d1  m_off
;   d2  m_len
;   d5  last_m_off
;
;   d6  constant: -$500
;   d7  constant: 0
;
;
; Notes:
;   we have max_match = 65535, so we can use word arithmetics on d2
;   we have max_offset < 2**23, so we can use partial word arithmetics on d1
;


; ------------- constants & macros -------------

#if !defined(NRV_NO_INIT)

                ;;move.l  #-$500,d6             ; 0xfffffb00
                moveq.l #-$50,d6                ;   0xffffffb0
                lsl.w   #4,d6                   ;   << 4

                moveq.l #0,d7
                moveq.l #-1,d5                  ; last_off = -1

                ; init d0 with high bit set
#if (NRV_BB == 8)
                ;;move.b  #$80,d0                 ; init d0.b for FILLBYTES
                moveq.l #-128,d0                ; d0.b = $80
#elif (NRV_BB == 32)
                ;;move.l  #$80000000,d0           ; init d0.l for FILLBYTES
                moveq.l #1,d0
                ror.l   #1,d0                   ; d0.l = $80000000
#endif
                bra     decompr_start

#endif


#include "bits.ash"


#if defined(FILLBYTES_SR)
fillbytes_sr:   FILLBYTES_SR
                rts                                             ; 16
#endif



; ------------- DECOMPRESSION -------------


decompr_literal:
                move.b  (a0)+,(a1)+

decompr_start:
decompr_loop:
#ifdef SMALL
        ;   cost literal:   4 + 10 + 10
        ;   cost match:     4 + 10 +  8
        ;   cost fillbits:  4 +  8
                GETBIT
                bcs     decompr_literal
#else
        ; optimization: carry is clear -> we know that bits are available
        ;   cost literal:   4 +  8 + 10
        ;   cost match:     4 + 10
        ;   cost fillbits:  4 +  8 +  8
                ADDBITS
                bcc     decompr_match
                bne     decompr_literal
                FILLBITS
                bcs     decompr_literal
#endif



decompr_match:
                moveq.l #1,d1
                moveq.l #0,d2
decompr_l1:
                GETBIT
                addx.w  d1,d1
#ifdef SMALL
        ;   cost loop continue:  4 + 10 +  8
        ;   cost loop break:     4 + 10 + 10
        ;   cost fillbits:       4 +  8
                GETBIT
                bcs     decompr_break1
#else
        ; optimization: carry is clear -> we know that bits are available
        ;   cost loop continue:  4 + 10
        ;   cost loop break:     4 +  8 + 10
        ;   cost fillbits:       4 +  8 +  8
                ADDBITS
                bcc     L(continue)
                bne     decompr_break1
                FILLBITS
                bcs     decompr_break1
L(continue):
#endif
                subq.w  #1,d1
                GETBIT
                addx.w  d1,d1
                bpl     decompr_l1
                bra     decompr_end
decompr_break1:
                subq.w  #3,d1
                bcs     decompr_prev_dist       ; last m_off
                lsl.l   #8,d1
                move.b  (a0)+,d1
                not.l   d1
                asr.l   #1,d1
                bcc     decompr_get_mlen2

decompr_get_mlen1:
                GETBIT
                addx.w  d2,d2
                bra     decompr_got_mlen
decompr_prev_dist:
                move.l  d5,d1
                GETBIT
                bcs     decompr_get_mlen1
decompr_get_mlen2:
                addq.w  #1,d2
                GETBIT
                bcs     decompr_get_mlen1

decompr_l2:     GETBIT
                addx.w  d2,d2
#ifdef SMALL
        ;   cost loop continue:  4 + 10 + 10
        ;   cost loop break:     4 + 10 +  8
        ;   cost fillbits:       4 +  8
                GETBIT
                bcc     decompr_l2
#else
        ; optimization: carry is clear -> we know that bits are available
        ;   cost loop continue:  4 + 10
        ;   cost loop break:     4 +  8 + 10
        ;   cost fillbits:       4 +  8 +  8
                ADDBITS
                bcc     decompr_l2
                bne     L(break)
                FILLBITS
                bcc     decompr_l2
L(break):
#endif
                addq.w  #2,d2



decompr_got_mlen:
                move.l  d1,d5
                lea     0(a1,d1.l),a3

                ; must use sub as cmp doesn't affect the X flag
                sub.l   d6,d1
                addx.w  d7,d2

; TODO: partly unroll this loop; could use some magic with d7 for address
;       computations, then compute a nice `jmp yyy(pc,dx.w)'

#if 1
        ;   cost for any m_len:   12 + 22 * (m_len - 1) + 4
        ;     38, 60, 82, 104, 126, 148, 170, 192, 214, 236
                move.b  (a3)+,(a1)+                             ; 12
L(copy):        move.b  (a3)+,(a1)+                             ; 12
                dbra    d2,L(copy)                              ; 10 / 14
#else
        ;   cost for even m_len:  18 + 34 * (m_len / 2) + 4
        ;   cost for odd m_len:   28 + 34 * (m_len / 2) + 4
        ;     56, 66, 90, 100, 124, 134, 158, 168, 192, 202
                lsr.w   #1,d2                                   ;  8
                bcc     L(copy)                                 ; 10 /  8
                move.b  (a3)+,(a1)+                             ; 12
L(copy):        move.b  (a3)+,(a1)+                             ; 12
                move.b  (a3)+,(a1)+                             ; 12
                dbra    d2,L(copy)                              ; 10 / 14
#endif

                bra     decompr_loop



decompr_end:


; vi:ts=8:et

