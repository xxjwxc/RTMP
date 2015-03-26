#include "stdafx.h"
#include "DSVideoCaptureDevice.h"

DSVideoCaptureDevice::DSVideoCaptureDevice()
{
    video_width_ = 0;
    video_height_ = 0;
    video_fps_ = 0;

    preferred_image_width_ = 0;
    preferred_image_height_ = 0;
    preferred_fps_ = 0;
}

DSVideoCaptureDevice::~DSVideoCaptureDevice()
{

}

