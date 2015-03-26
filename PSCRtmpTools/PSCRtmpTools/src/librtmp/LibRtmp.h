#ifndef _LIB_RTMP_H_
#define _LIB_RTMP_H_
#include <string>

#include "include/rtmp.h"
#include "include/log.h"

class LibRtmp
{
public:
    LibRtmp(bool isNeedLog, bool isNeedRecord = false);

    ~LibRtmp();

    bool Open(const char* url);

    void Close();

    bool Send(const char* buf, int bufLen, int type, unsigned int timestamp);

    void SendSetChunkSize(unsigned int chunkSize);

    void CreateSharedObject();

    void SetSharedObject(const std::string& objName, bool isSet);

    void SendSharedObject(const std::string& objName, int val);

private:
    RTMP* rtmp_;
    char* streming_url_;
    FILE* flog_;
    bool is_need_record_;
    std::string stream_name_;
};

#endif // _LIB_RTMP_H_
