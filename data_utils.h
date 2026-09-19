#ifndef ADD_RMS_NORM_BIAS_DATA_UTILS_H
#define ADD_RMS_NORM_BIAS_DATA_UTILS_H

#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

inline bool ReadFile(const std::string& filePath, size_t bufferSize, void* buffer, size_t bufferLen)
{
    if (buffer == nullptr || bufferSize > bufferLen) {
        return false;
    }
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    file.seekg(0, std::ios::end);
    const std::streamoff fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    if (fileSize < 0 || static_cast<size_t>(fileSize) != bufferSize) {
        return false;
    }
    file.read(static_cast<char*>(buffer), static_cast<std::streamsize>(bufferSize));
    return static_cast<bool>(file);
}

inline bool WriteFile(const std::string& filePath, const void* buffer, size_t size)
{
    if (buffer == nullptr && size > 0) {
        return false;
    }
    const int fd = open(filePath.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        return false;
    }
    size_t written = 0;
    while (written < size) {
        const ssize_t count = write(fd, static_cast<const char*>(buffer) + written, size - written);
        if (count < 0 && errno == EINTR) {
            continue;
        }
        if (count <= 0) {
            close(fd);
            return false;
        }
        written += static_cast<size_t>(count);
    }
    return close(fd) == 0;
}

#endif
