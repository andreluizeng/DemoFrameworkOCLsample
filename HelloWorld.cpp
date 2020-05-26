/****************************************************************************************************************************************************
* Copyright 2020 NXP
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*    * Redistributions of source code must retain the above copyright notice,
*      this list of conditions and the following disclaimer.
*
*    * Redistributions in binary form must reproduce the above copyright notice,
*      this list of conditions and the following disclaimer in the documentation
*      and/or other materials provided with the distribution.
*
*    * Neither the name of the NXP. nor the names of
*      its contributors may be used to endorse or promote products derived from
*      this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
****************************************************************************************************************************************************/

// OpenCL 1.2 project

#include "HelloWorld.hpp"
#include <FslBase/Log/Log3Fmt.hpp>
#include <FslBase/Exceptions.hpp>
#include <CL/cl.h>

#include <sys/time.h>
#include <time.h>

#define CL_ERROR !CL_SUCCESS

#define TRUE 	1
#define FALSE 	0

namespace Fsl
{
	namespace
	{
	}

	struct kernel_src_str
	{
		char *src;
		size_t size;
	};

	cl_platform_id 	platform_id;
	cl_device_id 	device_id;
	cl_context 	context;
	cl_command_queue cq;
	cl_program 	program;
	cl_kernel 	kernel;
	cl_mem helloworld_mem_input = NULL;
	cl_mem helloworld_mem_output = NULL;

	struct timeval start, end;
	long mtime, seconds, useconds;    

	cl_int LoadKernelSource (char *filename, kernel_src_str *kernel);
	cl_int BuildProgram (cl_program *program, cl_device_id *device_id, cl_context context, kernel_src_str *kernel);
	cl_int PrintInfo (cl_platform_id platform_id, cl_device_id device_id);
	cl_int Init (cl_platform_id *platform_id, cl_device_id *device_id, cl_context *context, cl_command_queue *cq);
	cl_int LoadKernelSource (char *filename, kernel_src_str *kernel_src);


	HelloWorld::HelloWorld(const DemoAppConfig& config)
	: DemoAppOpenCL(config)
	{
	}


	HelloWorld::~HelloWorld()
	{
	}


	void HelloWorld::Run()
	{
		struct kernel_src_str kernel_str;

		int dimension = 1;
		size_t global = 3840*2160 * 4;
		size_t local = 1024;

		int size;
		char *input_data_buffer;
		char *output_data_buffer;

		cl_int ret;


		size = global;


		input_data_buffer = (char *) malloc (sizeof (char) * size);
		if (! input_data_buffer)
		{
			printf ("\nFailed to allocate input data buffer memory\n");
			return;
		}

		output_data_buffer = (char *) malloc (sizeof (char) * size);
		if (! output_data_buffer)
		{
			printf ("\nFailed to allocate output data buffer memory\n");
			return;
		}

		// populate data_buffer with random values 
		for (int i = 0; i < size; i++)
		{
			input_data_buffer[i] = rand () % 255;
		}

		printf ("\nInitializing OpenCL:");

		ret = Init (&platform_id, &device_id, &context, &cq);

		if (ret != CL_SUCCESS) 
		{
			printf ("\nFailed Initializing OpenCL\n");
			exit (0);
		}
		else
			printf (" Ok\n");

		ret = PrintInfo (platform_id, device_id);

		if (ret != CL_SUCCESS) 
			printf ("\nFailed reading OpenCL Info\n");

		printf ("\nLoading CL programs: hello_world");

		ret = LoadKernelSource ((char *)"hello_world.cl", &kernel_str);

		if (ret != CL_SUCCESS) 
		{
			printf ("\nFailed loading hello_world kernel\n");
			exit (0);
		}
		else
			printf (" Ok\n");


		printf ("\nBuilding hello_world kernel: ");
		gettimeofday(&start, NULL);
		ret = BuildProgram (&program, &device_id, context, &kernel_str);
		gettimeofday(&end, NULL);
		//compute and print the elapsed time in millisec
		seconds  = end.tv_sec  - start.tv_sec;
		useconds = end.tv_usec - start.tv_usec;
		mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;

		if (ret != CL_SUCCESS) 
		{
			 printf ("Failed Building hello_world kernel\n");
			 exit (0);
		}
		else
			printf (" Ok - %ld ms\n", mtime);		


		printf ("\nCreating CL kernel...");
		gettimeofday(&start, NULL);
		kernel = clCreateKernel (program, "hello_world", &ret);
		gettimeofday(&end, NULL);
		//compute and print the elapsed time in millisec
		seconds  = end.tv_sec  - start.tv_sec;
		useconds = end.tv_usec - start.tv_usec;
		mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;

		if (ret != CL_SUCCESS) 
		{
			 printf ("Failed Creating hellow_world program\n");
			 exit (0);
		}
		else
			printf (" Ok - %ld ms\n", mtime);		

		printf ("\nAllocating buffers...");

		helloworld_mem_input = clCreateBuffer (context, CL_MEM_READ_ONLY, size, NULL, &ret);	

		if (ret != CL_SUCCESS) 
		{
			 printf ("Failed Allocation hello_world input buffer\n");
			 exit (0);
		}
			
		helloworld_mem_output = clCreateBuffer (context, CL_MEM_WRITE_ONLY, size, NULL, &ret);	

		if (ret != CL_SUCCESS) 
		{
			 printf ("Failed Allocation hello_world output buffer\n");
			 exit (0);
		}
		else
			printf (" Ok\n");

		clSetKernelArg (kernel, 0, sizeof(cl_mem), &helloworld_mem_input);
		clSetKernelArg (kernel, 1, sizeof(cl_mem), &helloworld_mem_output);

		ret = clEnqueueWriteBuffer(cq, helloworld_mem_input, CL_TRUE, 0, size, input_data_buffer, 0, NULL, NULL);

		if (ret != CL_SUCCESS) 
			printf ("\nError writing input buffer\n");


		// run the CL kernel, only  measure the time for enqueue_range function, writing and reading data can be slow
		// and a zero copy (mapping) procedure is necessary to accelerate it

		gettimeofday(&start, NULL);	
		ret = clEnqueueNDRangeKernel (cq, kernel, dimension, NULL, &global, &local, 0, NULL, NULL);
		clFlush(cq);
		gettimeofday(&end, NULL);


		if  (ret == CL_SUCCESS)
		{
			ret = clEnqueueReadBuffer(cq, helloworld_mem_output, CL_TRUE, 0, size, output_data_buffer, 0, NULL, NULL);
		}
		else
			printf ("\nError reading output buffer\n");


		//compute and print the elapsed time in millisec
		seconds  = end.tv_sec  - start.tv_sec;
		useconds = end.tv_usec - start.tv_usec;
		mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;


		// just a debug for some values
		bool flag = 0;
		for (int i = 0; i < size; i++)
		{
			if (input_data_buffer[i] != output_data_buffer[i])
				flag = 1;
		}

		if (flag)
			printf ("\noutput buffer is different from input\n");
		else
			printf ("\nAll values successfully copied\n");

		printf( "\nProcess time (gpu) = %ld ms\n", mtime);		


		// cpu copy
		gettimeofday(&start, NULL);
		memcpy (output_data_buffer, input_data_buffer, size);
		gettimeofday(&end, NULL);

		//compute and print the elapsed time in millisec
		seconds  = end.tv_sec  - start.tv_sec;
		useconds = end.tv_usec - start.tv_usec;
		mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
		printf( "\nProcess time (cpu) = %ld ms\n", mtime);		

		free (input_data_buffer);
		free (output_data_buffer);

		clFlush( cq);
		clFinish(cq);

		printf ("\nFinishing OpenCL...");
		clReleaseContext(context);
		clReleaseProgram(program);
		clReleaseCommandQueue(cq);
		clReleaseKernel (kernel);
		clReleaseMemObject (helloworld_mem_input);
		clReleaseMemObject (helloworld_mem_output);
		printf ("OK\n");

		return;
	}


	cl_int Init (cl_platform_id *platform_id, cl_device_id *device_id, cl_context *context, cl_command_queue *cq)
	{
		cl_uint  platforms, devices;
		cl_int error;
		
		error = clGetPlatformIDs (1, platform_id, &platforms);
		if (error != CL_SUCCESS) 
			return CL_ERROR;
		
		error = clGetDeviceIDs ((* platform_id), CL_DEVICE_TYPE_GPU, 1, device_id, &devices);
		if (error != CL_SUCCESS) 
			return CL_ERROR;
		
		cl_context_properties properties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)(* platform_id), 0};
		
		(* context) = clCreateContext (properties, 1, device_id, NULL, NULL, &error);
		
		if (error != CL_SUCCESS) 
			return CL_ERROR;
		
		(* cq) = clCreateCommandQueue ((* context), (* device_id), 0, &error);
		if (error != CL_SUCCESS) 
			return CL_ERROR;

		return CL_SUCCESS;
	}

	cl_int PrintInfo (cl_platform_id platform_id, cl_device_id device_id)
	{
		uint i;
		char buffer[10240];
		cl_int ret;

		printf ("\n-=-=-=- Platform Information -=-=-=-\n\n");
		ret = clGetPlatformInfo (platform_id, CL_PLATFORM_NAME, 10240, buffer, NULL);
		if (ret != CL_SUCCESS) 
			return CL_ERROR;
		printf ("Platform Name: %s\n", buffer);

		ret = clGetPlatformInfo (platform_id, CL_PLATFORM_PROFILE, 10240, buffer, NULL);
		if (ret != CL_SUCCESS) 
			return CL_ERROR;
		printf ("Platform Profile: %s\n", buffer);
		
		ret = clGetPlatformInfo (platform_id, CL_PLATFORM_VERSION, 10240, buffer, NULL);
		if (ret != CL_SUCCESS) 
			return CL_ERROR;
		printf ("Platform Version: %s\n", buffer);
		
		ret = clGetPlatformInfo (platform_id, CL_PLATFORM_VENDOR, 10240, buffer, NULL);
		if (ret != CL_SUCCESS) 
			return CL_ERROR;
		printf ("Platform Vendor: %s\n", buffer);
		
		printf ("\n-=-=-=- Device Information -=-=-=-\n\n");
		ret = clGetDeviceInfo( device_id, CL_DEVICE_NAME, 10240, buffer, NULL);
		if (ret != CL_SUCCESS)
			return CL_ERROR;
		printf ("Device Name: %s\n", buffer);
		
		ret = clGetDeviceInfo (device_id, CL_DEVICE_PROFILE, 10240, buffer, NULL);
		if (ret != CL_SUCCESS)
			return CL_ERROR;

		printf ("Device Profile: %s\n", buffer);

		ret = clGetDeviceInfo (device_id, CL_DEVICE_VERSION, 10240, buffer, NULL);
		if (ret != CL_SUCCESS) 
			return CL_ERROR;
		printf ("Device Version: %s\n", buffer);

		ret = clGetDeviceInfo (device_id, CL_DEVICE_VENDOR, 10240, buffer, NULL);
		if (ret != CL_SUCCESS) 
			return CL_ERROR;
		printf ("Device Vendor: %s\n", buffer);
	 
		ret = clGetDeviceInfo (device_id, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof (uint), &i, NULL);
		if (ret != CL_SUCCESS) 
			return CL_ERROR;
		printf ("Device Max Work Item Dimensions: %u-D\n", i);

		ret = clGetDeviceInfo (device_id, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof (uint), &i, NULL);
		if (ret != CL_SUCCESS) 
			return CL_ERROR;
		printf ("Device Max Work Group Size: %u\n", i);

	//	ret = clGetDeviceInfo(device_id, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof (uint), &i, NULL);
	//	if (ret != CL_SUCCESS) 
	//		return 0;
	//	printf ("Device Max Work Item Sizes: %u\n", i);
		
	//	ret = clGetDeviceInfo(device_id, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof (uint), &i, NULL);
	//	if (ret != CL_SUCCESS) 
	//		return 0;
	//	printf ("Device Global Memory Size: %u\n", i);
		printf ("\n-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");

		return CL_SUCCESS;
	}

	cl_int LoadKernelSource (char *filename, kernel_src_str *kernel_src)
	{
		
		FILE *fp = NULL;
		
		fp = fopen (filename, "rb");
		
		if (fp == NULL)
		{
			printf ("\nFailed to open: %s\n", filename); 
			return CL_ERROR;
		}
		
		fseek (fp, 0, SEEK_END);
		kernel_src->size = ftell (fp);
		rewind (fp);
		
		// prevent re-allocation
		//if (kernel->src) free (kernel->src);
		kernel_src->src =  (char *) malloc (sizeof (char) * kernel_src->size);
		if (! kernel_src->src)
		{
			printf ("\nError Allocating memory to load CL program");
			return CL_ERROR;
		}
		fread (kernel_src->src, 1, kernel_src->size, fp);

		kernel_src->src[kernel_src->size] = '\0';
		fclose (fp);
		
		return CL_SUCCESS;
	}

	cl_int BuildProgram (cl_program *program, cl_device_id *device_id, cl_context context, kernel_src_str *kernel)
	{
		cl_int error = CL_SUCCESS;
		
		(* program) = clCreateProgramWithSource (context, 1, (const char **)&kernel->src, &kernel->size, &error);
		if (error != CL_SUCCESS)
		{
			return CL_ERROR;
		}
		
		error = clBuildProgram ((* program), 1, device_id, "", NULL, NULL);
		if (error < 0)
		{
			clGetProgramBuildInfo((* program), (* device_id), CL_PROGRAM_BUILD_LOG, kernel->size, kernel->src, NULL);
			printf ("\n%s", kernel->src);
		}
		
		return error;
	}


}