#include "systemc.h"

#define SIZE 4

#ifdef DSE
    #define ATTR Cyber
    #define ARRAY_1 array=REG
    #define ARRAY_2 array=REG
    #define ARRAY_3 array=REG
    #define LOOP_1 unroll_times=all // 4
    #define LOOP_2 unroll_times=all // 32
    #define LOOP_3 unroll_times=all // 4
    #define LOOP_4 unroll_times=all // 4
    #define FUNC_1 func=inline
    #define FUNC_2 func=inline
    #define FUNC_3 func=inline
    #define FUNC_4 func=inline
    #define FUNC_5 func=inline
    #define FUNC_6 func=inline
    #define FUNC_7 func=inline
    #define FUNC_8 func=inline
#endif

typedef sc_uint<8> u8;
typedef sc_uint<32> u32;
typedef sc_biguint<64> u64;

SC_MODULE (main)
{
    sc_in_clk CLOCK;
    sc_in<bool> rst;

    sc_in<sc_uint<32> >  ks_in[SIZE];

    sc_out<sc_uint<32> >  ks_out[SIZE];

    /* Variables declaration*/
    sc_uint<32> ks[SIZE] /* ATTR ARRAY_3 */;

    /* LFSR */
    u32 LFSR_S0 ;
    u32 LFSR_S1 ;
    u32 LFSR_S2 ;
    u32 LFSR_S3 ;
    u32 LFSR_S4 ;
    u32 LFSR_S5 ;
    u32 LFSR_S6 ;
    u32 LFSR_S7 ;
    u32 LFSR_S8 ;
    u32 LFSR_S9 ;
    u32 LFSR_S10 ;
    u32 LFSR_S11 ;
    u32 LFSR_S12 ;
    u32 LFSR_S13 ;
    u32 LFSR_S14 ;
    u32 LFSR_S15 ;
    /* FSM */
    u32 FSM_R1 ;
    u32 FSM_R2 ;
    u32 FSM_R3 ;

    sc_uint<32> ClockFSM();
    void ClockLFSRInitializationMode(sc_uint<32>);
    void ClockLFSRKeyStreamMode();
    sc_uint<32> DIValpha(sc_uint<8>);
    void GenerateKeystream();
    void Initialize(sc_uint<32> *k, sc_uint<32> *);
    sc_uint<8> MULx(sc_uint<8>, sc_uint<8>);

    template <class T>
    sc_uint<8> MULxPOW(sc_uint<8> , T, sc_uint<8>);
    sc_uint<32>  MULalpha(sc_uint<8>);
    sc_uint<32> S1(sc_uint<32>);
    sc_uint<32> S2(sc_uint<32>);

    SC_CTOR(main)
    {
        SC_CTHREAD(GenerateKeystream, CLOCK.pos());
        reset_signal_is(rst, false);
    }

    ~main() {}
};

// Rijndael S-box SR
const u8 SR[256] /* ATTR ARRAY_1 */= {
0x63,0x7C,0x77,0x7B,0xF2,0x6B,0x6F,0xC5,0x30,0x01,0x67,0x2B,0xFE,0xD7,0xAB,0x76,
0xCA,0x82,0xC9,0x7D,0xFA,0x59,0x47,0xF0,0xAD,0xD4,0xA2,0xAF,0x9C,0xA4,0x72,0xC0,
0xB7,0xFD,0x93,0x26,0x36,0x3F,0xF7,0xCC,0x34,0xA5,0xE5,0xF1,0x71,0xD8,0x31,0x15,
0x04,0xC7,0x23,0xC3,0x18,0x96,0x05,0x9A,0x07,0x12,0x80,0xE2,0xEB,0x27,0xB2,0x75,
0x09,0x83,0x2C,0x1A,0x1B,0x6E,0x5A,0xA0,0x52,0x3B,0xD6,0xB3,0x29,0xE3,0x2F,0x84,
0x53,0xD1,0x00,0xED,0x20,0xFC,0xB1,0x5B,0x6A,0xCB,0xBE,0x39,0x4A,0x4C,0x58,0xCF,
0xD0,0xEF,0xAA,0xFB,0x43,0x4D,0x33,0x85,0x45,0xF9,0x02,0x7F,0x50,0x3C,0x9F,0xA8,
0x51,0xA3,0x40,0x8F,0x92,0x9D,0x38,0xF5,0xBC,0xB6,0xDA,0x21,0x10,0xFF,0xF3,0xD2,
0xCD,0x0C,0x13,0xEC,0x5F,0x97,0x44,0x17,0xC4,0xA7,0x7E,0x3D,0x64,0x5D,0x19,0x73,
0x60,0x81,0x4F,0xDC,0x22,0x2A,0x90,0x88,0x46,0xEE,0xB8,0x14,0xDE,0x5E,0x0B,0xDB,
0xE0,0x32,0x3A,0x0A,0x49,0x06,0x24,0x5C,0xC2,0xD3,0xAC,0x62,0x91,0x95,0xE4,0x79,
0xE7,0xC8,0x37,0x6D,0x8D,0xD5,0x4E,0xA9,0x6C,0x56,0xF4,0xEA,0x65,0x7A,0xAE,0x08,
0xBA,0x78,0x25,0x2E,0x1C,0xA6,0xB4,0xC6,0xE8,0xDD,0x74,0x1F,0x4B,0xBD,0x8B,0x8A,
0x70,0x3E,0xB5,0x66,0x48,0x03,0xF6,0x0E,0x61,0x35,0x57,0xB9,0x86,0xC1,0x1D,0x9E,
0xE1,0xF8,0x98,0x11,0x69,0xD9,0x8E,0x94,0x9B,0x1E,0x87,0xE9,0xCE,0x55,0x28,0xDF,
0x8C,0xA1,0x89,0x0D,0xBF,0xE6,0x42,0x68,0x41,0x99,0x2D,0x0F,0xB0,0x54,0xBB,0x16
};

// S-box SQ
const u8 SQ[256] /* ATTR ARRAY_2 */ = {
0x25,0x24,0x73,0x67,0xD7,0xAE,0x5C,0x30,0xA4,0xEE,0x6E,0xCB,0x7D,0xB5,0x82,0xDB,
0xE4,0x8E,0x48,0x49,0x4F,0x5D,0x6A,0x78,0x70,0x88,0xE8,0x5F,0x5E,0x84,0x65,0xE2,
0xD8,0xE9,0xCC,0xED,0x40,0x2F,0x11,0x28,0x57,0xD2,0xAC,0xE3,0x4A,0x15,0x1B,0xB9,
0xB2,0x80,0x85,0xA6,0x2E,0x02,0x47,0x29,0x07,0x4B,0x0E,0xC1,0x51,0xAA,0x89,0xD4,
0xCA,0x01,0x46,0xB3,0xEF,0xDD,0x44,0x7B,0xC2,0x7F,0xBE,0xC3,0x9F,0x20,0x4C,0x64,
0x83,0xA2,0x68,0x42,0x13,0xB4,0x41,0xCD,0xBA,0xC6,0xBB,0x6D,0x4D,0x71,0x21,0xF4,
0x8D,0xB0,0xE5,0x93,0xFE,0x8F,0xE6,0xCF,0x43,0x45,0x31,0x22,0x37,0x36,0x96,0xFA,
0xBC,0x0F,0x08,0x52,0x1D,0x55,0x1A,0xC5,0x4E,0x23,0x69,0x7A,0x92,0xFF,0x5B,0x5A,
0xEB,0x9A,0x1C,0xA9,0xD1,0x7E,0x0D,0xFC,0x50,0x8A,0xB6,0x62,0xF5,0x0A,0xF8,0xDC,
0x03,0x3C,0x0C,0x39,0xF1,0xB8,0xF3,0x3D,0xF2,0xD5,0x97,0x66,0x81,0x32,0xA0,0x00,
0x06,0xCE,0xF6,0xEA,0xB7,0x17,0xF7,0x8C,0x79,0xD6,0xA7,0xBF,0x8B,0x3F,0x1F,0x53,
0x63,0x75,0x35,0x2C,0x60,0xFD,0x27,0xD3,0x94,0xA5,0x7C,0xA1,0x05,0x58,0x2D,0xBD,
0xD9,0xC7,0xAF,0x6B,0x54,0x0B,0xE0,0x38,0x04,0xC8,0x9D,0xE7,0x14,0xB1,0x87,0x9C,
0xDF,0x6F,0xF9,0xDA,0x2A,0xC4,0x59,0x16,0x74,0x91,0xAB,0x26,0x61,0x76,0x34,0x2B,
0xAD,0x99,0xFB,0x72,0xEC,0x33,0x12,0xDE,0x98,0x3B,0xC0,0x9B,0x3E,0x18,0x10,0x3A,
0x56,0xE1,0x77,0xC9,0x1E,0x9E,0x95,0xA3,0x90,0x19,0xA8,0x6C,0x09,0xD0,0xF0,0x86
};


//
// Generation of Keystream.
// input n: number of 32-bit words of keystream.
// input z: space for the generated keystream, assumes
// memory is allocated already.
// output: generated keystream which is filled in z
// See section 4.2.
//
void main::GenerateKeystream()
{
    u32 t = 0;
    u32 F= 0x0;

    /* LFSR */
    LFSR_S0 = 0x00;
    LFSR_S1 = 0x00;
    LFSR_S2 = 0x00;
    LFSR_S3 = 0x00;
    LFSR_S4 = 0x00;
    LFSR_S5 = 0x00;
    LFSR_S6 = 0x00;
    LFSR_S7 = 0x00;
    LFSR_S8 = 0x00;
    LFSR_S9 = 0x00;
    LFSR_S10 = 0x00;
    LFSR_S11 = 0x00;
    LFSR_S12 = 0x00;
    LFSR_S13 = 0x00;
    LFSR_S14 = 0x00;
    LFSR_S15 = 0x00;
    /* FSM */
    FSM_R1 = 0x00;
    FSM_R2 = 0x00;
    FSM_R3 = 0x00;

    wait();

    while(1)
    {
        /* ATTR LOOP_3 */
        for (t=0 ; t < SIZE; t++)
            ks[t] = ks_in[t].read();

        ClockFSM(); /* Clock FSM once. Discard the output. */

        ClockLFSRKeyStreamMode(); /* Clock LFSR in keystream mode once. */
        /* ATTR LOOP_1 */
        for (t=0; t < SIZE; t++)
        {
            F = ClockFSM(); /* STEP 1 */

            ks[t] =  F ^ LFSR_S0; /* STEP 2 */

            /* Note that ks[t] corresponds to z_{t+1} in section 4.2*/
            ClockLFSRKeyStreamMode(); /* STEP 3 */
        }


        /* Write outputs */
        /* ATTR LOOP_4 */
        for(t=0; t<SIZE;t++)
            ks_out[t].write(ks[t]);

        wait();
    }
}

//
// MULx.
// Input V: an 8-bit input.
// Input c: an 8-bit input.
// Output : an 8-bit output.
// See section 3.1.1 for details.
//
/* ATTR FUNC_1 */
u8 main::MULx(u8 V, u8 c)
{
    u8 ret;
    if ( V & 0x80 )
        ret = (V << 1) ^ c;
    else
        ret = V << 1;
    return ret;
}

//
// MULxPOW.
// Input V: an 8-bit input.
// Input i: a positive integer.
// Input c: an 8-bit input.
// Output : an 8-bit output.
// See section 3.1.2 for details.
//
// PolyU DARC :  Modified to avoid recurssion not supported in HLS
/* ATTR FUNC_2 */
template <class T>
u8  main::MULxPOW(u8 V, T i,  u8 c)
{
    //if ( i == 0)  /* original recursive description */
    //  return V;
    //else
    // return MULx( MULxPOW( V, i-1, c ), c);

    while(i > 0)
    {
        V = MULx( V, c);
        i=i-1;
    }

    return V;
}

//
// The function MUL alpha.
// Input c: 8-bit input.
// Output : 32-bit output.
// See section 3.4.2 for details.
//
/* ATTR FUNC_3 */
u32 main::MULalpha(u8 c)
{
    return ( ( ((u32)MULxPOW(c, 23, 0xa9)) << 24 )|
        ( ((u32)MULxPOW(c, 245, 0xa9)) << 16 ) |
        ( ((u32)MULxPOW(c, 48, 0xa9)) << 8 ) |
        ( ((u32)MULxPOW(c, 239, 0xa9)) ) ) ;
}

//
// The function DIV alpha.
// Input c: 8-bit input.
// Output : 32-bit output.
// See section 3.4.3 for details.
//
/* ATTR FUNC_4 */
u32 main::DIValpha(u8 c)
{
    return ( ( ((u32)MULxPOW(c, 16, 0xa9)) << 24 )|
        ( ((u32)MULxPOW(c, 39, 0xa9)) << 16 ) |
        ( ((u32)MULxPOW(c, 6, 0xa9)) << 8 ) |
        ( ((u32)MULxPOW(c, 64, 0xa9)) ) ) ;
}

// The 32x32-bit S-Box S1
// Input: a 32-bit input.
// Output: a 32-bit output of S1 box.
// See section 3.3.1.
//
/* ATTR FUNC_5 */
u32 main::S1(u32 w)
{
    u8 r0=0, r1=0, r2=0, r3=0;
    u8 srw0 = SR[ (u8)((w >> 24) & 0xff) ];
    u8 srw1 = SR[ (u8)((w >> 16) & 0xff) ];
    u8 srw2 = SR[ (u8)((w >> 8) & 0xff) ];
    u8 srw3 = SR[ (u8)((w) & 0xff) ];

    r0 = ( ( MULx( srw0 , 0x1b) ) ^( srw1 ) ^( srw2 ) ^( (MULx( srw3, 0x1b)) ^ srw3 ));
    r1 = ( ( ( MULx( srw0 , 0x1b) ) ^ srw0 ) ^( MULx(srw1, 0x1b) ) ^( srw2 ) ^( srw3 ));
    r2 = ( ( srw0 ) ^( ( MULx( srw1 , 0x1b) ) ^ srw1 ) ^( MULx(srw2, 0x1b) ) ^( srw3 ));
    r3 = ( ( srw0 ) ^ ( srw1 ) ^( ( MULx( srw2 , 0x1b) ) ^ srw2 ) ^( MULx( srw3, 0x1b) ));
    return ( ( ((u32)r0) << 24 ) | ( ((u32)r1) << 16) | ( ((u32)r2) << 8 ) |( ((u32)r3) ) );
}

//
// The 32x32-bit S-Box S2
// Input: a 32-bit input.
// Output: a 32-bit output of S2 box.
// See section 3.3.2.
//
/* ATTR FUNC_6 */
u32 main::S2(u32 w)
{
    u8 r0=0, r1=0, r2=0, r3=0;

    u8 sqw0 = SQ[ (u8)((w >> 24) & 0xff) ];
    u8 sqw1 = SQ[ (u8)((w >> 16) & 0xff) ];
    u8 sqw2 = SQ[ (u8)((w >> 8) & 0xff) ];
    u8 sqw3 = SQ[ (u8)((w) & 0xff) ];
    r0 = ( ( MULx( sqw0 , 0x69) ) ^( sqw1 ) ^( sqw2 ) ^( (MULx( sqw3, 0x69)) ^ sqw3 ));
    r1 = ( ( ( MULx( sqw0 , 0x69) ) ^ sqw0 ) ^( MULx(sqw1, 0x69) ) ^( sqw2 ) ^( sqw3 ));
    r2 = ( ( sqw0 ) ^( ( MULx( sqw1 , 0x69) ) ^ sqw1 ) ^( MULx(sqw2, 0x69) ) ^( sqw3 ));
    r3 = ( ( sqw0 ) ^( sqw1 ) ^( ( MULx( sqw2 , 0x69) ) ^ sqw2 ) ^( MULx( sqw3, 0x69) ));

    return ( ( ((u32)r0) << 24 ) | ( ((u32)r1) << 16) | ( ((u32)r2) << 8 ) |( ((u32)r3) ) );
}

//
// Clocking LFSR in initialization mode.
// LFSR Registers S0 to S15 are updated as the LFSR receives a single clock.
// Input F: a 32-bit word comes from output of FSM.
// See section 3.4.4.
//
/* ATTR FUNC_7 */
void  main::ClockLFSRInitializationMode(u32 F)
{
    u32 v = ( ( (LFSR_S0 << 8) & 0xffffff00 )^( MULalpha( (u8)((LFSR_S0>>24) & 0xff)) ) ^( LFSR_S2 ) ^( (LFSR_S11 >> 8) & 0x00ffffff )^( DIValpha( (u8)( ( LFSR_S11) & 0xff )) ) ^( F ));
    LFSR_S0 = LFSR_S1;
    LFSR_S1 = LFSR_S2;
    LFSR_S2 = LFSR_S3;
    LFSR_S3 = LFSR_S4;
    LFSR_S4 = LFSR_S5;
    LFSR_S5 = LFSR_S6;
    LFSR_S6 = LFSR_S7;
    LFSR_S7 = LFSR_S8;
    LFSR_S8 = LFSR_S9;
    LFSR_S9 = LFSR_S10;
    LFSR_S10 = LFSR_S11;
    LFSR_S11 = LFSR_S12;
    LFSR_S12 = LFSR_S13;
    LFSR_S13 = LFSR_S14;
    LFSR_S14 = LFSR_S15;
    LFSR_S15 = v;
}

//
// Clocking LFSR in keystream mode.
// LFSR Registers S0 to S15 are updated as the LFSRreceives a single clock.
// See section 3.4.5.
//
void  main::ClockLFSRKeyStreamMode()
{
    u32 v = ( ( (LFSR_S0 << 8) & 0xffffff00 ) ^( MULalpha( (u8)((LFSR_S0>>24) & 0xff)) ) ^( LFSR_S2 ) ^( (LFSR_S11 >> 8) & 0x00ffffff )^( DIValpha( (u8)( ( LFSR_S11) & 0xff )) ));

    LFSR_S0 = LFSR_S1;
    LFSR_S1 = LFSR_S2;
    LFSR_S2 = LFSR_S3;
    LFSR_S3 = LFSR_S4;
    LFSR_S4 = LFSR_S5;
    LFSR_S5 = LFSR_S6;
    LFSR_S6 = LFSR_S7;
    LFSR_S7 = LFSR_S8;
    LFSR_S8 = LFSR_S9;
    LFSR_S9 = LFSR_S10;
    LFSR_S10 = LFSR_S11;
    LFSR_S11 = LFSR_S12;
    LFSR_S12 = LFSR_S13;
    LFSR_S13 = LFSR_S14;
    LFSR_S14 = LFSR_S15;
    LFSR_S15 = v;
}

//
// Clocking FSM.
// Produces a 32-bit word F.
// Updates FSM registers R1, R2, R3.
// See Section 3.4.6.
//
/* ATTR FUNC_8 */
u32 main::ClockFSM()
{
    u32 F = ( ( LFSR_S15 + FSM_R1 ) & 0xffffffff ) ^FSM_R2 ;
    u32 r = ( FSM_R2 + ( FSM_R3 ^ LFSR_S5 ) ) & 0xffffffff ;
    FSM_R3 = S2(FSM_R2);
    FSM_R2 = S1(FSM_R1);
    FSM_R1 = r;
    return F;
}

//
// Initialization.
// Input k[4]: Four 32-bit words making up 128-bit key.
// Input IV[4]: Four 32-bit words making 128-bit initialization variable.
// Output: All the LFSRs and FSM are initialized for key generation.
// See Section 4.1.
//
void main::Initialize(u32 k[4], u32 IV[4])
{
    u8 i=0;
    u32 F = 0x0;
    LFSR_S15 = k[3] ^ IV[0];
    LFSR_S14 = k[2];
    LFSR_S13 = k[1];
    LFSR_S12 = k[0] ^ IV[1];
    LFSR_S11 = k[3] ^ 0xffffffff;
    LFSR_S10 = k[2] ^ 0xffffffff ^ IV[2];
    LFSR_S9 = k[1] ^ 0xffffffff ^ IV[3];
    LFSR_S8 = k[0] ^ 0xffffffff;
    LFSR_S7 = k[3];
    LFSR_S6 = k[2];
    LFSR_S5 = k[1];
    LFSR_S4 = k[0];
    LFSR_S3 = k[3] ^ 0xffffffff;
    LFSR_S2 = k[2] ^ 0xffffffff;
    LFSR_S1 = k[1] ^ 0xffffffff;
    LFSR_S0 = k[0] ^ 0xffffffff;
    FSM_R1 = 0x0;
    FSM_R2 = 0x0;
    FSM_R3 = 0x0;

    /* ATTR LOOP_2 */
    for(i=0;i<32;i++)
    {
        F = ClockFSM();
        ClockLFSRInitializationMode(F);
    }
}