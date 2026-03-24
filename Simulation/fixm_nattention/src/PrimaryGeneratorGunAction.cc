/*
 * @Descripttion: 
 * @version: 
 * @Author: Qiu Yijia
 * @Date: 2023-09-15 10:49:47
 * @LastEditors: Qiu Yijia
 * @LastEditTime: 2023-09-18 13:05:08
 */
#include "PrimaryGeneratorGunAction.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4LogicalVolume.hh"
#include "G4Box.hh"
#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleGun.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include "globals.hh"
#include "math.h"
#include "G4RunManager.hh"

using namespace std;

PrimaryGeneratorGunAction::PrimaryGeneratorGunAction(RunAction* RunAction)
 : G4VUserPrimaryGeneratorAction(),
   fParticleGun(nullptr),
   frunaction(RunAction)
{
  G4int nofParticles = 1;
  fParticleGun = new G4ParticleGun(nofParticles);

  // default particle kinematic
  auto particleDefinition 
    = G4ParticleTable::GetParticleTable()->FindParticle("neutron");
  fParticleGun->SetParticleDefinition(particleDefinition);
  fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0., 0., -1.));

}

PrimaryGeneratorGunAction::~PrimaryGeneratorGunAction()
{
  delete fParticleGun;
}

void PrimaryGeneratorGunAction::GeneratePrimaries(G4Event* anEvent)
{
  // This function is called at the begining of event

  // In order to avoid dependence of PrimaryGeneratorAction
  // on DetectorConstruction class we get world volume 
  // from G4LogicalVolumeStore
  //

  // Set energy sample
    G4double random_E = -0.6+ 9.1 * G4UniformRand();
  //  G4double random_E =  2 * G4UniformRand();
    G4double energy = pow(10, random_E)*eV; //random * E
    fParticleGun->SetParticleEnergy(energy);    

    // Set gun position distribution
    G4double x0 = 0. *cm;
    G4double y0 = 0. *cm;
    G4double z0 = 40. *cm;

    G4double theta = CLHEP::twopi*G4UniformRand();
    G4double randomA = G4UniformRand();
    G4double sqr = sqrt(G4UniformRand()); 
    G4double a = 15. *mm;
    G4double b = 15. *mm;    

    x0 = a * sqr * cos(theta) *mm;  
    y0 = b * sqr * sin(theta) *mm; 

    //set
    fParticleGun->SetParticlePosition(G4ThreeVector(x0,y0,z0));
    fParticleGun->GeneratePrimaryVertex(anEvent);

}

