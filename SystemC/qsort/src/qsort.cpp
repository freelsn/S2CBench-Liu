/*
Original implementation by Alien Ryder Flex
http://alienryderflex.com/quicksort/
*/

#include "systemc.h"

#define SIZE 10
#define BW 8

#ifdef DSE
    #define ATTR Cyber
    #define ARRAY_1 array=REG
    #define ARRAY_2 array=REG
    #define ARRAY_3 array=REG
    #define LOOP_1 unroll_times=all // 10
    #define LOOP_2 unroll_times=all // 10
    #define LOOP_3 unroll_times=all // 0, all
    #define LOOP_4 unroll_times=all // 0, all
    #define LOOP_5 unroll_times=all // 0, all
    #define LOOP_6 unroll_times=all // 0, all
    #define FUNC_1 func=inline
#endif

SC_MODULE(main)
{
    sc_in_clk CLOCK;
    sc_in<bool> rst;

    sc_in<sc_uint<BW> > idata;

    sc_out<sc_uint<BW> > odata;

    void swap(int *, int *);
    void sort(int arr[SIZE]);
    void run();

    SC_CTOR(main)
    {
        SC_CTHREAD(run, CLOCK.pos());
        reset_signal_is(rst, false);
    }

    ~main() {}
};

void main::run()
{
    int i;
    int idata_buf[SIZE] /* ATTR ARRAY_1 */;

    wait();

    while(1)
    {
        /* ATTR LOOP_1 */
        for (i = 0; i < SIZE; i++)
            idata_buf[i] = idata.read().to_int();

        sort(idata_buf);
        /* ATTR LOOP_2 */
        for (i = 0; i < SIZE; i++)
            odata.write(idata_buf[i]);

        wait();
    }
}

void main::sort(int arr[SIZE])
{
    int piv;
    int beg[SIZE] /* ATTR ARRAY_2 */;
    int end[SIZE] /* ATTR ARRAY_3 */;
    int i = 0, L, R;

    beg[0] = 0;
    end[0] = SIZE;
    /* ATTR LOOP_3 */
    while (i >= 0)
    {

        L = beg[i];
        R = end[i]-1;

        if (L < R)
        {
            piv = arr[L];

            /* ATTR LOOP_4 */
            while (L < R)
            {
                /* ATTR LOOP_5 */
                while (arr[R] >= piv && L < R)
                    R--;

                if (L < R)
                    arr[L++] = arr[R];

                /* ATTR LOOP_6 */
                while (arr[L] <= piv && L < R)
                    L++;

                if (L<R)
                    arr[R--] = arr[L];
            }

            arr[L] = piv;
            beg[i+1] = L+1;
            end[i+1] = end[i];
            end[i++] = L;

            if (end[i] - beg[i] > end[i-1] - beg[i-1])
            {
                swap(&beg[i], &beg[i - 1]);
                swap(&end[i], &end[i - 1]);
            }
        }
        else
            i--;
    }
}

/* ATTR FUNC_1 */
void main::swap(int *a, int *b)
{
    int t = *a;
    *a = *b;
    *b = t;
}
