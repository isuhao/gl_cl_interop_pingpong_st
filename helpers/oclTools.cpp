#pragma once

#include "oclTools.hpp"


#ifdef _DEBUG
#endif

__oclErrMap __oclErrMap::instance;

namespace nv_helpers_ocl
{

  const char* AsyncTimer::TimerTypeName()
  {
    static const char oclStr[] = "OpenCL";
    return oclStr;
  }

  bool AsyncTimer::TimerAvailable( nv_helpers::Profiler::TimerIdx idx )
  {
    cl_int resVal;
    cl_int ret = clGetEventInfo( (cl_event)m_entries[idx]
                               , CL_EVENT_COMMAND_EXECUTION_STATUS
                               , sizeof(cl_int), &resVal, 0 );
    
    bool resultsAvailable = resVal == CL_COMPLETE;
    assert( resultsAvailable || ret != CL_INVALID_EVENT );
    return resultsAvailable;
  }

  void AsyncTimer::TimerSetup( nv_helpers::Profiler::TimerIdx idx )
  {
    CHECK_OCL_CALL( clEnqueueMarker( m_stream, &(cl_event)m_entries[idx] )
                  , "Failed to enqueue a marker" );
  }

  unsigned long long AsyncTimer::TimerResult( nv_helpers::Profiler::TimerIdx idxBegin
                                            , nv_helpers::Profiler::TimerIdx idxEnd )
  {
    cl_ulong timeBegin, timeEnd;
    
    clGetEventProfilingInfo( (cl_event)m_entries[idxBegin], CL_PROFILING_COMMAND_END
                           , sizeof(cl_ulong), &timeBegin, 0 );

    clGetEventProfilingInfo( (cl_event)m_entries[idxEnd], CL_PROFILING_COMMAND_START
                           , sizeof(cl_ulong), &timeEnd, 0 );
    
    //unsigned long long nanoseconds = unsigned long long(milliseconds * 1000000.0f);
    return unsigned long long(timeEnd - timeBegin);
  }

  void AsyncTimer::TimerEnsureSize(unsigned int timers)
  {
    size_t prevSize = m_entries.size();
    m_entries.resize( prevSize + timers, 0 );
  }

  AsyncTimer::~AsyncTimer()
  {
    for ( std::vector<cl_event>::const_iterator it = m_entries.begin()
        ; it != m_entries.end()
        ; it++ )
    {
      //Does this actually need to be done explicitly?
      clReleaseEvent( (cl_event)*it );
    }
  }

}