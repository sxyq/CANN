#include "judge_types.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <dlfcn.h>
#include <filesystem>
#include <random>
#include <string>
#include <vector>

namespace {
constexpr float kEpsilon = 1e-5f;
constexpr int kWarmup = 60, kBlocks = 2, kPairs = 31;
struct Case { const char* name; int rows; int width; bool timed; };
// First two shapes and the random input recipe are from W3 R2 V040.
// The tiny input is from R12. Remaining shapes cover source-derived bounds and both Load sites.
const Case cases[] = {{"on-16x2048",16,2048,true}, {"on-16x2056",16,2056,true},
    {"off-1x64",1,64,true}, {"lower-16x129",16,129,false},
    {"upper-16x4096",16,4096,false}, {"resident-80x2056",80,2056,false}};
struct Call { int ordinal; std::string shape, phase, side; int blocks; };
struct Sample { int ordinal; std::string shape; int block, pair, position; std::string side; double eventUs, wallUs; };
std::vector<Call> calls;
std::vector<Sample> samples;
void Require(aclError rc, const char* what) {
    if (rc != ACL_SUCCESS) { std::fprintf(stderr,"ACL_FAILURE operation=%s code=%d\n",what,rc); std::exit(2); }
}
FILE* Open(const std::string& path) {
    FILE* f=std::fopen(path.c_str(),"wx");
    if (!f) { std::perror(path.c_str()); std::exit(2); }
    return f;
}
void Memory(const char* phase) {
    size_t freeBytes=0,totalBytes=0;
    Require(aclrtGetMemInfo(ACL_HBM_MEM,&freeBytes,&totalBytes),"memory");
    std::printf("RESOURCE phase=%s free_hbm_mib=%.6f total_hbm_mib=%.6f\n",phase,freeBytes/1048576.,totalBytes/1048576.);
    if(freeBytes < 100ull*1024*1024) std::exit(4);
}
struct Data {
    const Case& spec;
    int64_t ds[2],ps[1]; TensorInfo dt,pt; TensorGroupInfo dg,pg;
    std::vector<float> x,r,g,b,out; std::vector<double> ref;
    void* dev[5]{}; size_t bytes[5]{};
    explicit Data(const Case& c):spec(c),ds{c.rows,c.width},ps{c.width},
        dt{ds,2,0},pt{ps,1,0},dg{&dt,1},pg{&pt,1},
        x(size_t(c.rows)*c.width),r(x.size()),g(c.width),b(c.width),out(x.size(),NAN),ref(x.size()) {
        std::mt19937 gen(314159u+c.width);
        std::uniform_real_distribution<float> dist(-.5f,.5f);
        for(auto& v:x) v=dist(gen);
        for(auto& v:r) v=dist(gen)*.25f;
        for(auto& v:g) v=dist(gen)+1.f;
        for(auto& v:b) v=dist(gen)*.125f;
        if(c.rows==1 && c.width==64) {
            for(int i=0;i<c.width;++i) {
                x[i]=.19f*std::sin(float((i*17)%997)*.013f);
                r[i]=.11f*std::cos(float((i*29)%991)*.017f);
                g[i]=.85f+float(i%23)*.002f; b[i]=-.025f+float(i%19)*.001f;
            }
        }
        for(int row=0;row<c.rows;++row) {
            double sum=0;
            for(int col=0;col<c.width;++col) { size_t i=size_t(row)*c.width+col; double y=double(x[i])+r[i]; sum+=y*y; }
            double inv=1./std::sqrt(sum/c.width+double(kEpsilon));
            for(int col=0;col<c.width;++col) { size_t i=size_t(row)*c.width+col; ref[i]=(double(x[i])+r[i])*inv*g[col]+b[col]; }
        }
        const std::vector<float>* host[]={&x,&r,&g,&b,&out};
        for(int j=0;j<5;++j) {
            bytes[j]=host[j]->size()*sizeof(float);
            Require(aclrtMalloc(&dev[j],bytes[j],ACL_MEM_MALLOC_HUGE_FIRST),"allocate");
            Require(aclrtMemcpy(dev[j],bytes[j],host[j]->data(),bytes[j],ACL_MEMCPY_HOST_TO_DEVICE),"input");
        }
    }
    ~Data() { for(void* p:dev) if(p) Require(aclrtFree(p),"free"); }
};
void Launch(Kernel kernel, Data& d, int64_t cores, aclrtStream stream,
            const char* phase,const char* side) {
    kernel(d.dev[0],d.dg,d.dev[1],d.dg,d.dev[2],d.pg,d.dev[3],d.pg,d.dev[4],d.dg,cores,stream,kEpsilon);
    calls.push_back({int(calls.size()+1),d.spec.name,phase,side,int(std::min<int64_t>(cores,d.spec.rows))});
}
bool Reference(Kernel kernel,Data& d,int64_t cores,aclrtStream stream,const char* side,FILE* f) {
    std::fill(d.out.begin(),d.out.end(),NAN);
    Require(aclrtMemcpy(d.dev[4],d.bytes[4],d.out.data(),d.bytes[4],ACL_MEMCPY_HOST_TO_DEVICE),"sentinel");
    Launch(kernel,d,cores,stream,"reference",side);
    Require(aclrtSynchronizeStream(stream),"reference sync");
    Require(aclrtMemcpy(d.out.data(),d.bytes[4],d.dev[4],d.bytes[4],ACL_MEMCPY_DEVICE_TO_HOST),"output");
    size_t failures=0; double maxAbs=0;
    for(size_t i=0;i<d.out.size();++i) {
        const size_t col=i%d.spec.width;
        double error=std::fabs(double(d.out[i])-d.ref[i]);
        double tol=std::ldexp(1.,-16)+std::ldexp(1.,-10)*std::fabs(d.ref[i]);
        bool pass=std::isfinite(d.out[i]) && error<=tol && error<=.01;
        failures+=!pass; if(std::isfinite(error)) maxAbs=std::max(error,maxAbs);
        std::fprintf(f,"%s\t%s\t%zu\t%.9g\t%.9g\t%.9g\t%.9g\t%.17g\t%.9g\t%.17g\t%.17g\t%d\n",
            d.spec.name,side,i,d.x[i],d.r[i],d.g[col],d.b[col],d.ref[i],d.out[i],error,tol,int(pass));
    }
    std::printf("CORRECTNESS case=%s side=%s reference=CPU_FP64 elements=%zu failures=%zu max_abs=%.17g result=%s\n",
        d.spec.name,side,d.out.size(),failures,maxAbs,failures?"FAIL":"PASS");
    return failures==0;
}
void Measure(Kernel p,Kernel c,bool same,Data& d,int64_t cores,aclrtStream stream,aclrtEvent start,aclrtEvent stop) {
    const char* labels[2]={same?"P1":"P",same?"P2":"C"};
    Kernel funcs[2]={p,c};
    for(int i=0;i<kWarmup;++i) { Launch(funcs[i%2],d,cores,stream,"warmup",labels[i%2]); Require(aclrtSynchronizeStream(stream),"warmup sync"); }
    for(int block=0;block<kBlocks;++block) for(int pair=0;pair<kPairs;++pair) for(int pos=0;pos<2;++pos) {
        int side=((block+pair)%2+pos)%2;
        auto t0=std::chrono::steady_clock::now();
        Require(aclrtRecordEvent(start,stream),"start");
        Launch(funcs[side],d,cores,stream,"timed",labels[side]);
        Require(aclrtRecordEvent(stop,stream),"stop");
        Require(aclrtSynchronizeEvent(stop),"sample sync");
        auto t1=std::chrono::steady_clock::now(); float elapsed=0;
        Require(aclrtEventElapsedTime(&elapsed,start,stop),"elapsed");
        samples.push_back({int(calls.size()),d.spec.name,block,pair,pos,labels[side],elapsed*1000.,std::chrono::duration<double,std::micro>(t1-t0).count()});
    }
}
Kernel Load(const std::filesystem::path& folder,const char* name,const char* symbol) {
    void* lib=dlopen((folder/name).c_str(),RTLD_NOW|RTLD_LOCAL);
    if(!lib) { std::fprintf(stderr,"DLOPEN_FAILURE %s\n",dlerror()); std::exit(2); }
    Kernel f=reinterpret_cast<Kernel>(dlsym(lib,symbol));
    if(!f) { std::fprintf(stderr,"DLSYM_FAILURE %s\n",dlerror()); std::exit(2); } return f;
}
}
int main(int argc,char** argv) {
    if(argc!=4) { std::fprintf(stderr,"usage: r05_probe parent|correctness|local DEVICE OUTPUT_PREFIX\n"); return 2; }
    std::string mode=argv[1],prefix=argv[3]; int device=std::stoi(argv[2]);
    if(mode!="parent" && mode!="correctness" && mode!="local") return 2;
    auto folder=std::filesystem::absolute(argv[0]).parent_path();
    Kernel parent=Load(folder,"libr05_parent.so","r05_parent_run_kernel");
    Kernel candidate=mode=="parent"?parent:Load(folder,"libr05_candidate.so","r05_candidate_run_kernel");
    Require(aclInit(nullptr),"init"); Require(aclrtSetDevice(device),"device");
    int64_t cores=0; Require(aclrtGetDeviceInfo(device,ACL_DEV_ATTR_VECTOR_CORE_NUM,&cores),"core count");
    if(cores<=0) return 2;
    aclrtStream stream=nullptr; Require(aclrtCreateStream(&stream),"stream");
    aclrtEvent start=nullptr,stop=nullptr;
    Require(aclrtCreateEvent(&start),"start event"); Require(aclrtCreateEvent(&stop),"stop event");
    calls.reserve(2000); samples.reserve(500);
    std::printf("RUN mode=%s device=%d cores=%ld warmup_total=%d blocks=%d pairs=%d one_output_address=YES raw_in_memory=YES\n",mode.c_str(),device,cores,kWarmup,kBlocks,kPairs);
    bool pass=true;
    if(mode!="local") {
        Memory("before_reference");
        FILE* ref=Open(prefix+".reference.tsv");
        std::fprintf(ref,"case\tside\tindex\tx\tresidual\tgamma\tbias\treference_fp64\tactual_fp32\tabs_error\ttolerance\tpass\n");
        for(const auto& spec:cases) {
            Data d(spec); pass=Reference(parent,d,cores,stream,"P",ref)&&pass;
            if(mode=="correctness") pass=Reference(candidate,d,cores,stream,"C",ref)&&pass;
        }
        std::fclose(ref);
    }
    if(pass && mode!="correctness") {
        Memory("before_local");
        for(const auto& spec:cases) if(spec.timed) {
            Data d(spec);
            std::printf("TIMING case=%s rows=%d width=%d blocks=%ld guard=%s\n",spec.name,spec.rows,spec.width,std::min<int64_t>(cores,spec.rows),spec.width>128?"ON":"OFF");
            Measure(parent,candidate,mode=="parent",d,cores,stream,start,stop);
        }
        Memory("after_local");
    }
    FILE* raw=Open(prefix+".raw.tsv");
    std::fprintf(raw,"launch_ordinal\tcase\tblock\tpair\tposition\tside\tdevice_us\twall_us\n");
    for(const auto& s:samples) std::fprintf(raw,"%d\t%s\t%d\t%d\t%d\t%s\t%.9f\t%.9f\n",s.ordinal,s.shape.c_str(),s.block,s.pair,s.position,s.side.c_str(),s.eventUs,s.wallUs);
    std::fclose(raw);
    FILE* trace=Open(prefix+".calls.tsv"); std::fprintf(trace,"launch_ordinal\tcase\tphase\tside\tblock_dim\n");
    for(const auto& c:calls) std::fprintf(trace,"%d\t%s\t%s\t%s\t%d\n",c.ordinal,c.shape.c_str(),c.phase.c_str(),c.side.c_str(),c.blocks);
    std::fclose(trace);
    Require(aclrtDestroyEvent(stop),"destroy stop"); Require(aclrtDestroyEvent(start),"destroy start");
    Require(aclrtDestroyStream(stream),"destroy stream"); Require(aclrtResetDevice(device),"release context"); Require(aclFinalize(),"finalize");
    std::printf("RUN_COMPLETE result=%d calls=%zu samples=%zu RUNNING_DEVICE_OPERATION=NONE\n",pass?0:3,calls.size(),samples.size());
    return pass?0:3;
}
