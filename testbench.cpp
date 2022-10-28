#include <iostream>
#include <cstdlib>
#include <chrono>
#include "Pipeline.h"

class stopwatch
{
public:
  double total_time, calls;
  std::chrono::time_point<std::chrono::high_resolution_clock> start_time, end_time;
  stopwatch() : total_time(0), calls(0){};

  inline void reset()
  {
    total_time = 0;
    calls = 0;
  }

  inline void start()
  {
    start_time = std::chrono::high_resolution_clock::now();
    calls++;
  };

  inline void stop()
  {
    end_time = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
    total_time += static_cast<double>(elapsed);
  };

  // return latency in ns
  inline double latency()
  {
    return total_time;
  };

  // return latency in ns
  inline double avg_latency()
  {
    return (total_time / calls);
  };
};

void Randomize_matrix(unsigned char * Input)
{
  for (int Y = 0; Y < SCALED_FRAME_HEIGHT; Y++)
    for (int X = 0; X < SCALED_FRAME_WIDTH ; X++)
      Input[Y * SCALED_FRAME_WIDTH + X] = rand();
}

bool Compare_filter(unsigned char *OP_SW, unsigned char *OP_HW){
	bool Equal=true;
	for(int i=0;i<OUTPUT_FRAME_HEIGHT*OUTPUT_FRAME_WIDTH;i++){
		if ((*(OP_SW+i))!=(*(OP_HW+i))){
			{	Equal=false;
			  	break;
			}
		}

	}
	return Equal;
}


void filter_gold(unsigned char * Input,
	           unsigned char * Output){
	Filter_SW(Input,Output);

	}

void filter_hw(unsigned char * Input,
	           unsigned char * Output){
	Filter_HW(Input,Output);

}

int main()
{
  unsigned char Input_Test[SCALED_FRAME_HEIGHT*SCALED_FRAME_WIDTH];
  unsigned char OP_SW[OUTPUT_FRAME_HEIGHT*OUTPUT_FRAME_WIDTH];
  unsigned char OP_HW[OUTPUT_FRAME_HEIGHT*OUTPUT_FRAME_WIDTH];
  Randomize_matrix(Input_Test);
  filter_gold(Input_Test,OP_SW);
  filter_hw(Input_Test,OP_HW);
  bool Equal=Compare_filter(OP_SW,OP_HW);
  std::cout << "TEST " << (Equal ? "PASSED" : "FAILED") << std::endl;
  //return (Equal ? 1 : 0);
}
