/*
 * @Begin: *********************************
 * @Author: Liuchangqi
 * @Company: LZU
 * @Date: 2020-03-23 08:31:16
 * @LastEditTime: 2024-01-02 01:08:08
 * @Email: liuchq16@lzu.edu.cn
 * @Descripttion: 
 * @End:   *********************************
 */

/// \file ActionInitialization.cc
/// \brief Implementation of the ActionInitialization class

#include "ActionInitialization.hh"
#include "PrimaryGeneratorAction.hh"
#include "PrimaryGeneratorGunAction.hh"
#include "RunAction.hh"
#include "EventAction.hh"
#include "TrackingAction.hh"
#include "SteppingAction.hh"
#include "DetectorConstruction.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

ActionInitialization::ActionInitialization(DetectorConstruction* detConstruction)
    : G4VUserActionInitialization(),
       fDetConstruction(detConstruction)
{
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

ActionInitialization::~ActionInitialization()
{
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void ActionInitialization::BuildForMaster() const
{
  EventAction *event = new EventAction();  
  RunAction *runAction = new RunAction(event, fDetConstruction);
  SetUserAction(runAction);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void ActionInitialization::Build() const
{
  EventAction *event = new EventAction();
  SetUserAction(event);

  RunAction *runAction = new RunAction(event, fDetConstruction);
  SetUserAction(runAction);

  //PrimaryGeneratorAction *primary = new PrimaryGeneratorAction(runAction);
  auto *primary = new PrimaryGeneratorGunAction(runAction);
  SetUserAction(primary);

  auto *trackingAction = new TrackingAction(event);
  SetUserAction(trackingAction);

  SteppingAction *steppingAction = new SteppingAction(event, fDetConstruction);
  SetUserAction(steppingAction);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
