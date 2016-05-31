#include "gl_cl_interop_pingpong_st.h"
#include <CL/cl_gl.h>
#include <CL/cl_gl_ext.h>

#ifdef _WIN32
#include <windows.h>
#endif

//#include <cudaGL.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////

void Sample::simulateOpenCL(float dt, bool oddStep)
{
  if( m_visualizeGLSync )
  {
    CHECK_OCL_ASYNC_CALL( m_visualizeGLEvent = clCreateEventFromGLsyncKHR( m_oContext, m_visualizeGLSync, &OCLERR )
      , "Failed to create an ocl event from an ogl sync" );
  }
  if( m_visualizeGLEvent )
  {
    //ISSUE: clEnqueueWaitForEvents() returns CL_INVALID_EVENT
    /*CHECK_OCL_CALL( */clEnqueueWaitForEvents( m_clCommandQueue, 1, &m_visualizeGLEvent )
    /*, "Failed to enqueue a wait for event" )*/;
  }

  NV_PROFILE_SECTION_EX("simulateOpenCL", m_oclTimer, false);

  if(!m_isRunning)
  {
    return;
  }

  const int block_size = 8;

  cl_event eventHeatCalc;

  CHECK_OCL_CALL( clSetKernelArg( m_entryPoint, 0, sizeof(float), &dt),                            "Could not set an argument");
  CHECK_OCL_CALL( clSetKernelArg( m_entryPoint, 1, sizeof(cl_mem), &m_clImages[ oddStep ? 0 : 1] ), "Could not set an argument" );
  CHECK_OCL_CALL( clSetKernelArg( m_entryPoint, 2, sizeof(cl_mem), &m_clImages[ oddStep ? 1 : 0] ), "Could not set an argument" );
  CHECK_OCL_CALL( clSetKernelArg( m_entryPoint, 3, sizeof(float), &m_cellWidth ),                  "Could not set an argument" );
  CHECK_OCL_CALL( clSetKernelArg( m_entryPoint, 4, sizeof(float), &m_alpha),                       "Could not set an argument" );

  size_t globalWorkSize[] = {m_volumeTexDimensionNonBoundary, m_volumeTexDimensionNonBoundary, m_volumeTexDimensionNonBoundary};
  size_t localWorkSize[] =  {block_size, block_size, block_size};

  {
    NV_PROFILE_SECTION_EX("simulateOpenCL - Acquire", m_oclTimer, false);
    CHECK_OCL_CALL( clEnqueueAcquireGLObjects(m_clCommandQueue, 2, m_clImages, 0, 0, 0)
                  , "Could not setup gl object acquisition" );

  }
  {
    NV_PROFILE_SECTION_EX("simulateOpenCL - Compute", m_oclTimer, false);
    CHECK_OCL_CALL( clEnqueueNDRangeKernel(m_clCommandQueue, m_entryPoint, 3, 0, globalWorkSize, localWorkSize, 0, 0, &eventHeatCalc)
                  , "Could not setup launch kernel command" );
  }

  {
    NV_PROFILE_SECTION_EX("simulateOpenCL - Release", m_oclTimer, false);
    CHECK_OCL_CALL( clEnqueueReleaseGLObjects(m_clCommandQueue, 2, m_clImages, 0, 0, &m_simulateOpenCLEvent)
                  , "Could not release gl object" );
  }

/*
  {
    NV_PROFILE_SECTION_EX("simulateOpenCL - Wait", m_oclTimer, false);
    CHECK_OCL_CALL( clWaitForEvents(1, &eventHeatCalc)
                  , "Could not sync kernel execution with the host" );
  }*/
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

void Sample::initOpenCL()
{
  m_clDevice = nv_helpers_ocl::findOpenCLDeviceDRV(false, &m_clPlatform);

  // Create a context
#ifdef _WIN32
  HGLRC hGLRC = wglGetCurrentContext();
  HDC hDC = wglGetCurrentDC();
#endif
  cl_context_properties properties[] =
  {
    CL_CONTEXT_PLATFORM, (cl_context_properties)m_clPlatform,
    CL_GL_CONTEXT_KHR,   (cl_context_properties)hGLRC,
    CL_WGL_HDC_KHR,      (cl_context_properties)hDC,
    0
  };
  CHECK_OCL_ASYNC_CALL( m_oContext = clCreateContext( properties, 1, &m_clDevice, 0, 0, &OCLERR )
                      , "Failed to create an OpenCL context" );

  // Create a command queue
  CHECK_OCL_ASYNC_CALL( m_clCommandQueue = clCreateCommandQueue( m_oContext, m_clDevice, 0, &OCLERR )
                      , "Failed to create a command queue" );


  initOpenCLEntryPoint( std::string(QUOTED_SAMPLE_DIR) + "/heatEquation.cl", "heatEquationStep" );
  initOpenCLArray();
  setOpenCLVars();
  initOpenCLSurfTexReferences();

  m_oclTimer = new nv_helpers_ocl::AsyncTimer( m_clCommandQueue );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

void Sample::initOpenCLArray()
{
  CHECK_OCL_ASYNC_CALL( m_clImages[0] = clCreateFromGLTexture( m_oContext, CL_MEM_READ_WRITE
                                                            , GL_TEXTURE_3D, 0
                                                            , m_volumeTextureName[0], &OCLERR )
                      , "Couldn't create an OpenCL image from an OpenGL texture" );
  
  CHECK_OCL_ASYNC_CALL( m_clImages[1] = clCreateFromGLTexture( m_oContext, CL_MEM_READ_WRITE
                                                            , GL_TEXTURE_3D, 0
                                                            , m_volumeTextureName[1], &OCLERR )
                      , "Couldn't create an OpenCL image from an OpenGL texture" );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

void Sample::initOpenCLSurfTexReferences()
{
  //cu//CHECK_OCL_ASYNC_CALL( cuModuleGetSurfRef( &m_surfWriteRef, m_module, "volumeTexOut" )
  //cu//               , "Failed to get surface reference" );
  
  //cu//CHECK_OCL_ASYNC_CALL( cuModuleGetTexRef( &m_surfReadRef, m_module, "volumeTexIn" )
  //cu//               , "Failed to get surface reference" );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

void Sample::setOpenCLVars()
{
  //CUdeviceptr  d_cellWidth;
  size_t       d_cellWidthBytes;

  // First grab the global device pointers from the CUBIN
  //cu//CHECK_OCL_ASYNC_CALL( cuModuleGetGlobal(&d_cellWidth,  &d_cellWidthBytes, m_module, "cellWidth")
  //cu//               , "Failed to get a pointer to constant in device memory" );

  // Copy the constants to video memory
  //cu//CHECK_OCL_ASYNC_CALL( cuMemcpyHtoD(d_cellWidth, reinterpret_cast<const void *>(&m_cellWidth), d_cellWidthBytes)
  //cu//               , "Failed to set a constant in device memory" );

  
  //CUdeviceptr  d_alpha;
  size_t       d_alphaBytes;

  // First grab the global device pointers from the CUBIN
  //cu//CHECK_OCL_ASYNC_CALL( cuModuleGetGlobal(&d_alpha,  &d_alphaBytes, m_module, "alpha")
  //cu//                , "Failed to get a pointer to constant in device memory" );

  // Copy the constants to video memory
  //cu//CHECK_OCL_ASYNC_CALL( cuMemcpyHtoD(d_alpha, reinterpret_cast<const void *>(&m_alpha), d_alphaBytes)
  //cu//               , "Failed to set a constant in device memory" );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

void Sample::initOpenCLEntryPoint( const std::string& kernelSourceFile, const std::string& entryPointName )
{
  m_module = nv_helpers_ocl::buildKernel( kernelSourceFile.c_str(), m_oContext, m_clDevice );


  CHECK_OCL_ASYNC_CALL( m_entryPoint = clCreateKernel( m_module, entryPointName.c_str(), &OCLERR )
                      , "Failed to load entry point" );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

void Sample::simStart()
{
  m_isRunning = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

void Sample::simPause()
{
  m_isRunning = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

void Sample::simReset()
{
  setInitialVolumeData();
}
