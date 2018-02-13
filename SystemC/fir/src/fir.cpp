#include "systemc.h"

#define FILTER_TAPS 10
#define MAX 255

#ifdef DSE
    #define ATTR Cyber
    #define ARRAY_1 array=REG
    #define ARRAY_2 array=REG
    #define LOOP_1 unroll_times=all // 9
    #define LOOP_2 unroll_times=all // 9
#endif

SC_MODULE (main) {
public:

   // Inputs
   sc_in_clk clk;
   sc_in<bool> rst;
   sc_in<sc_uint<8> > in_data[FILTER_TAPS] ;
   sc_in<sc_uint<16> > coeff[FILTER_TAPS] ;

   // Output
   sc_out< sc_uint<8> > filter_output ;


   /* F */
   void fir_main ( void );
   sc_uint<8> filter(sc_uint<8> *, sc_uint<16> *);


   // Constructor
 SC_CTOR (main) {

       SC_CTHREAD (fir_main, clk.pos() );
       reset_signal_is(rst, false) ;
       sensitive << clk.pos();

   }

   // Destructor
   ~main() {}
};

//  Main thread
void main::fir_main ( void ) {

   // Variables declaration
    sc_uint<8> filter_output_function ;
    sc_uint<8> in_data_read[9] /* ATTR ARRAY_1 */;
    sc_uint<16> coeff_read[9] /* ATTR ARRAY_2 */;
    int i;

    // Reset state - should be executable in 1 clock cycle

     wait();


   // Main thread
   while (1) {
    /* ATTR LOOP_1 */
     for(i=0;i<FILTER_TAPS-1;i++){
       in_data_read[i] = in_data[i].read();
           coeff_read[i] = coeff[i].read();
    }

     // Filter function
     filter_output_function = filter(in_data_read, coeff_read);


     filter_output.write(filter_output_function) ;
     wait();
}


}

// Filter function
sc_uint<8> main::filter(
    sc_uint<8>  *ary,
    sc_uint<16>  *coeff)
{
    int sop=0;
    sc_uint <8> filter_result ;
    int i ;


    // Sum of product (SOP) generation
    /* ATTR LOOP_2 */
    for(i=0;i<FILTER_TAPS-1;i++){
        sop += ary[i] * coeff[i] ;

 }

    // Sign adjustment and rounding to sc_unit <8>)
    if ( sop < 0 ){
        sop = 0 ;
    }

    filter_result = sc_uint<8>(sop);

    return filter_result;
}
