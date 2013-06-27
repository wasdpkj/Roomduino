#ifndef PKJ_h
#define PKJ_h
 
#include "Arduino.h"
 
class PKJ
{
  public:
    int VOLxianzhi(int,int,int);
    int VOLtiaobian(int,int,int);
  private:
    int _num;
    int _volL;
    int _volH;
};

#endif