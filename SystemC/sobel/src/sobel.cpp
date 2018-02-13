#include "systemc.h"

// High Resolution case 1920x1080
#ifdef  RESHD
#define BYTES_PER_PIXEL 3
#define COLS 1920* BYTES_PER_PIXEL
#define ROWS 1080* BYTES_PER_PIXEL
#define SIZE COLS*ROWS
#define IMAGE_IN "batmanjoker.bmp"
#define IMAGE_OUT "batmanjoker_out.bmp"
#define IMAGE_GOLDEN "batmanjoker_golden.bmp"


// 512x512 case
#else

#define BYTES_PER_PIXEL 1
#define COLS 512 * BYTES_PER_PIXEL
#define ROWS 512 * BYTES_PER_PIXEL
#define SIZE ROWS* COLS
#define IMAGE_IN "lena512.bmp"
#define IMAGE_OUT "lena512_out.bmp"
#define IMAGE_GOLDEN "lena512_golden.bmp"

#endif

#define SIZE_BUFFER 3

#ifdef DSE
#define ATTR Cyber
#define ARRAY_1 array=REG
#define ARRAY_2 array=REG
#define ARRAY_3 array=REG
#define ARRAY_4 array=REG
#define LOOP_1 unroll_times=all
#define LOOP_2 unroll_times=all
#define LOOP_3 unroll_times=all
#define LOOP_4 unroll_times=all
#define LOOP_5 unroll_times=all
#endif

typedef int LONG;
typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef char BYTE;


SC_MODULE (main) {

public:

    sc_in_clk clk; //clock
    sc_in<bool> rst; //reset

    //inputs
    sc_in <sc_uint <8> > input_row[SIZE_BUFFER];

    //outputs
    sc_out<sc_uint<8> > output_row;


// Variables declarations
    sc_uint<8>  line_buffer[SIZE_BUFFER][SIZE_BUFFER] /* ATTR ARRAY_1 */;

    //function prototypes

    /* S */
    sc_uint<8> sobel_filter(sc_uint<8> *);
    void sobel_main(void);

    SC_CTOR (main) {
        SC_CTHREAD(sobel_main, clk.pos());
        reset_signal_is(rst, false);
    };

    ~main() {};
};

void main::sobel_main(void)
{

    sc_uint<8> input_row_read[3] /* ATTR ARRAY_2 */;
    sc_uint<8> output_row_write;
    // int i,j,x;


    wait();

    while (1)
    {

        // Reading triplet data needed for filter

        input_row_read[0] = input_row[0].read();
        input_row_read[1] = input_row[1].read();
        input_row_read[2] = input_row[2].read();

        // Perform the filtering of a 3x3 pixel image segment
        output_row_write =  sobel_filter(input_row_read);
        // Writting filtered output back
        output_row.write(output_row_write);
        wait();
    }// end of while
}


//
// Sobel filter function
//
sc_uint<8>  main::sobel_filter(sc_uint<8> *input_row_r)
{

    unsigned int X, Y;
    unsigned char orow;
    int sumX, sumY;
    int SUM, rowOffset, colOffset;



    int Gx[3][3] /* ATTR ARRAY_3 */ = {{1 , 0 , -1},
        { 2, 0, -2},
        { 1, 0, -1}
    };


    int Gy[3][3] /* ATTR ARRAY_4 */ = {{1, 2, 1},
        {0, 0, 0},
        { -1, -2, -1}
    };


    /* Shifting 3x3 line buffer by one row  */
    /* ATTR LOOP_1 */
    for (Y = 2; Y > 0; Y--) {
        /* ATTR LOOP_2 */
        for (X = 0; X < 3; X++) {
            line_buffer[X][Y - 1] = line_buffer[X][Y];
        }
    }

    // Reading new data into the line buffer
    /* ATTR LOOP_3 */
    for (X = 0; X < SIZE_BUFFER; X++)
        line_buffer[X][2] = input_row_r[X];


    sumX = 0;
    sumY = 0;

    // Convolution starts here
    //-------X GRADIENT APPROXIMATION------
    //-------Y GRADIENT APPROXIMATION------
    /* ATTR LOOP_4 */
    for (rowOffset = -1; rowOffset <= 1; rowOffset++) {
        /* ATTR LOOP_5 */
        for (colOffset = -1; colOffset <= 1; colOffset++) {
            sumX = sumX + line_buffer[1 + rowOffset][1 - colOffset] * Gx[1 + rowOffset][1 + colOffset];
            sumY = sumY + line_buffer[1 + rowOffset][1 - colOffset] * Gy[1 + rowOffset][1 + colOffset];
        }
    }
    if (sumX < 0)      sumX = -sumX;
    if (sumX > 255)    sumX = 255;

    if (sumY < 0)      sumY = -sumY;
    if (sumY > 255)    sumY = 255;

    SUM = sumX + sumY;

    if (SUM > 255)    SUM = 255;

    orow = 255  - (unsigned char)(SUM);
    return ((unsigned char) orow);

}
