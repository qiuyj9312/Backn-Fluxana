/*
 * @Descripttion: 
 * @version: 
 * @Author: Qiu Yijia
 * @Date: 2023-08-16 17:54:31
 * @LastEditors: Qiu Yijia
 * @LastEditTime: 2024-01-09 21:21:44
 */

#include "SteppingAction.hh"
#include "DetectorConstruction.hh"
#include "EventAction.hh"
#include "RunAction.hh"
#include "Randomize.hh"
#include <G4VParticleChange.hh>
#include "TrackingAction.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTypes.hh"
#include "G4SteppingManager.hh"
#include "G4Track.hh"
#include "Analysis.hh"
#include "G4RunManager.hh"
#include "G4Step.hh"
#include "G4SystemOfUnits.hh"
#include <iostream>
#include <typeinfo>
#include <vector>
#include <cmath>
#include "math.h"
#include <fstream>
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "G4Box.hh"
using std::vector;
using namespace std; 

////#include "G4RunManager.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SteppingAction::SteppingAction(EventAction* evt, DetectorConstruction* detConstruction)
	:feventaction(evt),
	fDetConstruction(detConstruction)
{ 

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SteppingAction::~SteppingAction()
{ }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SteppingAction::UserSteppingAction(const G4Step* aStep)
{
	// get volume of the current step


	auto fpPreStepPoint = aStep ->GetPreStepPoint();//步的前点
	auto fpPostStepPoint = aStep ->GetPostStepPoint();//步的后点

	G4String Particle = aStep->GetTrack()->GetDefinition()->GetParticleName();
	G4int TrackID = aStep->GetTrack()->GetTrackID();
	G4int StepID = aStep -> GetTrack() ->GetCurrentStepNumber();
	if(aStep->GetTrack() ->GetNextVolume())//判断粒子是否在世界里
	{
		auto postVolume = fpPostStepPoint ->GetPhysicalVolume() ->GetName();//获取粒子所在体的名字并赋给volume，用于判断粒子位置
		auto preVolume = fpPreStepPoint ->GetPhysicalVolume() ->GetName();
		
		auto x = fpPostStepPoint->GetPosition().x()/mm;
		auto y = fpPostStepPoint->GetPosition().y()/mm;
		auto Energy_idea = fpPostStepPoint->GetKineticEnergy()/eV;

		if (Energy_idea>0.)
		{
			int ChannelID{-1};
			auto mSample = fDetConstruction->GetMSample();
    		auto iter = mSample.find(preVolume);			
			if (iter!=mSample.end())
			{
				ChannelID = iter->second;
			}
						
			if (ChannelID!=-1&&postVolume!=preVolume&&Particle=="neutron")
			{
				feventaction->addsampleTrackID(TrackID);
				feventaction->addsampleStepID(StepID);
				feventaction->addsampleNEnergy(Energy_idea);	
				feventaction->addsampleNx(x);		
				feventaction->addsampleNy(y);	
				feventaction->addsampleChannelID(ChannelID);
			}
		}
	}
}
