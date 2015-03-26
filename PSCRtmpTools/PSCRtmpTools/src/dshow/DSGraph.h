#ifndef _DS_GRAPH_H_
#define _DS_GRAPH_H_

#include <map>
#include <string>

#include "DSGrabberCallback.h"

class DSGraph
{
public:
    DSGraph();

    virtual ~DSGraph();

    void Start();

    void Stop();

    bool IsBufferAvailable() { return grabber_callback_.IsBufferAvailable(); }

    char* GetBuffer() { return grabber_callback_.GetBuffer(); }

    void ReleaseBuffer(char* buf) { grabber_callback_.ReleaseBuffer(buf); }

    int BufferSize() { return grabber_callback_.BufferSize(); }

    __int64 Timestamp() { return grabber_callback_.Timestamp(); }

    UINT FPSMeter() { return grabber_callback_.FPSMeter(); }

    void ResetFPSMeter() { return grabber_callback_.ResetFPSMeter(); }

protected:
    bool SetCapDevice(const IID& deviceIID, const CString& comObjID);

    HRESULT GetPin(IBaseFilter* pFilter, PIN_DIRECTION dirrequired, int iNum, IPin **ppPin);

    IPin* GetInPin(IBaseFilter* filter, int pinIndex);

    IPin* GetOutPin(IBaseFilter* filter, int pinIndex);

protected:
    IBaseFilter* null_render_filter_;
    IBaseFilter* device_filter_;
    ISampleGrabber* sample_grabber_;
    IBaseFilter* grabber_filter_;
    IGraphBuilder* graph_;
    ICaptureGraphBuilder2* capture_builder_;
    IMediaControl* media_control_;;
    IMediaEvent* media_event_;

    DSGrabberCallback grabber_callback_;
};

#endif // _DS_GRAPH_H_
