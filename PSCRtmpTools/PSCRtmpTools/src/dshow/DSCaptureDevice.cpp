#include "stdafx.h"
#include "DSCaptureDevice.h"

#include "baseclasses/mtype.h"

void DSCaptureDevice::ListCapDevices(const IID& deviceIID, std::map<CString, CString>& deviceList)
{
    CComPtr<IEnumMoniker> enum_moniker;

    CComPtr<ICreateDevEnum> create_dev_enum;
    create_dev_enum.CoCreateInstance(CLSID_SystemDeviceEnum);
    create_dev_enum->CreateClassEnumerator(deviceIID, &enum_moniker, 0);

    if (!enum_moniker)
    {
        return;
    }

    enum_moniker->Reset();
    deviceList.clear();

    UINT index = 0;
    while (true)
    {
        CComPtr<IMoniker> moniker;

        ULONG ulFetched = 0;
        HRESULT hr = enum_moniker->Next(1, &moniker, &ulFetched);
        if(hr != S_OK)
        {
            break;
        }

        CComPtr< IPropertyBag > pBag;
        hr = moniker->BindToStorage( 0, 0, IID_IPropertyBag, (void**) &pBag );
        if( hr != S_OK )
        {
            continue;
        }

        CComVariant var;
        var.vt = VT_BSTR;
        pBag->Read( L"FriendlyName", &var, NULL );

        LPOLESTR wszDeviceID;
        moniker->GetDisplayName( 0, NULL, &wszDeviceID );

        deviceList[(const CString)wszDeviceID] = var.bstrVal;

        index++;
    }

    create_dev_enum.Release();
    enum_moniker.Release();
}

void DSCaptureDevice::ListVideoCapDeviceWH(const CString& vDeviceID,
    std::vector<int>& widthList, std::vector<int>& heightList)
{
    // 获取设备Filter
    IBaseFilter* device_filter = NULL;

    CComPtr<ICreateDevEnum> create_dev_enum;
    create_dev_enum.CoCreateInstance(CLSID_SystemDeviceEnum);

    CComPtr<IEnumMoniker> enum_moniker;
    create_dev_enum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &enum_moniker, 0);
    enum_moniker->Reset();

    IBindCtx* bind_ctx = NULL;
    HRESULT hr = ::CreateBindCtx( 0, &bind_ctx );
    if (SUCCEEDED(hr))
    {
        ULONG ulFetched;
        CComPtr<IMoniker> moniker;
        hr = ::MkParseDisplayName( bind_ctx,  vDeviceID, &ulFetched, &moniker);
        if (bind_ctx) bind_ctx->Release();
        if (SUCCEEDED(hr))
        {
            hr = moniker->BindToObject(0,0,IID_IBaseFilter, (void **)&device_filter);
        }
        moniker.Release();
    }
    create_dev_enum.Release();
    enum_moniker.Release();

    // 获取分辨率
    ICaptureGraphBuilder2* graph_builder = NULL;
    hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER,
        IID_ICaptureGraphBuilder2,(void **)&graph_builder);
    if (SUCCEEDED(hr))
    {
        IAMStreamConfig *stream_config = NULL;
        hr = graph_builder->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, 
            device_filter, IID_IAMStreamConfig, (void**)&stream_config);
        if (SUCCEEDED(hr))
        {
            int iCount = 0, iSize  = 0;
            stream_config->GetNumberOfCapabilities(&iCount, &iSize);
            if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS))
            {
                for (int iFormat = 0; iFormat < iCount; iFormat++)
                {
                    VIDEO_STREAM_CONFIG_CAPS scc;
                    AM_MEDIA_TYPE *pmtConfig;
                    hr = stream_config->GetStreamCaps(iFormat, &pmtConfig, (BYTE*)&scc);

                    if (SUCCEEDED(hr))
                    {
                        if ((pmtConfig->majortype == MEDIATYPE_Video) &&
                            /*(pmtConfig->subtype == MEDIASUBTYPE_RGB24) &&*/
                            (pmtConfig->formattype == FORMAT_VideoInfo) &&
                            (pmtConfig->cbFormat >= sizeof (VIDEOINFOHEADER)) &&
                            (pmtConfig->pbFormat != NULL))
                        {
                            VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat;
                            widthList.push_back(pVih->bmiHeader.biWidth);
                            heightList.push_back(pVih->bmiHeader.biHeight);
                        }
                        DeleteMediaType(pmtConfig);
                    }
                }
            }
        }

        if (stream_config) stream_config->Release();
    }
    if (device_filter) device_filter->Release();
    if (graph_builder) graph_builder->Release();
}

DSCaptureDevice::DSCaptureDevice()
{

}

DSCaptureDevice::~DSCaptureDevice()
{

}
