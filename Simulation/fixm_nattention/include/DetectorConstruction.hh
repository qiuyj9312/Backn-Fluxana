/*
 * @Begin: *********************************
 * @Author: Liuchangqi
 * @Company: LZU
 * @Date: 2020-06-29 16:29:47
 * @LastEditTime: 2024-01-09 20:38:36
 * @Email: liuchq16@lzu.edu.cn
 * @Descripttion: 
 * @End:   *********************************
 */ 

#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"
#include "G4RotationMatrix.hh"
#include "MaterialsDef.hh"
#include <map>

class G4GenericMessenger;
class G4VPhysicalVolume;
class G4Material;
class G4Tubs;
class G4Box;
class DetectorConstruction : public G4VUserDetectorConstruction
{
  public:
    DetectorConstruction();
    ~DetectorConstruction();
      G4VPhysicalVolume* Construct();
    void SetSamplePosZ(G4double val);
    void SetSampleMaterial(G4String val);
    void SetSampleThickness(G4double val);
    G4GenericMessenger* fMessenger;

    std::map<G4String, G4int>& GetMSample(){return m_sample;};

  private:
    void DefineCommands();
	  G4Material *FindMaterial(G4String);
    G4bool  fCheckOverlaps; // option to activate checking of volumes overlaps
    //Materials
    MaterialsDef *materials;
    //world
    G4VPhysicalVolume *World_phys;
    std::map<G4String, G4int> m_sample;
};

#endif

