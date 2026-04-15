#pragma once

// ============================================================
// CUDA IntelliSense compatibility layer (macOS / non-nvcc)
// This file is NEVER seen by nvcc on Linux GPU builds
// ============================================================

#ifdef __INTELLISENSE__

// ---------- Function / attribute qualifiers ----------
#define __global__
#define __host__
#define __device__
#define __shared__
#define __constant__
#define __align__(n)
#define __launch_bounds__(t, b)
#define __device_builtin__
#define __cudart_builtin__
#define __noinline__
#define __forceinline__

// ---------- CUDA built-in vector types ----------
struct uint3 { unsigned int x, y, z; };
struct dim3 {
    unsigned int x, y, z;
    constexpr dim3(unsigned int vx = 1,
                   unsigned int vy = 1,
                   unsigned int vz = 1)
        : x(vx), y(vy), z(vz) {}
};

// ---------- CUDA runtime basic types ----------
typedef int cudaError_t;

static constexpr cudaError_t cudaSuccess = 0;

enum cudaMemcpyKind {
    cudaMemcpyHostToHost = 0,
    cudaMemcpyHostToDevice = 1,
    cudaMemcpyDeviceToHost = 2,
    cudaMemcpyDeviceToDevice = 3,
    cudaMemcpyDefault = 4,
    cudaMemcpyToSymbol = 5
};

// ---------- Memory management ----------
inline cudaError_t cudaMalloc(void** ptr, size_t size) {
    *ptr = nullptr;
    (void)size;
    return cudaSuccess;
}

inline cudaError_t cudaFree(void* ptr) {
    (void)ptr;
    return cudaSuccess;
}

// ---------- Memory copy ----------
inline cudaError_t cudaMemcpy(
    void* dst,
    const void* src,
    size_t count,
    cudaMemcpyKind kind
) {
    (void)dst;
    (void)src;
    (void)count;
    (void)kind;
    return cudaSuccess;
}

inline cudaError_t cudaMemset(
    void* devPtr,
    int value,
    size_t count
) {
    (void)devPtr;
    (void)value;
    (void)count;
    return cudaSuccess;
}

// ---------- Error handling ----------
inline const char* cudaGetErrorString(cudaError_t) {
    return "cudaSuccess (IntelliSense stub)";
}

inline cudaError_t cudaDeviceSynchronize() {
    return cudaSuccess;
}

// ---------- Streams ----------
typedef struct cudaStream_t_struct* cudaStream_t;

inline cudaError_t cudaStreamCreate(cudaStream_t* stream) {
    *stream = nullptr;
    return cudaSuccess;
}

inline cudaError_t cudaStreamDestroy(cudaStream_t stream) {
    (void)stream;
    return cudaSuccess;
}

inline cudaError_t cudaMemcpyAsync(
    void* dst,
    const void* src,
    size_t count,
    cudaMemcpyKind kind,
    cudaStream_t stream
) {
    (void)dst;
    (void)src;
    (void)count;
    (void)kind;
    (void)stream;
    return cudaSuccess;
}

// ---------- CUDA built-in variables ----------
extern uint3 threadIdx;
extern uint3 blockIdx;
extern dim3  blockDim;
extern dim3  gridDim;

// ---------- CUDA device intrinsics (stubs) ----------
__device__ __forceinline__ int   __syncthreads() { return 0; }
__device__ __forceinline__ int   __syncthreads_count(int) { return 0; }
__device__ __forceinline__ int   __syncthreads_and(int) { return 0; }
__device__ __forceinline__ int   __syncthreads_or(int) { return 0; }

__device__ __forceinline__ int   __lane_id() { return 0; }
__device__ __forceinline__ int   __warp_id() { return 0; }
__device__ __forceinline__ int   warpSize = 32;

// ---------- Atomic operation stubs ----------
template <typename T>
__device__ __forceinline__ T atomicAdd(T*, T) { return T{}; }

template <typename T>
__device__ __forceinline__ T atomicSub(T*, T) { return T{}; }

template <typename T>
__device__ __forceinline__ T atomicExch(T*, T) { return T{}; }

template <typename T>
__device__ __forceinline__ T atomicMin(T*, T) { return T{}; }

template <typename T>
__device__ __forceinline__ T atomicMax(T*, T) { return T{}; }

template <typename T>
__device__ __forceinline__ T atomicCAS(T*, T, T) { return T{}; }

// ---------- Math intrinsics (minimal) ----------
__device__ __forceinline__ float __fdividef(float a, float b) { return a / b; }
__device__ __forceinline__ float __sinf(float x) { return x; }
__device__ __forceinline__ float __cosf(float x) { return x; }
__device__ __forceinline__ float __sqrtf(float x) { return x; }

// ---------- Memory fence intrinsics ----------
__device__ __forceinline__ void __threadfence() {}
__device__ __forceinline__ void __threadfence_block() {}
__device__ __forceinline__ void __threadfence_system() {}

#endif // __INTELLISENSE__
