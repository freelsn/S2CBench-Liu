#include "systemc.h"

#define FIR_TAPS 6

SC_MODULE (main)
{
    sc_in_clk CLOCK;
    sc_in<bool> rst;

    sc_in<sc_uint<8> > idata[FIR_TAPS];
    sc_in<sc_uint<16> > coeff[FIR_TAPS];

    sc_out<sc_uint<8> > odata;

    void fir_main();
    unsigned char fir_func(unsigned char *data, unsigned short int *coeff);

    SC_CTOR(main)
    {
        SC_CTHREAD(fir_main, CLOCK.pos());
        reset_signal_is(rst, false);
    }

    ~main() {}
};

void main::fir_main()
{
    unsigned char idata_t[FIR_TAPS] /* ATTR ARRAY_1 */;
    unsigned short int coeff_t[FIR_TAPS] /* ATTR ARRAY_2 */;
    unsigned char fir_out;
    int i;

    wait();

    while(1)
    {
        // Reading inputs
        /* ATTR LOOP_2 */
        for (i = 0; i < FIR_TAPS; i++)
            idata_t[i] = idata[i].read();
        /* ATTR LOOP_3 */
        for (i = 0; i < FIR_TAPS; i++)
            coeff_t[i] = coeff[i].read();

        // Computing
        fir_out = fir_func(idata_t, coeff_t);

        // Writing outputs
        odata.write(fir_out);

        wait();
    }
}
/* ATTR FUNC_1 */
unsigned char main::fir_func(unsigned char data[FIR_TAPS], unsigned short int coeff[FIR_TAPS])
{
    int i;
    int sum = 0;
    /* ATTR LOOP_1 */
    for (i = 0; i < FIR_TAPS; i++)
        sum += data[i] * coeff[i];

    if (sum < 0)
        sum = 0;
    else if (sum > (255 << 10))
        sum = 255 << 10;

    sum = (sum >> 10) & 0xFF;

    return sum;
}
