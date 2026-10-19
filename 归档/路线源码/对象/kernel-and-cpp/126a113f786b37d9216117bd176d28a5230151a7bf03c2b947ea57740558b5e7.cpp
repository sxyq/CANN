#include <cstdint>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

using aclrtStream = void*;
#ifndef TENSOR_GROUP_INFO_DEFINED
#define TENSOR_GROUP_INFO_DEFINED
struct TensorInfo { const int64_t* shape; int64_t numDims; int32_t dtype; };
struct TensorGroupInfo { const TensorInfo* tensors; int64_t numTensors; };
#endif

#include "submission.asc"

extern "C" {
typedef int aclError;
enum { ACL_SUCCESS = 0 };
aclError aclInit(const char*);
aclError aclrtSetDevice(int);
aclError aclrtGetDeviceInfo(int, int, long long*);
aclError aclrtCreateStream(aclrtStream*);
aclError aclrtDestroyStream(aclrtStream);
aclError aclrtMalloc(void**, unsigned long, int);
aclError aclrtFree(void*);
aclError aclrtMemcpy(void*, unsigned long, const void*, unsigned long, int);
aclError aclrtSynchronizeStream(aclrtStream);
aclError aclFinalize();
}

static uint16_t F2B(float v){uint32_t b;memcpy(&b,&v,4);b+=0x7fffU+((b>>16)&1U);return (uint16_t)(b>>16);}
static float B2F(uint16_t v){uint32_t b=(uint32_t)v<<16;float r;memcpy(&r,&b,4);return r;}
static uint16_t F2H(float v){uint32_t b;memcpy(&b,&v,4);uint32_t s=(b>>16)&0x8000U;int e=(int)((b>>23)&0xFF)-127+15;uint32_t m=b&0x7FFFFFU;if(e<=0)return (uint16_t)s;if(e>=31)return (uint16_t)(s|0x7C00U);return (uint16_t)(s|((uint32_t)e<<10)|(m>>13));}
static float H2F(uint16_t v){uint32_t s=(uint32_t)(v&0x8000U)<<16;uint32_t e=(v>>10)&0x1FU;uint32_t m=v&0x3FFU;uint32_t b;if(e==0)b=s;else if(e==31)b=s|0x7F800000U|(m<<13);else b=s|((e-15+127)<<23)|(m<<13);float r;memcpy(&r,&b,4);return r;}
static void St(uint8_t* d,size_t i,int32_t dt,float v){if(dt==0){memcpy(d+i*4,&v,4);}else if(dt==1){uint16_t h=F2H(v);memcpy(d+i*2,&h,2);}else{uint16_t h=F2B(v);memcpy(d+i*2,&h,2);}}
static float Ld(const uint8_t* d,size_t i,int32_t dt){if(dt==0){float v;memcpy(&v,d+i*4,4);return v;}uint16_t b;memcpy(&b,d+i*2,2);return dt==1?H2F(b):B2F(b);}

static bool RunCase(aclrtStream stream,long long cores,int32_t dt,int64_t rows,int64_t width,const char* note){
    size_t inE=(size_t)(rows*width),vcE=(size_t)width;
    size_t es=dt==0?4:2;
    size_t inB=inE*es,vcB=vcE*es;
    std::vector<uint8_t> x(inB),r(inB),g(vcB),b(vcB),out(inB);
    for(size_t i=0;i<inE;i++){St(x.data(),i,dt,(float)((int)(i*17%101)-50)*0.0015f);St(r.data(),i,dt,(float)((int)(i*13%71)-35)*0.0011f);}
    for(size_t j=0;j<vcE;j++){St(g.data(),j,dt,0.8f+(float)(j%19)*0.003f);St(b.data(),j,dt,(float)((int)(j%23)-11)*0.001f);}
    void *dx=0,*dr=0,*dg=0,*db=0,*dout=0;
    size_t ab[]={inB,inB,vcB,vcB,inB};void** dp[]={&dx,&dr,&dg,&db,&dout};
    for(int i=0;i<5;i++)if(aclrtMalloc(dp[i],ab[i],0)!=0)return false;
    if(aclrtMemcpy(dx,inB,x.data(),inB,1)||aclrtMemcpy(dr,inB,r.data(),inB,1)||aclrtMemcpy(dg,vcB,g.data(),vcB,1)||aclrtMemcpy(db,vcB,b.data(),vcB,1))return false;
    int64_t is[]={rows,width};int64_t vs[]={width};
    TensorInfo ii{is,2,dt},vi{vs,1,dt};TensorGroupInfo ig{&ii,1},vg{&vi,1};
    run_kernel((GM_ADDR)dx,ig,(GM_ADDR)dr,ig,(GM_ADDR)dg,vg,(GM_ADDR)db,vg,(GM_ADDR)dout,ig,cores,stream,1e-5f);
    if(aclrtSynchronizeStream(stream)||aclrtMemcpy(out.data(),inB,dout,inB,2))return false;
    float tol=dt==0?3e-5f:(dt==1?2.5e-3f:1.5e-2f);
    float maxErr=0;long long bad=0;
    for(int64_t row=0;row<rows;row++){
        double sq=0;for(int64_t j=0;j<width;j++){float v=Ld(x.data(),(size_t)(row*width+j),dt)+Ld(r.data(),(size_t)(row*width+j),dt);sq+=(double)v*v;}
        float invRms=1.0f/std::sqrt((float)(sq/width)+1e-5f);
        for(int64_t j=0;j<width;j++){
            size_t idx=(size_t)(row*width+j);
            float v=(Ld(x.data(),idx,dt)+Ld(r.data(),idx,dt))*invRms*Ld(g.data(),(size_t)j,dt)+Ld(b.data(),(size_t)j,dt);
            uint8_t exp[4];St(exp,0,dt,v);
            float err=std::fabs(Ld(out.data(),idx,dt)-Ld(exp,0,dt));
            if(err>maxErr)maxErr=err;if(err>tol){bad++;if(bad<=3)fprintf(stderr,"mismatch dt=%d r=%lld w=%lld idx=%zu\n",dt,(long long)rows,(long long)width,idx);}
        }
    }
    printf("CASE dtype=%d rows=%lld width=%lld note=%s max_abs=%.8g bad=%lld %s\n",dt,(long long)rows,(long long)width,note,maxErr,(long long)bad,bad==0?"PASS":"FAIL");
    for(void** p:dp)if(*p)aclrtFree(*p);
    return bad==0;
}
int main(int argc,char**argv){
    int dev=argc>1?atoi(argv[1]):4;
    if(aclInit(0)||aclrtSetDevice(dev))return 1;
    long long cores=0;aclrtGetDeviceInfo(dev,0,&cores);
    printf("device=%d cores=%lld\n",dev,cores);
    aclrtStream s=0;if(aclrtCreateStream(&s))return 1;
    bool ok=true;
    struct{int32_t dt;int64_t r,w;const char*n;}cs[]={
      {1,8,16384,"lp-primary-fp16-tc4"},
      {1,4,32768,"lp-primary-fp16-tc8"},
      {2,8,16384,"lp-primary-bf16-tc4"},
      {1,8,12288,"lp-small-fp16-tc3"},
      {1,8,8192,"control-non-lp-fp16"},
      {0,4,16384,"control-fullcache-fp32"},
      {0,33,100,"control-narrow-fp32"}
    };
    for(auto&c:cs)if(!RunCase(s,cores,c.dt,c.r,c.w,c.n))ok=false;
    aclrtDestroyStream(s);aclFinalize();
    printf("NPU_CORRECTNESS_SUMMARY %s\n",ok?"PASS":"FAIL");
    return ok?0:1;
}
