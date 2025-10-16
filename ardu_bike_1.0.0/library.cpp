#include "library.h"

//calculate a point on a circle
void getCirclePoint(float input, float radius, float centerX, float centerY, float *outX, float *outY, bool p) {
  float angle;  
  if (p){
    angle = ((((input + 40.0f) / 80.0f) * 3.14159265f) - 90);
  } else {
    angle = ((((input + 40.0f) / 80.0f) * 3.14159265f) + 90);
  }

  *outX = centerX + radius * cosf(angle);
  *outY = centerY + radius * sinf(angle);
}

//calculate the bell curve for pedalling sound
float bell(float x) {
  float mid = 0.0f;
  float range = 40.0f;
  float normalized = fabs((x - mid)) / range; // -1 -> 1
  //float y = 1.0f - powf(normalized, 12.0f); //higher for steeper curve
  float y = powf(1.0f - normalized, 12.0f);
  return y;
}

//get the Y on a line segment given the x
int16_t lineY(int16_t x,
              int16_t x1, int16_t y1,
              int16_t x2, int16_t y2) {

  if (x2 == x1) return (y1 < y2 ? y1 : y2); //vetical edge
  return y1 + (int32_t)(y2 - y1) * (x -x1) / (x2 -x1);
}

//make it easy to check if a tire is on the second line
bool secondLine(int16_t lineTimer) {
  if (lineTimer < -116){
    return true;
  }
  else {
    return false;
  }
}

//get the difference between two numbers
float simularity(float a, float b) {
  //clamp the inputs between 0-6 just in case
  if (a < 0) { a = 0; }
  if (a > 6) { a = 6; }
  if (b < 0) { b = 0; }
  if (b > 6) { a = 6; }
  //compute normalized difference
  float diff = fabs(a - b);
  //difference between 1 and 9
  float result = 1.0 - (diff / 6.0); //linear mapping to 0-1 range
  //ensure result is .1
  if (result < .1) { result = .1; }

  return result;
}
