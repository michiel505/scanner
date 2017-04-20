#include "libs.h"
#include "save_data.h"

SaveData::SaveData()
  : m_data(NULL)
{
}

SaveData::~SaveData()
{
  delete[] m_data;
}
/*
    Save the save data to a file
*/
bool SaveData::SaveToFile(const char* filename)
{
  std::ofstream file_out;
  file_out.open(filename, std::ios_base::binary);

  if (!file_out.is_open()) {
    return false;
  }

  // Write the header
  file_out.write((const char*)this, sizeof(SaveData) - sizeof(ScanDataPack*));

  // Write the data
  for (uint32_t i=0; i<m_num_packs; ++i) {
    char* data_buffer;
    int data_size = m_data[i].SaveToNewBuffer(&data_buffer);
    file_out.write(data_buffer, data_size);
    delete[] data_buffer;
  }

  file_out.close();

  return true;
}
/*
    Read RoadRecord data from a file
*/
bool SaveData::ReadFromFile(const char* filename)
{
  std::ifstream fin;

  fin.open(filename, std::ios_base::binary);
  if (!fin.is_open()) {
    return false;
  }

  // Read the header
  fin.read((char*)this, sizeof(SaveData) - sizeof(ScanDataPack*));

  // Read the data
  m_data = new ScanDataPack[m_num_packs];
  fin.seekg(sizeof(SaveData) - sizeof(ScanDataPack*), std::ios::beg); // Incompatibility between OSs

  for (uint32_t i=0; i<m_num_packs; ++i) {
    uint32_t num_points;
    fin.read((char*)&num_points, sizeof(uint32_t));

    TimeStamp time_stamp;
    fin.read((char*)&time_stamp, sizeof(TimeStamp));

    uint32_t step;
    fin.read((char*)&step, sizeof(uint32_t));

    uint16_t* ranges = new uint16_t[num_points];
    fin.read((char*)ranges, sizeof(uint16_t)*num_points);

    m_data[i].SetNumPoints(num_points);
    m_data[i].SetTimeStamp(time_stamp);
    m_data[i].SetstepCount(step);
    m_data[i].SetRangeList(ranges);
  }

  fin.close();

  return true;
}
