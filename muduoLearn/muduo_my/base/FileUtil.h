#ifndef MYMUDUO_BASE_FILEUTIL_H
#define MYMUDUO_BASE_FILEUTIL_H

#include "noncopyable.h"
#include "StringPiece.h"
#include <sys/types.h>  // for off_t

namespace muduo {
namespace FileUtil {

// read small file < 64kb
class ReadSmallFile : noncopyable {
public:
    ReadSmallFile(StringArg filename);
    ~ReadSmallFile();

    // return errno
    template <class String>
    int readToString(int maxSize,
                     String* content,   // 读入content缓冲区
                     int64_t* fileSize,
                     int64_t* modifyTime,
                     int64_t* createTime);

    /// Read at maxium kBufferSize into buf_
    // return errno
    int readToBuffer(int* size);

    const char* buffer() const { return buf_; }                 

    static const int kBufferSize = 64*1024

private:
    int err_;
    int fd_;
    char buf_[kBufferSize];
};

// readFile函数，调用ReadSmallFile类中的readToString方法，供外部将小文件中的内容转化为字符串
// read the file content, returns errno if error happens.
template<typename String>
int readFile(StringArg filename,
             int maxSize,
             String* content,
             int64_t* fileSize = NULL,
             int64_t* modifyTime = NULL,
             int64_t* createTime = NULL)
{
    ReadSmallFile file(filename);
    return file.readToString(maxSize, content, fileSize, modifyTime, createTime);
}

// not thread safe
class AppendFile : noncopyable {
public:
    explicit AppendFile(StringArg filename);

    ~AppendFile();

    void append(const char* logline, size_t len);

    void flush();

    off_t writtenBytes() const { return writtenBytes_; }

private:

    size_t write(const char* logline, size_t len);

    FILE* fp_;  // 文件指针
    char buffer_[64*1024];  // 缓冲区 64K
    off_t writtenBytes_;    // 已经写入的字节数
};

} // namespace FileUtil
} // namespace muduo

#endif // MYMUDUO_BASE_FILEUTIL_H