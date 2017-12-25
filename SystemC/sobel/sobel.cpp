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

const char Gx[3][3] /* ATTR ARRAY_1 */ = {{-1, -2, -1},{ 0,  0,  0},{ +1, 2, +1}};
const char Gy[3][3] /* ATTR ARRAY_2 */ = {{-1, 0, 1},{-2, 0, 2},{-1, 0, 1}};

SC_MODULE(main)
{
    sc_in_clk CLOCK;
    sc_in<bool> rst;

    sc_in<sc_uint<8> > input_row[SIZE_BUFFER];

    sc_out<sc_uint<8> > output_row;

    unsigned char line_buffer[SIZE_BUFFER][SIZE_BUFFER] /* ATTR ARRAY_3 */;

    void sobel_main();
    unsigned char sobel_filter(unsigned char input_row_t[SIZE_BUFFER]);
    int sum_check(int sum);

    SC_CTOR(main)
    {
        SC_CTHREAD(sobel_main, CLOCK.pos());
        reset_signal_is(rst, false);
    }

    ~main() {}
};

void main::sobel_main()
{
    unsigned char input_row_read[SIZE_BUFFER] /* ATTR ARRAY_4 */;
    unsigned char output_row_write;
    int i, j;
    int row, col;
    int index;
    for (i = 0; i < SIZE_BUFFER; i++)
        for (j = 0; j < SIZE_BUFFER; j++)
            line_buffer[i][j] = 0;

    wait();

    while(1)
    {
        for (row = 0; row < ROWS; row++)
        {
            for (col = 1; col < COLS; col++)
            {   
                /* ATTR LOOP_1 */
                for (i = col - 1; i < col + 1; i++)
                {
                    index = i - col + 1;
                    input_row_read[index] = input_row[index].read();
                }

                output_row_write = sobel_filter(input_row_read);
                output_row.write(output_row_write);

                wait();
            }
        }
    }
}

/* ATTR FUNC_1 */
unsigned char main::sobel_filter(unsigned char input_row_t[SIZE_BUFFER])
{
    int x, y;
    int row_offset, col_offset;
    int sum_x, sum_y;
    int sum;

    // Shift line buffer by one row
    /* ATTR LOOP_2 */
    for (y = SIZE_BUFFER - 1; y > 0; y--)
    {
        /* ATTR LOOP_3 */
        for (x = 0; x < SIZE_BUFFER; x++)
        {
            line_buffer[y][x] = line_buffer[y-1][x];
        }
    }
    // Reading new data into the line buffer
    /* ATTR LOOP_4 */
    for (x = 0; x < SIZE_BUFFER; x++)
        line_buffer[0][x] = input_row_t[x];

    x = 1;
    y = 1;
    sum_x = 0;
    sum_y = 0;

    // Convolution starts
    /* ATTR LOOP_5 */
    for (row_offset = -1; row_offset <= 1; row_offset++)
    {
        /* ATTR LOOP_6 */
        for (col_offset = -1; col_offset <= 1; col_offset++)
        {
            sum_x += line_buffer[y - row_offset][x + col_offset] * Gx[1 + row_offset][1 + col_offset];
            sum_y += line_buffer[y - row_offset][x + col_offset] * Gy[1 + row_offset][1 + col_offset];
        }
    }

    sum = sum_check(sum_x) + sum_check(sum_y);
    sum = sum_check(sum);

    sum = 255 - sum;

    return (unsigned char)sum;
}

/* ATTR FUNC_2 */
int main::sum_check(int sum)
{
    if (sum > 255)
        sum = 255;
    else if (sum < 0)
        sum = -sum;
    else
        sum = sum;

    return sum;
}
