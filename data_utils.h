#ifndef ADD_RMS_NORM_BIAS_DATA_UTILS_H
#define ADD_RMS_NORM_BIAS_DATA_UTILS_H

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#define ERROR_LOG(fmt, args...) fprintf(stdout, "[ERROR] " fmt "\n", ##args)

inline bool ReadFile(const char *filePath, size_t bufferSize, void *buffer, size_t bufferLen)
{
    if (filePath == nullptr || buffer == nullptr || bufferSize > bufferLen) {
        ERROR_LOG("invalid read buffer");
        return false;
    }
    int fd = open(filePath, O_RDONLY);
    if (fd < 0) {
        ERROR_LOG("open failed");
        return false;
    }
    ssize_t total = 0;
    while (static_cast<size_t>(total) < bufferSize) {
        ssize_t current = read(fd, static_cast<char *>(buffer) + total, bufferSize - total);
        if (current <= 0) {
            close(fd);
            ERROR_LOG("read failed");
            return false;
        }
        total += current;
    }
    close(fd);
    return static_cast<size_t>(total) == bufferSize;
}

inline bool WriteFile(const char *filePath, const void *buffer, size_t size)
{
    if (filePath == nullptr || buffer == nullptr) {
        ERROR_LOG("invalid write buffer");
        return false;
    }
    int fd = open(filePath, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        ERROR_LOG("open failed");
        return false;
    }
    ssize_t written = write(fd, buffer, size);
    close(fd);
    return written == static_cast<ssize_t>(size);
}

#endif
