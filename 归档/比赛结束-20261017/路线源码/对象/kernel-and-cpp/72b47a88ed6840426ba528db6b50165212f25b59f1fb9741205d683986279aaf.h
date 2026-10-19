// ============================================================================
// data_utils.h - File I/O utilities for Ascend C Kernel direct invocation
// ============================================================================

#ifndef DATA_UTILS_H
#define DATA_UTILS_H

#include <stdio.h>
#include <stddef.h>

inline bool ReadFile(const char *filePath, size_t bufferSize, void *buffer, size_t bufferLen)
{
    if (buffer == nullptr) {
        return false;
    }
    if (bufferSize > bufferLen) {
        return false;
    }

    FILE *file = fopen(filePath, "rb");
    if (file == nullptr) {
        return false;
    }
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return false;
    }
    long fileSize = ftell(file);
    if (fileSize < 0 || static_cast<size_t>(fileSize) != bufferSize) {
        fclose(file);
        return false;
    }
    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return false;
    }
    size_t readSize = fread(buffer, 1, bufferSize, file);
    fclose(file);
    return readSize == bufferSize;
}

inline bool WriteFile(const char *filePath, const void *buffer, size_t size)
{
    if (buffer == nullptr && size > 0) {
        return false;
    }

    FILE *file = fopen(filePath, "wb");
    if (file == nullptr) {
        return false;
    }
    size_t written = fwrite(buffer, 1, size, file);
    int closeResult = fclose(file);
    return written == size && closeResult == 0;
}

#endif
