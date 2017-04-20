#ifndef _DATA_STREAMER_H_
#define _DATA_STREAMER_H_

#include "libs.h"
#include "usb.h"
#include "usb_device.h"
#include "scan_data_frame.h"
#include "save_data.h"

#define SICK_ID                 0x19A2
#define SICKTIM_ID              0x5001

#define REQUEST_DATA          "\x02sRN LMDscandata\x03" // Free

#define START_STREAM          "\x02sEN LMDscandata 1\x03" // Free
#define START_STREAM_RESPONSE "\x02sEA LMDscandata 1\x03"

#define STOP_STREAM           "\x02sEN LMDscandata 0\x03" // Free
#define STOP_STREAM_RESPONSE  "\x02sEA LMDscandata 0\x03"

#define START_MEASUREMENT           "\x02sMN LMCstartmeas\x03" // Authorized client
#define START_MEASUREMENT_RESPONSE  "\x02sAN LMCstartmeas\x03"

#define STOP_MEASUREMENT          "\x02sMN LMCstopmeas\x03" // Authorized client
#define STOP_MEASUREMENT_RESPONSE "\x02sAN LMCstopmeas\x03"

#define READ_FREQUENCY        "\x02sRN LMPscancfg\x03" // Free

#define READ_OUTPUT_RANGE     "\x02sRN LMPoutputRange\x03" // Authorized client

#define LOGIN                 "\x02sMN SetAccessMode 04 81BE23AA\x03" // Password: "servicelevel" (service)
#define LOGIN_RESPONSE        "\x02sAN SetAccessMode\x03"

#define CONFIGURE_DATA        "\x02sWN LMDscandatacfg 01 00 1 0 0 00 00 1 1 0 1 +1\x03"

#define CONFIGURE_MEASUREMENT "\x02sWN LMPoutputRange 1 D05 0 270F\x03" // From 0 to 0,33 degree

#define SAVE_CONFIGURATION    "\x02sMN mEEwriteall\x03"

#define RUN                   "\x02sMN Run\x03" // Same with logging out
#define RUN_RESPONSE          "\x02sAN Run 1\x03"

#define DATA_PACK_HEADER      "\x02sSN LMDscandata"

class DataStreamer
{
private:
  Usb*         m_usb; // USB interface
  uint32_t     m_num_packs; // Number of 2D data packets

  // Sick stream
  UsbDevice*    m_tim;
  ScanDataPack* m_packs; // List of data packs acquired
  bool SickInit();

  // Arduini stream and stepCount
  bool ArduinoInit();

  // Other members
  bool m_flag_exit;
  uint32_t m_num_backups;
  ScanDataFrame* m_first_frame;
  std::chrono::system_clock::time_point m_start_system_time;
  // void ParseArduinoData(const char* buffer);
  bool m_flushing;

public:
  DataStreamer();
  ~DataStreamer();

  bool Init();
  void AcquireSickData();
  void AcquireArduinoData();
  void FlushData();

  void SetExitFlag() { m_flag_exit = true; }

  bool GetExitFlag() { return m_flag_exit; }
};

#endif // _DATA_STREAMER_H_
