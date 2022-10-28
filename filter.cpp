#include "Pipeline.h"
#include <stdlib.h>

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

void Filter_SW(const unsigned char * Input,
	           unsigned char * Output)
{
  unsigned char * Temp = (unsigned char *) malloc(SCALED_FRAME_HEIGHT * OUTPUT_FRAME_WIDTH);
  Filter_horizontal_SW(Input, Temp);
  Filter_vertical_SW(Temp, Output);
  free(Temp);
}

void Filter_Horizontal_HW(const unsigned char * Input, unsigned char *Output){
	int X,Y,i;
	unsigned char input[7];
	unsigned int next_ip;
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
#pragma HLS unroll
				Sum+=Coefficients[i]*input[i];
			}
			Output[Y*OUTPUT_FRAME_WIDTH+X]=Sum>>8;
			for(i=0;i<6;i++){
#pragma HLS pipeline
				input[i]=input[i+1];
			}
			input[6]=next_ip;
		}

	}
}

void Filter_Vertical_HW(const unsigned char * Input, unsigned char *Output){
	int X,Y,i;
	unsigned char input[7];
	unsigned char next_ip;
	for (X=0;X<OUTPUT_FRAME_WIDTH;X++){
		for(i=0;i<FILTER_LENGTH;i++){
#pragma HLS unroll
			input[i]=Input[(i)*OUTPUT_FRAME_WIDTH+X];
		}
		for(Y=0;Y< OUTPUT_FRAME_HEIGHT;Y++){
#pragma HLS pipeline
			next_ip=Input[(Y+FILTER_LENGTH)*OUTPUT_FRAME_WIDTH+X];
			unsigned int Sum=0;
			for(i=0;i<FILTER_LENGTH;i++){
#pragma HLS unroll
				Sum+=Coefficients[i]*input[i];
			}
			Output[Y*OUTPUT_FRAME_WIDTH+X]=Sum>>8;
			for(i=0;i<6;i++){
#pragma HLS pipeline
				input[i]=input[i+1];
			}
			input[6]=next_ip;
		}


	}

}
void Filter_HW(const unsigned char * Input,
	           unsigned char * Output){

	  unsigned char _Temp[SCALED_FRAME_HEIGHT][OUTPUT_FRAME_WIDTH];
	  unsigned char *Temp = &_Temp[0][0];
	  Filter_Horizontal_HW(Input, Temp);
	  Filter_Vertical_HW(Temp, Output);

}
