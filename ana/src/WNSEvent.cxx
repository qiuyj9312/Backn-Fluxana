/******************************************************
 * File: Event.cxx
 * Description: C++ source file of the WNS event class.
 * Comments:
 * Author: GMH(gumh@ihep.ac.cn)
 * Created: 2017/11/06
------------------------------------------------------- 
 * Modified by CHEN Yonghao
* 2019/03
******************************************************/

#include <math.h>
#include <TMath.h>
#include <TMatrixD.h>
#include <TVectorD.h>
#include <TDecompSVD.h>

#include "WNSEvent.h"

ClassImp(EventHeader) ;
ClassImp(WNSEvent) ;

WNSEvent::WNSEvent() : fArrayLength(0)
{
}

WNSEvent::~WNSEvent()  
{
}