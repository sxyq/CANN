#include "paired_probe_bridge.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

int main(int argc, char** argv)
{
    if (argc != 4 || (std::strcmp(argv[1], "V016") != 0 &&
                      std::strcmp(argv[1], "V021") != 0) ||
        (std::strcmp(argv[2], "0") != 0 && std::strcmp(argv[2], "1") != 0)) {
        fprintf(stderr, "usage: %s V016|V021 EXPECTED_DISPATCH MARKER_PATH|-\n", argv[0]);
        return 2;
    }

    const bool expectDispatch = std::strcmp(argv[2], "1") == 0;
    const char* marker = argv[3];
    if (std::strcmp(marker, "-") == 0) {
        unsetenv("R31A_PAIRED_PROBE_HOST_MARKER");
    } else {
        std::remove(marker);
        if (setenv("R31A_PAIRED_PROBE_HOST_MARKER", marker, 1) != 0) {
            perror("setenv");
            return 2;
        }
    }

    const TensorGroupInfo emptyInfo{};
    const bool dispatched = std::strcmp(argv[1], "V016") == 0
        ? r31a_run_kernel_v016(nullptr, emptyInfo, nullptr, emptyInfo,
                               nullptr, emptyInfo, nullptr, emptyInfo,
                               nullptr, emptyInfo, 0, nullptr, 0.0f)
        : r31a_run_kernel_v021(nullptr, emptyInfo, nullptr, emptyInfo,
                               nullptr, emptyInfo, nullptr, emptyInfo,
                               nullptr, emptyInfo, 0, nullptr, 0.0f);
    if (dispatched != expectDispatch) {
        fprintf(stderr, "dispatch result mismatch: expected=%d actual=%d\n",
                expectDispatch, dispatched);
        return 1;
    }

    if (expectDispatch) {
        FILE* called = fopen(marker, "r");
        if (called == nullptr) {
            fprintf(stderr, "entry was not called: marker=%s\n", marker);
            return 1;
        }
        fclose(called);
        std::remove(marker);
    }
    printf("host bridge %s: PASS\n", argv[1]);
    return 0;
}
