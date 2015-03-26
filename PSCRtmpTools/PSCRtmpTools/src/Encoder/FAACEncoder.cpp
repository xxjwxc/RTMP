#include "stdafx.h"
#include "FAACEncoder.h"
#ifdef WIN32
#ifdef _DEBUG
#pragma comment(lib,"libfaacd.lib")
#else
#pragma comment(lib,"libfaac.lib")
#endif
#endif


FAACEncoder::FAACEncoder()
{
    input_sams_ = 0;
    max_output_bytes_ = 0;
}

FAACEncoder::~FAACEncoder()
{
    Destroy();
}

void FAACEncoder::Init(unsigned int samRate, unsigned int channels, int bitsPerSample)
{
    faac_handle_ = faacEncOpen(samRate, channels, &input_sams_, &max_output_bytes_);
    faacEncConfigurationPtr enc_cfg = faacEncGetCurrentConfiguration(faac_handle_);
    switch(bitsPerSample)
    {
    case 16:
        enc_cfg->inputFormat=FAAC_INPUT_16BIT;
        break;
    case 24:
        enc_cfg->inputFormat=FAAC_INPUT_24BIT;
        break;
    case 32:
        enc_cfg->inputFormat=FAAC_INPUT_32BIT;
        break;
    default:
        enc_cfg->inputFormat=FAAC_INPUT_NULL;
        break;
    }
    enc_cfg->mpegVersion = MPEG4;
    enc_cfg->aacObjectType = LOW;
    enc_cfg->allowMidside = 1;
    enc_cfg->useLfe = 0;
    enc_cfg->useTns = 0;
    enc_cfg->bitRate = 16000;
    enc_cfg->quantqual = 100;
    enc_cfg->bandWidth = 0;
    enc_cfg->outputFormat = 0;
    faacEncSetConfiguration(faac_handle_, enc_cfg);
}

void FAACEncoder::Encode(unsigned char* inputBuf, unsigned int samCount, unsigned char* outBuf, unsigned int& bufSize)
{
    bufSize = faacEncEncode(faac_handle_, (int*)inputBuf, samCount, outBuf, max_output_bytes_);
}

void FAACEncoder::Destroy()
{
    faacEncClose(faac_handle_);
}
