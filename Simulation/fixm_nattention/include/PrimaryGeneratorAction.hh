
#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
//class EventAction;
class RunAction;
class G4ParticleGun;
class G4Event;
class G4GeneralParticleSource;

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
public:
	PrimaryGeneratorAction(RunAction*);
	~PrimaryGeneratorAction();

public:
	void GeneratePrimaries(G4Event* anEvent);
	RunAction* theRun;
private:
//    G4ParticleGun* particleGun;
	G4GeneralParticleSource*       particleGun;

};

#endif


