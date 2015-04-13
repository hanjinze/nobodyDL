#ifndef XPU_H_
#define XPU_H_

#include <stdio.h>
#include <sys/types.h>
#ifdef __CUDACC__
  #include <cuda.h>
  #include <driver_types.h>
  #include <thrust/device_vector.h>
  #include <cublas_v2.h>
  #include <cusparse.h>
  #include <curand.h>
  #include <cudnn.h>
  #include <npp.h>
  #include <nppdefs.h>
#else
  #include <math.h>
  #include <mkl.h>
  #include <mkl_cblas.h>
  #include <mkl_vsl.h>
  #include <mkl_vsl_functions.h>
#endif

#define CUDA_NUM_THREADS 1024

#ifdef __CUDACC__

  #ifdef __CUDA_ARCH__
    #define HEMI_DEV_CODE
  #endif

  #define CUDA_MANAGED true
  #define USE_CUDNN false

  #define XPU_KERNEL(name)		__global__ void name ## _kernel

  #if defined(DEBUG) || defined(_DEBUG)
    #define XPU_KERNEL_LAUNCH(name, gridDim, blockDim, sharedBytes, streamId, ...) \
      do { \
        name ## _kernel<<< (gridDim), (blockDim), (sharedBytes), (streamId) >>>(__VA_ARGS__); \
        cudaAsyncCheck (name); \
      } while(0)
  #else
    #define XPU_KERNEL_LAUNCH(name, gridDim, blockDim, sharedBytes, streamId, ...) \
      name ## _kernel<<< (gridDim) , (blockDim), (sharedBytes), (streamId) >>>(__VA_ARGS__)
  #endif

  #define XPU_CALLABLE			__host__ __device__
  #define XPU_CALLABLE_INLINE		__host__ __device__ inline

  #if !defined(HEMI_ALIGN)
    #define HEMI_ALIGN(n)		__align__(n)
  #endif

  #define XPU_GET_ELEMENT_OFFSET	blockDim.x * blockIdx.x + threadIdx.x
  #define XPU_GET_ELEMENT_STRIDE	blockDim.x * gridDim.x

#else
  #define CUDA_MANAGED false
  #define XPU_KERNEL(name)		void name
  #define XPU_KERNEL_LAUNCH(name, gridDim, blockDim, sharedBytes, streamId, ...) name(__VA_ARGS__)

  #define XPU_CALLABLE
  #define XPU_CALLABLE_INLINE		inline

  #define XPU_GET_ELEMENT_OFFSET	0
  #define XPU_GET_ELEMENT_STRIDE	1

#endif

#define kernel_for(i, n) \
  _Pragma ("omp parallel for") \
  for (int i = XPU_GET_ELEMENT_OFFSET; i < n; i += XPU_GET_ELEMENT_STRIDE)

void cuda_set_device (const int device_id);
int  cuda_get_blocks (const int N);

class XPUCtx {
public:
  void set (const int device_id);
#ifdef __CUDACC__
  cudaStream_t stream_;
  cudaEvent_t  event_;
  cublasHandle_t    cublas_;
  curandGenerator_t curand_;
  cudnnHandle_t     cudnn_;
#endif
};

extern XPUCtx *dnn_context;

enum memcpy_t
{ CPU2GPU	= 1,
  GPU2CPU	= 2,
  CPU2CPU	= 3,
  GPU2GPU	= 4
};

template <typename T>
void cuda_check (const T &status);
void cuda_sync_check  (const char *msg);
void cuda_async_check (const char *msg);

#ifdef __CUDACC__
static cublasHandle_t    cublasHandle;

enum cudaMemcpyKind get_memcpy_type (enum memcpy_t kind);
void cuda_malloc (void **ptr, const size_t len);
void cuda_memcpy       (void *dst, const void *src, const size_t size, enum memcpy_t kind);
void cuda_memcpy_async (void *dst, const void *src, const size_t size, enum memcpy_t kind);

const char *cuda_get_status (const cudaError_t    &status);
const char *cuda_get_status (const cublasStatus_t &status);
const char *cuda_get_status (const curandStatus_t &status);
const char *cuda_get_status (const NppStatus      &status);

template <typename DT>
struct SharedMemory
{ __device__ DT*     getPointer() { return (DT*)0;  }
};
template <>
struct SharedMemory <float>
{ __device__ float * getPointer() { extern __shared__ float  s_float [];  return s_float;  }
};
template <>
struct SharedMemory <double>
{ __device__ double* getPointer() { extern __shared__ double s_double[];  return s_double;  }
};
#endif

class GPU {
public:
  void *operator new (size_t len);
  void operator delete(void *ptr);
};

class CPU {
};

#endif