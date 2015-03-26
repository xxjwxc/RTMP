#include "stdafx.h"
#include "X264Encoder.h"
#include "..\librtmp\AmfByteStream.h"
#include "..\tools\MyIni.h"
#ifdef WIN32
#ifdef _DEBUG
#pragma comment(lib,"libx264d.lib")
#else
#pragma comment(lib,"libx264.lib")
#endif
#endif

X264Encoder::X264Encoder()
{
    m_h = NULL;
    x264_param_default(&m_param);

    sps_ = NULL;
    sps_size_ = 0;
    pps_ = NULL;
    pps_size_ = 0;
}

X264Encoder::~X264Encoder()
{
    Destroy();

    if (sps_)
        free(sps_);
    if (pps_)
        free(pps_);
}

int X264Encoder::Initialize(int iWidth, int iHeight, int iRateBit, int iFps)
{
	CMyIni _init;
    m_param.i_width = iWidth;
    m_param.i_height = iHeight;

    m_param.i_fps_num = iFps;
    m_param.i_fps_den = 1;

    m_param.rc.i_bitrate = iRateBit;
	m_param.rc.i_rc_method = _init.GetIntConfig("X264_RC");/*X264_RC_ABR*/;

    m_param.i_frame_reference = 2; /* 参考帧的最大帧数 */
    //m_param.i_keyint_max = 8;
    //m_param.i_keyint_min = 4;
    m_param.i_frame_total = 0;
    //m_param.i_bframe = 1;
    m_param.i_threads = 1;
    //m_param.rc.i_lookahead = 0;
    //m_param.i_sync_lookahead = X264_SYNC_LOOKAHEAD_AUTO;

    /* 根据输入参数param初始化总结构 x264_t *h     */
    if( ( m_h = x264_encoder_open( &m_param ) ) == NULL )
    {
        //fprintf( stderr, "x264 [error]: x264_encoder_open failed\n" );
        
        return -1;
    }

    x264_picture_alloc( &m_pic, X264_CSP_I420, m_param.i_width, m_param.i_height );
    m_pic.i_type = X264_TYPE_AUTO;
    m_pic.i_qpplus1 = 0;

    return 0;
}

int X264Encoder::Destroy()
{
    if (m_h)
    {
        x264_picture_clean( &m_pic );

        x264_encoder_close(m_h);
		m_h = nullptr;
    }

    return 0;
}

static const char *ff_avc_find_startcode_internal(const char *p, const char *end)
{
    const char *a = p + 4 - ((intptr_t)p & 3);

    for (end -= 3; p < a && p < end; p++) {
        if (p[0] == 0 && p[1] == 0 && p[2] == 1)
            return p;
    }

    for (end -= 3; p < end; p += 4) {
        unsigned int x = *(const unsigned int*)p;
        //      if ((x - 0x01000100) & (~x) & 0x80008000) // little endian
        //      if ((x - 0x00010001) & (~x) & 0x00800080) // big endian
        if ((x - 0x01010101) & (~x) & 0x80808080) { // generic
            if (p[1] == 0) {
                if (p[0] == 0 && p[2] == 1)
                    return p;
                if (p[2] == 0 && p[3] == 1)
                    return p+1;
            }
            if (p[3] == 0) {
                if (p[2] == 0 && p[4] == 1)
                    return p+2;
                if (p[4] == 0 && p[5] == 1)
                    return p+3;
            }
        }
    }

    for (end += 3; p < end; p++) {
        if (p[0] == 0 && p[1] == 0 && p[2] == 1)
            return p;
    }

    return end + 3;
}

static const char *ff_avc_find_startcode(const char *p, const char *end)
{
    const char *out= ff_avc_find_startcode_internal(p, end);
    if(p<out && out<end && !out[-1]) out--;
    return out;
}

static void ff_avc_parse_nal_units(const char *bufIn, int inSize, char* bufOut, int* outSize)
{
    const char *p = bufIn;
    const char *end = p + inSize;
    const char *nal_start, *nal_end;

    char* pbuf = bufOut;

    *outSize = 0;
    nal_start = ff_avc_find_startcode(p, end);
    while (nal_start < end)
    {
        while(!*(nal_start++));

        nal_end = ff_avc_find_startcode(nal_start, end);

        unsigned int nal_size = nal_end - nal_start;
        pbuf = UI32ToBytes(pbuf, nal_size);
        memcpy(pbuf, nal_start, nal_size);
        pbuf += nal_size;

        nal_start = nal_end;
    }

    *outSize = (pbuf - bufOut);
}

void X264Encoder::FindSpsAndPPsFromBuf(const char* dataBuf, int bufLen)
{
    char* tmp_buf = new char[5*1024*1024];
    int tmp_len = 0;

    ff_avc_parse_nal_units(dataBuf, bufLen, tmp_buf, &tmp_len);

    char* start = tmp_buf;
    char* end = tmp_buf + tmp_len;

    /* look for sps and pps */
    while (start < end) 
    {
        unsigned int size = BytesToUI32(start);
        unsigned char nal_type = start[4] & 0x1f;

        if (nal_type == 7)        /* SPS */
        {
            sps_size_ = size;
            sps_ = (char*)malloc(sps_size_);
            memcpy(sps_, start + 4, sps_size_);
        }
        else if (nal_type == 8)   /* PPS */
        {
            pps_size_ = size;
            pps_ = (char*)malloc(pps_size_);
            memcpy(pps_, start + 4, pps_size_);
        }
        start += size + 4;
    }

    delete[] tmp_buf;
}

int X264Encoder::Encode(unsigned char* szYUVFrame, unsigned char* outBuf, int& outLen, bool& isKeyframe)
{
    // 可以优化为m_pic中保存一个指针,直接执行szYUVFrame
    memcpy(m_pic.img.plane[0], szYUVFrame, m_param.i_width * m_param.i_height*3 / 2);
    //m_pic.img.plane[0] = szYUVFrame;

    m_param.i_frame_total++;
    m_pic.i_pts = (int64_t)m_param.i_frame_total * m_param.i_fps_den;
    if (isKeyframe)
        m_pic.i_type = X264_TYPE_IDR;
    else
        m_pic.i_type = X264_TYPE_AUTO;

    x264_picture_t pic_out;
    x264_nal_t *nal=0;
    int i_nal, i; // nal的个数

    if( x264_encoder_encode( m_h, &nal, &i_nal, &m_pic, &pic_out ) < 0 )
    {
        //fprintf( stderr, "x264 [error]: x264_encoder_encode failed\n" );
        return -1;
    }

    char* tmpbuf = NULL;
    char* ptmpbuf = NULL;
    if (pps_ == NULL)
    {
        tmpbuf = (char*)malloc(m_param.i_width * m_param.i_height * 100);
        ptmpbuf = tmpbuf;
    }

    int maxlen = outLen;
    outLen = 0;
    for( i = 0; i < i_nal; i++ )
    {
        int i_size = 0;
        x264_nal_encode(outBuf+outLen, &i_size, 1, &nal[i] );

        if (ptmpbuf)
        {
            memcpy(ptmpbuf, outBuf+outLen, i_size);
            ptmpbuf += i_size;
        }

        // 将起始码0x00000001，替换为nalu的大小
        UI32ToBytes((char*)(outBuf+outLen), i_size-4);
        outLen += i_size;
    }

    if (tmpbuf)
    {
        FindSpsAndPPsFromBuf(tmpbuf, ptmpbuf-tmpbuf);
        free(tmpbuf);
    }

    isKeyframe = (pic_out.i_type == X264_TYPE_IDR);

    return 0;
}

// void X264Encoder::CleanNAL(TNAL* pNALArray, int iNalNum)
// {
//     for(int i = 0; i < iNalNum; i++)
//     {
//         delete []pNALArray[i].data;
//         pNALArray[i].data = NULL;
//     }
//     delete []pNALArray;
//     pNALArray = NULL;
// }
