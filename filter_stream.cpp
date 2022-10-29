#include "Pipeline.h"
#include <stdlib.h>
#include  <hls_stream.h>

static unsigned Coefficients[] = {2, 15, 62, 98, 62, 15, 2};

void Filter_horizontal_SW(const unsigned char * Input,
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
void Filter_Horizontal_HW(const unsigned char * Input, unsigned char *Output){
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
void Filter_Horizontal_HW_stream(const unsigned char *Input, hls::stream<unsigned char> & Output){
	int X,Y,i;
	unsigned char input[7];
#pragma HLS DATAFLOW
	unsigned char local_coeff[7];
	for(i=0;i<FILTER_LENGTH;i++){
#pragma HLS unroll
		local_coeff[i]=Coefficients[i];
	}
	unsigned char next_ip;

	for (Y=0;Y<SCALED_FRAME_HEIGHT;Y++){
			input[0]=Input[Y*SCALED_FRAME_WIDTH+0];
			input[1]=Input[Y*SCALED_FRAME_WIDTH+1];
			input[2]=Input[Y*SCALED_FRAME_WIDTH+2];
			input[3]=Input[Y*SCALED_FRAME_WIDTH+3];
			input[4]=Input[Y*SCALED_FRAME_WIDTH+4];
			input[5]=Input[Y*SCALED_FRAME_WIDTH+5];
//			input[6]=Input[Y*SCALED_FRAME_WIDTH+6];
		for(X=0;X< OUTPUT_FRAME_WIDTH;X++){
#pragma HLS pipeline
			input[6]=Input[Y*SCALED_FRAME_WIDTH+FILTER_LENGTH+X-1];
			unsigned int Sum=0;
				Sum+=Coefficients[0]*input[0];
				Sum+=Coefficients[1]*input[1];
				Sum+=Coefficients[2]*input[2];
				Sum+=Coefficients[3]*input[3];
				Sum+=Coefficients[4]*input[4];
				Sum+=Coefficients[5]*input[5];
				Sum+=Coefficients[6]*input[6];
//			Output[Y*OUTPUT_FRAME_WIDTH+X]=Sum>>8;
			Output.write(Sum>>8);
			for(i=0;i<FILTER_LENGTH-1;i++){
#pragma HLS unroll
				input[i]=input[i+1];
			}
//			input[6]=next_ip;
		}

	}
}
void Filter_vertical_SW(const unsigned char * Input,
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

void Filter_Vertical_HW(const unsigned char * Input,
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

void Filter_Vertical_HW_stream(hls::stream<unsigned char>& Input,
		                    unsigned char * Output)
{
//	unsigned char Input_buf[SCALED_FRAME_HEIGHT * OUTPUT_FRAME_WIDTH];
	unsigned char Input_buf[7][OUTPUT_FRAME_WIDTH];
	unsigned char local_coeff[7];
#pragma HLS DATAFLOW
	for(int i=0;i<FILTER_LENGTH;i++){
#pragma HLS unroll
		local_coeff[i]=Coefficients[i];
	}
  int X, Y, i;
  int lastrow=6;
	for(int y=0;y<6;y++){
//#pragma HLS pipeline
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
					Sum += local_coeff[0] *Input_buf[(0+(Y+6)%7+1)%7][X];
					Sum += local_coeff[1] *Input_buf[(1+(Y+6)%7+1)%7][X];
					Sum += local_coeff[2] *Input_buf[(2+(Y+6)%7+1)%7][X];
					Sum += local_coeff[3] *Input_buf[(3+(Y+6)%7+1)%7][X];
					Sum += local_coeff[4] *Input_buf[(4+(Y+6)%7+1)%7][X];
					Sum += local_coeff[5] *Input_buf[(5+(Y+6)%7+1)%7][X];
					Sum += local_coeff[6] *Input_buf[(6+(Y+6)%7+1)%7][X];
				  Output[Y * OUTPUT_FRAME_WIDTH + X] = Sum >> 8;
				  //Input_buf[i][j]=Input_buf[i+1][j];
				}
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
#pragma HLS stream
#pragma HLS DATAFLOW
	//#define NO_SYNTH
//#define NORMAL	//question 2
#ifdef NO_SYNTH
  unsigned char * Temp = (unsigned char *) malloc(SCALED_FRAME_HEIGHT * OUTPUT_FRAME_WIDTH);
#endif
  hls:: stream<unsigned char> Temp;
  Filter_Horizontal_HW_stream(Input, Temp);
//  Filter_horizontal_SW(Input,Temp);
  Filter_Vertical_HW_stream(Temp, Output);



#ifdef NO_SYNTH
  free(Temp);
#endif

}
