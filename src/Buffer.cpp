#include "Buffer.h"

using namespace std;

Buffer::Buffer(Msg_Format sep, size_t buf_size): 
        sep_(sep), buf_(PreBuf + buf_size), read_index_(PreBuf), write_index_(PreBuf)
{

}

Buffer& Buffer::operator=(const Buffer &buf)
{
    if(this == &buf)
        return *this;
    sep_ = buf.sep_;
    buf_ = buf.buf_;
    read_index_ = buf.read_index_;
    write_index_ = buf.write_index_;
    return *this;
}

size_t Buffer::ReadableBytes() const
{
    return write_index_ - read_index_;
}

size_t Buffer::WriteableBytes() const
{
    return buf_.size() - write_index_;
}

size_t Buffer::PrependableBytes() const
{
    return read_index_;
}

const char* Buffer::Peek() const
{
    return &buf_[read_index_];
}

void Buffer::EnsureWriteable(size_t len)
{
    if(WriteableBytes() < len)
        MakeSpace(len);
}

void Buffer::HasWritten(size_t len)
{
    write_index_ += len;
}

void Buffer::HasRead(size_t len)
{
    read_index_ += len;
}

void Buffer::ReadUntil(const char* end)
{
    assert(Peek() <= end);
    HasRead(end - Peek());
}

bool Buffer::PickMsg(Buffer &buf)
{
    if(ReadableBytes() == 0)
        return false;

    switch(sep_){
        case Msg_Format::NONE:{
            // DEBUG("NONE");
            buf.Append(Peek(), ReadableBytes());
            Clear();
            break;
        }
        case Msg_Format::HEAD:{
            // DEBUG("HEAD");
            int len = 0;
            memcpy(&len, Peek(), INT_BYTES);

            if(ReadableBytes() < static_cast<size_t>(len + INT_BYTES))
                return false;
            HasRead(INT_BYTES);
            buf.Append(Peek(), len);
            HasRead(len);

            if(ReadableBytes() == 0)
                Clear();
            break;
        }
        case Msg_Format::HTTP:{
            // DEBUG("HTTP");
            int len = 0;
            string end_flag = "\r\n\r\n";
            auto it_end_flag = search(BeginRead(), BeginWrite(), end_flag.begin(), end_flag.end());
            if(it_end_flag == BeginWrite())
                return false;

            string data_len_flag = "Content-Length: ";
            string sep_flag = "\r\n";
            auto it_data_len_flag = search(BeginRead(), BeginWrite(), data_len_flag.begin(), data_len_flag.end());
            if(it_data_len_flag == BeginWrite()){
                len = distance(BeginRead(), it_end_flag) + end_flag.length();
            }
            else{
                auto it1 = it_data_len_flag + data_len_flag.length();
                auto it2 = search(it_data_len_flag, BeginWrite(), sep_flag.begin(), sep_flag.end());
                string data_len(it1, distance(it1, it2));
                len = distance(BeginRead(), it_end_flag) + end_flag.length() + stoi(data_len);
            }
            if(ReadableBytes() < static_cast<size_t>(len))
                return false;

            buf.Append(Peek(), len);
            HasRead(len);
            if(ReadableBytes() == 0)
                Clear();
            else
                MoveReadBuf();
            break;
        }
        default:{
            // DEBUG("default!");
            break;
        }
    }
    return true;
}

string Buffer::Str()
{
    string str(Peek(), ReadableBytes());
    return str;
}

void Buffer::Append(const char* data, size_t len)
{   
    EnsureWriteable(len);
    copy(data, data + len, BeginWrite());
    HasWritten(len);
}

void Buffer::AppendWithSep(const char* data, size_t len)
{
    switch(sep_){
        case Msg_Format::NONE:{
            Append(data, len);
            break;
        }
        case Msg_Format::HEAD:{
            Append(reinterpret_cast<char*>(&len), INT_BYTES);
            Append(data, len);
            break;
        }
        case Msg_Format::HTTP:{
            Append(data, len);
            break;
        }
    }
}

void Buffer::AppendWithSep(const std::string &data)
{
    AppendWithSep(data.c_str(), data.length());
}

void Buffer::AppendWithSep(const Buffer &buffer)
{
    AppendWithSep(buffer.Peek(), buffer.ReadableBytes());
}   

const char* Buffer::Begin() const
{
    return &*buf_.begin();
}

const char* Buffer::BeginPre() const
{
    return &buf_[PreBuf];
}

const char* Buffer::BeginRead() const
{
    return &buf_[read_index_];
}

const char* Buffer::BeginWrite() const
{
    return &buf_[write_index_];
}

char* Buffer::Begin()
{
    return &*buf_.begin();
}

char* Buffer::BeginPre()
{
    return &buf_[PreBuf];
}

char* Buffer::BeginRead()
{
    return &buf_[read_index_];
}

char* Buffer::BeginWrite()
{
    return &buf_[write_index_];
}

void Buffer::MoveReadBuf()
{
    size_t read_bytes = ReadableBytes();
    copy(BeginRead(), BeginWrite(), BeginPre());
    read_index_ = PreBuf;
    write_index_ = read_index_ + read_bytes;
}

void Buffer::MakeSpace(size_t len)
{
    if(PrependableBytes() + WriteableBytes() < len + PreBuf){
        buf_.resize(PreBuf + ReadableBytes() + len);
        MoveReadBuf();
    }
    else if((WriteableBytes() < len) && (PrependableBytes() + WriteableBytes() > len + PreBuf))
        MoveReadBuf();
}

size_t Buffer::Size()
{
    return buf_.size();
}

void Buffer::Clear()
{
    size_t cur_size = Size();
    if(cur_size > CHUNK_SIZE + 10)
        Reset();
    else{
        buf_.clear();
        buf_.resize(cur_size);
        read_index_ = PreBuf;
        write_index_ = PreBuf;
    }
}

void Buffer::Reset(size_t buf_size)
{
    buf_.clear();
    buf_.resize(PreBuf + buf_size);
    read_index_ = PreBuf;
    write_index_ = PreBuf;
}