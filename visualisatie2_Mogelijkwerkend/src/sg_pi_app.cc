#include "libs.h"
#include "sg_pi_app.h"
#include "utils.h"
#include "timer.h"

#define NUM_THREADS 3 //(4 if VISION_THREAD is included)

#define SICK_STREAM_THREAD      0
#define ARDUINO_STREAM_THREAD   1
#define FLUSH_THREAD            2
//#define VISION_THREAD           3


// These variables are the latest update of the data acquired from Sick sensor
// and Arduino
ScanDataFrame* g_data_frame = NULL;
ScanDataPack* g_data_pack = NULL;
uint16_t stepCount = 0;
float g_initial_velocity_setpoint = 0.0f;
float g_velocity_setpoint = 0.0f;
char* g_output_filename;
/*
    SGPiApp implements multithreading process to ensure real-time acquisition
    of data from SickTim and Arduino
*/
static void* SickStreamThread(void* arg);
static void* ArduinoStreamThread(void* arg);
static void* FlushDataThread(void* arg);

DataStreamer* stream = NULL;

SGPiApp::SGPiApp()
  : m_threads(NULL)
{
}

SGPiApp::~SGPiApp()
{
  for (int i=0; i<NUM_THREADS; ++i) {
    void* status;
    int rc = pthread_join(m_threads[i], &status);
    if (rc)
    {
      DEBUG_PRINT("Error joining thread %d", i);
    }
    DEBUG_PRINT("Main: completed joining with thread %d having a status of %d",
      i, status);
  }

  delete[] m_threads;

  delete stream;
}
/*
    Initialize data stream and creating new threads
*/
bool SGPiApp::Init()
{
  g_velocity_setpoint = g_initial_velocity_setpoint;

  // Initialize all resources for all the threads here
  stream = new DataStreamer();
  if (!stream->Init()) {
    return false;
  }

  // Then start the threads
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  m_threads = new pthread_t[NUM_THREADS];
  int rc = 0;

  rc = pthread_create(&m_threads[SICK_STREAM_THREAD], &attr, SickStreamThread, NULL);
  if (rc) {
    printf("ERROR; return code from pthread_create() is %d\n", rc);
    return false;
  }

  rc = pthread_create(&m_threads[ARDUINO_STREAM_THREAD], &attr, ArduinoStreamThread, NULL);
    if (rc) {
      printf("ERROR; return code from pthread_create() is %d\n", rc);
      return false;
    }

  rc = pthread_create(&m_threads[FLUSH_THREAD], &attr, FlushDataThread, NULL);
  if (rc) {
	  printf("ERROR; return code from pthread_create() is %d\n", rc);
	  return false;

	}
  pthread_attr_destroy(&attr);

  return true;
}
/*
    Main loop
*/
void SGPiApp::Run ()
{
  // Run the loop forever until 'q' is entered
  Timer* delay_timer = new Timer();

  while (!stream->GetExitFlag()) {
    delay_timer->Reset();
    delay_timer->Wait(10e3);

    char ch = getchar();

    if (ch=='Q' || ch=='q') {
      stream->SetExitFlag();
      break;
    }
  }

  delete delay_timer;
}

void* SickStreamThread(void* arg)
{
  DEBUG_PRINT("Start streaming");
  stream->AcquireSickData(); // This function should run its own loop
  return NULL;
}

void* ArduinoStreamThread(void* arg)
{
  DEBUG_PRINT("Start streaming");
  stream->AcquireArduinoData(); // This function should run its own loop
  return NULL;
}

void* FlushDataThread(void* arg)
{
	puts("Start flushing");
	stream->FlushData();
	return NULL;
}
/* void* VisionThread(void* arg)
{
    puts("Start vision");
    // Vision code goes here
    return NULL;
}*/
