#include "systemc.h"

#define DCTSIZE 8
#define DCTSIZE2 64

typedef short INT16;
typedef long  INT32;

#define ONE ((INT32) 1)
#define RIGHT_SHIFT(x,shft) ((x) >> (shft))
#define DESCALE(x,n) RIGHT_SHIFT((x) + (ONE << ((n)-1)), n)

#define MAXJSAMPLE 255
#define RANGE_MASK (MAXJSAMPLE * 4 + 3)

#define CONST_BITS  13
#define PASS1_BITS  2

#define FIX_0_298631336  ((INT32)  2446)    /* FIX(0.298631336) */
#define FIX_0_390180644  ((INT32)  3196)    /* FIX(0.390180644) */
#define FIX_0_541196100  ((INT32)  4433)    /* FIX(0.541196100) */
#define FIX_0_765366865  ((INT32)  6270)    /* FIX(0.765366865) */
#define FIX_0_899976223  ((INT32)  7373)    /* FIX(0.899976223) */
#define FIX_1_175875602  ((INT32)  9633)    /* FIX(1.175875602) */
#define FIX_1_501321110  ((INT32)  12299)   /* FIX(1.501321110) */
#define FIX_1_847759065  ((INT32)  15137)   /* FIX(1.847759065) */
#define FIX_1_961570560  ((INT32)  16069)   /* FIX(1.961570560) */
#define FIX_2_053119869  ((INT32)  16819)   /* FIX(2.053119869) */
#define FIX_2_562915447  ((INT32)  20995)   /* FIX(2.562915447) */
#define FIX_3_072711026  ((INT32)  25172)   /* FIX(3.072711026) */


#define MULTIPLY16C16(var,const)  (((INT16) (var)) * ((INT16) (const)))

#define MULTIPLY(var,const)  MULTIPLY16C16(var,const)

#define DEQUANTIZE(coef,quantval)  (((sc_int<32>) (coef)) * (quantval))

#ifdef DSE
#define ARRAY_1 array=REG
#define ARRAY_2 array=REG
#define ARRAY_3 array=REG
#define ARRAY_4 array=REG
#define ARRAY_5 array=REG
#define LOOP_1 unroll_times=all // 8
#define LOOP_2 unroll_times=all // 8
#define LOOP_3 unroll_times=all // 8
#define LOOP_4 unroll_times=all // 8
#endif

#ifdef TLV
#define VAL_SIG Cyber
#endif


SC_MODULE( main ) {

    typedef int        workspace_t;

    // Inputs
    sc_in_clk              clk;
    sc_in< bool >          rst;

    // sc_in<  bool >         input_ready /* VAL_SIG valid_sig_gen = v_i_1 */;
    sc_in< sc_int<16>  >   input_coef /* VAL_SIG valid_sig_gen = v_i_1 */;
    sc_in< sc_uint<6>  >   input_quant /* VAL_SIG valid_sig_gen = v_i_2 */;

    // Outputs
    sc_out< bool >         output_start /* VAL_SIG valid_sig_gen = v_o_1 */;
    sc_out< sc_uint<8>  >  output_sample /* VAL_SIG valid_sig_gen = v_o_2 */;
    sc_out< sc_uint<3> >   output_row /* VAL_SIG valid_sig_gen = v_o_3 */;
    sc_out< sc_uint<3> >   output_col /* VAL_SIG valid_sig_gen = v_o_4 */;

    /* E */
    void entry();

    /* J */
    void jpeg_idct_islow();

    /* R */
    void run();



    SC_CTOR( main ) {
        SC_CTHREAD(run, clk.pos());
        reset_signal_is(rst, false);
    };



    ~main() {};

};

void main::jpeg_idct_islow()
{
    INT32 tmp0, tmp1, tmp2, tmp3;
    INT32 tmp10, tmp11, tmp12, tmp13;
    INT32 z1, z2, z3, z4, z5;
    int ctr;

    sc_int<16>      inptr[8] /* ATTR ARRAY_1 */;
    sc_uint<6>   quantptr[8] /* ATTR ARRAY_2 */;


    workspace_t workspace[ DCTSIZE2 ] /* ATTR ARRAY_3 */;

    sc_uint<8> range_limit[ 1028 ] /* ATTR ARRAY_4 */ = {
#   include "range_limit.dat"
    };

    /* Pass 1: process columns from input, store into work array. */
    /* Note results are scaled up by sqrt(8) compared to a true IDCT; */
    /* furthermore, we scale the results by 2**PASS1_BITS. */

    workspace_t* wsptr = workspace;
    /* ATTR LOOP_1 */
    for (ctr = DCTSIZE; ctr > 0; ctr--) {


        // Read inputs
        /* ATTR LOOP_2 */
        for ( int j = 0; j < 8; j++ ) {
            inptr[j]    = input_coef.read();
            quantptr[j] = input_quant.read();
            // wait();
        }

        if (inptr[1] == 0 && inptr[2] == 0 &&
                inptr[3] == 0 && inptr[4] == 0 &&
                inptr[5] == 0 && inptr[6] == 0 &&
                inptr[7] == 0) {
            /* AC terms all zero */
            workspace_t dcval = DEQUANTIZE(inptr[0], quantptr[0]) << PASS1_BITS;

            wsptr[DCTSIZE * 0] = dcval;
            wsptr[DCTSIZE * 1] = dcval;
            wsptr[DCTSIZE * 2] = dcval;
            wsptr[DCTSIZE * 3] = dcval;
            wsptr[DCTSIZE * 4] = dcval;
            wsptr[DCTSIZE * 5] = dcval;
            wsptr[DCTSIZE * 6] = dcval;
            wsptr[DCTSIZE * 7] = dcval;

            wsptr++;          /* advance pointers to next column */
            continue;
        }

        /* Even part: reverse the even part of the forward DCT. */
        /* The rotator is sqrt(2)*c(-6). */

        z2 = DEQUANTIZE(inptr[2], quantptr[2]);
        z3 = DEQUANTIZE(inptr[6], quantptr[6]);

        z1 = MULTIPLY(z2 + z3, FIX_0_541196100);
        tmp2 = z1 + MULTIPLY(z3, - FIX_1_847759065);
        tmp3 = z1 + MULTIPLY(z2, FIX_0_765366865);

        z2 = DEQUANTIZE(inptr[0], quantptr[0]);
        z3 = DEQUANTIZE(inptr[4], quantptr[4]);

        tmp0 = (z2 + z3) << CONST_BITS;
        tmp1 = (z2 - z3) << CONST_BITS;

        tmp10 = tmp0 + tmp3;
        tmp13 = tmp0 - tmp3;
        tmp11 = tmp1 + tmp2;
        tmp12 = tmp1 - tmp2;

        /* Odd part per figure 8; the matrix is unitary and hence its
         * transpose is its inverse.  i0..i3 are y7,y5,y3,y1 respectively.
         */

        tmp0 = DEQUANTIZE(inptr[7], quantptr[7]);
        tmp1 = DEQUANTIZE(inptr[5], quantptr[5]);
        tmp2 = DEQUANTIZE(inptr[3], quantptr[3]);
        tmp3 = DEQUANTIZE(inptr[1], quantptr[1]);

        z1 = tmp0 + tmp3;
        z2 = tmp1 + tmp2;
        z3 = tmp0 + tmp2;
        z4 = tmp1 + tmp3;
        z5 = MULTIPLY(z3 + z4, FIX_1_175875602); /* sqrt(2) * c3 */

        tmp0 = MULTIPLY(tmp0, FIX_0_298631336); /* sqrt(2) * (-c1+c3+c5-c7) */
        tmp1 = MULTIPLY(tmp1, FIX_2_053119869); /* sqrt(2) * ( c1+c3-c5+c7) */
        tmp2 = MULTIPLY(tmp2, FIX_3_072711026); /* sqrt(2) * ( c1+c3+c5-c7) */
        tmp3 = MULTIPLY(tmp3, FIX_1_501321110); /* sqrt(2) * ( c1+c3-c5-c7) */
        z1 = MULTIPLY(z1, - FIX_0_899976223); /* sqrt(2) * (c7-c3) */
        z2 = MULTIPLY(z2, - FIX_2_562915447); /* sqrt(2) * (-c1-c3) */
        z3 = MULTIPLY(z3, - FIX_1_961570560); /* sqrt(2) * (-c3-c5) */
        z4 = MULTIPLY(z4, - FIX_0_390180644); /* sqrt(2) * (c5-c3) */

        z3 += z5;
        z4 += z5;

        tmp0 += z1 + z3;
        tmp1 += z2 + z4;
        tmp2 += z2 + z3;
        tmp3 += z1 + z4;

        /* Final output stage: inputs are tmp10..tmp13, tmp0..tmp3 */

        wsptr[DCTSIZE * 0] = (workspace_t) DESCALE(tmp10 + tmp3, CONST_BITS - PASS1_BITS);
        wsptr[DCTSIZE * 7] = (workspace_t) DESCALE(tmp10 - tmp3, CONST_BITS - PASS1_BITS);
        wsptr[DCTSIZE * 1] = (workspace_t) DESCALE(tmp11 + tmp2, CONST_BITS - PASS1_BITS);
        wsptr[DCTSIZE * 6] = (workspace_t) DESCALE(tmp11 - tmp2, CONST_BITS - PASS1_BITS);
        wsptr[DCTSIZE * 2] = (workspace_t) DESCALE(tmp12 + tmp1, CONST_BITS - PASS1_BITS);
        wsptr[DCTSIZE * 5] = (workspace_t) DESCALE(tmp12 - tmp1, CONST_BITS - PASS1_BITS);
        wsptr[DCTSIZE * 3] = (workspace_t) DESCALE(tmp13 + tmp0, CONST_BITS - PASS1_BITS);
        wsptr[DCTSIZE * 4] = (workspace_t) DESCALE(tmp13 - tmp0, CONST_BITS - PASS1_BITS);

        wsptr++;            /* advance pointers to next column */
    }

    /* Pass 2: process rows from work array, store into output array. */
    /* Note that we must descale the results by a factor of 8 == 2**3, */
    /* and also undo the PASS1_BITS scaling. */

    wsptr = workspace;

    /* ATTR LOOP_3 */
    for (ctr = 0; ctr < DCTSIZE; ctr++) {
        sc_uint<8> outptr[8] /* ATTR ARRAY_5 */;

#ifndef NO_ZERO_ROW_TEST
        if (wsptr[1] == 0 && wsptr[2] == 0 && wsptr[3] == 0 && wsptr[4] == 0 &&
                wsptr[5] == 0 && wsptr[6] == 0 && wsptr[7] == 0) {
            /* AC terms all zero */
            sc_uint<8> dcval = range_limit[(int) DESCALE((INT32) wsptr[0], PASS1_BITS + 3)
                                           & RANGE_MASK];

            outptr[0] = dcval;
            outptr[1] = dcval;
            outptr[2] = dcval;
            outptr[3] = dcval;
            outptr[4] = dcval;
            outptr[5] = dcval;
            outptr[6] = dcval;
            outptr[7] = dcval;

            wsptr += DCTSIZE;     /* advance pointer to next row */
            goto OUTPUT;
        }
#endif

        /* Even part: reverse the even part of the forward DCT. */
        /* The rotator is sqrt(2)*c(-6). */

        z2 = (INT32) wsptr[2];
        z3 = (INT32) wsptr[6];

        z1 = MULTIPLY(z2 + z3, FIX_0_541196100);
        tmp2 = z1 + MULTIPLY(z3, - FIX_1_847759065);
        tmp3 = z1 + MULTIPLY(z2, FIX_0_765366865);

        tmp0 = ((INT32) wsptr[0] + (INT32) wsptr[4]) << CONST_BITS;
        tmp1 = ((INT32) wsptr[0] - (INT32) wsptr[4]) << CONST_BITS;

        tmp10 = tmp0 + tmp3;
        tmp13 = tmp0 - tmp3;
        tmp11 = tmp1 + tmp2;
        tmp12 = tmp1 - tmp2;

        /* Odd part per figure 8; the matrix is unitary and hence its
         * transpose is its inverse.  i0..i3 are y7,y5,y3,y1 respectively.
         */

        tmp0 = (INT32) wsptr[7];
        tmp1 = (INT32) wsptr[5];
        tmp2 = (INT32) wsptr[3];
        tmp3 = (INT32) wsptr[1];

        z1 = tmp0 + tmp3;
        z2 = tmp1 + tmp2;
        z3 = tmp0 + tmp2;
        z4 = tmp1 + tmp3;
        z5 = MULTIPLY(z3 + z4, FIX_1_175875602); /* sqrt(2) * c3 */

        tmp0 = MULTIPLY(tmp0, FIX_0_298631336);  /* sqrt(2) * (-c1+c3+c5-c7) */
        tmp1 = MULTIPLY(tmp1, FIX_2_053119869);  /* sqrt(2) * ( c1+c3-c5+c7) */
        tmp2 = MULTIPLY(tmp2, FIX_3_072711026);  /* sqrt(2) * ( c1+c3+c5-c7) */
        tmp3 = MULTIPLY(tmp3, FIX_1_501321110);  /* sqrt(2) * ( c1+c3-c5-c7) */
        z1 = MULTIPLY(z1, - FIX_0_899976223);    /* sqrt(2) * (c7-c3) */
        z2 = MULTIPLY(z2, - FIX_2_562915447);    /* sqrt(2) * (-c1-c3) */
        z3 = MULTIPLY(z3, - FIX_1_961570560);    /* sqrt(2) * (-c3-c5) */
        z4 = MULTIPLY(z4, - FIX_0_390180644);    /* sqrt(2) * (c5-c3) */

        z3 += z5;
        z4 += z5;

        tmp0 += z1 + z3;
        tmp1 += z2 + z4;
        tmp2 += z2 + z3;
        tmp3 += z1 + z4;

        /* Final output stage: inputs are tmp10..tmp13, tmp0..tmp3 */

        outptr[0] = range_limit[(int) DESCALE(tmp10 + tmp3,
                                              CONST_BITS + PASS1_BITS + 3)
                                & RANGE_MASK];
        outptr[7] = range_limit[(int) DESCALE(tmp10 - tmp3,
                                              CONST_BITS + PASS1_BITS + 3)
                                & RANGE_MASK];
        outptr[1] = range_limit[(int) DESCALE(tmp11 + tmp2,
                                              CONST_BITS + PASS1_BITS + 3)
                                & RANGE_MASK];
        outptr[6] = range_limit[(int) DESCALE(tmp11 - tmp2,
                                              CONST_BITS + PASS1_BITS + 3)
                                & RANGE_MASK];
        outptr[2] = range_limit[(int) DESCALE(tmp12 + tmp1,
                                              CONST_BITS + PASS1_BITS + 3)
                                & RANGE_MASK];
        outptr[5] = range_limit[(int) DESCALE(tmp12 - tmp1,
                                              CONST_BITS + PASS1_BITS + 3)
                                & RANGE_MASK];
        outptr[3] = range_limit[(int) DESCALE(tmp13 + tmp0,
                                              CONST_BITS + PASS1_BITS + 3)
                                & RANGE_MASK];
        outptr[4] = range_limit[(int) DESCALE(tmp13 - tmp0,
                                              CONST_BITS + PASS1_BITS + 3)
                                & RANGE_MASK];

        wsptr += DCTSIZE;       /* advance pointer to next row */

        // Write outputs
OUTPUT:
        output_start.write( 1 );
        /* ATTR LOOP_4 */
        for ( int j = 0; j < 8; j++ ) {
            output_sample.write( outptr[j] );
            output_row.write( ctr );
            output_col.write( j );

            wait();
        }
        output_start.write( 0 );



    }
}


void main::run()
{

    //---  Reset- should be executable in 1 clock cycle
    output_start.write( 0 );
    output_sample.write( 0 );
    output_row.write( 0 );
    output_col.write( 0 );


    wait();

    //---- Main IDCT computation
    while ( true ) {

        jpeg_idct_islow();


    }
}
