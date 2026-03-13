/*****************************************************
 * File: Event.h
 * Description: header file of the WNS event class.
 * Comments:
 * Author: GMH(gumh@ihep.ac.cn)
 * Created: 2017/11/06
 
 ----------------------------------------------------- 
 * Modified by CHEN Yonghao
* 2019/03
 ****************************************************/

#ifndef __WNSEvent
#define __WNSEvent 1

#include "TObject.h"
#include "TClonesArray.h"
#include "TVectorD.h"
#include <iostream>
#include <TClass.h>

#define _IS_DECODE 1

using namespace std ;

class EventHeader 
{
  private:
        Int_t   fRunNumber ;    
        Int_t   fEventNumber ;            
        Int_t   fFileNumber ;
        
        ULong64_t   fT0Sec ;
        ULong64_t   fT0NanoSec ;

  public:
        EventHeader() : fRunNumber(0), fEventNumber(0), fFileNumber(0), fT0Sec(0), fT0NanoSec(0) { }
        virtual ~EventHeader() { }
        
        // Set methods
        void   Set(Int_t r, Int_t e, Int_t f, ULong64_t s, ULong64_t ns) 
        { 
          fRunNumber = r ;           
          fEventNumber = e ; 
          fFileNumber = f ;
          fT0Sec = s ;
          fT0NanoSec = ns ; 
        }   
        
        // Get methods
        Int_t  GetEventNumber()   const { return fEventNumber ; }
        Int_t  GetRunNumber()     const { return fRunNumber ; }
        Int_t  GetFileNumber()    const { return fFileNumber ; }
        ULong64_t  GetT0Sec()     const { return fT0Sec ;}
        ULong64_t  GetT0NanoSec() const { return fT0NanoSec ; }

        ClassDef(EventHeader,1)  //Event Header
} ;



class WNSEvent : public TObject 
{
  private:
        EventHeader       fEventHeader ;    
        Int_t             fArrayLength ;                
        vector <Int_t>    fvChannelID ; 
                                  
        vector <Double_t> fvT0 ;   // Zero-crossing timing of derivative signal
        vector <Double_t> fvTc1 ;  // constant fraction (0.9) timing of derivative signal
        vector <Double_t> fvTc2 ;  // constant fraction (1.0) timing of derivative signal
        
        vector <Double_t> fvhpn ;
        vector <Double_t> fvhp  ;
        vector <Double_t> fvhn  ;


  public:
        WNSEvent() ;
        virtual ~WNSEvent() ;
        
        void ClearVector()
        {
          fvChannelID.clear() ;
          
          fvT0.clear() ;
          fvTc1.clear() ;
          fvTc2.clear() ;
          
          fvhpn.clear() ;
          fvhp.clear() ;
          fvhn.clear() ;
        }

        // Set Methods
        void SetHeader(Int_t run, Int_t event, Int_t file, ULong64_t t0s, ULong64_t t0ns) 
        {
          fEventHeader.Set(run, event, file, t0s, t0ns) ;
        }
        
        void SetArrayLength(Int_t ArrayLength) { fArrayLength = ArrayLength ; }
        
        void Insert(Int_t ChannelID, Double_t T0, Double_t Tc1, Double_t Tc2, Double_t hpn, Double_t hp, Double_t hn)
        {
          fvChannelID.push_back(ChannelID) ;
          
          fvT0.push_back(T0) ;
          fvTc1.push_back(Tc1) ;
          fvTc2.push_back(Tc2) ;
          
          fvhpn.push_back(hpn) ;
          fvhp.push_back(hp) ;
          fvhn.push_back(hn) ;
        }

        // Get Methods
        EventHeader      *GetHeader()            { return &fEventHeader ; }
        Int_t             GetArrayLength() const { return fArrayLength ; }
        vector <Int_t>    GetChannelID()   const { return fvChannelID ; }
        
        vector <Double_t> GetvT0()         const { return fvT0 ; }
        vector <Double_t> GetvTc1()        const { return fvTc1 ; }
        vector <Double_t> GetvTc2()        const { return fvTc2 ; }
        
        vector <Double_t> Getvhpn()        const { return fvhpn ; }
        vector <Double_t> Getvhp()         const { return fvhp ; }
        vector <Double_t> Getvhn()         const { return fvhn ; }

        ClassDef(WNSEvent,1) 
} ;

#endif
