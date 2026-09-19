// ============================================================================
// data_utils.h - File I/O utilities for Ascend C Kernel direct invocation
// ============================================================================

#ifndef DATA_UTILS_H
#define DATA_UTILS_H

#include <fcntl.h>
#include <errno.h>
#include <stddef.h>
#include <sys/stat.h>
#include <unistd.h>

inline bool ReadFile(const char *filePath, size_t bufferSize, void *buffer, size_t bufferLen)
{
    if (filePath == nullptr || buffer == nullptr) {
        return false;
    }
    if (bufferSize > bufferLen) {
        return false;
    }

    int fd = open(filePath, O_RDONLY);
    if (fd < 0) {
        return false;
    }

    size_t totalRead = 0;
    while (totalRead < bufferSize) {
        ssize_t n = read(fd, static_cast<char *>(buffer) + totalRead, bufferSize - totalRead);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            close(fd);
            return false;
        }
        if (n == 0) {
            close(fd);
            return false;
        }
        totalRead += static_cast<size_t>(n);
    }

    return close(fd) == 0;
}

inline bool WriteFile(const char *filePath, const void *buffer, size_t size)
{
    if (filePath == nullptr || (buffer == nullptr && size > 0)) {
        return false;
    }

    int fd = open(filePath, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        return false;
    }

    size_t totalWritten = 0;
    while (totalWritten < size) {
        ssize_t w = write(fd, static_cast<const char *>(buffer) + totalWritten, size - totalWritten);
        if (w < 0) {
            if (errno == EINTR) continue;
            close(fd);
            return false;
        }
        if (w == 0) {
            close(fd);
            return false;
        }
        totalWritten += static_cast<size_t>(w);
    }
    if (close(fd) != 0) {
        return false;
    }

    return true;
}

#endif
