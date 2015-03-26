#include "stdafx.h"
#include "LibRtmp.h"
#include "AmfByteStream.h"

#ifdef WIN32
#include <windows.h>
#pragma comment(lib,"WS2_32.lib")
#pragma comment(lib,"winmm.lib")
#ifdef _DEBUG
#pragma comment(lib,"librtmpD.lib")
#else
#pragma comment(lib,"librtmp.lib")
#endif
#endif

inline int InitSockets()
{
#ifdef WIN32  
	WORD version;
	WSADATA wsaData;
	version = MAKEWORD(1, 1);
	return (WSAStartup(version, &wsaData) == 0);
#else  
	return TRUE;
#endif  
}

LibRtmp::LibRtmp(bool isNeedLog, bool isNeedRecord)
{
    if (isNeedLog)
    {
		fopen_s(&flog_,"librtmp.log", "w");
        RTMP_LogSetLevel(RTMP_LOGDEBUG2);
        RTMP_LogSetOutput(flog_);
    }
    else
    {
        flog_ = NULL;
    }

	InitSockets();
    rtmp_ = RTMP_Alloc();
	
    RTMP_Init(rtmp_);
    //RTMP_SetBufferMS(rtmp_, 300);

    streming_url_ = NULL;
    is_need_record_ = isNeedRecord;
}

LibRtmp::~LibRtmp()
{
    Close();
	WSACleanup();
	RTMP_DeleteStream(rtmp_);
    RTMP_Free(rtmp_);

    if (streming_url_)
    {
        free(streming_url_);
        streming_url_ = NULL;
    }

    if (flog_) fclose(flog_);
}

bool LibRtmp::Open(const char* url)
{
	if (streming_url_)
	{
		free(streming_url_);
		streming_url_ = NULL;
	}

	streming_url_ = (char*)calloc(strlen(url) + 1, sizeof(char));
	memcpy(streming_url_,url,strlen(url)+1);
	//strcpy_s(streming_url_, strlen(url), url);

    std::string tmp(url);
    stream_name_ = tmp.substr(tmp.rfind("/")+1);

    //AVal flashver = AVC("flashver");
    //AVal flashver_arg = AVC("WIN 9,0,115,0");
    AVal swfUrl = AVC("swfUrl");
    AVal swfUrl_arg = AVC("http://localhost/RtmpSDK.swf");
    AVal pageUrl = AVC("pageUrl");
    AVal pageUrl_arg = AVC("http://localhost/RtmpSDK.swf");
    //RTMP_SetOpt(rtmp_, &flashver, &flashver_arg);
    RTMP_SetOpt(rtmp_, &swfUrl, &swfUrl_arg);
    RTMP_SetOpt(rtmp_, &pageUrl, &pageUrl_arg);

    if (is_need_record_)
    {
        AVal record = AVC("record");
        AVal record_arg = AVC("on");
        RTMP_SetOpt(rtmp_, &record, &record_arg);
    }

    int err = RTMP_SetupURL(rtmp_, streming_url_);
    if (err <= 0) return false;

    RTMP_EnableWrite(rtmp_);
    
    err = RTMP_Connect(rtmp_, NULL);
    if (err <= 0) return false;

    err = RTMP_ConnectStream(rtmp_, 0);
    if (err <= 0) return false;

    rtmp_->m_outChunkSize = 2*1024*1024;
    SendSetChunkSize(rtmp_->m_outChunkSize);

    return true;
}

void LibRtmp::Close()
{
    RTMP_Close(rtmp_);
}

bool LibRtmp::Send(const char* buf, int bufLen, int type, unsigned int timestamp)
{
    RTMPPacket rtmp_pakt;
    RTMPPacket_Reset(&rtmp_pakt);
    RTMPPacket_Alloc(&rtmp_pakt, bufLen);

    rtmp_pakt.m_packetType = type;
    rtmp_pakt.m_nBodySize = bufLen;
    rtmp_pakt.m_nTimeStamp = timestamp;
    rtmp_pakt.m_nChannel = 4;
    rtmp_pakt.m_headerType = RTMP_PACKET_SIZE_LARGE;
    rtmp_pakt.m_nInfoField2 = rtmp_->m_stream_id;
    memcpy(rtmp_pakt.m_body, buf, bufLen);

	int retval = RTMP_SendPacket(rtmp_, &rtmp_pakt, 0);
    RTMPPacket_Free(&rtmp_pakt);

    return retval;
}

void LibRtmp::SendSetChunkSize(unsigned int chunkSize)
{
    RTMPPacket rtmp_pakt;
    RTMPPacket_Reset(&rtmp_pakt);
    RTMPPacket_Alloc(&rtmp_pakt, 4);

    rtmp_pakt.m_packetType = 0x01;
    rtmp_pakt.m_nChannel = 0x02;    // control channel
    rtmp_pakt.m_headerType = RTMP_PACKET_SIZE_LARGE;
    rtmp_pakt.m_nInfoField2 = 0;


    rtmp_pakt.m_nBodySize = 4;
    UI32ToBytes(rtmp_pakt.m_body, chunkSize);

    RTMP_SendPacket(rtmp_, &rtmp_pakt, 0);
    RTMPPacket_Free(&rtmp_pakt);
}

void LibRtmp::CreateSharedObject()
{
    char data_buf[4096];
    char* pbuf = data_buf;

    pbuf = AmfStringToBytes(pbuf, stream_name_.c_str());

    pbuf = UI32ToBytes(pbuf, 0);    // version
    pbuf = UI32ToBytes(pbuf, 0);    // persistent
    pbuf += 4;

    pbuf = UI08ToBytes(pbuf, RTMP_SHARED_OBJECT_DATATYPE_CONNECT);

    char* pbuf_datalen = pbuf;
    pbuf += 4;

    UI32ToBytes(pbuf_datalen, (int)(pbuf - pbuf_datalen - 4));

    int buflen = (int)(pbuf - data_buf);

    LibRtmp::Send(data_buf, buflen, TAG_TYPE_SHARED_OBJECT, 0);
}

void LibRtmp::SetSharedObject(const std::string& objName, bool isSet)
{
    char data_buf[4096];
    char* pbuf = data_buf;

    pbuf = AmfStringToBytes(pbuf, stream_name_.c_str());

    pbuf = UI32ToBytes(pbuf, 0);    // version
    pbuf = UI32ToBytes(pbuf, 0);    // persistent
    pbuf += 4;

    pbuf = UI08ToBytes(pbuf, RTMP_SHARED_OBJECT_DATATYPE_SET_ATTRIBUTE);

    char* pbuf_datalen = pbuf;
    pbuf += 4;

    pbuf = AmfStringToBytes(pbuf, objName.c_str());
    pbuf = AmfBoolToBytes(pbuf, isSet);
    UI32ToBytes(pbuf_datalen, (int)(pbuf - pbuf_datalen - 4));

    int buflen = (int)(pbuf - data_buf);

    LibRtmp::Send(data_buf, buflen, TAG_TYPE_SHARED_OBJECT, 0);
}

void LibRtmp::SendSharedObject(const std::string& objName, int val)
{
    char data_buf[4096];
    char* pbuf = data_buf;

    pbuf = AmfStringToBytes(pbuf, stream_name_.c_str());

    pbuf = UI32ToBytes(pbuf, 0);    // version
    pbuf = UI32ToBytes(pbuf, 0);    // persistent
    pbuf += 4;

    pbuf = UI08ToBytes(pbuf, RTMP_SHARED_OBJECT_DATATYPE_SET_ATTRIBUTE);

    char* pbuf_datalen = pbuf;
    pbuf += 4;

    pbuf = AmfStringToBytes(pbuf, objName.c_str());
    pbuf = AmfDoubleToBytes(pbuf, val);
    UI32ToBytes(pbuf_datalen, (int)(pbuf - pbuf_datalen - 4));

    int buflen = (int)(pbuf - data_buf);

    LibRtmp::Send(data_buf, buflen, TAG_TYPE_SHARED_OBJECT, 0);
}
