#ifndef _TEMPBUFFER_H_
#define _TEMPBUFFER_H_

#include "../logger/Logger.h"

#include <unistd.h>
#include <sys/uio.h>
#include <string.h>

#include <iostream>
#include <string>
#include <cassert>
#include <vector>
#include <algorithm>

#define INT_BYTES 4
#define CHUNK_SIZE 1048576  // 分块大小1M

enum class Msg_Format{
    NONE = 0,
    HEAD,
    HTTP
};

class Buffer
{
public:
    static const size_t PreBuf = 8;
    static const size_t BufSize = 1024;
private:
    Msg_Format sep_;
    std::vector<char> buf_;
    size_t read_index_;
    size_t write_index_;
public:
    Buffer(Msg_Format sep = Msg_Format::NONE, size_t buf_size = BufSize);
    ~Buffer() = default;
    Buffer& operator=(const Buffer &buf);
    size_t ReadableBytes() const;
    size_t WriteableBytes() const;
    size_t PrependableBytes() const;
    const char* Peek() const;
    void EnsureWriteable(size_t len);
    void HasWritten(size_t len);
    void HasRead(size_t len);
    void ReadUntil(const char* end);
    bool PickMsg(Buffer &buf);

    std::string Str();
 
    void Append(const char* data, size_t len);
    void AppendWithSep(const char* data, size_t len);
    void AppendWithSep(const std::string &data);
    void AppendWithSep(const Buffer &buffer);

    size_t Size();
    void Clear();
    void Reset(size_t buf_size = BufSize);
    const char* Begin() const;
    const char* BeginPre() const;
    const char* BeginRead() const;
    const char* BeginWrite() const;
    char* Begin();
    char* BeginPre();
    char* BeginRead();
    char* BeginWrite();
private:
    void MoveReadBuf();
    void MakeSpace(size_t len);
};

#endif // _TEMPBUFFER_H_