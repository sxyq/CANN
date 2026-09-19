#ifndef ADD_RMS_NORM_BIAS_DATA_UTILS_H
#define ADD_RMS_NORM_BIAS_DATA_UTILS_H

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <string>

#define ERROR_LOG(fmt, args...) fprintf(stdout, "[ERROR] " fmt "\n", ##args)

inline bool ReadFile(const std::string &filePath, size_t bufferSize, void *buffer, size_t bufferLen)
{
    if (buffer == nullptr || bufferSize > bufferLen) {
        ERROR_LOG("invalid read buffer");
        return false;
    }
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        ERROR_LOG("open failed: %s", filePath.c_str());
        return false;
    }
    file.seekg(0, std::ios::end);
    size_t fileSize = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);
    if (fileSize != bufferSize) {
        ERROR_LOG("size mismatch: %zu != %zu", fileSize, bufferSize);
        return false;
    }
    file.read(static_cast<char *>(buffer), static_cast<std::streamsize>(bufferSize));
    return static_cast<bool>(file);
}

inline bool WriteFile(const std::string &filePath, const void *buffer, size_t size)
{
    if (buffer == nullptr) {
        ERROR_LOG("invalid write buffer");
        return false;
    }
    int fd = open(filePath.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        ERROR_LOG("open failed: %s", filePath.c_str());
        return false;
    }
    ssize_t written = write(fd, buffer, size);
    close(fd);
    return written == static_cast<ssize_t>(size);
}

#endif
