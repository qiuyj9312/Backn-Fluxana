/*
 * @Descripttion: 
 * @version: 
 * @Author: Qiu Yijia
 * @Date: 2023-08-16 15:53:02
 * @LastEditors: Qiu Yijia
 * @LastEditTime: 2023-08-16 20:37:46
 */

#ifndef SteppingAction_h
#define SteppingAction_h 1
//my code end
#include "G4UserSteppingAction.hh"

class DetectorConstruction;
class EventAction;
class RunAction;
class TrackingAction;
class G4Track;
class G4ParticleDefinition;
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class SteppingAction : public G4UserSteppingAction
{
  public:
    SteppingAction(EventAction* evt, DetectorConstruction* detConstruction);
   ~SteppingAction();

    void UserSteppingAction(const G4Step*);
  private:
    EventAction*          feventaction;  
    DetectorConstruction* fDetConstruction;

};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
