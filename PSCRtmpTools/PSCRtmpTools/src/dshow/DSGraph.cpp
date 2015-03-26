#include "stdafx.h"
#include "DSGraph.h"

DSGraph::DSGraph()
{
    if (FAILED(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, 
        IID_IGraphBuilder, (void **)&graph_)))
        return;

    if (FAILED(CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER,
        IID_ICaptureGraphBuilder2,(void **)&capture_builder_)))
        return;

    if (FAILED(capture_builder_->SetFiltergraph(graph_)))
        return ;

    // sample grabber

    if (FAILED(CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC, 
        IID_IBaseFilter,(void **)&grabber_filter_)))
        return;

    if (FAILED(grabber_filter_->QueryInterface(IID_ISampleGrabber, (void **)&sample_grabber_)))
        return;

    if ( FAILED(CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, 
        IID_IBaseFilter, (void**)&null_render_filter_)) )
        return;

    media_control_ = NULL;
    media_event_ = NULL;
}

DSGraph::~DSGraph()
{
    if (device_filter_) device_filter_->Release();

    if (sample_grabber_) sample_grabber_->Release();

    if (grabber_filter_) grabber_filter_->Release();

    if (null_render_filter_) null_render_filter_->Release();

    if (graph_) graph_->Release();

    if (capture_builder_) capture_builder_->Release();


    if (media_control_) media_control_->Release();

    if (media_event_) media_event_->Release();
}

void DSGraph::Start()
{
    if (media_control_)
    {
        media_control_->Run();
    }
}

void DSGraph::Stop()
{
    if (media_control_)
    {
        media_control_->Stop();
    }

    if (media_event_)
    {
        long event_code = 0;
        if (FAILED(media_event_->WaitForCompletion(INFINITE, &event_code)))
            return;
    }

    graph_->Abort();
}

bool DSGraph::SetCapDevice(const IID& deviceIID, const CString& comObjID)
{
    HRESULT hr;

    CComPtr<ICreateDevEnum> create_dev_enum;
    create_dev_enum.CoCreateInstance(CLSID_SystemDeviceEnum);

    CComPtr<IEnumMoniker> enum_moniker;
    create_dev_enum->CreateClassEnumerator(deviceIID, &enum_moniker, 0);

    enum_moniker->Reset();

    IBindCtx* bind_ctx;
    hr = ::CreateBindCtx( 0, &bind_ctx );
    if( hr != S_OK )
    {
        return false;
    }

    ULONG ulFetched;
    CComPtr<IMoniker> moniker;
    hr = ::MkParseDisplayName( bind_ctx, comObjID, &ulFetched, &moniker );
    SAFE_RELEASE( bind_ctx );

    if( hr != S_OK )
    {
        return false;
    }

    hr = moniker->BindToObject(0,0,IID_IBaseFilter, (void **)&device_filter_);
    if( hr != S_OK )
    {
        return false;
    }

    moniker.Release();
    create_dev_enum.Release();
    enum_moniker.Release();

    return true;
}

IPin* DSGraph::GetInPin(IBaseFilter* filter, int pinIndex)
{
    IPin *pComPin = NULL;
    GetPin(filter, PINDIR_INPUT, pinIndex, &pComPin);
    return pComPin;
}

IPin* DSGraph::GetOutPin(IBaseFilter* filter, int pinIndex)
{
    IPin *pComPin = NULL;
    GetPin(filter, PINDIR_OUTPUT, pinIndex, &pComPin);
    return pComPin;
}

HRESULT DSGraph::GetPin(IBaseFilter* pFilter, PIN_DIRECTION dirrequired, int iNum, IPin **ppPin)
{
    CComPtr< IEnumPins > pEnum;
    *ppPin = NULL;

    HRESULT hr = pFilter->EnumPins( &pEnum );
    if( FAILED( hr ) ) 
    {
        return hr;
    }

    ULONG ulFound;
    IPin *pPin;
    hr = E_FAIL;

    while( S_OK == pEnum->Next( 1, &pPin, &ulFound ) )
    {
        PIN_DIRECTION pindir = (PIN_DIRECTION)3;

        pPin->QueryDirection( &pindir );
        if( pindir == dirrequired )
        {
            if( iNum == 0 )
            {
                *ppPin = pPin;  // Return the pin's interface
                hr = S_OK;      // Found requested pin, so clear error
                break;
            }
            iNum--;
        } 

        pPin->Release();
    } 

    pEnum.Release();

    return hr;
}
