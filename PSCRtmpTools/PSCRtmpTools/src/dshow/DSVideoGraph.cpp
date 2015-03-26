#include "stdafx.h"
#include "DSVideoGraph.h"
#include "baseclasses/mtype.h"

DSVideoGraph::DSVideoGraph()
{
    active_video_window_ = NULL;
}

DSVideoGraph::~DSVideoGraph()
{
    Destroy();
}

HRESULT DSVideoGraph::Create(DSVideoCaptureDevice* videoCaptureDevice)
{
    HRESULT hr;

    video_cap_device_ = videoCaptureDevice;

    SetCapDevice(CLSID_VideoInputDeviceCategory, video_cap_device_->GetID());

    //RouteCrossbar();

    SetVideoFormat(video_cap_device_->GetPreferredVideoWidth(), 
        video_cap_device_->GetPreferredVideoHeight(),
        video_cap_device_->GetPreferredVideoFPS());

    if (FAILED(graph_->AddFilter(device_filter_, L"Source" )))
        return -1;

    if (FAILED(graph_->AddFilter(grabber_filter_, L"Grabber")))
        return -1;

    CMediaType GrabType;
    GrabType.SetType(&MEDIATYPE_Video);
    GrabType.SetSubtype(&MEDIASUBTYPE_RGB24);
    if (FAILED(sample_grabber_->SetMediaType( &GrabType)))
        return -1;

    if (FAILED(sample_grabber_->SetBufferSamples(TRUE)))
        return -1;
    sample_grabber_->SetOneShot(FALSE);

    if (FAILED(sample_grabber_->SetCallback((ISampleGrabberCB*)&grabber_callback_, 1)))
        return -1;

    if (FAILED(graph_->AddFilter(null_render_filter_, L"NullRenderer")))
        return -1;

    if ( FAILED(capture_builder_->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, 
        device_filter_, grabber_filter_, null_render_filter_)) )
    {
        return -1;
    }

    IMediaFilter *pMediaFilter = 0;
    hr = graph_->QueryInterface(IID_IMediaFilter, (void**)&pMediaFilter);
    if (SUCCEEDED(hr))
    {
        pMediaFilter->SetSyncSource(NULL);
        pMediaFilter->Release();
    }

    AM_MEDIA_TYPE mt;
    if (FAILED(sample_grabber_->GetConnectedMediaType(&mt)))
        return -1;

    VIDEOINFOHEADER * vih = (VIDEOINFOHEADER*)mt.pbFormat;
    video_cap_device_->SetVideoWidth(vih->bmiHeader.biWidth);
    video_cap_device_->SetVideoHeight(vih->bmiHeader.biHeight);
    //video_cap_device_->SetVideoFPS(10000000/vih->AvgTimePerFrame);
    video_cap_device_->SetVideoFPS(video_cap_device_->GetPreferredVideoFPS());
    FreeMediaType(mt);

    if (SUCCEEDED(capture_builder_->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, 
        device_filter_, NULL, NULL)))
    {
        if (SUCCEEDED(graph_->QueryInterface(IID_IVideoWindow, (void **)&active_video_window_)))
        {
            if (active_video_window_)
            {    
                hr = active_video_window_->put_AutoShow(OAFALSE);
            }
        }
    }

    if (FAILED(graph_->QueryInterface(IID_IMediaControl, (void **)&media_control_)))
        return 0;

    if (FAILED(graph_->QueryInterface(IID_IMediaEvent, (void **)&media_event_)))
        return 0;

    return 0;
}

void DSVideoGraph::Destroy()
{
    if (active_video_window_) active_video_window_->Release();
}

HRESULT DSVideoGraph::SetVideoFormat(UINT preferredImageWidth, UINT preferredImageHeight, 
    REFERENCE_TIME preferredFPS)
{
    IAMStreamConfig *pConfig = NULL;

    HRESULT hRet;
    if (FAILED(hRet = capture_builder_->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, 
        device_filter_, IID_IAMStreamConfig, (void**)&pConfig)))
    {
        return -1;
    }

    int iCount = 0, iSize  = 0;
    if (FAILED(pConfig->GetNumberOfCapabilities(&iCount, &iSize)))
        return -1;

    bool find_fmt = false;
    if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS))
    {
        for (int iFormat = 0; iFormat < iCount; iFormat++)
        {
            VIDEO_STREAM_CONFIG_CAPS scc;
            AM_MEDIA_TYPE *pmtConfig;
            HRESULT hr = pConfig->GetStreamCaps(iFormat, &pmtConfig, (BYTE*)&scc);

            if (SUCCEEDED(hr))
            {
                if ((pmtConfig->majortype == MEDIATYPE_Video) &&
                    /*(pmtConfig->subtype == MEDIASUBTYPE_RGB24) &&*/
                    (pmtConfig->formattype == FORMAT_VideoInfo) &&
                    (pmtConfig->cbFormat >= sizeof (VIDEOINFOHEADER)) &&
                    (pmtConfig->pbFormat != NULL))
                {
                    VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat;

                    LONG lWidth  = pVih->bmiHeader.biWidth;
                    LONG lHeight = pVih->bmiHeader.biHeight;

                    if ((lWidth == preferredImageWidth) && (lHeight == preferredImageHeight))
                    {
                        find_fmt = true;

                        pVih->AvgTimePerFrame = 10000000/preferredFPS;
                        hr = pConfig->SetFormat(pmtConfig);
                        break;
                    }
                }

                DeleteMediaType(pmtConfig);
            }
        }
    }

    SAFE_RELEASE(pConfig);

    return 0;
}

bool DSVideoGraph::RouteCrossbar()
{
    IAMCrossbar *pXBar1 = NULL;
    HRESULT hr = capture_builder_->FindInterface(&LOOK_UPSTREAM_ONLY, NULL, device_filter_,
        IID_IAMCrossbar, (void**)&pXBar1);
    if (SUCCEEDED(hr)) 
    {
        bool foundDevice = false;

        pXBar1->Release();

        IAMCrossbar *Crossbar;
        hr = capture_builder_->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Interleaved, 
            device_filter_, IID_IAMCrossbar, (void **)&Crossbar);

        if(hr != NOERROR){
            hr = capture_builder_->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, 
                device_filter_, IID_IAMCrossbar, (void **)&Crossbar);
        }

        LONG lInpin, lOutpin;
        hr = Crossbar->get_PinCounts(&lOutpin , &lInpin); 

        BOOL IPin=TRUE; LONG pIndex=0 , pRIndex=0 , pType=0;
        while( pIndex < lInpin)
        {
            hr = Crossbar->get_CrossbarPinInfo( IPin , pIndex , &pRIndex , &pType);

            if( pType == PhysConn_Video_Composite)
            {
                foundDevice = true;
                break;
            }
            pIndex++;
        }

        if(foundDevice)
        {
            BOOL OPin=FALSE; LONG pOIndex=0 , pORIndex=0 , pOType=0;
            while( pOIndex < lOutpin)
            {
                hr = Crossbar->get_CrossbarPinInfo( OPin , pOIndex , &pORIndex , &pOType);

                if( pOType == PhysConn_Video_VideoDecoder)
                    break;
            }
            Crossbar->Route(pOIndex,pIndex); 
        }
        else
        {
            //if(verbose)printf("SETUP: Didn't find specified Physical Connection type. Using Defualt. \n");	
        }			

        //we only free the crossbar when we close or restart the device
        //we were getting a crash otherwise
        //if(Crossbar)Crossbar->Release();
        //if(Crossbar)Crossbar = NULL;

        if(pXBar1)pXBar1->Release();
        if(pXBar1)pXBar1 = NULL;

    }
    else
    {
        //if(verbose)printf("SETUP: You are a webcam or snazzy firewire cam! No Crossbar needed\n");
        return hr;
    }

    return hr;
}

void DSVideoGraph::AdjustVideoWindow(OAHWND owner, unsigned int width, unsigned int height)
{
    if (active_video_window_)
    {
        active_video_window_->put_Visible(OAFALSE);
        active_video_window_->put_Owner(NULL);

        active_video_window_->put_Owner(owner);
        active_video_window_->put_WindowStyle(WS_CHILD | WS_CLIPCHILDREN);
        active_video_window_->put_Visible(OATRUE);
        active_video_window_->SetWindowPosition(0, 0, width, height);
    }
}
