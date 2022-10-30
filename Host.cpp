#define CL_HPP_CL_1_2_DEFAULT_BUILD
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY 1
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#include <CL/cl2.hpp>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <vector>

#include "Utilities.h"
#include "Pipeline.h"
#include "EventTimer.h"

#define STAGES (4)

// unsigned char * Data[STAGES + 1];
#define FRAME_SIZE INPUT_FRAME_SIZE
//#define FRAMES 200

#define MAX_OUTPUT_SIZE 5000*1024
unsigned int Frame_size[4]={SCALED_FRAME_SIZE, OUTPUT_FRAME_SIZE,OUTPUT_FRAME_SIZE,OUTPUT_FRAME_SIZE};


int main(int argc, char** argv)
{
    
    unsigned char *Input_data = (unsigned char *)malloc(FRAMES * FRAME_SIZE);
    unsigned char *Temp_data[STAGES - 1];
    unsigned char *Output_data = (unsigned char *)malloc(MAX_OUTPUT_SIZE);
    Load_data(Input_data);
    int total_size=0;
     int Size = 0;

    size_t input_bytes = SCALED_FRAME_SIZE * sizeof(unsigned char);
    size_t output_bytes = OUTPUT_FRAME_SIZE * sizeof(unsigned char);


    for (int Stage = 0; Stage < STAGES - 1; Stage++)
    {
      Temp_data[Stage] = (unsigned char *)malloc(Frame_size[Stage]);
      if (Temp_data[Stage] == NULL)
        Exit_with_error("malloc failed at main for Temp_data");
    }



    //// Step 1: Initialize the OpenCL environment

    EventTimer timer1;
    cl_int err;
    std::string binaryFile = argv[1];
    unsigned fileBufSize;
    std::vector<cl::Device> devices = get_xilinx_devices();
    devices.resize(1);
    cl::Device device = devices[0];
    cl::Context context(device, NULL, NULL, NULL, &err);
    char *fileBuf = read_binary_file(binaryFile, fileBufSize);
    cl::Program::Binaries bins{{fileBuf, fileBufSize}};
    cl::Program program(context, devices, bins, NULL, &err);
    cl::CommandQueue q(context, device, CL_QUEUE_PROFILING_ENABLE|CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &err);
    cl::Kernel krnl_filter_HW(program, "Filter_HW", &err);
    //  Done
    

    // ------------------------------------------------------------------------------------
    // Step 2: Create buffers and initialize test values
    // ------------------------------------------------------------------------------------

    cl::Buffer input_buf;
    cl::Buffer output_buf;
   
    input_buf = cl::Buffer(context, CL_MEM_READ_ONLY,input_bytes, NULL, &err);
    output_buf = cl::Buffer(context, CL_MEM_READ_ONLY, output_bytes, NULL, &err);

    // unsigned char *input_arr[NUM_MAT];
    // unsigned char *output_arr[NUM_MAT];
    Temp_data[0] = (unsigned char*)q.enqueueMapBuffer(input_buf, CL_TRUE, CL_MAP_WRITE, 0, input_bytes);
    Temp_data[1] = (unsigned char*)q.enqueueMapBuffer(output_buf, CL_TRUE, CL_MAP_READ, 0, output_bytes);
    // std::cout<<"After start"<<std::endl;


   timer1.add("App_Time");
  std::vector<cl::Event> write_events;
  std::vector<cl::Event> exec_events, read_events;
  cl::Event write_ev, exec_ev, read_ev;
  for (int Frame = 0; Frame < FRAMES; Frame++)
  {
  
      Scale_SW(Input_data + Frame * FRAME_SIZE, Temp_data[0]);
        // std::cout<<"After scale"<<std::endl;



      krnl_filter_HW.setArg(0, input_buf);
      krnl_filter_HW.setArg(1, output_buf);

      if(Frame==0)
          q.enqueueMigrateMemObjects({input_buf}, 0 /* 0 means from host*/, NULL, &write_ev);
      else
      {
          q.enqueueMigrateMemObjects({input_buf}, 0 /* 0 means from host*/, &write_events,
          &write_ev);
          write_events.pop_back();
      }
      write_events.push_back(write_ev);
      q.enqueueTask(krnl_filter_HW, &write_events, &exec_ev);
      exec_events.push_back(exec_ev);
      q.enqueueMigrateMemObjects({output_buf}, CL_MIGRATE_MEM_OBJECT_HOST, &exec_events, &read_ev);
      read_events.push_back(read_ev);
      read_ev.wait();
        // std::cout<<"After filter"<<std::endl;

      Differentiate_SW(Temp_data[1], Temp_data[2]);
        // std::cout<<"After differentiate"<<std::endl;

      Size = Compress_SW(Temp_data[2], Output_data);
      Store_data("../data/Output.bin",Output_data , Size);
     //total_size+= Compress_SW(Temp_data[2], Output_data + total_size);
            // std::cout<<"After compress"<<std::endl;
      
      std::cout<<Frame<<"\tsize="<<Size<<std::endl;
      
    }
q.finish();

    



  
    std::cout<<"before timer.finish"<<std::endl;

  timer1.finish();    
  std::cout << "--------------- Total time ---------------"
    << std::endl;
    timer1.print();
  // complete executation 
  // for (int i = 0; i <= STAGES; i++)
  //   free(Data[i]);
  // complete free data 

  puts("Application completed successfully.");
  return 0;
}
