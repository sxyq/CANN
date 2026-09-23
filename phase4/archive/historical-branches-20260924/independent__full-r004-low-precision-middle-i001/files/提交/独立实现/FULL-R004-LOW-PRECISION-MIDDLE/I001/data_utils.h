#ifndef ADD_RMS_NORM_BIAS_DATA_UTILS_H
#define ADD_RMS_NORM_BIAS_DATA_UTILS_H

#include <stdio.h>
#include <stddef.h>

inline bool ReadFile(const char* filePath, size_t bufferSize, void* buffer, size_t bufferLen)
{
    if (buffer == nullptr || bufferSize > bufferLen) {
        return false;
    }
    FILE* file = fopen(filePath, "rb");
    if (file == nullptr) {
        return false;
    }
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return false;
    }
    const long fileSize = ftell(file);
    if (fileSize < 0 || (size_t)fileSize != bufferSize || fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return false;
    }
    const size_t readSize = fread(buffer, 1, bufferSize, file);
    fclose(file);
    return readSize == bufferSize;
}

inline bool WriteFile(const char* filePath, const void* buffer, size_t size)
{
    if (buffer == nullptr && size > 0) {
        return false;
    }
    FILE* file = fopen(filePath, "wb");
    if (file == nullptr) {
        return false;
    }
    const size_t written = fwrite(buffer, 1, size, file);
    const int closeResult = fclose(file);
    return written == size && closeResult == 0;
}

#endif
