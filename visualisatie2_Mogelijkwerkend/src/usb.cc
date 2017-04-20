#include "libs.h"
#include "usb.h"

Usb::Usb()
  : m_context(NULL)
{
}

Usb::~Usb()
{
  if (m_context) {
    libusb_exit(m_context);
    DEBUG_PRINT("Libusb released!");
  }
}

bool Usb::Init()
{
  int r = libusb_init(&m_context);
  if (r<0) {
    DEBUG_PRINT("Error initializing");
    return false;
  }
  DEBUG_PRINT("Libusb initialized!");

  libusb_set_debug(m_context, 3); // set verbosity level to 3, as suggested in the documentation

  return true;
}
