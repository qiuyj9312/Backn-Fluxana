/*
 * @Descripttion: 
 * @version: 
 * @Author: Qiu Yijia
 * @Date: 2023-08-15 15:51:04
 * @LastEditors: Qiu Yijia
 * @LastEditTime: 2024-01-07 23:39:21
 */
#include "EventAction.hh"

#include "RunAction.hh"

#include "G4Event.hh"
#include "G4TrajectoryContainer.hh"
#include "G4VTrajectory.hh"
#include "G4VVisManager.hh"
#include "G4UnitsTable.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "Analysis.hh"
#include "Randomize.hh"
#include <iomanip>
#include <math.h>
#include <string>
using namespace std;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
EventAction::EventAction()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
EventAction::~EventAction()
{}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void EventAction::BeginOfEventAction(const G4Event* evt)
{  
  paraInitialization();
  En=evt->GetPrimaryVertex()->GetPrimary()->GetKineticEnergy() /eV;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void EventAction::EndOfEventAction(const G4Event* evt)
{
  auto analysisManager = G4AnalysisManager::Instance();
  //auto evtNb = evt->GetEventID();
  analysisManager->FillNtupleDColumn(0, En);//0
  analysisManager->FillH1(0,log10(En));    
  analysisManager->AddNtupleRow();    
}  

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
