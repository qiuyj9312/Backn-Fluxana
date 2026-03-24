/*
 * @Descripttion: 
 * @version: 
 * @Author: Qiu Yijia
 * @Date: 2023-08-16 14:55:17
 * @LastEditors: Qiu Yijia
 * @LastEditTime: 2024-01-07 23:17:06
 */

#ifndef EventAction_h
#define EventAction_h 1

#include "G4UserEventAction.hh"
#include "globals.hh"
#include <vector>
class RunAction;
//class EventActionMessenger;
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class EventAction : public G4UserEventAction
{
 public:
   EventAction();
  ~EventAction();

 public:
   void  BeginOfEventAction(const G4Event*);
   void    EndOfEventAction(const G4Event*);
   
   //void  AddEnergyDepositDec1(G4double de) {EnergyDepositDec1 += de;};
  std::vector<G4double>& GetsampleNEnergy(){return vec_sampleNEnergy;};
  std::vector<G4int>& GetsampleStepID(){return vec_sampleStepID;};
  std::vector<G4int>& GetsampleTrackID(){return vec_sampleTrackID;};
  std::vector<G4int>& GetsampleChannelID(){return vec_channelID;};  
  std::vector<G4double>& GetsampleNx(){return vec_sampleNx;}; 
  std::vector<G4double>& GetsampleNy(){return vec_sampleNy;}; 
 
  void addsampleNEnergy(G4double val){vec_sampleNEnergy.emplace_back(val);};
  void addsampleStepID(G4int val){vec_sampleStepID.emplace_back(val);};
  void addsampleTrackID(G4int val){vec_sampleTrackID.emplace_back(val);};
  void addsampleChannelID(G4int val){vec_channelID.emplace_back(val);};  
  void addsampleNx(G4double val){vec_sampleNx.emplace_back(val);};  
  void addsampleNy(G4double val){vec_sampleNy.emplace_back(val);};    


 private:
   void paraInitialization(){
    vec_sampleNEnergy.clear();
    vec_sampleTrackID.clear();
    vec_sampleStepID.clear();
    vec_sampleNx.clear();
    vec_sampleNy.clear();
    vec_channelID.clear();
   }; 

 private:
    std::vector<G4double> vec_sampleNEnergy;   
    std::vector<G4int> vec_sampleTrackID;      
    std::vector<G4int> vec_sampleStepID;       
    std::vector<G4double> vec_sampleNx;        
    std::vector<G4double> vec_sampleNy;     
    std::vector<G4int> vec_channelID;        

    G4double En;

};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif

    
