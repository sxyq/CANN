#include <cstdint>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

using aclrtStream = void*;
#ifndef TENSOR_GROUP_INFO_DEFINED
#define TENSOR_GROUP_INFO_DEFINED
struct TensorInfo {
    const int64_t* shape;
    int64_t numDims;
    int32_t dtype;
};
struct TensorGroupInfo {
    const TensorInfo* tensors;
    int64_t numTensors;
};
#endif

#include "submission.asc"

// ACL declarations (host runtime)
extern "C" {
typedef int aclError;
typedef void* aclrtContext;
enum { ACL_SUCCESS = 0 };
enum { ACL_MEM_MALLOC_HUGE_FIRST = 0 };
enum { ACL_MEMCPY_HOST_TO_DEVICE = 1 };
enum { ACL_MEMCPY_DEVICE_TO_HOST = 2 };
enum { ACL_DEV_ATTR_VECTOR_CORE_NUM = 0 };
aclError aclInit(const char*);
aclError aclrtSetDevice(int);
aclError aclrtGetDeviceInfo(int, int, long long*);
aclError aclrtCreateStream(aclrtStream*);
aclError aclrtDestroyStream(aclrtStream);
aclError aclrtMalloc(void**, size_t, int);
aclError aclrtFree(void*);
aclError aclrtMemcpy(void*, size_t, const void*, size_t, int);
aclError aclrtSynchronizeStream(aclrtStream);
aclError aclFinalize();
}

namespace {
constexpr int32_t kFp32 = 0;
constexpr int32_t kFp16 = 1;
constexpr int32_t kBf16 = 2;
constexpr float kEpsilon = 1.0e-5f;

size_t ElementSize(int32_t dtype) { return dtype == kFp32 ? 4 : 2; }

uint16_t FloatToBf16(float v) {
    uint32_t b; std::memcpy(&b, &v, 4);
    b += 0x7fffU + ((b >> 16) & 1U);
    return (uint16_t)(b >> 16);
}
float Bf16ToFloat(uint16_t v) {
    uint32_t b = (uint32_t)v << 16; float r; std::memcpy(&r, &b, 4); return r;
}
uint16_t FloatToFp16(float v) {
    uint32_t b; std::memcpy(&b, &v, 4);
    uint32_t sign = (b >> 16) & 0x8000U;
    int exp = (int)((b >> 23) & 0xFF) - 127 + 15;
    uint32_t man = b & 0x7FFFFFU;
    if (exp <= 0) return (uint16_t)sign;
    if (exp >= 31) return (uint16_t)(sign | 0x7C00U);
    return (uint16_t)(sign | ((uint32_t)exp << 10) | (man >> 13));
}
float Fp16ToFloat(uint16_t v) {
    uint32_t sign = (uint32_t)(v & 0x8000U) << 16;
    uint32_t exp = (v >> 10) & 0x1FU;
    uint32_t man = v & 0x3FFU;
    uint32_t b;
    if (exp == 0) b = sign;
    else if (exp == 31) b = sign | 0x7F800000U | (man << 13);
    else b = sign | ((exp - 15 + 127) << 23) | (man << 13);
    float r; std::memcpy(&r, &b, 4); return r;
}
void Store(std::vector<uint8_t>& d, size_t i, int32_t dt, float v) {
    if (dt == kFp32) { std::memcpy(d.data()+i*4, &v, 4); }
    else if (dt == kFp16) { uint16_t h=FloatToFp16(v); std::memcpy(d.data()+i*2, &h, 2); }
    else { uint16_t h=FloatToBf16(v); std::memcpy(d.data()+i*2, &h, 2); }
}
float Load(const std::vector<uint8_t>& d, size_t i, int32_t dt) {
    if (dt == kFp32) { float v; std::memcpy(&v, d.data()+i*4, 4); return v; }
    uint16_t b; std::memcpy(&b, d.data()+i*2, 2);
    return dt == kFp16 ? Fp16ToFloat(b) : Bf16ToFloat(b);
}
float LoadRaw(const uint8_t* d, size_t i, int32_t dt) {
    if (dt == kFp32) { float v; std::memcpy(&v, d+i*4, 4); return v; }
    uint16_t b; std::memcpy(&b, d+i*2, 2);
    return dt == kFp16 ? Fp16ToFloat(b) : Bf16ToFloat(b);
}
void StoreRaw(uint8_t* d, size_t i, int32_t dt, float v) {
    if (dt == kFp32) { std::memcpy(d+i*4, &v, 4); }
    else if (dt == kFp16) { uint16_t h=FloatToFp16(v); std::memcpy(d+i*2, &h, 2); }
    else { uint16_t h=FloatToBf16(v); std::memcpy(d+i*2, &h, 2); }
}

bool RunCase(aclrtStream stream, int64_t cores, int32_t dtype, int64_t rows, int64_t width, const char* note) {
    const size_t inElems = (size_t)(rows * width);
    const size_t vecElems = (size_t)width;
    const size_t inBytes = inElems * ElementSize(dtype);
    const size_t vecBytes = vecElems * ElementSize(dtype);
    std::vector<uint8_t> x(inBytes), r(inBytes), g(vecBytes), b(vecBytes), out(inBytes), exp(ElementSize(dtype));
    for (size_t i = 0; i < inElems; ++i) {
        Store(x, i, dtype, (float)((int)(i*17%101)-50)*0.0015f);
        Store(r, i, dtype, (float)((int)(i*13%71)-35)*0.0011f);
    }
    for (size_t j = 0; j < vecElems; ++j) {
        Store(g, j, dtype, 0.8f + (float)(j%19)*0.003f);
        Store(b, j, dtype, (float)((int)(j%23)-11)*0.001f);
    }
    void *dx=nullptr,*dr=nullptr,*dg=nullptr,*db=nullptr,*dout=nullptr;
    size_t allocBytes[] = {inBytes,inBytes,vecBytes,vecBytes,inBytes};
    void** devPtrs[] = {&dx,&dr,&dg,&db,&dout};
    auto release=[&](){ for(void** p:devPtrs) if(*p){ aclrtFree(*p); *p=nullptr; } };
    for (int i=0;i<5;i++) if(aclrtMalloc(devPtrs[i],allocBytes[i],0)!=0){ release(); return false; }
    if (aclrtMemcpy(dx,inBytes,x.data(),inBytes,1)!=0 || aclrtMemcpy(dr,inBytes,r.data(),inBytes,1)!=0 ||
        aclrtMemcpy(dg,vecBytes,g.data(),vecBytes,1)!=0 || aclrtMemcpy(db,vecBytes,b.data(),vecBytes,1)!=0) {
        release(); return false;
    }
    int64_t inShape[] = {rows, width};
    int64_t vecShape[] = {width};
    TensorInfo inInfo{inShape, 2, dtype};
    TensorInfo vecInfo{vecShape, 1, dtype};
    TensorGroupInfo inGroup{&inInfo, 1};
    TensorGroupInfo vecGroup{&vecInfo, 1};
    run_kernel((GM_ADDR)dx, inGroup, (GM_ADDR)dr, inGroup, (GM_ADDR)dg, vecGroup,
               (GM_ADDR)db, vecGroup, (GM_ADDR)dout, inGroup, cores, stream, kEpsilon);
    if (aclrtSynchronizeStream(stream)!=0 || aclrtMemcpy(out.data(),inBytes,dout,inBytes,2)!=0) {
        release(); return false;
    }
    const float tol = dtype==kFp32 ? 3e-5f : (dtype==kFp16 ? 2.5e-3f : 1.5e-2f);
    float maxErr=0; int64_t bad=0;
    for (int64_t row=0;row<rows;row++) {
        double sq=0;
        for (int64_t j=0;j<width;j++) {
            float v = Load(x,(size_t)(row*width+j),dtype)+Load(r,(size_t)(row*width+j),dtype);
            sq += (double)v*v;
        }
        float invRms = 1.0f/std::sqrt((float)(sq/width)+kEpsilon);
        for (int64_t j=0;j<width;j++) {
            size_t idx=(size_t)(row*width+j);
            float v = (Load(x,idx,dtype)+Load(r,idx,dtype))*invRms*Load(g,(size_t)j,dtype)+Load(b,(size_t)j,dtype);
            StoreRaw(exp.data(),0,dtype,v);
            float err = std::fabs(Load(out,idx,dtype)-LoadRaw(exp.data(),0,dtype));
            if(err>maxErr) maxErr=err;
            if(err>tol){ bad++; if(bad<=3) fprintf(stderr,"mismatch dt=%d r=%lld w=%lld idx=%zu got=%.8g exp=%.8g err=%.8g\n",dtype,(long long)rows,(long long)width,idx,Load(out,idx,dtype),LoadRaw(exp.data(),0,dtype),err); }
        }
    }
    printf("CASE dtype=%d rows=%lld width=%lld note=%s max_abs_error=%.8g bad=%lld %s\n",
           dtype,(long long)rows,(long long)width,note,maxErr,(long long)bad,bad==0?"PASS":"FAIL");
    release();
    return bad==0;
}
} // namespace

int main(int argc, char** argv) {
    int deviceId = argc>1 ? atoi(argv[1]) : 4;
    if (aclInit(nullptr)!=0 || aclrtSetDevice(deviceId)!=0) return 1;
    long long cores=0;
    aclrtGetDeviceInfo(deviceId, 0, (long long*)&cores);
    printf("device=%d availableCoreNum=%lld\n", deviceId, (long long)cores);
    aclrtStream stream=nullptr;
    if (aclrtCreateStream(&stream)!=0) return 1;
    struct { int32_t dt; int64_t rows; int64_t width; const char* note; } cases[] = {
        {0, 33, 100, "rowGroup=2"},
        {0, 17, 256, "rowGroup=1-control"},
        {1, 33, 100, "rowGroup=4"},
        {2, 33, 100, "rowGroup=4"},
        {1, 17, 257, "rowGroup=16"},
        {0, 7, 65, "rowGroup=8-edge"},
    };
    bool passed=true;
    for (auto& c : cases) if(!RunCase(stream,cores,c.dt,c.rows,c.width,c.note)) passed=false;
    aclrtDestroyStream(stream); aclFinalize();
    printf("NPU_CORRECTNESS_SUMMARY %s\n", passed?"PASS":"FAIL");
    return passed?0:1;
}
