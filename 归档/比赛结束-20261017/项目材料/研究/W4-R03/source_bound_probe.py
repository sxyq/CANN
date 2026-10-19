#!/usr/bin/env python3
"""Emit a host-only trace from the committed Parent; no NPU kernel is launched."""

import re
import subprocess


SOURCE_REF = "de70b634813dea80783fc57716d6e95c158edeec"
SOURCE_PATH = "线上结果/R31B/V011/submission.asc"
WORKTREE = "/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R03-owner-occupancy-guard-x"


def without_comments(text):
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    return re.sub(r"//[^\n]*", "", text)


source = subprocess.check_output(
    ["git", "show", f"{SOURCE_REF}:{SOURCE_PATH}"], cwd=WORKTREE, text=True
)
host_start = source.index('extern "C" void run_kernel(')
host_end = source.index("\n#endif", host_start)
host = without_comments(source[host_start:host_end])
host, launch_count = re.subn(
    r"add_rms_norm_bias_custom<(float|half|bfloat16_t)>"
    r"<<<blockCount, nullptr, stream>>>",
    r"TraceLaunch<\1>",
    host,
)
assert launch_count == 3

dispatch_start = source.index("    __aicore__ inline void Process(")
dispatch_end = source.index("        const bool cacheRow", dispatch_start)
dispatch = without_comments(source[dispatch_start:dispatch_end])
dispatch = dispatch.replace(
    "__aicore__ inline void Process(", "template <typename T> void SourceDispatch(", 1
)
dispatch = dispatch.replace(
    "{", "{\n    const bool widePath_ = rowWidth > static_cast<uint64_t>(kCacheElems);", 1
)
dispatch = re.sub(r"\b(Process\w+)\(", r'TracePath("\1", ', dispatch)
dispatch += '\n    TracePath("GenericRow");\n}\n'

allocation_start = source.index("        const uint64_t blockIdx", dispatch_start)
allocation_end = source.index("        if constexpr", allocation_start)
allocation = without_comments(source[allocation_start:allocation_end])

constant_names = sorted(set(re.findall(r"\bk[A-Z]\w*", dispatch)))
constants = []
for name in constant_names:
    match = re.search(rf"static constexpr int32_t {name} = ([^;]+);", source)
    assert match is not None, name
    constants.append(f"static constexpr int32_t {name} = {match[1]};")

print(r'''
#include <acl/acl.h>
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <type_traits>

using GM_ADDR = void*;
struct half { uint16_t value; };
struct bfloat16_t { uint16_t value; };
struct TensorInfo { const int64_t* shape; int64_t numDims; int32_t dtype; };
struct TensorGroupInfo { const TensorInfo* tensors; int64_t numTensors; };

namespace AscendC {
uint32_t traceBlock = 0;
uint32_t GetBlockIdx() { return traceBlock; }
template <typename A, typename B> using IsSameType = std::is_same<A, B>;
}

int64_t liveCoreCount = 0;
const char* caseName = nullptr;
std::string selectedPath;
uint32_t observedLaunches = 0;

template <typename... Args>
void TracePath(const char* name, Args...) { selectedPath = name; }
''')
print("\n".join(constants))
print(dispatch)
print(r'''
template <typename T>
void TraceLaunch(GM_ADDR, GM_ADDR, GM_ADDR, GM_ADDR, GM_ADDR,
                 uint64_t rowCount, uint64_t rowWidth, uint32_t blockCount,
                 float invRowWidth, float epsilon)
{
    ++observedLaunches;
    const int dtype = std::is_same<T, float>::value ? 0 :
                      std::is_same<T, half>::value ? 1 : 2;
    const uint64_t expectedBlocks = std::min<uint64_t>(liveCoreCount, rowCount);
    if (blockCount != expectedBlocks) throw std::runtime_error("launch count differs");
    std::cout << "HOST case=" << caseName
              << " dtype=" << dtype << " availableCoreNum=" << liveCoreCount
              << " M=" << rowCount << " D=" << rowWidth
              << " blockCount=" << blockCount
              << " rowsPerBlock_floor=" << rowCount / blockCount
              << " rowsPerBlock_ceil=" << (rowCount + blockCount - 1) / blockCount
              << " extraRows=" << rowCount % blockCount
              << " occupancy=" << double(blockCount) / liveCoreCount << '\n';
    uint64_t nextRow = 0;
    uint32_t multirowBlocks = 0;
    std::map<std::string, uint32_t> pathCounts;
    for (uint32_t bid = 0; bid < blockCount; ++bid) {
        AscendC::traceBlock = bid;
''')
print(allocation)
print(r'''
        if (beginRow != nextRow || localRows == 0)
            throw std::runtime_error("row coverage differs");
        nextRow += localRows;
        multirowBlocks += localRows > 1;
        selectedPath.clear();
        SourceDispatch<T>(rowCount, rowWidth, blockCount, invRowWidth, epsilon);
        ++pathCounts[selectedPath];
        std::cout << "BLOCK case=" << caseName << " id=" << bid
                  << " beginRow=" << beginRow << " localRows=" << localRows
                  << " selectedPath=" << selectedPath << '\n';
    }
    if (nextRow != rowCount) throw std::runtime_error("total row count differs");
    std::cout << "COVERAGE case=" << caseName << " rows=" << nextRow
              << " localRows_gt_1_blocks=" << multirowBlocks
              << " unique_contiguous_rows=PASS\n";
    for (const auto& item : pathCounts)
        std::cout << "DISPATCH case=" << caseName << " path=" << item.first
                  << " blocks=" << item.second << '\n';
}
''')
print(host)
print(r'''
struct CaseSpec { const char* name; int64_t rows; int64_t width; int32_t dtype; };

int main(int argc, char** argv)
{
    if (argc != 2) return 2;
    std::cout.setf(std::ios::unitbuf);
    const int device = std::atoi(argv[1]);
    const int initRc = aclInit(nullptr);
    std::cout << "ACL_INIT_RC=" << initRc << '\n';
    if (initRc != 0) return 1;
    const int infoRc = aclrtGetDeviceInfo(device, ACL_DEV_ATTR_VECTOR_CORE_NUM, &liveCoreCount);
    std::cout << "DEVICE=" << device
              << " ATTRIBUTE=ACL_DEV_ATTR_VECTOR_CORE_NUM ATTRIBUTE_VALUE="
              << static_cast<int>(ACL_DEV_ATTR_VECTOR_CORE_NUM)
              << " ACL_INFO_RC=" << infoRc << " availableCoreNum=" << liveCoreCount << '\n';
    int result = 0;
    if (infoRc != 0 || liveCoreCount <= 0) result = 1;
    else try {
        const CaseSpec cases[] = {
            {"R2-V001-target", 2, 2048, 0},
            {"R2-V001-control", 2, 2056, 0},
            {"R2-V009-target", 8, 2048, 0},
            {"R2-V009-control", 8, 2056, 0},
            {"R2-V040-target", 16, 2048, 0},
            {"R2-V040-control", 16, 2056, 0},
            {"R11-V002-control", 12, 8192, 0},
            {"R11-V002-target", 64, 8192, 0},
            {"R10-multibatch-parent", 128, 12288, 2},
        };
        uint8_t dummy = 0;
        for (const auto& spec : cases) {
            caseName = spec.name;
            const int64_t shape[] = {spec.rows, spec.width};
            const int64_t paramShape[] = {spec.width};
            const TensorInfo tensor{shape, 2, spec.dtype};
            const TensorInfo param{paramShape, 1, spec.dtype};
            const TensorGroupInfo info{&tensor, 1};
            const TensorGroupInfo params{&param, 1};
            run_kernel(&dummy, info, &dummy, info, &dummy, params,
                       &dummy, params, &dummy, info, liveCoreCount, nullptr, 1.0e-5f);
        }
        if (observedLaunches != 9) throw std::runtime_error("case count differs");
        std::cout << "HOST_SOURCE_TRACE=PASS CASE_COUNT=" << observedLaunches << '\n';
    } catch (const std::exception& error) {
        std::cerr << "HOST_SOURCE_TRACE=FAIL reason=" << error.what() << '\n';
        result = 1;
    }
    const int finalRc = aclFinalize();
    std::cout << "ACL_FINALIZE_RC=" << finalRc << '\n';
    std::cout << "NPU_KERNEL_LAUNCH=NO\nDEVICE_REFERENCE=NOT_RUN\nLOCAL_SCORE=NONE\n";
    return result != 0 || finalRc != 0 ? 1 : 0;
}
''')
