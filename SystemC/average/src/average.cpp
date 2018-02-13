#include "systemc.h"

#define SIZE 8

#ifdef DSE
#define ATTR Cyber
#define ARRAY_1 array=RAM
#define LOOP_1 unroll_times=0 // 7
#define LOOP_2 unroll_times=0 // 7
#endif

#ifdef TLV
#define VAL_SIG Cyber
#endif


SC_MODULE(main)
{
    sc_in_clk clk;
    sc_in<bool> rst;
    sc_in<sc_uint<8> > in_data /* VAL_SIG valid_sig_gen = v_i_1 */;

    sc_out<sc_uint<8> > ave8_output /* VAL_SIG valid_sig_gen = v_o_1 */;

    void ave8_main(void);

    SC_CTOR(main)
    {
        SC_CTHREAD(ave8_main, clk.pos());
        reset_signal_is(rst, false);
        sensitive << clk.pos();
    }

    ~main() {}
};

void main::ave8_main(void)
{
    int sum = 0;
    sc_uint<8> buffer[SIZE] /* ATTR ARRAY_1 */ = {0};
    int i = 0;

    ave8_output.write(0);

    wait();

    while (1)
    {
        /* ATTR LOOP_1 */
        for (i = SIZE - 1; i > 0; i--)
            buffer[i] = buffer[i - 1];
        buffer[0] = in_data.read();

        sum = buffer[0];
        /* ATTR LOOP_2 */
        for (i = 1; i < SIZE; i++)
            sum += buffer[i];

        ave8_output.write(sum / SIZE);
        wait();
    }
}
