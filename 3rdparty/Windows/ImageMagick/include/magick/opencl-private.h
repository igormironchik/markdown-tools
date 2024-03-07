/*
Copyright 1999-2021 ImageMagick Studio LLC, a non-profit organization
dedicated to making software imaging solutions freely available.

You may not use this file except in compliance with the License.  You may
obtain a copy of the License at

https://imagemagick.org/script/license.php

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

MagickCore OpenCL private methods.
*/
#ifndef MAGICKCORE_OPENCL_PRIVATE_H
#define MAGICKCORE_OPENCL_PRIVATE_H

/*
Include declarations.
*/
#include "magick/studio.h"
#include "magick/opencl.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if !defined(MAGICKCORE_OPENCL_SUPPORT)
  typedef void* cl_event;
  typedef void* cl_mem;
  typedef void* cl_uint;
#else

#define MAX_COMMAND_QUEUES 16

/*
 *
 * function pointer typedefs
 *
 */

/* Platform APIs */
typedef CL_API_ENTRY cl_int (CL_API_CALL *MAGICKpfn_clGetPlatformIDs)(
                 cl_uint          num_entries,
                 cl_platform_id * platforms,
                 cl_uint *        num_platforms) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *MAGICKpfn_clGetPlatformInfo)(
    cl_platform_id   platform, 
    cl_platform_info param_name,
    size_t           param_value_size, 
    void *           param_value,
    size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

/* Device APIs */
typedef CL_API_ENTRY cl_int (CL_API_CALL *MAGICKpfn_clGetDeviceIDs)(
    cl_platform_id   platform,
    cl_device_type   device_type, 
    cl_uint          num_entries, 
    cl_device_id *   devices, 
    cl_uint *        num_devices) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *MAGICKpfn_clGetDeviceInfo)(
    cl_device_id    device,
    cl_device_info  param_name, 
    size_t          param_value_size, 
    void *          param_value,
    size_t *        param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

/* Context APIs */
typedef CL_API_ENTRY cl_context (CL_API_CALL *MAGICKpfn_clCreateContext)(
    const cl_context_properties * properties,
    cl_uint                 num_devices,
    const cl_device_id *    devices,
    void (CL_CALLBACK *pfn_notify)(const char *, const void *, size_t, void *),
    void *                  user_data,
    cl_int *                errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *MAGICKpfn_clReleaseContext)(
    cl_context context) CL_API_SUFFIX__VERSION_1_0;

/* Command Queue APIs */
typedef CL_API_ENTRY cl_command_queue (CL_API_CALL *MAGICKpfn_clCreateCommandQueue)(
    cl_context                     context, 
    cl_device_id                   device, 
    cl_command_queue_properties    properties,
    cl_int *                       errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *MAGICKpfn_clReleaseCommandQueue)(
    cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0;

/* Memory Object APIs */
typedef CL_API_ENTRY cl_mem (CL_API_CALL *MAGICKpfn_clCreateBuffer)(
    cl_context   context,
    cl_mem_flags flags,
    size_t       size,
    void *       host_ptr,
    cl_int *     errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int
  (CL_API_CALL *MAGICKpfn_clRetainMemObject)(cl_mem memobj)
    CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int
  (CL_API_CALL *MAGICKpfn_clReleaseMemObject)(cl_mem memobj)
    CL_API_SUFFIX__VERSION_1_0;

/* Program Object APIs */
typedef CL_API_ENTRY cl_program (CL_API_CALL *MAGICKpfn_clCreateProgramWithSource)(
    cl_context        context,
    cl_uint           count,
    const char **     strings,
    const size_t *    lengths,
    cl_int *          errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_program (CL_API_CALL *MAGICKpfn_clCreateProgramWithBinary)(
    cl_context                     context,
    cl_uint                        num_devices,
    const cl_device_id *           device_list,
    const size_t *                 lengths,
    const unsigned char **         binaries,
    cl_int *                       binary_status,
    cl_int *                       errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *MAGICKpfn_clReleaseProgram)(cl_program program) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *MAGICKpfn_clBuildProgram)(
    cl_program           program,
    cl_uint              num_devices,
    const cl_device_id * device_list,
    const char *         options, 
    void (CL_CALLBACK *pfn_notify)(cl_program program, void * user_data),
    void *               user_data) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *MAGICKpfn_clGetProgramInfo)(
    cl_program         program,
    cl_program_info    param_name,
    size_t             param_value_size,
    void *             param_value,
    size_t *           param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *MAGICKpfn_clGetProgramBuildInfo)(
    cl_program            program,
    cl_device_id          device,
    cl_program_build_info param_name,
    size_t                param_value_size,
    void *                param_value,
    size_t *              param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;
                            
/* Kernel Object APIs */
typedef CL_API_ENTRY cl_kernel (CL_API_CALL *MAGICKpfn_clCreateKernel)(
    cl_program      program,
    const char *    kernel_name,
    cl_int *        errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *MAGICKpfn_clReleaseKernel)(cl_kernel   kernel) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *MAGICKpfn_clSetKernelArg)(
    cl_kernel    kernel,
    cl_uint      arg_index,
    size_t       arg_size,
    const void * arg_value) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int
  (CL_API_CALL *MAGICKpfn_clFlush)(cl_command_queue command_queue)
    CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *MAGICKpfn_clFinish)(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0;

/* Enqueued Commands APIs */
typedef CL_API_ENTRY cl_int (CL_API_CALL *MAGICKpfn_clEnqueueReadBuffer)(
    cl_command_queue    command_queue,
    cl_mem              buffer,
    cl_bool             blocking_read,
    size_t              offset,
    size_t              cb, 
    void *              ptr,
    cl_uint             num_events_in_wait_list,
    const cl_event *    event_wait_list,
    cl_event *          event) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY void * (CL_API_CALL *MAGICKpfn_clEnqueueMapBuffer)(
    cl_command_queue command_queue,
    cl_mem           buffer,
    cl_bool          blocking_map, 
    cl_map_flags     map_flags,
    size_t           offset,
    size_t           cb,
    cl_uint          num_events_in_wait_list,
    const cl_event * event_wait_list,
    cl_event *       event,
    cl_int *         errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *MAGICKpfn_clEnqueueUnmapMemObject)(
    cl_command_queue command_queue,
    cl_mem           memobj,
    void *           mapped_ptr,
    cl_uint          num_events_in_wait_list,
    const cl_event *  event_wait_list,
    cl_event *        event) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *MAGICKpfn_clEnqueueNDRangeKernel)(
    cl_command_queue command_queue,
    cl_kernel        kernel,
    cl_uint          work_dim,
    const size_t *   global_work_offset,
    const size_t *   global_work_size,
    const size_t *   local_work_size,
    cl_uint          num_events_in_wait_list,
    const cl_event * event_wait_list,
    cl_event *       event) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int(CL_API_CALL *MAGICKpfn_clGetEventProfilingInfo)(
    cl_event event,
    cl_profiling_info param_name,
    size_t param_value_size,
    void *param_value,
    size_t *param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int
  (CL_API_CALL *MAGICKpfn_clGetEventInfo)(cl_event event,
    cl_profiling_info param_name,size_t param_value_size,void *param_value,
    size_t *param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int(CL_API_CALL *MAGICKpfn_clWaitForEvents)(
    cl_uint num_events,
    const cl_event *event_list) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int(CL_API_CALL *MAGICKpfn_clReleaseEvent)(
    cl_event event) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int(CL_API_CALL *MAGICKpfn_clRetainEvent)(
    cl_event event) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int(CL_API_CALL *MAGICKpfn_clSetEventCallback)(
    cl_event event,cl_int command_exec_callback_type,
    void (CL_CALLBACK *MAGICKpfn_notify)(cl_event,cl_int,void *),
    void *user_data) CL_API_SUFFIX__VERSION_1_1;

/*
 *
 * vendor dispatch table structure
 *
 * note that the types in the structure KHRicdVendorDispatch mirror the function 
 * names listed in the string table khrIcdVendorDispatchFunctionNames
 *
 */

typedef struct MagickLibraryRec MagickLibrary;

struct MagickLibraryRec
{
  void * base;

  MAGICKpfn_clGetPlatformIDs          clGetPlatformIDs;
  MAGICKpfn_clGetPlatformInfo         clGetPlatformInfo;

  MAGICKpfn_clGetDeviceIDs            clGetDeviceIDs;
  MAGICKpfn_clGetDeviceInfo           clGetDeviceInfo;

  MAGICKpfn_clCreateContext           clCreateContext;
  MAGICKpfn_clReleaseContext          clReleaseContext;

  MAGICKpfn_clCreateCommandQueue      clCreateCommandQueue;
  MAGICKpfn_clReleaseCommandQueue     clReleaseCommandQueue;
  MAGICKpfn_clFlush                   clFlush;
  MAGICKpfn_clFinish                  clFinish;

  MAGICKpfn_clCreateBuffer            clCreateBuffer;
  MAGICKpfn_clRetainMemObject         clRetainMemObject;
  MAGICKpfn_clReleaseMemObject        clReleaseMemObject;
  MAGICKpfn_clCreateProgramWithSource clCreateProgramWithSource;
  MAGICKpfn_clCreateProgramWithBinary clCreateProgramWithBinary;
  MAGICKpfn_clReleaseProgram          clReleaseProgram;
  MAGICKpfn_clBuildProgram            clBuildProgram;
  MAGICKpfn_clGetProgramInfo          clGetProgramInfo;
  MAGICKpfn_clGetProgramBuildInfo     clGetProgramBuildInfo;

  MAGICKpfn_clCreateKernel            clCreateKernel;
  MAGICKpfn_clReleaseKernel           clReleaseKernel;
  MAGICKpfn_clSetKernelArg            clSetKernelArg;

  MAGICKpfn_clEnqueueReadBuffer       clEnqueueReadBuffer;
  MAGICKpfn_clEnqueueMapBuffer        clEnqueueMapBuffer;
  MAGICKpfn_clEnqueueUnmapMemObject   clEnqueueUnmapMemObject;
  MAGICKpfn_clEnqueueNDRangeKernel    clEnqueueNDRangeKernel;

  MAGICKpfn_clGetEventProfilingInfo   clGetEventProfilingInfo;

  MAGICKpfn_clGetEventInfo            clGetEventInfo;
  MAGICKpfn_clWaitForEvents           clWaitForEvents;
  MAGICKpfn_clReleaseEvent            clReleaseEvent;
  MAGICKpfn_clRetainEvent             clRetainEvent;
  MAGICKpfn_clSetEventCallback        clSetEventCallback;
};

struct _MagickCLEnv {
  MagickBooleanType OpenCLInitialized;  /* whether OpenCL environment is initialized. */
  MagickBooleanType OpenCLDisabled;	/* whether if OpenCL has been explicitely disabled. */

  MagickLibrary * library;

  /*OpenCL objects */
  cl_platform_id platform;
  cl_device_type deviceType;
  cl_device_id device;
  cl_context context;

  MagickBooleanType disableProgramCache; /* disable the OpenCL program cache */
  cl_program programs[MAGICK_OPENCL_NUM_PROGRAMS]; /* one program object maps one kernel source file */

  MagickBooleanType regenerateProfile;   /* re-run the microbenchmark in auto device selection mode */ 

  SemaphoreInfo* lock;

  cl_command_queue commandQueues[MAX_COMMAND_QUEUES];
  ssize_t commandQueuesPos;
  SemaphoreInfo* commandQueuesLock;
};


#if defined(MAGICKCORE_HDRI_SUPPORT)
#define CLOptions "-cl-single-precision-constant -cl-mad-enable -DMAGICKCORE_HDRI_SUPPORT=1 "\
  "-DCLQuantum=float -DCLSignedQuantum=float -DCLPixelType=float4 -DQuantumRange=%f " \
  "-DQuantumScale=%f -DCharQuantumScale=%f -DMagickEpsilon=%f -DMagickPI=%f "\
  " -DMaxMap=%u -DMAGICKCORE_QUANTUM_DEPTH=%u"
#define CLPixelPacket  cl_float4
#define CLCharQuantumScale 1.0f
#elif (MAGICKCORE_QUANTUM_DEPTH == 8)
#define CLOptions "-cl-single-precision-constant -cl-mad-enable " \
  "-DCLQuantum=uchar -DCLSignedQuantum=char -DCLPixelType=uchar4 -DQuantumRange=%ff " \
  "-DQuantumScale=%ff -DCharQuantumScale=%ff -DMagickEpsilon=%ff -DMagickPI=%ff "\
  "-DMaxMap=%u -DMAGICKCORE_QUANTUM_DEPTH=%u"
#define CLPixelPacket  cl_uchar4
#define CLCharQuantumScale 1.0f
#elif (MAGICKCORE_QUANTUM_DEPTH == 16)
#define CLOptions "-cl-single-precision-constant -cl-mad-enable " \
  "-DCLQuantum=ushort -DCLSignedQuantum=short -DCLPixelType=ushort4 -DQuantumRange=%f "\
  "-DQuantumScale=%f -DCharQuantumScale=%f -DMagickEpsilon=%f -DMagickPI=%f "\
  "-DMaxMap=%u -DMAGICKCORE_QUANTUM_DEPTH=%u"
#define CLPixelPacket  cl_ushort4
#define CLCharQuantumScale 257.0f
#elif (MAGICKCORE_QUANTUM_DEPTH == 32)
#define CLOptions "-cl-single-precision-constant -cl-mad-enable " \
  "-DCLQuantum=uint -DCLSignedQuantum=int -DCLPixelType=uint4 -DQuantumRange=%f "\
  "-DQuantumScale=%f -DCharQuantumScale=%f -DMagickEpsilon=%f -DMagickPI=%f "\
  "-DMaxMap=%u -DMAGICKCORE_QUANTUM_DEPTH=%u"
#define CLPixelPacket  cl_uint4
#define CLCharQuantumScale 16843009.0f
#elif (MAGICKCORE_QUANTUM_DEPTH == 64)
#define CLOptions "-cl-single-precision-constant -cl-mad-enable " \
  "-DCLQuantum=ulong -DCLSignedQuantum=long -DCLPixelType=ulong4 -DQuantumRange=%f "\
  "-DQuantumScale=%f -DCharQuantumScale=%f -DMagickEpsilon=%f -DMagickPI=%f "\
  "-DMaxMap=%u -DMAGICKCORE_QUANTUM_DEPTH=%u"
#define CLPixelPacket  cl_ulong4
#define CLCharQuantumScale 72340172838076673.0f
#endif

typedef enum {
  AddNoiseKernel,
  BlurRowKernel,
  BlurColumnKernel,
  CompositeKernel,
  ComputeFunctionKernel,
  ContrastKernel,
  ContrastStretchKernel,
  ConvolveKernel,
  EqualizeKernel,
  GrayScaleKernel,
  HistogramKernel,
  HullPass1Kernel,
  HullPass2Kernel,
  LocalContrastBlurRowKernel,
  LocalContrastBlurApplyColumnKernel,
  ModulateKernel,
  MotionBlurKernel,
  RadialBlurKernel,
  RandomNumberGeneratorKernel,
  ResizeHorizontalKernel,
  ResizeVerticalKernel,
  UnsharpMaskBlurColumnKernel,
  UnsharpMaskKernel,
  WaveletDenoiseKernel,
  KERNEL_COUNT
} ProfiledKernels;

extern MagickPrivate cl_context
  GetOpenCLContext(MagickCLEnv);

extern MagickPrivate cl_kernel
  AcquireOpenCLKernel(MagickCLEnv, MagickOpenCLProgram, const char*);

extern MagickPrivate cl_command_queue
  AcquireOpenCLCommandQueue(MagickCLEnv);

extern MagickPrivate MagickBooleanType
  OpenCLThrowMagickException(ExceptionInfo *,
    const char *,const char *,const size_t,
    const ExceptionType,const char *,const char *,...),
  RecordProfileData(MagickCLEnv,ProfiledKernels,cl_event),
  RelinquishMagickOpenCLEnv(MagickCLEnv),
  RelinquishOpenCLCommandQueue(MagickCLEnv, cl_command_queue),
  RelinquishOpenCLKernel(MagickCLEnv, cl_kernel);

extern MagickPrivate MagickCLEnv
  AcquireMagickOpenCLEnv(),
  SetDefaultOpenCLEnv(MagickCLEnv);

extern MagickPrivate unsigned long
  GetOpenCLDeviceLocalMemorySize(MagickCLEnv),
  GetOpenCLDeviceMaxMemAllocSize(MagickCLEnv);

extern MagickPrivate const char*
  GetOpenCLCachedFilesDirectory();

extern MagickPrivate void
  OpenCLLog(const char*),
  OpenCLTerminus();

/* #define OPENCLLOG_ENABLED 1 */
static inline void OpenCLLogException(const char* function, 
                        const unsigned int line, 
                        ExceptionInfo* exception) {
#ifdef OPENCLLOG_ENABLED
  if (exception->severity!=0) {
    char message[MaxTextExtent];
    /*  dump the source into a file */
    (void) FormatLocaleString(message,MaxTextExtent,"%s:%d Exception(%d):%s "
        ,function,line,exception->severity,exception->reason);
    OpenCLLog(message);
  }
#else
  magick_unreferenced(function);
  magick_unreferenced(line);
  magick_unreferenced(exception);
#endif
}
#endif

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
