#include "systemc.h"

#ifdef DSE
    #define ATTR Cyber
    #define ARRAY_1 array=RAM
    #define LOOP_1 unroll_times=0
    #define FUNC_1 func=goto
    #define FUNC_2 func=inline
#endif

SC_MODULE (main)
{
    sc_in_clk CLOCK;
    sc_in<bool> rst;

    sc_in<sc_uint<16> > idata;

    sc_out<sc_uint<4> > odata;

    void run();
    sc_uint<32> div_mod(sc_uint<18>, sc_uint<15>);
    sc_uint<4> get_index_delta(sc_uint<4>);

    SC_CTOR(main)
    {
        SC_CTHREAD(run, CLOCK.pos());
        reset_signal_is(rst, false);
    }

    ~main() {}
};

void main::run()
{
    struct width_data {
        sc_uint<16> in_data;    // Current input data
        sc_uint<16> pre_data;    // Previous input data
        sc_uint<18> diff_data;
        sc_uint<4> enc_data;
    } width;

    width.in_data = 0;
    width.pre_data = 0;
    width.diff_data = 0;
    width.enc_data = 0;

    sc_uint<15> step_table[89] /* ATTR ARRAY_1 */ = {
        7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
        19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
        50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
        130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
        337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
        876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
        2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
        5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
        15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
    };

    sc_uint<7> index = 0;
    sc_uint<4> index_delta;
    sc_uint<15> divider;
    sc_uint<15> remainder;
    sc_uint<2> remainder1;
    bool neg_flag;
    sc_uint<19> dec_tmp;
    sc_uint<32> temp = 0;
    sc_uint<17> diff;
    sc_uint<4> odata_t;

    wait();

    while(1)
    {

        width.in_data = idata.read();
        divider = step_table[index];

        // Encode 
        diff = (width.in_data - width.pre_data) & 0x0000ffff;
        if (diff[15]==1)
        {
            width.diff_data = ((diff^0xffff) + 1);
            neg_flag = true;
        }
        else
        {
            width.diff_data = diff;
            neg_flag = false;
        }

        width.diff_data = (width.diff_data<<2);
        temp = div_mod(width.diff_data, divider);
        width.enc_data = temp.range(31,17);
        remainder = temp.range(14,0);
        remainder *=2;
        if (remainder >= divider)
            width.enc_data += 1;

        // Decode in the case of overflow 
        if (width.enc_data > 7)
        {
            width.enc_data = 7;
            dec_tmp = width.enc_data * divider;
            remainder1 = dec_tmp.range(1,0);
            if(remainder1 >= 2)
                width.pre_data += (dec_tmp >> 2) + 1;
            else
                width.pre_data += (dec_tmp >> 2);
        }
        else
            width.pre_data = width.in_data;

        // Output encoded data
        if (neg_flag == 1)
            odata_t = width.enc_data + 0x8;
        else
            odata_t = width.enc_data;
        odata = odata_t;

        // Next step preparation 
        index_delta = get_index_delta(width.enc_data);

        if (index==0 && index_delta==1)
            index = 0;
        else if (index_delta==1)
            index -= 1;
        else
            index += index_delta;

        wait();
    }
}

/* ATTR FUNC_1 */
sc_uint<4> main::get_index_delta(sc_uint<4> enc)
{
    sc_uint<4> ret;
    if (enc<=3) ret = 1;
    // if (enc>=0 && enc<=3) ret = 1;
    else if (enc==4) ret = 2;
    else if (enc==5) ret = 4;
    else if (enc==6) ret = 6;
    else             ret = 8;

    return ret;
}

/* ATTR FUNC_2 */
sc_uint<32> main::div_mod(sc_uint<18> numerator, sc_uint<15> denominator)
{
    sc_uint<18> quotient=0;
    sc_uint<15> remainder;
    sc_uint<32> temp;
    sc_uint<18> d=0;
    int i;
    /* ATTR LOOP_1 */
    for (i = 17; i >= 0; i--)
    {

        d = (d<<1) + numerator[i]; 

        if (d >= denominator)
        {
            d -= denominator;
            quotient += (0x1<<i);
        }
    }
    remainder = d;
    temp = (quotient << 17) + remainder;
    return temp;
}
