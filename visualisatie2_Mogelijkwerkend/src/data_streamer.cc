#include "libs.h"
#include "data_streamer.h"
#include "timer.h"
#include "utils.h"
#include "scan_data_frame.h"
#include "global_var.h"
#include "save_data.h"
#include "scan_data_pack.h"
#include "rs232.h"

#define FLUSHING_CYCLE_TIME 5e6 // 5s
#define SICK_ACQUISITION_CYCLE_TIME 66666 // Sick DAQ 15Hz
#define ARDUINO_ACQUISITION_CYCLE_TIME 33333 // Arduino DAQ 30 Hz
#define ARDUINO_BAUD_RATE 115200

#define MAXNUMPACKS 100
#define COMPORT_NUMBER 24

DataStreamer::DataStreamer()
  : m_usb(NULL), m_tim(NULL), m_packs(NULL),
    m_flag_exit(false), m_num_backups(0), m_first_frame (NULL),
    m_flushing(false)
{
  m_start_system_time = std::chrono::system_clock::now();
}

DataStreamer::~DataStreamer()
{
   // Reset arduino for stopping
   char mode[] = "8N1"; // {'8','N','1', 0}; 8 databits, no parity, 1 stop bit
   RS232_CloseComport(COMPORT_NUMBER);
   RS232_OpenComport(COMPORT_NUMBER, ARDUINO_BAUD_RATE, mode);
   RS232_CloseComport(COMPORT_NUMBER);

// Combine the backed-up files
  // Read data packs from backed-up files
	 SaveData* backup_recs = new SaveData[m_num_backups];
	 uint32_t num_packs = m_num_packs;

	 for (uint32_t i = 0; i<m_num_backups; ++i)
	     {
		   char filename[100];
		   sprintf(filename, "backup_files/backup_%d.dat", i);
		   backup_recs[i].ReadFromFile(filename);
		   num_packs += backup_recs[i].GetNumPacks();
	     }

	 ScanDataPack* packs = new ScanDataPack[num_packs];
	 uint32_t k = 0;

	 for (uint32_t i = 0; i<m_num_backups; ++i)
	       {
		   ScanDataPack* pack_list = backup_recs[i].GetData();
		   for (uint32_t j = 0; j<backup_recs[i].GetNumPacks(); ++j)
		        {
			      packs[k++] = pack_list[j];
		        }
	      }
	for (uint32_t i = 0; i<m_num_packs; ++i)
	 {
		packs[k++] = m_packs[i];
	 }

	 // Save a new data
	 SaveData* rec = new SaveData();
	 rec->SetNumPacks(num_packs);
	 rec->SetStartTime(backup_recs[0].GetStartTime());
	 rec->SetStartAngle(m_first_frame->GetStartAngle());
	 rec->SetEndAngle(m_first_frame->GetEndAngle());
	 rec->SetAngularStep(m_first_frame->GetAngularStep());
	 rec->SetData(packs);

	 if (!rec->SaveToFile(g_output_filename))
	 {
		DEBUG_PRINT("Failed to save file!");
	 }

	 delete[] backup_recs;
	 delete rec;

// Stop streaming for the SICK
  char* response = new char[MAXSIZE];

  // Stop streaming
  DEBUG_PRINT("+ Send command: %s", STOP_STREAM);
  m_tim->SendCommand(STOP_STREAM);
  m_tim->WaitUntilResponse(STOP_STREAM_RESPONSE);
  DEBUG_PRINT("-> Received data: %s", STOP_STREAM_RESPONSE);

  m_tim->SendRequest(LOGIN, response);
  m_tim->SendRequest(STOP_MEASUREMENT, response);
  m_tim->SendRequest(SAVE_CONFIGURATION, response);
  m_tim->SendRequest(RUN, response);
  m_tim->VerifyResponse(RUN_RESPONSE, response);

  delete[] response;

  delete[] m_packs;

  delete m_tim;
  delete m_usb;

  delete m_first_frame;
}
/*
    Initialize interface with Arduino and SICK sensor
*/
bool DataStreamer::Init()
{
  m_usb = new Usb();
  if (!m_usb->Init()) {
    return false;
  }
  if (!ArduinoInit()) {
    ERROR_PRINT("Failed to initialize Arduino");
    return false;
  }
  if (!SickInit()) {
    ERROR_PRINT("Failed to initialize SICK");
    return false;
  }
  return true;
}
/*
    SICK DAQ
*/
void DataStreamer::AcquireSickData()
{
  Timer* sick_daq_timer = new Timer();
  sick_daq_timer->Reset();

  char* response = new char[MAXSIZE];
  std::chrono::high_resolution_clock::time_point start_time;
  g_data_frame = NULL;
  g_data_pack = NULL;

  while (!m_flag_exit)
{
    // Get data from SICK
    if (sick_daq_timer->TimeOut(SICK_ACQUISITION_CYCLE_TIME))
       {
      int r = m_tim->ReceiveResponse(response, MAXSIZE);
      if (r==0) { // Data received
        DEBUG_PRINT("received the SICK data");
        if (m_first_frame==NULL)
          {
          start_time = std::chrono::high_resolution_clock::now();
          m_start_system_time = std::chrono::system_clock::now();
          }

        delete g_data_frame;
        g_data_frame = new ScanDataFrame();
        g_data_frame->SetTimeStampToCurrentTime();
        g_data_frame->ParseDatagram(response, strlen(response));

	DEBUG_PRINT(response);


	// Update current data pack
	      delete g_data_pack;
	      g_data_pack = new ScanDataPack();
        g_data_pack->SetstepCount(stepCount);
	      //g_data_pack->SetVelocity(g_velocity);
	      g_data_pack->ExtractTimFrame(g_data_frame, start_time);

        if (m_first_frame==NULL)
        {
          m_first_frame = new ScanDataFrame(*g_data_frame);
        }

    // Add new pack to the list if flushing is not happening
		    if (!m_flushing)
		      {
			     m_packs[m_num_packs] = *g_data_pack;
			     ++m_num_packs;
			     if (m_num_packs == MAXNUMPACKS)
			        {
				      m_num_packs = MAXNUMPACKS - 1;
			        }
		      }
      }

      sick_daq_timer->Reset();
    }
    Utils::Delay(100); // Delay 100us
}

  delete[] response;
  delete sick_daq_timer;

  delete g_data_frame;
  delete g_data_pack;
}

bool IsWeirdAngle(float a)
{
  if (a < -1e6 || a > 1e6)
  {
    return true;
  }
  return false;
}
/*
    Arduino DAQ
*/
void DataStreamer::AcquireArduinoData()
{
  Timer mega_daq_timer;
  mega_daq_timer.Reset();
  char buf[100];
  while (!m_flag_exit) {
     // Get data from Arduino
    if (mega_daq_timer.TimeOut(ARDUINO_ACQUISITION_CYCLE_TIME)) {
      RS232_SendByte(COMPORT_NUMBER, 'S');
      RS232_flushTX(COMPORT_NUMBER);
      while (RS232_Available(COMPORT_NUMBER) < 3) { // Wait until receive the packet
        Utils::Delay(10); // Delay 10us
      }
      RS232_PollComport(COMPORT_NUMBER, (unsigned char*)buf, 100);
      RS232_flushRX(COMPORT_NUMBER);
      int16_t stepCount = *(int16_t*)(&buf[0]);
      uint8_t last = *(uint8_t*)(&buf[2]); // Last byte

      if (last != 0xFA) {
        ERROR_PRINT("Last byte is different (expected 0xFA)");
      }
      DEBUG_PRINT ("steps: %.2d last: %.2i", stepCount, last); // "%.2d" decimal, "%.2i" integer
      mega_daq_timer.Reset();
    }
    Utils::Delay(100);
  }
}
/*
    After some fixed amount of time (FLUSHING_CYCLE_TIME), the acquired data is
    written to hard disk to save memory space
*/
void DataStreamer::FlushData()
{
	// Flush periodically
	Timer* flush_timer = new Timer();
	flush_timer->Reset();
	while (!m_flag_exit)
	{
		DEBUG_PRINT("FLUSHING");
		if (flush_timer->TimeOut(FLUSHING_CYCLE_TIME))
		{
			DEBUG_PRINT("Flush data here");
			m_flushing = true;

			// Saving the record
			SaveData* rec = new SaveData();
			rec->SetNumPacks(m_num_packs);
			if (m_num_backups == 0) {
				time_t t = time(0);
				tm* now = localtime(&t);
				TimeStamp ts = m_packs[0].GetTimeStamp();
				rec->SetStartTime(
					now->tm_year + 1900,
					now->tm_mon + 1,
					now->tm_mday,
					ts.hour,
					ts.minute,
					ts.second
					);
			}
			else {
				rec->SetStartTime(0, 0, 0, 0, 0, 0);
			}
			rec->SetStartAngle(m_first_frame->GetStartAngle());
			rec->SetEndAngle(m_first_frame->GetEndAngle());
			rec->SetAngularStep(m_first_frame->GetAngularStep());

			ScanDataPack* pack_list = new ScanDataPack[m_num_packs];
			for (uint32_t i = 0; i<m_num_packs; i++) {
				pack_list[i] = m_packs[i];
			}
			rec->SetData(pack_list);

			DEBUG_PRINT("Flushing finished!");

			char* filename = new char[MAXSIZE];
			sprintf(filename, "backup_files/backup_%d.dat", m_num_backups);
			if (!rec->SaveToFile(filename)) {
				DEBUG_PRINT("Failed to save %s file!", filename);
			}
			delete[] filename;
			delete rec;
			++m_num_backups;
			delete[] m_packs;
			m_num_packs = 0;
			m_packs = new ScanDataPack[MAXNUMPACKS];

			m_flushing = false;
			flush_timer->Reset();
		}

		Utils::Delay(100e3); // Delay 100ms
	}

	delete flush_timer;
}
/*
    Initialize SICK TIM data stream
*/
bool DataStreamer::SickInit()
{
  m_tim = new UsbDevice(1,2);
  char* response = new char[MAXSIZE];
  bool connected = false;

// Connect to SICK
  for (int i=0; i<3; ++i) { // Try to connect 3 times
    DEBUG_PRINT("Trying to connect to SickTim...");
    if (m_tim->Connect(m_usb->GetContext(), SICK_ID, SICKTIM_ID)) {
      DEBUG_PRINT("Connection succeeded!");
      connected = true;
      break;
    }
    Utils::Delay(1e6); // Delay 1s
  }
  if (!connected) {
    DEBUG_PRINT("Failed to connect to SickTim!");
    return false;
  }

// Start streaming data from SICK
  // Stop streaming
  DEBUG_PRINT("+ Send command: %s", STOP_STREAM);
  m_tim->SendCommand(STOP_STREAM);
  m_tim->WaitUntilResponse(STOP_STREAM_RESPONSE);
  DEBUG_PRINT("-> Received data: %s", STOP_STREAM_RESPONSE);

  m_tim->WaitForAllResponse(); // Making sure SICK doesn't talk anymore

  m_tim->SendRequest(LOGIN, response);
  m_tim->SendRequest(STOP_MEASUREMENT, response);
  m_tim->SendRequest(SAVE_CONFIGURATION, response);
  m_tim->SendRequest(RUN, response);
  m_tim->VerifyResponse(RUN_RESPONSE, response);

  Utils::Delay(1e6); // Delay 1s

  m_tim->SendRequest(LOGIN, response);
  m_tim->SendRequest(CONFIGURE_DATA, response);
  m_tim->SendRequest(CONFIGURE_MEASUREMENT, response);
  m_tim->SendRequest(SAVE_CONFIGURATION, response);
  m_tim->SendRequest(RUN, response);
  m_tim->VerifyResponse(RUN_RESPONSE, response);

  m_tim->SendRequest(LOGIN, response);
  m_tim->SendRequest(START_MEASUREMENT, response);
  m_tim->SendRequest(SAVE_CONFIGURATION, response);
  m_tim->SendRequest(RUN, response);

  m_tim->SendRequest(START_STREAM, response);
  m_tim->VerifyResponse(START_STREAM_RESPONSE, response);

  m_tim->WaitUntilResponseHeader(DATA_PACK_HEADER);

  // This makes sure the data packs will be received correctly
  int r;
  do
   {
    r = m_tim->ReceiveResponse(response, MAXSIZE);
   }
  while (r!=0);

  delete[] response;
  m_num_packs = 0;
  m_packs = new ScanDataPack [MAXNUMPACKS];
  return true;
}
/*
    Initialize serial communication with Arduino
*/
bool DataStreamer::ArduinoInit()
{
  while (1) {
  // Connect to Arduino
    char mode[] = "8N1"; // {'8','N','2', 0}; 8 databits, no parity, 1 stop bit
    Timer timeout;
    timeout.Reset();
    RS232_OpenComport(COMPORT_NUMBER, ARDUINO_BAUD_RATE, mode);
    RS232_CloseComport(COMPORT_NUMBER);
    if (RS232_OpenComport(COMPORT_NUMBER, ARDUINO_BAUD_RATE, mode)) {
      ERROR_PRINT("Can not open comport");
      return false;
    }
    RS232_flushRX(COMPORT_NUMBER);
    RS232_flushTX(COMPORT_NUMBER);
    DEBUG_PRINT("Starting Arduino");
    while (RS232_Available(COMPORT_NUMBER) < 1 && !timeout.TimeOut(60e6)) { // Timeout 1 minute
      Utils::Delay(1e6);
      DEBUG_PRINT("Waiting for Arduino %llds", (int64_t)(timeout.GetElapsed()/1e6));
    }
    uint8_t tmp;
    RS232_PollComport(COMPORT_NUMBER, (unsigned char*)&tmp, sizeof(uint8_t));
    if (tmp != 0xAF) {
      return false;
    }
    break;
    /*
    // Request first packet
    float t = 0;
    char buf[100];
    RS232_SendByte(COMPORT_NUMBER, 'S');
    RS232_flushTX(COMPORT_NUMBER);
    while (RS232_Available(COMPORT_NUMBER) < 3) {} // Wait until receive the packet
    RS232_PollComport(COMPORT_NUMBER, (unsigned char*)buf, 100);
    RS232_flushRX(COMPORT_NUMBER);
    int16_t stepCount = *(int16_t*)(&buf[0]);
    if (stepCount == 1) break;
    YawPitchRoll received_angle;
    received_angle.yaw = *(float*)(&buf[0]);
    received_angle.pitch = *(float*)(&buf[4]);
    received_angle.roll = *(float*)(&buf[8]);

    DEBUG_PRINT("yaw: %f, pitch: %f, roll: %f", received_angle.yaw, received_angle.pitch, received_angle.roll);*/

  /*  if ((received_angle.roll>-1 && received_angle.roll<1) &&
        (received_angle.pitch>-1 && received_angle.pitch<1))
        {
          // Good roll and pitch
          break;
        }

    DEBUG_PRINT("Resetting...!");
    RS232_SendByte(COMPORT_NUMBER, 'E');
    RS232_CloseComport(COMPORT_NUMBER);
    */
  }
  return true;
}
