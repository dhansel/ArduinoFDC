typedef unsigned char byte;

//static byte read_data_gcr(byte bitlen, byte *buffer, unsigned int n, byte verify)
static byte read_data_gcr(byte bitlen, byte *buffer, unsigned int n, byte verify, byte mode)
// mode: 0  = read after header search  1: read nibble (no search), 2: flux
{
  // Parameter mode:
  // 1: read mibbles without trigger
  // 2: read flux
  // 3  read nibbles for header
  // 4  read ribbles for data

  // iProblem: modes 3 and 4 hang if no header found, e.g.when head positioned beyond track 35
  
  // verifay not used any more

	byte status;
#ifndef readpulse
   asm volatile 
    (
     // define READPULSE macro (wait for pulse)
     // macro arguments: 
     //         length: none => just wait for pulse, don't check         ( 9 cycles)
     //                 1    => wait for pulse and jump if NOT short  (12/14 cycles)
     //                 2    => wait for pulse and jump if NOT medium (14/16 cycles)
     //                 3    => wait for pulse and jump if NOT long   (12/14 cycles)
     //         dst:    label to jump to if DIFFERENT pulse found
     // 
     // on entry: r16 contains minimum length of medium pulse
     //           r17 contains minimum length of long   pulse
     //           r18 contains time of previous pulse
     // on exit:  r18 is updated to the time of this pulse
     //           r22 contains the pulse length in timer ticks (=processor cycles)     
     // CLOBBERS: r19
     ".macro READGCR length=0,dst=undefined\n"
     "        sbis    TIFR, ICF\n"     // (1/2) skip next instruction if timer input capture seen
     "        rjmp    .-4\n"           // (2)   wait more 
     "        lds     r19, ICRL\n"     // (2)   get time of input capture (ICR1L, lower 8 bits only)
     "        sbi     TIFR, ICF\n "    // (2)   clear input capture flag
     "        mov     r22, r19\n"      // (1)   calculate time since previous capture...
     "        sub     r22, r18\n"      // (1)   ...into r22
     "        mov     r18, r19\n"      // (1)   set r18 to time of current capture
     "  .if \\length == 1\n"           //       waiting for short pule?
     "        cp      r22, r16\n"      // (1)   compare r22 to min medium pulse
     "        brlo   .+2\n"            // (1/2) skip jump if less
     "        jmp   \\dst\n"          // (3)   not the expected pulse => jump to dst
     "  .else \n"
     "    .if \\length == 2\n"         // waiting for medium pulse?
     "        cp      r16, r22\n"      // (1)   min medium pulse < r22? => carry set if so
     "        brcc    .+2\n"           // (1/2) skip next instruction if carry is clear
     "        cp      r22, r17\n"      // (1)   r22 < min long pulse? => carry set if so
     "        brcs   .+2\n"            // (1/2) skip jump if greater
     "        jmp   \\dst\n"          // (3)   not the expected pulse => jump to dst
     "    .else\n"
     "      .if \\length == 3\n" 
     "        cp      r22, r17\n"      // (1)   min long pulse < r22?
     "        brsh   .+2\n"            // (1/2) skip jump if greater
     "        jmp   \\dst\n"          // (3)   not the expected pulse => jump to dst
     "      .endif\n"
     "    .endif\n"
     "  .endif\n"
     ".endm\n"

     // define STOREGCR macro for storing or verifying data bit 
     // storing  data  : 5/14 cycles for "1", 4/13 cycles for "0"
     ".macro STOREGCR data:req,done:req\n"
     "        lsl     r20\n"           // (1)   shift received data
     ".if \\data != 0\n"
     "        ori     r20, 1\n"        // (1)   store "1" bit
     ".endif\n"
     "        dec     r21\n"           // (1)   decrement bit counter
     "        brne    .+10\n"          // (1/2) skip if bit counter >0
     "        st      Z+, r20\n"       // (2)   store received data byte
     "        ldi     r21, 8\n"        // (1)   re-initialize bit counter
     "        subi    r26, 1\n"        // (1)   subtract one from byte counter
     "        sbci    r27, 0\n"        // (1) 
     "        brmi    \\done\n"        // (1/2) done if byte counter<0
     ".endm\n" );
#endif
   
     asm volatile 
    (
     "        push r21\n"   //  this was experimental .... 
     "        push r20\n"   //  save r21/r2o
     "        cli\n"   // disable interrupts

     // prepare for reading nibbles
 //    "        mov         r16, %2\n"   // (1)   r16 = 3 * (MFM bit len) = minimum length of medium pulse
 //    "        lsr         r16\n"       // (1)
 //    "        add         r16, %2\n"   // (1)  r
 //    "        add         r16, %2\n"   // (1)
 //    "        mov         r17, r16\n"  // (1)   r17 = 5 * (MFM bit len) = minimum length of long pulse
 //    "        add         r17, %2\n"   // (1) 
 //    "        add         r17, %2\n"   // (1)
     "        ldi	r16,96\n"   // set fixed limits
     "        ldi       r17,160\n"
 //  "        ldi         %0, 0\n"     // (1)   default return status is S_OK //  why did this work before &0=7
 //  "        mov         r15,%0\n"   // (1)   initialize timer overflow counter
     "        ldi         r19, 0\n"     // (1)   default return status is S_OK // 
     "        mov         r15, r19\n"   // (1)   initialize timer overflow counter
     "        mov         %0,r19\n"   // (1)  default return status is S_OK // 
     "        READGCR     \n"   // preset r18 .etc, . required ?
     "        sbi         TIFR, TOV\n" // (2)   reset timer overflow flag
//
//  select mode
//     "       ldi r19,2\n"
//     "       cp %3,r19 \n" //mode == 2, flux
     "       cpi %3,2 \n" //mode == 2, flux
     "       brne m1\n" 
     "       rjmp floop\n" 
//
     "m1:    cpi %3,1 \n" //mode == 1, no sync, , leave somthing in status, ?
     "       brne gsync\n"  // rest is mode 0,3 ... sync
     "       rjmp grdo\n"  // 
// expect remaining part of first sync mark (..0011111111 1101 0101 10101010 10010110 ff d5 aa 96 )
     "gsync:  READGCR 3,gsync\n"     // (12) expect  001 so gehts 
     "bffst:  ldi r21,7\n"   // start 
     "bff:    READGCR 1,gsync\n"     // (14) expect 1 
     "        dec r21\n"
     "        brne bff \n"
     "        READGCR   \n"     // 1101 wait for 1st bit of d5
     "        cp      r17, r22\n"      // (1)   pulse length >= 3 bits
     "        brlo    bffst\n"          // 001 found, go back waiting for 1111111
     "        READGCR   1,gsync\n"     //  2nd
     "        ldi   r21,3\n"     //  
     "b3x01:  READGCR   2,gsync\n"     // 01 0101   3x 01
     "        dec r21\n"
     "        brne b3x01 \n"
     "        READGCR   1,gsync\n"     // 10101010 1
     "        ldi   r21,4\n"     // 4 x 01 01 01 01 
     "b4x01:  READGCR   2,gsync\n"     //  
     "        dec r21\n"
     "        brne b4x01 \n"
     "        cpi %3,3 \n" //mode == 4, data with praeamble
     "        brne mo4\n" 
     "mo3:    READGCR   3,gsync\n"     // 0010110 
     "        READGCR   2,gsync\n"     // 0110 
     "        READGCR   1,gsync\n"     // 10  ...  one bit bissing
     "        rjmp      bit1\n"
     "mo4:    READGCR   2,gsync\n"     // 1/0101101  d5 aa ad   data preamble
     "        READGCR   2,gsync\n"     // 01101
     "        READGCR   1,gsync\n"     // 101 
     "        READGCR   2,gsync\n"     // 01
     "bit1:   READGCR   \n"     // preload 1st bit of data, 0-bit in between preamble and data
     "        ldi     r21, 8\n"        // init bit counter with 1 prestored bit
     "        STOREGCR 1,grdone\n"      // (5/14) store "1" bit
     "        rjmp  grdo\n"        // (1)   store "1" bit
     "bc:     ldi     r21, 8\n"        // (1)   initialize bit counter (8 bits per byte)
     // nibble read loop
     "grdo:   READGCR\n"             // (9)   wait for pulse
     "        cp      r22, r16\n"      // (1)   pulse length > 2 bits
     "        brlo    grone\n"          // (1/2) jump if not
     "        cp      r22, r17\n"      // (1)   pulse length >= 3 bits
     "        brlo    grtwo\n"          // (1/2) jump if not
     //  001-Pulse, or longer ..
     "        STOREGCR 0,grdone\n"      // (5/14) store "1" bit
     "        STOREGCR 0,grdone\n"      // (5/14) store "1" bit
     "        STOREGCR 1,grdone\n"      // (5/14) store "1" bit
     "        rjmp    grdo\n"            // (2)    back to start (still odd)

     // jump target for relative conditional jumps in STOREGCR macro
     "grdone:  rjmp    grend\n"
     
     // 2 bit pulse
     "grtwo:  STOREGCR 0,grdone\n"      // 2 zeros
     "        STOREGCR 1,grdone\n"
     "        rjmp    grdo\n"            // (2)   back to start (now even)

     // short pulse (01) => read "1", still odd
     "grone:  STOREGCR 1,grdone\n"      // 1 zero 
     "        rjmp    grdo\n"            // (2)    back to start (still odd)
     
     // loop for reading flux data
     "floop:    READGCR\n"         // (9)   wait for pulse
     "        st      Z+, r22\n"       // (2)   store received data byte
     "        subi    r26, 1\n"        // (1)   subtract one from byte counter
     "        sbci    r27, 0\n"        // (1)
     "        brmi    grend\n"        // (1/2) done if byte counter<0
     "        rjmp    floop\n"
     //
     "grend:  pop r20       \n"   // restore r20,21 (test)
     "        pop r21       \n"   // 
     "        sei       \n"   //enable interrups
     : "=r"(status)                         // outputs
     : "r"(verify), "r"(bitlen), "r"(mode),"x"(n-1), "z"(buffer)   // inputs  (x=r26/r27, z=r30/r31)
     :  "r15", "r16", "r17", "r18", "r19", "r20", "r21", "r22");  // clobbers
    //}
     
  return status;
}

