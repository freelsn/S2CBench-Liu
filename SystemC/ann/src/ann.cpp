#include <systemc.h>


/* Synthesizable parameters */
#define NbitW 16   // bw of weights and bias
#define NbitIn 8   // bw of ANN input
#define NbitOut 8  // bw of ANN output

#define trunc_out 14 // Truncate this bits from the output of each layer,
// in order to place the decimal dot.


#define NumI 16 // Number of inputs
#define NumO 16 // Number of outputs
#define baseB 4 // Sqrt of NumI. It represents the side of the patches which the image is divided into

#define MAX_N 16

#ifdef NLAYER_4
    #define Nlayer 4
    const int NumN[Nlayer+1]={16,12,8,12,16};//4-layer+1 output layer. Editable
#else
    #define Nlayer 2
    const int NumN[Nlayer+1]={16,8,16};     // 2-Layer+1 output layer Editable
#endif

#ifdef DSE
    #define ATTR Cyber
    #define ARRAY_1 array=REG
    #define ARRAY_2 array=REG
    #define ARRAY_3 array=REG
    #define LOOP_1 unroll_times=all // 0, all
    #define LOOP_2 unroll_times=all // 0, all
    #define LOOP_3 unroll_times=all // 0, all
    #define LOOP_4 unroll_times=all // 0, all
    #define LOOP_5 unroll_times=all // 0, all
    #define LOOP_6 unroll_times=all // 0, all
    #define FUNC_1 func=inline
    #define FUNC_2 func=inline
#endif


template <const int N_In, const int N_N>
struct synth_layer_data
{
   int W[N_N][N_In] /* ATTR ARRAY_1 */;
   int b[N_N] /* ATTR ARRAY_2 */;
};


/* Layer types definitions */
#ifdef NLAYER_4
   typedef synth_layer_data<16,12> type_0;
   typedef synth_layer_data<12,8> type_1;
   typedef synth_layer_data<8,12> type_2;
   typedef synth_layer_data<12,16> type_3;
#else
   typedef synth_layer_data<16,8> type_0;
   typedef synth_layer_data<8,16> type_1;
#endif


#ifdef TRAIN
extern int *a[Nlayer-1]; //global variable for training
#endif

SC_MODULE(main)
{
  // Inputs
   sc_in< bool > clk;
   sc_in< bool > rst;

   sc_in< bool > run_in;
   sc_in< sc_int<NbitIn> > inputs[NumI];

   sc_in< bool > wr;
   sc_in< sc_int<NbitW> > wdata;

   // Outputs
   sc_out< sc_int<NbitOut> > outputs[NumO];
   sc_out< bool > run_out;


   /* Internal variables */
   int buff[Nlayer+1][MAX_N] /* ATTR ARRAY_3 */;

   /* Weight and bias declaration */

#ifdef NLAYER_4
   type_0 layer_0;
   type_1 layer_1;
   type_2 layer_2;
   type_3 layer_3;
#else
   type_0 layer_0;
   type_1 layer_1;
#endif


   /* Processes */
   /* A */
   void ann_cthread();


   /* L */
   template <class Lt>
   void layer(Lt &Wyb, int l);

   /* W */
   // void Wyb_cthread();
   template <class Lt>
   void write_Wyb(Lt &Wyb, int l);


   /* Constructors */
   SC_HAS_PROCESS(main);
   main(sc_module_name nm);

   /* Destructor */
   //~main();
};


/* Constructors */
main::main(sc_module_name nm) : sc_module(nm)
{
   // SC_CTHREAD(Wyb_cthread, clk.pos());
   //    reset_signal_is(rst,true);

   SC_CTHREAD(ann_cthread, clk.pos());
      reset_signal_is(rst,true);
}


// void ann::Wyb_cthread()
// {
//    /* Memories do not need initialization */

//    wait();

//    for(;;)
//    {
//       if(wr->read())
//       {
// #ifdef NLAYER_4
//          write_Wyb< type_0 >(layer_0, 0);
//          write_Wyb< type_1 >(layer_1, 1);
//          write_Wyb< type_2 >(layer_2, 2);
//          write_Wyb< type_3 >(layer_3, 3);
// #else
//          write_Wyb< type_0 >(layer_0, 0);
//          write_Wyb< type_1 >(layer_1, 1);
// #endif
//       }
//       else
//          wait();
//    }
// }

/* ATTR FUNC_1 */
template <class Lt>
void main::write_Wyb(Lt &Wyb, int l){

   /* Write weights first */
    /* ATTR LOOP_1 */
   for(int n=0; n<NumN[l+1]; n++)
   {
        /* ATTR LOOP_2 */
      for(int i=0; i<NumN[l]; i++)
      {
         Wyb.W[n][i] = wdata->read();
         wait();
      }
   }
   /* Write biases second */
   /* ATTR LOOP_3 */
   for(int n=0; n<NumN[l+1]; n++)
   {
      Wyb.b[n] = wdata->read();
      wait();
   }
}


/***************************
 **
 ** Main ANN SC_CThread
 **
 ***************************/
void main::ann_cthread(){


  /* Reset condition */

   wait();

   /* Main loop */
   for(;;)
   {
      if(wr->read())
      {
#ifdef NLAYER_4
         write_Wyb< type_0 >(layer_0, 0);
         write_Wyb< type_1 >(layer_1, 1);
         write_Wyb< type_2 >(layer_2, 2);
         write_Wyb< type_3 >(layer_3, 3);
#else
         write_Wyb< type_0 >(layer_0, 0);
         write_Wyb< type_1 >(layer_1, 1);
#endif
      }


      run_out->write(false);
      if (run_in->read())
      {

         /* 1.)  Read inputs to buffer */
         for(int i=0; i<NumN[0]; i++)
         {
            buff[0][i] = inputs[i]->read();
         }


         /* 2.) Calls to the layer function: */
#ifdef NLAYER_4
    layer< type_0 >(layer_0, 0);
    layer< type_1 >(layer_1, 1);
    layer< type_2 >(layer_2, 2);
    layer< type_3 >(layer_3, 3);
#else
    layer< type_0 >(layer_0, 0);
    layer< type_1 >(layer_1, 1);
#endif

     /* 3.) Write outputs */

#ifdef TRAIN
        /* ATTR LOOP_4 */
         for(int l=0; l<(Nlayer-1); l++)
         {
            for(int i=0; i<NumN[l+1]; i++)
               a[l][i] = buff[l+1][i];
         }
#endif
         /* ATTR LOOP_4 */
         for(int i=0; i<NumN[Nlayer]; i++)
         {
            int clipp = buff[Nlayer][i];
            if (clipp >= 1<<(NbitOut-1)) //Clipping
               outputs[i]->write((1<<(NbitOut-1))-1); //0x00007FFF for 16 bits;
            else if (clipp <= -1<<(NbitOut-1))
               outputs[i]->write(-1<<(NbitOut-1)); //0xFFFF8000 for 16 bits;
            else
               outputs[i]->write(clipp);

         }

         run_out->write(true);
      }
      wait();
   }
}

/* ATTR FUNC_2 */
template <class Lt>
void main::layer(Lt &Wyb, int l)
{
    /* ATTR LOOP_5 */
   for(int n=0; n<NumN[l+1]; n++) //Important: Order of nested loops for inputs and neurons
   {
      int acc = Wyb.b[n]; //bias
      /* ATTR LOOP_6 */
      for(int i=0; i<NumN[l]; i++)
      {
         acc += buff[l][i]*Wyb.W[n][i];
      }
      buff[l+1][n] = acc>>trunc_out;
   }
}
