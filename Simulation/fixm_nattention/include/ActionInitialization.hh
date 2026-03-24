/*
 * @Begin: *********************************
 * @Author: Liuchangqi
 * @Company: LZU
 * @Date: 2020-03-23 08:31:15
 * @LastEditTime: 2024-01-02 01:06:43
 * @Email: liuchq16@lzu.edu.cn
 * @Descripttion: 
 * @End:   *********************************
 */

#ifndef ActionInitialization_h
#define ActionInitialization_h 1

#include "G4VUserActionInitialization.hh"

class DetectorConstruction;

class ActionInitialization : public G4VUserActionInitialization
{
  public:
    ActionInitialization(DetectorConstruction* detConstruction);
    virtual ~ActionInitialization();

    virtual void BuildForMaster() const;
    virtual void Build() const;

  private:
    DetectorConstruction* fDetConstruction;    
};

#endif

    
