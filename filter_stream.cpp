#include "Pipeline.h"
#include <stdlib.h>
#include  <hls_stream.h>

static unsigned Coefficients[] = {2, 15, 62, 98, 62, 15, 2};

static void Filter_horizontal_SW(const unsigned char * Input,
		                      unsigned char * Output)
{
  int X, Y, i;
  for (Y = 0; Y < SCALED_FRAME_HEIGHT; Y++)
    for (X = 0; X < OUTPUT_FRAME_WIDTH; X++)
    {
      unsigned int Sum = 0;
      for (i = 0; i < FILTER_LENGTH; i++)
        Sum += Coefficients[i] * Input[Y * SCALED_FRAME_WIDTH + X + i];
      Output[Y * OUTPUT_FRAME_WIDTH + X] = Sum >> 8;
    }
}
static void Filter_Horizontal_HW(const unsigned char * Input, unsigned char *Output){
	int X,Y,i;
	unsigned char input[7];
	unsigned char next_ip;
	for (Y=0;Y<SCALED_FRAME_HEIGHT;Y++){
		for(i=0;i<FILTER_LENGTH;i++){
#pragma HLS unroll
			input[i]=Input[Y*SCALED_FRAME_WIDTH+i];
		}
		for(X=0;X< OUTPUT_FRAME_WIDTH;X++){
#pragma HLS pipeline
			next_ip=Input[Y*SCALED_FRAME_WIDTH+FILTER_LENGTH+X];
			unsigned int Sum=0;
			for(i=0;i<FILTER_LENGTH;i++){
				Sum+=Coefficients[i]*input[i];
			}
			Output[Y*OUTPUT_FRAME_WIDTH+X]=Sum>>8;
			for(i=0;i<6;i++){
#pragma HLS unroll
				input[i]=input[i+1];
			}
			input[6]=next_ip;
		}

	}
}
static void Filter_Horizontal_HW_stream(const unsigned char *Input, hls::stream<unsigned char> & Output){
	int X,Y,i;
	unsigned char input[7];
	unsigned char next_ip;

	for (Y=0;Y<SCALED_FRAME_HEIGHT;Y++){
		for(i=0;i<FILTER_LENGTH;i++){
#pragma HLS unroll
			input[i]=Input[Y*SCALED_FRAME_WIDTH+i];
		}
		for(X=0;X< OUTPUT_FRAME_WIDTH;X++){
#pragma HLS pipeline
			next_ip=Input[Y*SCALED_FRAME_WIDTH+FILTER_LENGTH+X];
			unsigned int Sum=0;
			for(i=0;i<FILTER_LENGTH;i++){
				Sum+=Coefficients[i]*input[i];
			}
//			Output[Y*OUTPUT_FRAME_WIDTH+X]=Sum>>8;
			Output.write(Sum>>8);
			for(i=0;i<6;i++){
#pragma HLS unroll
				input[i]=input[i+1];
			}
			input[6]=next_ip;
		}

	}
}
static void Filter_vertical_SW(const unsigned char * Input,
		                    unsigned char * Output)
{
  int X, Y, i;
  for (Y = 0; Y < OUTPUT_FRAME_HEIGHT; Y++)
    for (X = 0; X < OUTPUT_FRAME_WIDTH; X++)
    {
      unsigned int Sum = 0;
      for (i = 0; i < FILTER_LENGTH; i++)
        Sum += Coefficients[i] * Input[(Y + i) * OUTPUT_FRAME_WIDTH + X];
      Output[Y * OUTPUT_FRAME_WIDTH + X] = Sum >> 8;
    }
}

static void Filter_Vertical_HW(const unsigned char * Input,
		                    unsigned char * Output)
{
  int X, Y, i;
  unsigned char buffer[7];
  for (X = 0; X < OUTPUT_FRAME_WIDTH; X++){
	  for(int j=0;j<FILTER_LENGTH-1;j++){
#pragma HLS unroll
		  buffer[j]=Input[j*OUTPUT_FRAME_WIDTH+X];
	  }
    for (Y = 0; Y < OUTPUT_FRAME_HEIGHT; Y++)
    {
#pragma HLS pipeline
	    buffer[6]=Input[(Y+6)*OUTPUT_FRAME_WIDTH+X];

      unsigned int Sum = 0;
      for (i = 0; i < FILTER_LENGTH; i++){
        Sum += Coefficients[i] *buffer[i] ;
      }
      Output[Y * OUTPUT_FRAME_WIDTH + X] = Sum >> 8;

	  for(int j=0;j<FILTER_LENGTH-1;j++){
#pragma HLS unroll
		  buffer[j]=buffer[j+1];
	  }
    }

  }

}
//static void Filter_Vertical_HW_stream(hls::stream<unsigned char>& Input,
//		                    unsigned char * Output)
//{
//	unsigned char Input_buf[SCALED_FRAME_HEIGHT * OUTPUT_FRAME_WIDTH];
//	for(int y=0;y<SCALED_FRAME_HEIGHT;y++){
//		for(int x=0;x<OUTPUT_FRAME_WIDTH;x++){
//			Input_buf[y*OUTPUT_FRAME_WIDTH+x]=Input.read();
//		}
//	}
//
//  int X, Y, i;
//  unsigned char buffer[7];
//
//  for (X = 0; X < OUTPUT_FRAME_WIDTH; X++){
//
//		  for(int j=0;j<FILTER_LENGTH-1;j++){
//			#pragma HLS unroll
//			  buffer[j]=Input_buf[j*OUTPUT_FRAME_WIDTH+X];
//		  }
//		for (Y = 0; Y <OUTPUT_FRAME_HEIGHT; Y++)
//		{
//			#pragma HLS pipeline
//			buffer[6]=Input_buf[(Y+6)*OUTPUT_FRAME_WIDTH+X];
//
//		  unsigned int Sum = 0;
//		  for (i = 0; i < FILTER_LENGTH; i++){
//			Sum += Coefficients[i] *buffer[i] ;
//		  }
//		  Output[Y * OUTPUT_FRAME_WIDTH + X] = Sum >> 8;
//
//		  for(int j=0;j<FILTER_LENGTH-1;j++){
//	#pragma HLS unroll
//			  buffer[j]=buffer[j+1];
//		  }
//		}
//  }
//
//}

static void Filter_Vertical_HW_stream(hls::stream<unsigned char>& Input,
		                    unsigned char * Output)
{
//	unsigned char Input_buf[SCALED_FRAME_HEIGHT * OUTPUT_FRAME_WIDTH];
	unsigned char Input_buf[7][OUTPUT_FRAME_WIDTH];
  int X, Y, i;
  int lastrow=6;
	for(int y=0;y<6;y++){
		for(int x=0;x<OUTPUT_FRAME_WIDTH;x++){
//#pragma HLS unroll
#pragma HLS pipeline
			Input_buf[y][x]=Input.read();
		}
	}

		for(Y=0;Y<OUTPUT_FRAME_HEIGHT;Y++){

			for(X=0;X<OUTPUT_FRAME_WIDTH;X++){
#pragma HLS pipeline
				Input_buf[(Y+6)%7][X]=Input.read();
				  unsigned int Sum = 0;
				  for (i = 0; i < FILTER_LENGTH; i++){
					Sum += Coefficients[i] *Input_buf[(i+(Y+6)%7+1)%7][X];
				  }
				  Output[Y * OUTPUT_FRAME_WIDTH + X] = Sum >> 8;
			}
//			for(i=0;i<6;i++){
//#pragma HLS unroll
//				for(int j=0;j<OUTPUT_FRAME_WIDTH;j++){
//					Input_buf[i][j]=Input_buf[i+1][j];
//				}
//			}
		}
}


void Filter_SW(const unsigned char * Input,
	           unsigned char * Output)
{
  unsigned char * Temp = (unsigned char *) malloc(SCALED_FRAME_HEIGHT * OUTPUT_FRAME_WIDTH);
  Filter_horizontal_SW(Input, Temp);
  Filter_vertical_SW(Temp, Output);
  free(Temp);

}


void Filter_HW(const unsigned char * Input,
	           unsigned char * Output)
{
//#pragma HLS stream
//#pragma HLS DATAFLOW
	//#define NO_SYNTH
//#define NORMAL	//question 2
#ifdef NO_SYNTH
  unsigned char * Temp = (unsigned char *) malloc(SCALED_FRAME_HEIGHT * OUTPUT_FRAME_WIDTH);
#endif

#ifdef NORMAL
    unsigned char temp[SCALED_FRAME_HEIGHT * OUTPUT_FRAME_WIDTH];
    unsigned char * Temp = &temp[0];
    Filter_Horizontal_HW(Input, Temp);
  //  Filter_horizontal_SW(Input,Temp);
    Filter_Vertical_HW(Temp, Output);
#else

  hls:: stream<unsigned char> Temp;
  Filter_Horizontal_HW_stream(Input, Temp);
//  Filter_horizontal_SW(Input,Temp);
  Filter_Vertical_HW_stream(Temp, Output);
#endif



#ifdef NO_SYNTH
  free(Temp);
#endif

}
