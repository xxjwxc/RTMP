#ifndef _DS_VIDEO_GRAPH_H_
#define _DS_VIDEO_GRAPH_H_

#include "DSGraph.h"
#include "DSVideoCaptureDevice.h"

class DSVideoGraph : public DSGraph
{
public:
    DSVideoGraph();

    virtual ~DSVideoGraph();

    HRESULT Create(DSVideoCaptureDevice* videoCaptureDevice);

    void Destroy();

    void AdjustVideoWindow(OAHWND owner, unsigned int width, unsigned int height);

	UINT Width() { return video_cap_device_->GetVideoWidth(); }

	UINT Height() { return video_cap_device_->GetVideoHeight(); }

    REFERENCE_TIME FPS() { return video_cap_device_->GetVideoFPS(); }

private:
    HRESULT SetVideoFormat(UINT preferredImageWidth, UINT preferredImageHeight, 
        REFERENCE_TIME preferredFPS);

    bool RouteCrossbar();

private:
    DSVideoCaptureDevice* video_cap_device_;
    IVideoWindow* active_video_window_;
};

#endif // _DS_VIDEO_GRAPH_H_
