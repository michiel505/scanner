#include "libs.h"
#include "utils.h"
#include "usb.h"
#include "usb_device.h"

////////////////////////////////////////////////////////////////////////////////
// UsbDevice Constructor
////////////////////////////////////////////////////////////////////////////////
UsbDevice::UsbDevice(int in_port, int out_port)
  : m_dev_handle(NULL), m_in_port(in_port), m_out_port(out_port)
{
}

////////////////////////////////////////////////////////////////////////////////
// UsbDevice Destructor
// Release the libusb device handle
////////////////////////////////////////////////////////////////////////////////
UsbDevice::~UsbDevice()
{
  if (m_dev_handle) {
    int r = libusb_release_interface(m_dev_handle, 0);
    if (r!=0) {
      DEBUG_PRINT("Cannot release interface");
    }
    DEBUG_PRINT("Interface released");

    libusb_close(m_dev_handle);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Connect the device handle to the sensor
////////////////////////////////////////////////////////////////////////////////
bool UsbDevice::Connect(libusb_context* usb_context, uint16_t vendor_id, uint16_t product_id)
{
  m_dev_handle = libusb_open_device_with_vid_pid(usb_context, vendor_id, product_id);
  if (m_dev_handle == NULL) {
    DEBUG_PRINT("Cannot open device");
    return false;
  }
  DEBUG_PRINT("Device opened");

  if (libusb_kernel_driver_active(m_dev_handle, 0)==1) {
    DEBUG_PRINT("Kernel driver active");
    if (libusb_detach_kernel_driver(m_dev_handle, 0)==0) {
      DEBUG_PRINT("Kernel driver detached!");
    }
  }

  int r = libusb_claim_interface(m_dev_handle, 0);
  if (r<0) {
    DEBUG_PRINT("Cannot claim interface");
    return false;
  }
  DEBUG_PRINT("Interface claimed");

  return true;
}

////////////////////////////////////////////////////////////////////////////////
// Send a string command to the sensor (Send an ASCII command to sensor)
////////////////////////////////////////////////////////////////////////////////
int UsbDevice::SendCommand(const char* command)
{
  int num_bytes_sent;
  int command_length = strlen(command);
  int r = libusb_bulk_transfer(
        m_dev_handle,
        (m_out_port | LIBUSB_ENDPOINT_OUT),
        (unsigned char*)command,
        command_length,
        &num_bytes_sent,
        0);
  if (r==0 && num_bytes_sent==command_length) {
    // Transmission OK
  } else {
    DEBUG_PRINT("Error %d sending command %s", r, command);
  }
  return r;
}

////////////////////////////////////////////////////////////////////////////////
// Receive a message from the sensor and store it in a string
////////////////////////////////////////////////////////////////////////////////
int UsbDevice::ReceiveResponse(char* buffer, int max_length)
{
  int received_buffer_length = 0;
  int r = libusb_bulk_transfer(
        m_dev_handle,
        (m_in_port | LIBUSB_ENDPOINT_IN),
        (unsigned char*)buffer,
        max_length,
        &received_buffer_length,
        USB_TIMEOUT);
  if (r != 0) {
    // DEBUG_PRINT("Error receiving data");
  } else {
    buffer[received_buffer_length] = '\0';
  }
  return r;
}

////////////////////////////////////////////////////////////////////////////////
// Send a command to the sensor and receive back a response from it, the data
// is stored in the response pointer (to a buffer)
////////////////////////////////////////////////////////////////////////////////
int UsbDevice::SendRequest(const char* command, char* response)
{
  DEBUG_PRINT("+ Send command: %s", command);
  int r = SendCommand(command);
  if (r!=0) {
    return r;
  }

  char* str = new char[MAXSIZE];
  r = ReceiveResponse(str, MAXSIZE);
  if (r!=0) {
    return r;
  }
  strcpy(response, str);
  delete[] str;

  DEBUG_PRINT(" -> Received data: %s\n", response);

  return 0;
}

bool UsbDevice::VerifyResponse(const char* response, const char* buffer)
{
  int d = strcmp(response, buffer);
  if (d != 0) {
    DEBUG_PRINT("Response is incorrect!");
    return false;
  }
  return true;
}

void UsbDevice::WaitUntilResponse(const char* desired_response)
{
  int r = 0;
  int d = 0;
  char* response = new char[MAXSIZE];
  do {
    r = ReceiveResponse(response, MAXSIZE);
    d = strcmp(response, desired_response);
  } while (r==0 && d!=0);
  delete[] response;
}

void UsbDevice::WaitUntilResponseHeader(const char* target_response_header)
{
  int r = 0;
  char* response = new char[MAXSIZE];
  do {
    r = ReceiveResponse(response, MAXSIZE);
    if (Utils::PrefixMatched(target_response_header, response)) {
      break;
    }
  } while (r==0);
  delete[] response;
}

void UsbDevice::WaitForAllResponse()
{
  int r = 0;
  char* response = new char[MAXSIZE];
  do {
    r = ReceiveResponse(response, MAXSIZE);
  } while (r==0);
  delete[] response;
}
