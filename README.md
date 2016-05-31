# gl\_cl\_interop\_pingpong_st

This is a small sample that demonstrates the way to use the *OpenCL-OpenGL* *interop* API in a single-threaded manner, using **explicit** synchronization.

This sample uses the extension [cl\_khr\_gl\_event](https://www.khronos.org/registry/cl/sdk/1.1/docs/man/xhtml/gl_event.html) & [GL\_ARB\_cl\_event](https://www.opengl.org/registry/specs/ARB/cl_event.txt)

This example computes with *OpenCL* a temperature scalar field that gets updated every frame. The visual result is a a 256 x 256 x 256 uniform grid. It is rendered in *OpenGL* with a basic ray-marching fragment shader.

The *OpenCL* compute part is a simple heat propogator. Since at every time step our result depends on the result of the previous frame, we **pingpong** the 3D texture resource handles back and forth every frame.

When building this sample **please be sure to have repositories** **[shared_sources](https://github.com/nvpro-samples/shared_sources)** and **[shared_external](https://github.com/nvpro-samples/shared_external)** in the same level folder!

![sample screenshot](https://github.com/nvpro-samples/gl_cl_interop_pingpong_st/blob/master/doc/sample.png?raw=true)

# What does the sample do?

The example itself is pretty straightforward: We just start have a starting temperature scalar field and propogate heat by computing the next frame based on the previous (below). In this example, the scalar field is just a cubic volume allocated in *OpenGL* as a `GL_TEXTURE_3D`

![pic1](https://github.com/nvpro-samples/gl_cl_interop_pingpong_st/blob/master/doc/pic_1.png?raw=true)

> **Note:** As shown above this example numerically solves the heat equation explicitly. While this is a sufficient example to showcase the type of problem where you need to ping-pong GPU memory resources, please keep in mind this isn't the best way to compute heat propagation. This approach will cause a greater error drift and is less numerically stable than say, a finite volume method approach.

To numerically determine the temperature scalar field of the volume after a given amount of time, we need to know what the temperature scalar field was immediately before. For this reason we need to allocate two identical *OpenGL* `GL_TEXTURE_3D` textures.
Every next frame we need to swap handles to these `GL_TEXTURE_3D` textures so that we're always reading from one and writing to the other when running the *OpenCL* simulation (below)

![pic2](https://github.com/nvpro-samples/gl_cl_interop_pingpong_st/blob/master/doc/pic_2.png?raw=true)

# ...But how do I read and write to `GL_TEXTURE_3D` in *CUDA* and what will it cost me?

In *OpenGL* a GPU texture resource is referenced by a `GLuint`

In *OpenCL* a GPU texture resource is referenced by a `cl_mem`

![pic3](https://github.com/nvpro-samples/gl_cl_interop_pingpong_st/blob/master/doc/pic_3.png?raw=true)

Below is a code snippet from the sample showing how the above "bind" is made.


```cpp

  GLuint m_volumeTextureName[2];
  cl_mem m_clImages[2];

    ...

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

```

There that's it! **The above code only needs to be executed once!**. There is a misconception going around that this "bind" needs to be done every frame. This isn't true, this only needs to be done once and it's been this way for years now.

As a result there is **no additional overhead** that comes with *OpenCL*-*OpenGL* interop.

# Synchronization of resources between OpenGL and OpenCL

This sample uses an explicit way to synchronize OpenGL with OpenCL work: using events to make sure that
- OpenGL is done before OpenCL can recycle the resource (texture)
- OpenCL is dones with the simulation before OpenGL can use it as a texture for rendering

When OpenCL computation is being initiated, a set of commands will be sent to the OpenCL command-queue.

- **clEnqueueAcquireGLObjects(...m_clImages)** will acquire the OpenGL resources pointed by m_clImages
- **clEnqueueNDRangeKernel(...)** will enqueue the work to do with these resources
- **clEnqueueReleaseGLObjects(...m_clImages, m_CLevent)** will enqueue the action of releasing the resources. *m_simulateOpenCLEvent* of type cl_event will receive the Event handle that you need to query completeness

When switching to OpenGL, you will have to make sure OpenCL work on target resource is done:
- before rendering, call "**s = glCreateSyncFromCLeventARB(...m_CLevent)**" to convert to OpenGL Sync object the OpenCL Event
- then wait for this Sync object to be done: glWaitSync(s,...) (in fact this commmand will be pushed to the push-buffer (== queue)
- render the scene

OpenGL will then ensure that m_CLeven got signaled prior to rendering with the resource OpenCL was writing to.

Later, OpenGL would do the same but on the other direction. To make sure it is done with its rendering prior to have OpenCL use the resources, OpenGL will issue a Fence: "**glSync = glFenceSync( GL_SYNC_GPU_COMMANDS_COMPLETE, 0 );**"

Later, this Fence can be converted for OpenCL: "**glascl = clCreateEventFromGLsyncKHR(...glSync)**" and OpenCL code would enqueue the action of waiting on this fence: "**clEnqueueWaitForEvents( ...glascl)**"


# Requirements:
 - Visual Studio 2010 or higher
 - OpenCL library available in shared_external repository
 - OpenGL 4.5 capable NVIDIA GPU

## Repositories
 - **[shared_sources](https://github.com/nvpro-samples/shared_sources)**
 - **[shared_external](https://github.com/nvpro-samples/shared_external)**

# Contact:

Any feedback is welcome and should be sent to Dennis Sandler (dsandler "at' nvidia.com) / Tristan Lorach

```
    Copyright (c) 2014, NVIDIA CORPORATION. All rights reserved.
 
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Neither the name of NVIDIA CORPORATION nor the names of its
       contributors may be used to endorse or promote products derived
       from this software without specific prior written permission.
 
    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
    PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
    CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
    PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
    PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
    OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```
