
#ifndef PrimaryGeneratorGunAction_h
#define PrimaryGeneratorGunAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "globals.hh"
#include "RunAction.hh"

class G4ParticleGun;

class G4Event;

class PrimaryGeneratorGunAction : public G4VUserPrimaryGeneratorAction
{
  public:
    PrimaryGeneratorGunAction(RunAction*);
    virtual ~PrimaryGeneratorGunAction();

  public:
      virtual void GeneratePrimaries(G4Event* event);

      const G4ParticleGun* GetParticleGun() const { return fParticleGun; }

  private:
      G4ParticleGun*  fParticleGun; // G4 particle gun
      RunAction* frunaction;

};

#endif


