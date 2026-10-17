#include "probe_prelude.inc"
#include <cstdio>
#include <cstdlib>

static void WriteMarker()
{
    const char* marker = std::getenv("R31A_PAIRED_PROBE_HOST_MARKER");
    if (marker == nullptr) return;
    FILE* output = fopen(marker, "w");
    if (output == nullptr) return;
    fputs("entry-called\n", output);
    fclose(output);
}

extern "C" void r31a_host_entry_v016(
    void*, const TensorGroupInfo&, void*, const TensorGroupInfo&,
    void*, const TensorGroupInfo&, void*, const TensorGroupInfo&,
    void*, const TensorGroupInfo&, int64_t, aclrtStream, float)
{
    WriteMarker();
}

extern "C" void r31a_host_entry_v021(
    void*, const TensorGroupInfo&, void*, const TensorGroupInfo&,
    void*, const TensorGroupInfo&, void*, const TensorGroupInfo&,
    void*, const TensorGroupInfo&, int64_t, aclrtStream, float)
{
    WriteMarker();
}
