#ifndef _FAAC_ENCODER_H_
#define _FAAC_ENCODER_H_

#include "libfaac/faac.h"

class FAACEncoder
{
public:
    FAACEncoder();

    ~FAACEncoder();

    void Init(unsigned int samRate, unsigned int channels, int bitsPerSample);

    void Encode(unsigned char* inputBuf, unsigned int samCount, unsigned char* outBuf, unsigned int& bufSize);

    unsigned long InputSamples() { return input_sams_; }

    unsigned long MaxOutBytes() { return max_output_bytes_; }

    void Destroy();

private:
    faacEncHandle faac_handle_;
    unsigned long input_sams_;
    unsigned long max_output_bytes_;
};

#endif // _FAAC_ENCODER_H_
