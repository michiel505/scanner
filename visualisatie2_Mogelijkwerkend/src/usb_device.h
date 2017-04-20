#ifndef _USB_DEVICE_H_
#define _USB_DEVICE_H_

#include "libs.h"

#define USB_TIMEOUT 100 // 100ms

class UsbDevice
{
protected:
  libusb_device_handle* m_dev_handle;
  int m_in_port;
  int m_out_port;

public:
  UsbDevice(int in_port=0, int out_port=0);
  virtual ~UsbDevice();

  bool Connect(libusb_context* usb_context, uint16_t vendor_id, uint16_t product_id);
  int SendCommand(const char* command);
  int ReceiveResponse(char* buffer, int max_length);
  int SendRequest(const char* command, char* response);
  bool VerifyResponse(const char* response, const char* buffer);
  void WaitUntilResponse(const char* desired_response);
  void WaitUntilResponseHeader(const char* target_response_header);
  void WaitForAllResponse();
};

#endif // _USB_DEVICE_H_
