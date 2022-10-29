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
//#pragma HLS dataflow
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
void Load(const unsigned char *Input_, hls::stream<unsigned char>& instream){
#pragma HLS dataflow
	for(int x=0;x<SCALED_FRAME_HEIGHT*SCALED_FRAME_WIDTH;x++)
		instream.write(Input_[x]);
}
static void Filter_Horizontal_HW_stream_LCS(hls::stream<unsigned char> & Input_, hls::stream<unsigned char> & Output_){
	int X,Y,i;
	unsigned char input[7];
	unsigned char next_ip;
#pragma HLS dataflow
	for (Y=0;Y<SCALED_FRAME_HEIGHT;Y++){
		for(i=0;i<FILTER_LENGTH-1;i++){
#pragma HLS unroll
			input[i]=Input_.read();
		}
		for(X=0;X< OUTPUT_FRAME_WIDTH;X++){
#pragma HLS pipeline
//			next_ip=Input[Y*SCALED_FRAME_WIDTH+FILTER_LENGTH+X];
			input[6]=Input_.read();

			unsigned int Sum=0;
			for(i=0;i<FILTER_LENGTH;i++){
				Sum+=Coefficients[i]*input[i];
			}
//			Output[Y*OUTPUT_FRAME_WIDTH+X]=Sum>>8;
			Output_.write(Sum>>8);
			for(i=0;i<6;i++){
#pragma HLS unroll
				input[i]=input[i+1];
			}
//			input[6]=next_ip;
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

static void Filter_Vertical_HW_stream(hls::stream<unsigned char>& Input,
		                    unsigned char * Output)
{
//#pragma HLS dataflow
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
static void Filter_Vertical_HW_stream_LCS(hls::stream<unsigned char>& Input_,
		hls::stream<unsigned char>& Output_){
#pragma HLS dataflow
	unsigned char Input_buf[7][OUTPUT_FRAME_WIDTH];
  int X, Y, i;
	for(int y=0;y<6;y++){
		for(int x=0;x<OUTPUT_FRAME_WIDTH;x++){
#pragma HLS pipeline
			Input_buf[y][x]=Input_.read();
		}
	}

		for(Y=0;Y<OUTPUT_FRAME_HEIGHT;Y++){
			for(X=0;X<OUTPUT_FRAME_WIDTH;X++){
#pragma HLS pipeline
				Input_buf[(Y+6)%7][X]=Input_.read();
				  unsigned int Sum = 0;
				  for (i = 0; i < FILTER_LENGTH; i++){
					Sum += Coefficients[i] *Input_buf[(i+(Y+6)%7+1)%7][X];
				  }
//				  Output[Y * OUTPUT_FRAME_WIDTH + X] = Sum >> 8;
				  Output_<<(Sum >> 8);
			}
		}
}
void Store(hls::stream<unsigned char>& Input_,unsigned char* Output_){
//#pragma HLS dataflow
	for(int Y=0;Y<OUTPUT_FRAME_HEIGHT;Y++){
		for(int X=0;X<OUTPUT_FRAME_WIDTH;X++){
			Output_[Y * OUTPUT_FRAME_WIDTH + X] =Input_.read();
		}
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


void Filter_HW(const unsigned char * Input_,
	           unsigned char * Output_)
{

//#pragma HLS DATAFLOW
//#define NO_SYNTH
#define NORMAL	//question 2
//#define STREAM	//QUESTION 3
//#define LCS		// question 4
#ifdef NO_SYNTH
  unsigned char * Temp = (unsigned char *) malloc(SCALED_FRAME_HEIGHT * OUTPUT_FRAME_WIDTH);
#endif

#ifdef NORMAL
    unsigned char temp[SCALED_FRAME_HEIGHT * OUTPUT_FRAME_WIDTH];
    unsigned char * Temp = &temp[0];
    Filter_Horizontal_HW(Input_, Temp);
  //  Filter_horizontal_SW(Input,Temp);
    Filter_Vertical_HW(Temp, Output_);
#endif

#ifdef STREAM
//#pragma HLS dataflow
  hls:: stream<unsigned char> Temp;
  Filter_Horizontal_HW_stream(Input_, Temp);
  Filter_Vertical_HW_stream(Temp, Output_);
#endif

#ifdef LCS
#pragma HLS dataflow
#pragma HLS stream
  hls:: stream<unsigned char> instream("input_stream");
  hls:: stream<unsigned char> outstream("output_stream");
  hls:: stream<unsigned char> Temp("temp_stream");
  Load(Input_,instream);
  Filter_Horizontal_HW_stream_LCS(instream, Temp);
  Filter_Vertical_HW_stream_LCS(Temp, outstream);
  Store(outstream,Output_);

#endif



#ifdef NO_SYNTH
  free(Temp);
#endif

}
