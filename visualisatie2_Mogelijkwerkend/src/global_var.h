#ifndef _GLOBAL_VAR_H_
#define _GLOBAL_VAR_H_

#include "libs.h"
#include "sg_pi_app.h"

extern ScanDataFrame* g_data_frame;
extern ScanDataPack* g_data_pack;
//extern float g_velocity;
extern float g_initial_velocity_setpoint;
extern float g_velocity_setpoint;
extern int16_t stepCount;
extern char* g_output_filename;

#endif // _GLOBAL_VAR_H_
