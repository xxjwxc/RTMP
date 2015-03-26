#ifndef _DATA_BUFFER_H_
#define _DATA_BUFFER_H_

namespace base{

class DataBuffer
{
public:
    DataBuffer(char* buf, unsigned int bufLen, bool isNeedFree = false)
    {
        buf_ = buf;
        buf_len_ = bufLen;
        is_need_free_ = isNeedFree;
    }

    DataBuffer(const DataBuffer& other)
    {
        buf_ = other.buf_;
        buf_len_ = other.buf_len_;
        is_need_free_ = other.is_need_free_;
    }

    ~DataBuffer()
    {
        if (is_need_free_)
        {
            delete[] buf_;
        }
    }

    char* Buf() { return buf_; }

    unsigned int BufLen() { return buf_len_; }

    DataBuffer* Clone()
    {
        char* buf_clone = new char[buf_len_];
        memcpy(buf_clone, buf_, buf_len_);
        DataBuffer* data_buf = new DataBuffer(buf_clone, buf_len_, true);
        return data_buf;
    }

private:
    char* buf_;
    unsigned int buf_len_;
    bool is_need_free_;
};

} // namespace Base

#endif // _DATA_BUFFER_H_
