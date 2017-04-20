#ifndef _SAVE_DATA_H_
#define _SAVE_DATA_H_

#include "scan_data_pack.h"

/*
    Data structure for representation of date
*/
struct date_stamp_t
{
  uint32_t  year;
  uint8_t   month;
  uint8_t   day;
  uint8_t   hour;
  uint8_t   minute;
  uint8_t   second;
};

typedef date_stamp_t DateStamp;

/*
    SaveData contains all information of a save data session
    Information includes:
        - number of 2D scans
        - starting time
        - ending time
        - start angle of each 2D scan
        - end angle of each 2D scan
        - angle step of each 2D scan
        - List of all 2D scan
*/
class SaveData
{
private:
  uint32_t m_num_packs;
  date_stamp_t m_start_time;
  float m_start_angle;
  float m_end_angle;
  float m_angular_step;
  ScanDataPack* m_data;

public:
  SaveData();
  ~SaveData();

  bool SaveToFile(const char* filename);
  bool ReadFromFile(const char* filename);

  void SetNumPacks(uint32_t num_packs) { m_num_packs = num_packs; }
  void SetStartTime(DateStamp new_time) { m_start_time = new_time; }
  void SetStartTime(
          uint32_t year,
          uint32_t month,
          uint32_t day,
          uint32_t hour,
          uint32_t minute,
          uint32_t second)
      {
        m_start_time.year = year, m_start_time.month = month,
        m_start_time.day = day, m_start_time.hour = hour,
        m_start_time.minute = minute, m_start_time.second = second;
      }

  void SetStartAngle(float angle) { m_start_angle = angle; }
  void SetEndAngle(float angle) { m_end_angle = angle; }
  void SetAngularStep(float step) { m_angular_step = step; }
  void SetData(ScanDataPack* data) { m_data = data; }

  uint32_t GetNumPacks() { return m_num_packs; }
  DateStamp GetStartTime() { return m_start_time; }
  float GetStartAngle() { return m_start_angle; }
  float GetEndAngle() { return m_end_angle; }
  float GetAngularStep() { return m_angular_step; }
  ScanDataPack* GetData() { return m_data; }
};

#endif // _SAVE_DATA_H_
