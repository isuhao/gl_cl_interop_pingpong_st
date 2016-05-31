#pragma once

#include <assert.h>
#include <iostream>
#include <map>
#include <CL/cl.h>

#include "nv_helpers_gl/profilertimersgl.hpp"
#include <nv_helpers/misc.hpp>

#ifdef _DEBUG

# define MAX_OCL_PLATFORMS_ASSUMED 5
# define MAX_OCL_DEVICES_ASSUMED 5

# define __ERRMAPADD(errcode) (*this)[errcode] = std::string(#errcode)

  struct __oclErrMap : public std::map<int, std::string>
  {
    __oclErrMap()
    {
      __ERRMAPADD(CL_SUCCESS);
      __ERRMAPADD(CL_DEVICE_NOT_FOUND);
      __ERRMAPADD(CL_DEVICE_NOT_AVAILABLE);
      __ERRMAPADD(CL_COMPILER_NOT_AVAILABLE);
      __ERRMAPADD(CL_MEM_OBJECT_ALLOCATION_FAILURE);
      __ERRMAPADD(CL_OUT_OF_RESOURCES);
      __ERRMAPADD(CL_OUT_OF_HOST_MEMORY);
      __ERRMAPADD(CL_PROFILING_INFO_NOT_AVAILABLE);
      __ERRMAPADD(CL_MEM_COPY_OVERLAP);
      __ERRMAPADD(CL_IMAGE_FORMAT_MISMATCH);
      __ERRMAPADD(CL_IMAGE_FORMAT_NOT_SUPPORTED);
      __ERRMAPADD(CL_BUILD_PROGRAM_FAILURE);
      __ERRMAPADD(CL_MAP_FAILURE);
      __ERRMAPADD(CL_MISALIGNED_SUB_BUFFER_OFFSET);
      __ERRMAPADD(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST);
      __ERRMAPADD(CL_INVALID_VALUE);
      __ERRMAPADD(CL_INVALID_DEVICE_TYPE);
      __ERRMAPADD(CL_INVALID_PLATFORM);
      __ERRMAPADD(CL_INVALID_DEVICE);
      __ERRMAPADD(CL_INVALID_CONTEXT);
      __ERRMAPADD(CL_INVALID_QUEUE_PROPERTIES);
      __ERRMAPADD(CL_INVALID_COMMAND_QUEUE);
      __ERRMAPADD(CL_INVALID_HOST_PTR);
      __ERRMAPADD(CL_INVALID_MEM_OBJECT);
      __ERRMAPADD(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR);
      __ERRMAPADD(CL_INVALID_IMAGE_SIZE);
      __ERRMAPADD(CL_INVALID_SAMPLER);
      __ERRMAPADD(CL_INVALID_BINARY);
      __ERRMAPADD(CL_INVALID_BUILD_OPTIONS);
      __ERRMAPADD(CL_INVALID_PROGRAM);
      __ERRMAPADD(CL_INVALID_PROGRAM_EXECUTABLE);
      __ERRMAPADD(CL_INVALID_KERNEL_NAME);
      __ERRMAPADD(CL_INVALID_KERNEL_DEFINITION);
      __ERRMAPADD(CL_INVALID_KERNEL);
      __ERRMAPADD(CL_INVALID_ARG_INDEX);
      __ERRMAPADD(CL_INVALID_ARG_VALUE);
      __ERRMAPADD(CL_INVALID_ARG_SIZE);
      __ERRMAPADD(CL_INVALID_KERNEL_ARGS);
      __ERRMAPADD(CL_INVALID_WORK_DIMENSION);
      __ERRMAPADD(CL_INVALID_WORK_GROUP_SIZE);
      __ERRMAPADD(CL_INVALID_WORK_ITEM_SIZE);
      __ERRMAPADD(CL_INVALID_GLOBAL_OFFSET);
      __ERRMAPADD(CL_INVALID_EVENT_WAIT_LIST);
      __ERRMAPADD(CL_INVALID_EVENT);
      __ERRMAPADD(CL_INVALID_OPERATION);
      __ERRMAPADD(CL_INVALID_GL_OBJECT);
      __ERRMAPADD(CL_INVALID_BUFFER_SIZE);
      __ERRMAPADD(CL_INVALID_MIP_LEVEL);
      __ERRMAPADD(CL_INVALID_GLOBAL_WORK_SIZE);
      __ERRMAPADD(CL_INVALID_PROPERTY);
    }
    
    static __oclErrMap instance;
  };
  
# define CHECK_OCL_CALL(cerr, str)  { cl_int __tmpclErr = cerr;\
                                    if(__tmpclErr != CL_SUCCESS)\
                                    {\
                                      std::cout << str << ": " << __oclErrMap::instance[__tmpclErr] << "\n";\
                                      assert(false);\
                                    }\
                                    }

# define CHECK_OCL_ASYNC_CALL(cerr, str) {cl_int OCLERR;\
                                         cerr;\
                                         if( OCLERR != CL_SUCCESS )\
                                         {\
                                           std::cout << str << ": " << __oclErrMap::instance[OCLERR] << "\n";\
                                           assert(false);\
                                         }\
                                         }
#else
# define CHECK_OCL_CALL(cerr, str) cerr
# define CHECK_OCL_ASYNC_CALL(cerr, str) cerr
#endif

namespace nv_helpers_ocl
{
  // General initialization call to pick the best CUDA Device
  inline cl_device_id findOpenCLDeviceDRV(bool verbose = false, cl_platform_id* clPlatformOut = 0)
  {
    cl_platform_id clPlatformsAvailable[MAX_OCL_PLATFORMS_ASSUMED];
    cl_uint clNumPlatformsActual;
    cl_platform_id clPlatform;

    cl_device_id clDevicesAvailable[MAX_OCL_DEVICES_ASSUMED];
    cl_uint clNumDevicesActual;
    cl_device_id clDevice;

    CHECK_OCL_CALL( clGetPlatformIDs( MAX_OCL_PLATFORMS_ASSUMED
                                    , clPlatformsAvailable
                                    , &clNumPlatformsActual)
                  , "Failed to get platforms" );
    
    for(cl_uint i = 0; i < clNumPlatformsActual; i++)
    {
      //Make sure we get an nvidia platform
      char vendorStr[128];
      size_t vendorStrLen;

      CHECK_OCL_CALL( clGetPlatformInfo( clPlatformsAvailable[i], CL_PLATFORM_VENDOR
                                       , sizeof(vendorStr), vendorStr, &vendorStrLen )
                    , "Failed to get platform info" );

      if( strncmp(vendorStr, "NVIDIA", sizeof("NVIDIA") - 1 ) == 0 )
      {
        clPlatform = clPlatformsAvailable[i];
        if( clPlatformOut )
        {
          *clPlatformOut = clPlatform;
        }
        break;
      }
    }
    
    CHECK_OCL_CALL( clGetDeviceIDs( clPlatform, CL_DEVICE_TYPE_GPU
                                  , MAX_OCL_PLATFORMS_ASSUMED
                                  , clDevicesAvailable, &clNumDevicesActual )
                  , "Failed to get device ids" );


    for(cl_uint i = 0; i < clNumDevicesActual; i++)
    {
      //Make sure we get an nvidia platform
      char deviceStr[128];
      size_t deviceStrLen;

      CHECK_OCL_CALL( clGetDeviceInfo( clDevicesAvailable[i], CL_DEVICE_VENDOR
                                     , sizeof(deviceStr), deviceStr, &deviceStrLen )
                    , "Failed to get device info" );

      if( strncmp(deviceStr, "NVIDIA", sizeof("NVIDIA") - 1 ) == 0 )
      {
        char deviceNameStr[128];
        char versionStr[128];
        std::string extStr;
        size_t extStrLen;

        CHECK_OCL_CALL( clGetDeviceInfo( clDevicesAvailable[i], CL_DEVICE_NAME
                                       , sizeof(deviceNameStr), deviceNameStr, 0 )
                      , "Failed to get device info" );

        CHECK_OCL_CALL( clGetDeviceInfo( clDevicesAvailable[i], CL_DEVICE_VERSION
                                       , sizeof(versionStr), versionStr, 0 )
                      , "Failed to get device info" );

        extStrLen = 8192;
        extStr.resize(extStrLen);
        CHECK_OCL_CALL( clGetDeviceInfo( clDevicesAvailable[i], CL_DEVICE_EXTENSIONS
                                       , extStr.length(), &extStr[0], 0 )
                      , "Failed to get device info" );

        clDevice = clDevicesAvailable[i];
        break;
      }
    }

    return clDevice;
  }

  inline cl_program buildKernel( const char* kernelSourceFile, const cl_context& context, const cl_device_id& deviceId )
  {
    std::string kernelSourceStr = nv_helpers::loadFile( kernelSourceFile );
    const char* szKernelSrc = kernelSourceStr.c_str();
    size_t lenKernelSrc = kernelSourceStr.length();

    cl_program module;
    CHECK_OCL_ASYNC_CALL( module = clCreateProgramWithSource( context, 1, &szKernelSrc, &lenKernelSrc, &OCLERR )
                        , "Failed to compile ocl kernel module" );

    if( clBuildProgram( module, 1, &deviceId, 0, 0, 0 ) != CL_SUCCESS )
    {
      char buffer[10240];
      clGetProgramBuildInfo(module, deviceId, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, NULL);
      std::cerr << kernelSourceFile << "\n" << buffer << "\n";
      return 0;
    }

    CHECK_OCL_CALL( clUnloadCompiler()
                  , "Failed to unload compiler" );

    return module;
  }

  class AsyncTimer : public nv_helpers::Profiler::GPUInterface
  {
  public:
    AsyncTimer(const cl_command_queue& cmdStream)
    {
      // Only use one command stream for now
      m_stream = cmdStream;
	  TimerEnsureSize(nv_helpers::Profiler::START_TIMERS);
    }
    ~AsyncTimer();
  public:
    const char* TimerTypeName();
    bool    TimerAvailable( nv_helpers::Profiler::TimerIdx idx );
    void    TimerSetup( nv_helpers::Profiler::TimerIdx idx );
    unsigned long long  TimerResult( nv_helpers::Profiler::TimerIdx idxBegin
                                   , nv_helpers::Profiler::TimerIdx idxEnd );
	void    TimerEnsureSize(unsigned int timers);
	void    TimerFlush() {};

  private:

    std::vector<cl_event> m_entries;
    cl_command_queue m_stream;
  };

}