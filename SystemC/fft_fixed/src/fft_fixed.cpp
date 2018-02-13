#include "systemc.h"

#define SC_INCLUDE_FX
#include "math.h"

#ifdef DSE
    #define ATTR Cyber
    #define ARRAY_1 array=REG
    #define ARRAY_2 array=REG
    #define LOOP_1 unroll_times=all // 0, all
    #define LOOP_2 unroll_times=all // 0, all
    #define LOOP_3 unroll_times=all // 0, all
    #define LOOP_4 unroll_times=all // 0, all
    #define LOOP_5 unroll_times=all // 0, all
    #define LOOP_6 unroll_times=all // 0, all
    #define LOOP_7 unroll_times=all // 0, all
#endif

struct main: sc_module {

// Inputs
  sc_in_clk clk;
  sc_in<bool> rst;

  sc_in<sc_fixed<32,16, SC_RND, SC_SAT> > in_real;
  sc_in<sc_fixed<32,16, SC_RND, SC_SAT> > in_imag;
  sc_in<bool> data_valid;
  sc_in<bool> data_ack;

// Outputs
  sc_out<sc_fixed<32,16, SC_RND, SC_SAT> > out_real;
  sc_out<sc_fixed<32,16, SC_RND, SC_SAT> > out_imag;
  sc_out<bool> data_req;
  sc_out<bool> data_ready;



  SC_CTOR(main){

      SC_CTHREAD(entry, clk.pos() );
      reset_signal_is(rst, false);

    };

// Destructor
  ~main(){};


void entry(void);


};


void main::entry()
{

// Variables declaration
  //float sample[16][2];  // Note: Float is not synthesizable
 sc_fixed<32,16, SC_RND, SC_SAT> sample[16][2] /* ATTR ARRAY_1 */;
 unsigned int index;

// Reset state -- should be executable in a single clock cycle
   // for (unsigned int i = 0; i < 16; i++) {
   //  for(unsigned h=0; h <2; h++){
   //       sample[i][h] = 0;
   //      }
   //  }
   index = 0;

wait();

// Main computational loop
  while(true)
  {

    data_req.write(false);
    data_ready.write(false);
    index = 0;


    //Reading in the Samples
    /* ATTR LOOP_1 */
      while( index < 16 )
      {


       data_req.write(true);
       do {     wait();

    } while (data_valid.read() == true);

         sample[index][0] = in_real.read();
         sample[index][1] = in_imag.read();
         index++;

     data_req.write(false);
       wait();
      }
      index = 0;


      //////////////////////////////////////////////////////////////////////////
      ///  Computation - 1D Complex DFT In-Place DIF Computation Algorithm  ////
      //////////////////////////////////////////////////////////////////////////

      //Size of FFT, N = 2**M
       unsigned int N, M, len ;
       //float theta;   // Note: float is non-synthesizble as indicated in the SystemC synthesizable subset draft 1.3
       //float  W[7][2], w_real, w_imag, w_rec_real, w_rec_imag, w_temp;
       const sc_fixed<32,4, SC_RND, SC_SAT> theta=0.392699, w_real=0.92388, w_imag=-0.382683;
       sc_fixed<32,16, SC_RND, SC_SAT>  W[7][2] /* ATTR ARRAY_2 */,w_rec_real, w_rec_imag, w_temp;


       //Initialize
       M = 4; N = 16;
       len = N/2;
       //     theta = 8*atan(1.0)/N;  // subsitutited by constant in declaration


       //Calculate the W-values recursively
       //  w_real =  cos(theta);    // substituted by constant in declaration
       //  w_imag =  -sin(theta);   // substituted by contant in declaration

       //cout << endl << "theta:" << theta << ", w_real:" << w_real << ", w_imag:" << w_imag;

        w_rec_real = 1;
        w_rec_imag = 0;

        index = 0;
    /* ATTR LOOP_2 */
        while(index < len-1)
        {
           w_temp = w_rec_real*w_real - w_rec_imag*w_imag;
           w_rec_imag =  w_rec_real*w_imag + w_rec_imag*w_real;
           w_rec_real = w_temp;
           W[index][0] = w_rec_real;
           W[index][1] = w_rec_imag;
           index++;
        }


  //     float tmp_real, tmp_imag, tmp_real2, tmp_imag2;
    sc_fixed<32,16, SC_RND, SC_SAT>  tmp_real, tmp_imag, tmp_real2, tmp_imag2;
       unsigned int stage, i, j,index2, windex, incr;

      //Begin Computation
       stage = 0;

       len = N;
       incr = 1;

    /* ATTR LOOP_3 */
       while (stage < M)
       {
        len = len/2;

        //First Iteration :  With No Multiplies
          i = 0;


    /* ATTR LOOP_4 */
          while(i < N)
          {
             index =  i; index2 = index + len;

             tmp_real = sample[index][0] + sample[index2][0];
             tmp_imag = sample[index][1] + sample[index2][1];

             sample[index2][0] = sample[index][0] - sample[index2][0];
             sample[index2][1] = sample[index][1] - sample[index2][1];

             sample[index][0] = tmp_real;
             sample[index][1] = tmp_imag;


             i = i + 2*len;

          }


        //Remaining Iterations: Use Stored W
         j = 1; windex = incr - 1;
    /* ATTR LOOP_5 */
         while (j < len) // This loop executes N/2 times at first stage, .. once at last stage.
         {
            i = j;
    /* ATTR LOOP_6 */
            while (i < N)
            {
              index = i;
              index2 = index + len;

              tmp_real = sample[index][0] + sample[index2][0];
              tmp_imag = sample[index][1] + sample[index2][1];
              tmp_real2 = sample[index][0] - sample[index2][0];
              tmp_imag2 = sample[index][1] - sample[index2][1];

              sample[index2][0] = tmp_real2*W[windex][0] - tmp_imag2*W[windex][1];
              sample[index2][1] = tmp_real2*W[windex][1] + tmp_imag2*W[windex][0];

              sample[index][0] = tmp_real;
              sample[index][1] = tmp_imag;

              i = i + 2*len;

            }
            windex = windex + incr;
            j++;
         }
          stage++;
          incr = 2*incr;
       }

     //////////////////////////////////////////////////////////////////////////

     //Writing out the normalized transform values in bit reversed order
      sc_uint<4> bits_i;
      sc_uint<4> bits_index;
      bits_i = 0;
      i = 0;

      //    cout << "Writing the transform values..." << endl;
    /* ATTR LOOP_7 */
      while( i < 16)
      {
       bits_i = i;
       bits_index[3]= bits_i[0];
       bits_index[2]= bits_i[1];
       bits_index[1]= bits_i[2];
       bits_index[0]= bits_i[3];
       index = bits_index;

       out_real.write(sample[index][0]);
       out_imag.write(sample[index][1]);
       data_ready.write(true);


       do {
     wait();
       } while ( !(data_ack.read() == true) );

       data_ready.write(false);
       i++;
       wait();
      }
      index = 0;
      // cout << "Done..." << endl;
  }
 }
