#ifndef _USB_H_
#define _USB_H_

#include "libs.h"

class Usb
{
private:
  libusb_context* m_context;

public:
  Usb();
  ~Usb();

  bool Init();
  libusb_context* GetContext() const { return m_context; }
};

#endif // _USB_H_
