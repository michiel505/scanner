#ifndef _UTILS_H_
#define _UTILS_H_

#include "libs.h"

#define MAXSIZE 65536
#define DEG2RAD(x) (x*PI/180)

namespace Utils
{

  class Vector2D
  {
  private:
    double m_x, m_y;

  public:
    Vector2D() : m_x(0.0), m_y(0.0) {}
    Vector2D(double x, double y) : m_x(x), m_y(y) {}
    ~Vector2D() {}

    double GetX() { return m_x; }
    double GetY() { return m_y; }
  };

  class Vector3D
  {
  private:
    double m_x, m_y, m_z;

  public:
    Vector3D() : m_x(0.0), m_y(0.0), m_z(0.0) {}
    Vector3D(double x, double y, double z) : m_x(x), m_y(y), m_z(z) {}
    ~Vector3D() {}

    double GetX() { return m_x; }
    double GetY() { return m_y; }
    double GetZ() { return m_z; }

    void SetX(double x) { m_x = x; }
    void SetY(double y) { m_y = y; }
    void SetZ(double z) { m_z = z; }
  };

  void GetTimeString(timeval* time_val, char* result_str);
  bool PrefixMatched(const char* prefix, const char* str);
  void Delay(const int64_t micros);
};

#endif // _UTILS_H_
