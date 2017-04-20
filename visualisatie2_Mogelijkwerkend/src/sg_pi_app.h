#ifndef _SG_PI_APP_H_
#define _SG_PI_APP_H_

#include "libs.h"
#include "usb.h"
#include "scan_data_frame.h"
#include "data_streamer.h"

class SGPiApp
{
private:
  pthread_t* m_threads;

public:
  SGPiApp();
  ~SGPiApp();

  bool Init();
  void Run();
};

#endif // _SG_PI_APP_H_
