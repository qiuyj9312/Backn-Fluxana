/*
 * @Author: Huangchang
 * @Description: TODO
 * @Company: CSNS
 * @Date: 2019-10-15 12:58:33
 * @LastEditors: HuangChang
 * @LastEditTime: 2019-12-01 20:20:13
 * @Email: huangchann@gmail.com
 */
#ifndef Materialsdef_h
#define Materialsdef_h 1

#include "globals.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"


class MaterialsDef{
public:

    ~MaterialsDef();
    static MaterialsDef* GetInstance();
    G4Material* GetMaterial(const G4String);


private:
    // Methods

    MaterialsDef();
    void CreateMaterials();


    // Fields
    static MaterialsDef* instance;   /* Materials Instance         */
    G4NistManager* nistMan;  /* NIST Material Manager           */


};

#endif

