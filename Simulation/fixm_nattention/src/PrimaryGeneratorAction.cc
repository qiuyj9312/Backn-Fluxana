#include "PrimaryGeneratorAction.hh"

#include "G4Event.hh"

#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "globals.hh"

#include "G4GeneralParticleSource.hh"
#include "G4ParticleGun.hh"

#include "Randomize.hh"
#include "G4RandomDirection.hh"
#include "RunAction.hh"
#include <math.h>
//#include "EventAction.hh"
PrimaryGeneratorAction::PrimaryGeneratorAction(RunAction* tRun):theRun(tRun)
{
  //  G4int n_particle = 1;
  //  particleGun = new G4ParticleGun(n_particle);
  particleGun = new G4GeneralParticleSource ( );

}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
  delete particleGun;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
  
  particleGun->GeneratePrimaryVertex(anEvent);
}


