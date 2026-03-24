/*
 * @Descripttion: 
 * @version: 
 * @Author: Qiu Yijia
 * @Date: 2023-08-16 15:06:55
 * @LastEditors: Qiu Yijia
 * @LastEditTime: 2024-01-02 01:10:59
 */


#ifndef RunAction_h
#define RunAction_h 1

#include "G4UserRunAction.hh"
#include "globals.hh"
#include <fstream>


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class G4Run;
class G4Track;
class G4ParticleDefinition;
class EventAction;
class DetectorConstruction;

#include <vector>


class RunAction : public G4UserRunAction
{
public:
	RunAction(EventAction*, DetectorConstruction *);
	~RunAction();

public:
	void BeginOfRunAction(const G4Run*);
	void   EndOfRunAction(const G4Run*);
	//自定义成员函数

	G4double theParticleEnergy;
//   G4String  theParticleTypeOpen[50],theParticleTypeFast[50],theParticleTypeClosed[50],theParticleTypeAlbedo[50];

	std::vector<G4String> theParticleTypeDec1;
//能谱输出成员
	G4double MaxE,MinE;
	G4int    MaxChannel, nChannel;
	G4double ChannelStepLength;
	G4int    CountOfChannelDec1[1024];
	//G4int    CountOfGPSChannel[1024];

private:
	EventAction* fEventAction;
	DetectorConstruction* fDetConstruction;
//    G4double ResponseHpAlbedo, ResponseHpClosed, ResponseHpFast, ResponseHpOpen, DoseCoefficient, TotalHpResponse;
    std::fstream fileoutput;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif

