#include "Arduino.h"
#include "PKJ.h"
 
int PKJ::VOLxianzhi(int _num,int _volL,int _volH)
{
	if (_num>_volH)
	_num=_volH;
	if(_num<_volL)
	_num=_volL;
	return _num;
}

int PKJ::VOLtiaobian(int _num,int _volL,int _volH)
{
	if (_num>_volH)
	_num=_volL;
	if(_num<_volL)
	_num=_volH;
	return _num;
}
