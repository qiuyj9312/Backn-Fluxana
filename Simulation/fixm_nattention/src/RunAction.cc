/*
 * @Descripttion: 
 * @version: 
 * @Author: Qiu Yijia
 * @Date: 2023-08-16 20:16:57
 * @LastEditors: Qiu Yijia
 * @LastEditTime: 2024-01-07 23:37:22
 */
#include "RunAction.hh"

#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4UnitsTable.hh"
#include "G4Track.hh"
#include "G4ParticleDefinition.hh"
#include "Analysis.hh"
#include <stdio.h>
#include <iostream>
#include <ctime>
#include <fstream>
#include "EventAction.hh"
#include "G4SystemOfUnits.hh"
#include <cmath> 
#include "DataManger.hh"
#include "DetectorConstruction.hh"
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

RunAction::RunAction(EventAction*  eventAction, DetectorConstruction *detConstruction):
fEventAction(eventAction),
fDetConstruction(detConstruction)
{
	//============     ROOT  ======================//	

	G4RunManager::GetRunManager()->SetPrintProgress(10000);
    
    // Create analysis manager
    // The choice of analysis technology is done via selectin of a namespace
    // in Analysis.hh
    auto analysisManager = G4AnalysisManager::Instance();
    G4cout << "Using " << analysisManager->GetType() << G4endl;
    
    analysisManager->SetVerboseLevel(0);
    analysisManager->SetNtupleMerging(true);

	// Creating histograms
	analysisManager->CreateH1("En","En ParticleGenerator", 10000000, -1., 10.);

    // Creating ntuple
	//
	analysisManager->CreateNtuple("tree", "Information");
	analysisManager->CreateNtupleDColumn("EnIn");
    analysisManager->CreateNtupleIColumn("vec_sampleTrackID",fEventAction->GetsampleTrackID());
    analysisManager->CreateNtupleIColumn("vec_sampleStepID",fEventAction->GetsampleStepID());
    analysisManager->CreateNtupleIColumn("vec_ChannelID",fEventAction->GetsampleChannelID());
    analysisManager->CreateNtupleDColumn("vec_sampleNEnergy",fEventAction->GetsampleNEnergy());
    analysisManager->CreateNtupleDColumn("vec_sampleNx",fEventAction->GetsampleNx());
    analysisManager->CreateNtupleDColumn("vec_sampleNy",fEventAction->GetsampleNy());


	analysisManager->FinishNtuple();

	//=============================================//

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

RunAction::~RunAction()
{
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::BeginOfRunAction(const G4Run* aRun)
{
	auto analysisManager = G4AnalysisManager::Instance();

	//============     setname  ======================//	
	G4int RunIndex = DataManger::GetInstance()->GetRun();
	G4String runumb = DataManger::GetInstance()->convert<G4String>(RunIndex);	
	G4String rootfileN = "run" + runumb  + ".root";
	G4String Dir = DataManger::GetInstance()->GetDir();
	G4String OutputFileName = "run" + runumb + ".txt";
	G4String fileName = Dir + "/" +rootfileN;
	G4String filetxtName = Dir + "/" +OutputFileName;
	
	fileoutput.open(filetxtName, std::ofstream::out | std::ofstream::app);	
	analysisManager->OpenFile(fileName);
	//=============================================//
	
  if (IsMaster())
	{
		time_t t = time(nullptr);
		char ch[64] = {0};
		strftime(ch, sizeof(ch) - 1, "%Y%m%d%H%M%S ", localtime(&t));
		G4String datatime{ch};
		fileoutput<< " StartTime: " << datatime <<std::endl;
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::EndOfRunAction(const G4Run* aRun)
{
	//=============================================//
  auto analysisManager = G4AnalysisManager::Instance();
  
	if ( analysisManager->GetH1(0) ) {
		if(IsMaster()) {
				DataManger::GetInstance()->AddRun();
		G4int bintot = analysisManager->GetH1(0)->get_bins();

		G4int binEmin{0};
		G4int binEmax{0};
		for (size_t i = 0; i < bintot; i++)
		{
			if (analysisManager->GetH1(0)->bin_height(i)>0)
			{
				binEmin = i;
				break;
			}
		}
		for (size_t i = bintot-1; i < bintot; i--)
		{
			if (analysisManager->GetH1(0)->bin_height(i)>0)
			{
				binEmax = i;
				break;
			}
		}

		auto minEn = pow(10,analysisManager->GetH1(0)->bin_center(binEmin));
		auto maxEn = pow(10,analysisManager->GetH1(0)->bin_center(binEmax));

		fileoutput << "NeutronEnergy: " << minEn << " eV"<< std::endl;		

		fileoutput << "Entries: " << aRun->GetNumberOfEvent() << std::endl;		
		
		time_t t = time(nullptr);
		char ch[64] = {0};
		strftime(ch, sizeof(ch) - 1, "%Y%m%d%H%M%S", localtime(&t));
		G4String datatime{ch};
		fileoutput<< "StoptTime: " << datatime <<std::endl; 
		}
	}
	//============     ROOT  ======================//	


  	analysisManager->Write();
	analysisManager->CloseFile();
	fileoutput.close();
	G4cout<<" process complete!"<<G4endl;
}