#ifndef _DS_CAPTURE_DEVICE_H_
#define _DS_CAPTURE_DEVICE_H_

#include <map>
#include <vector>
#include <string>

class DSCaptureDevice
{
public:
    static void ListCapDevices(const IID& deviceIID, std::map<CString, CString>& deviceList);

    static void ListVideoCapDeviceWH(const CString& vDeviceID, 
        std::vector<int>& widthList, std::vector<int>& heightList);

public:
    DSCaptureDevice();

    virtual ~DSCaptureDevice();

    // -----------------------------------------------------------------------
    // set和get数据成员

    void SetDeviceID(const CString& deviceID) { com_obj_id_ = deviceID; }
    const CString& GetID() { return com_obj_id_; }

    void SetDeviceName(const CString& deviceName) { device_name_ = deviceName; }
    const CString& GetDeviceName() { return device_name_; }

private:
    CString com_obj_id_;
    CString device_name_;
};

#endif // _DS_CAPTURE_DEVICE_H_
