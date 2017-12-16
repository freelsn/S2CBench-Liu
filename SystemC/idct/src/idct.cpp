#include "systemc.h"

#define c1d4 362L

#define c1d8 473L
#define c3d8 196L

#define c1d16 502L
#define c3d16 426L
#define c5d16 284L
#define c7d16 100L

#define DCTSIZE 64

#ifdef DSE
    #define ATTR Cyber
    #define ARRAY_1 array=REG
    #define ARRAY_2 array=REG
    #define LOOP_1 unroll_times=64 // 64
    #define LOOP_2 unroll_times=8 // 8
    #define LOOP_3 unroll_times=8 // 8
    #define LOOP_4 unroll_times=0 // 64
    #define LOOP_5 unroll_times=64 // 64
    #define FUNC_1 func=inline
    #define FUNC_2 func=inline
    #define FUNC_3 func=inline
#endif

SC_MODULE (main)
{
    sc_in_clk CLOCK;
    sc_in<bool> rst;

    sc_in<sc_uint<32> > idata;

    sc_out<sc_uint<8> > odata;

    int x[DCTSIZE] /* ATTR ARRAY_1 */;
    int y[DCTSIZE] /* ATTR ARRAY_2 */;

    void idct_main();
    int LS(int r, int s);
    int RS(int r, int s);
    int MSCALE(int expr);

    SC_CTOR(main)
    {
        SC_CTHREAD(idct_main, CLOCK.pos());
        reset_signal_is(rst, false);
    }

    ~main() {}
};

void main::idct_main()
{

    int i;
    int *aptr;
    int a0, a1, a2, a3;
    int b0, b1, b2, b3;
    int c0, c1, c2, c3;

    wait();

    while(1)
    {
         /* ATTR LOOP_1 */
        for (i = 0; i < DCTSIZE; i++)
            x[i] = idata.read().to_int();

        /* Loop over columns */
        /* ATTR LOOP_2 */
        for (i = 0; i < 8; i++)
        {
            aptr = x + i;
            b0 = LS (*aptr, 2);
            aptr += 8;
            a0 = LS (*aptr, 2);
            aptr += 8;
            b2 = LS (*aptr, 2);
            aptr += 8;
            a1 = LS (*aptr, 2);
            aptr += 8;
            b1 = LS (*aptr, 2);
            aptr += 8;
            a2 = LS (*aptr, 2);
            aptr += 8;
            b3 = LS (*aptr, 2);
            aptr += 8;
            a3 = LS (*aptr, 2);

            /* Split into even mode  b0 = x0  b1 = x4  b2 = x2  b3 = x6.
            And the odd terms a0 = x1 a1 = x3 a2 = x5 a3 = x7.
            */

            c0 = MSCALE ((c7d16 * a0) - (c1d16 * a3));
            c1 = MSCALE ((c3d16 * a2) - (c5d16 * a1));
            c2 = MSCALE ((c3d16 * a1) + (c5d16 * a2));
            c3 = MSCALE ((c1d16 * a0) + (c7d16 * a3));

            /* First Butterfly on even terms. */

            a0 = MSCALE (c1d4 * (b0 + b1));
            a1 = MSCALE (c1d4 * (b0 - b1));

            a2 = MSCALE ((c3d8 * b2) - (c1d8 * b3));
            a3 = MSCALE ((c1d8 * b2) + (c3d8 * b3));

            b0 = a0 + a3;
            b1 = a1 + a2;
            b2 = a1 - a2;
            b3 = a0 - a3;

            /* Second Butterfly */

            a0 = c0 + c1;
            a1 = c0 - c1;
            a2 = c3 - c2;
            a3 = c3 + c2;

            c0 = a0;
            c1 = MSCALE (c1d4 * (a2 - a1));
            c2 = MSCALE (c1d4 * (a2 + a1));
            c3 = a3;

            aptr = y + i;
            *aptr = b0 + c3;
            aptr += 8;
            *aptr = b1 + c2;
            aptr += 8;
            *aptr = b2 + c1;
            aptr += 8;
            *aptr = b3 + c0;
            aptr += 8;
            *aptr = b3 - c0;
            aptr += 8;
            *aptr = b2 - c1;
            aptr += 8;
            *aptr = b1 - c2;
            aptr += 8;
            *aptr = b0 - c3;
        }

        /* Loop over rows */
        /* ATTR LOOP_3 */
        for (i = 0; i < 8; i++)
        {
            aptr = y + LS (i, 3);
            b0 = *(aptr++);
            a0 = *(aptr++);
            b2 = *(aptr++);
            a1 = *(aptr++);
            b1 = *(aptr++);
            a2 = *(aptr++);
            b3 = *(aptr++);
            a3 = *(aptr);

            /*
            Split into even mode  b0 = x0  b1 = x4  b2 = x2  b3 = x6.
            And the odd terms a0 = x1 a1 = x3 a2 = x5 a3 = x7.
            */

            c0 = MSCALE ((c7d16 * a0) - (c1d16 * a3));
            c1 = MSCALE ((c3d16 * a2) - (c5d16 * a1));
            c2 = MSCALE ((c3d16 * a1) + (c5d16 * a2));
            c3 = MSCALE ((c1d16 * a0) + (c7d16 * a3));

            /* First Butterfly on even terms. */

            a0 = MSCALE (c1d4 * (b0 + b1));
            a1 = MSCALE (c1d4 * (b0 - b1));

            a2 = MSCALE ((c3d8 * b2) - (c1d8 * b3));
            a3 = MSCALE ((c1d8 * b2) + (c3d8 * b3));

            /* Calculate last set of b's */

            b0 = a0 + a3;
            b1 = a1 + a2;
            b2 = a1 - a2;
            b3 = a0 - a3;

            /* Second Butterfly */

            a0 = c0 + c1;
            a1 = c0 - c1;
            a2 = c3 - c2;
            a3 = c3 + c2;

            c0 = a0;
            c1 = MSCALE (c1d4 * (a2 - a1));
            c2 = MSCALE (c1d4 * (a2 + a1));
            c3 = a3;

            aptr = y + LS (i, 3);
            *(aptr++) = b0 + c3;
            *(aptr++) = b1 + c2;
            *(aptr++) = b2 + c1;
            *(aptr++) = b3 + c0;
            *(aptr++) = b3 - c0;
            *(aptr++) = b2 - c1;
            *(aptr++) = b1 - c2;
            *(aptr) = b0 - c3;
        }

        /*
        Retrieve correct accuracy. We have additional factor
        of 16 that must be removed.
        */
        aptr = y;
        /* ATTR LOOP_4 */
        for (i = 0; i < 64; i++)
        {
            *aptr = (((*aptr < 0) ? (*aptr - 8) : (*aptr + 8)) / 16);
            aptr++;
        }

        /* ATTR LOOP_5 */
        for (i = 0; i < DCTSIZE; i++)
            odata = y[i];

        wait();
    }
}

/* FUNC_1 */
int main::LS(int r, int s)
{
    return (r << s);
}
/* FUNC_2 */
int main::RS(int r, int s)
{
    return (r >> s);
}
/* FUNC_3 */
int main::MSCALE(int expr)
{
    return RS(expr, 9);
}
