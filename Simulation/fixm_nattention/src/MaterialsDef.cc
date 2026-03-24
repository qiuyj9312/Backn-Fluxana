/*
 * @Author: Huangchang
 * @Description: TODO
 * @Company: CSNS
 * @Date: 2019-10-15 18:54:27
 * @LastEditors: HuangChang
 * @LastEditTime: 2019-12-01 20:45:51
 * @Email: huangchann@gmail.com
 */
#include "MaterialsDef.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4OpticalSurface.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "globals.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
MaterialsDef *MaterialsDef::instance = nullptr;
MaterialsDef::MaterialsDef() {
  // Getting the NIST Material Manager
  nistMan = G4NistManager::Instance();
  nistMan->SetVerbose(0);
  CreateMaterials();
}

MaterialsDef::~MaterialsDef() {}

MaterialsDef *MaterialsDef::GetInstance() {
  if (instance == nullptr) {
    // static Materials material;
    // instance = &material;
    instance = new MaterialsDef();
  }
  return instance;
}

G4Material *MaterialsDef::GetMaterial(const G4String material) {
  // Trying to build the material
  G4Material *mat = nistMan->FindOrBuildMaterial(material);
  if (!mat)
    mat = G4Material::GetMaterial(material);
  // if (!mat) mat =FindMaterials(material);
  //  Exceptional error if it isn't found
  if (!mat) {
    std::ostringstream o;
    o << "Material " << material << " not found.";
    G4Exception("Materials::GetMaterial", "", FatalException, o.str().c_str());
  }
  return mat;
}

void MaterialsDef::CreateMaterials() {
  //=================elements and materias
  G4double density;
  G4double fractionmass;
  G4int ncomponents;
  G4double abundance;
  std::vector<G4int> natoms;
  std::vector<G4String> elements;
  //----------------------------------------------------------------------
  // Vacuum
  //----------------------------------------------------------------------
  auto fVacuum = new G4Material("Vacuum", 1., 1.01 * g / mole,
                                density = universe_mean_density, kStateGas,
                                0.1 * kelvin, 1.e-19 * pascal);

  //----------------------------------------------------------------------
  // AlAlloy5052
  //----------------------------------------------------------------------
  density = 2.50 * g / cm3;
  auto fAl_Alloy5052 = new G4Material("Al_Alloy5052", density, 8, kStateSolid);
  fAl_Alloy5052->AddMaterial(nistMan->FindOrBuildMaterial("G4_Si"),
                             0.25 * perCent);
  fAl_Alloy5052->AddMaterial(nistMan->FindOrBuildMaterial("G4_Fe"),
                             0.4 * perCent);
  fAl_Alloy5052->AddMaterial(nistMan->FindOrBuildMaterial("G4_Cu"),
                             0.1 * perCent);
  fAl_Alloy5052->AddMaterial(nistMan->FindOrBuildMaterial("G4_Mn"),
                             0.1 * perCent);
  fAl_Alloy5052->AddMaterial(nistMan->FindOrBuildMaterial("G4_Mg"),
                             2.8 * perCent);
  fAl_Alloy5052->AddMaterial(nistMan->FindOrBuildMaterial("G4_Cr"),
                             0.35 * perCent);
  fAl_Alloy5052->AddMaterial(nistMan->FindOrBuildMaterial("G4_Zn"),
                             0.1 * perCent);
  fAl_Alloy5052->AddMaterial(nistMan->FindOrBuildMaterial("G4_Al"),
                             95.9 * perCent);
  //----------------------------------------------------------------------
  // 238U materials & 235U materials
  //----------------------------------------------------------------------
  G4Isotope *U4 = new G4Isotope("U234", 92, 234, 234.0409 * g / mole);
  G4Isotope *U5 = new G4Isotope("U235", 92, 235, 235.0439 * g / mole);
  G4Isotope *U6 = new G4Isotope("U236", 92, 236, 236.04556 * g / mole);
  G4Isotope *U8 = new G4Isotope("U238", 92, 238, 238.0507 * g / mole);
  // 238U materials
  G4Element *U238 = new G4Element("High_purity_238U", "U238", ncomponents = 4);
  U238->AddIsotope(U4, abundance = 0.0000173 * perCent);
  U238->AddIsotope(U5, abundance = 0.002108 * perCent);
  U238->AddIsotope(U6, abundance = 0.0000027 * perCent);
  U238->AddIsotope(U8, abundance = 99.998 * perCent);
  density = 8.3 * g / cm3;
  auto fHP_U238 = new G4Material("HP_U238", density, ncomponents = 2);
  fHP_U238->AddElement(U238, 3);
  fHP_U238->AddElement(nistMan->FindOrBuildElement("O"), 8);
  // 235U materials
  G4Element *U235 = new G4Element("U235", "U235", ncomponents = 4);
  U235->AddIsotope(U4, abundance = 1.256e-3 * perCent);
  U235->AddIsotope(U5, abundance = 99.985 * perCent);
  U235->AddIsotope(U6, abundance = 4.1160e-3 * perCent);
  U235->AddIsotope(U8, abundance = 9.6260e-3 * perCent);
  density = 8.25 * g / cm3;
  auto fEr_U235 = new G4Material("Er_U235", density, ncomponents = 2);
  fEr_U235->AddElement(U235, 3);
  fEr_U235->AddElement(nistMan->FindOrBuildElement("O"), 8);

  // 贫铀 materials
  G4Element *Depleted_U = new G4Element("Depleted_U", "Depleted_U", ncomponents = 4);
  Depleted_U->AddIsotope(U4, abundance = 1.0 * perCent);
  Depleted_U->AddIsotope(U5, abundance = 90.0 * perCent);
  Depleted_U->AddIsotope(U6, abundance = 0.01 * perCent);
  Depleted_U->AddIsotope(U8, abundance = 8.99 * perCent);
  density = 8.25 * g / cm3;
  auto fDepleted_U = new G4Material("Depleted_U", density, ncomponents = 2);
  fDepleted_U->AddElement(Depleted_U, 3);
  fDepleted_U->AddElement(nistMan->FindOrBuildElement("O"), 8);
  //----------------------------------------------------------------------
  // 239Pu materials
  //----------------------------------------------------------------------
  G4Isotope *Pu9 = new G4Isotope("Pu9", 94, 239, 239.0521 * g / mole);
  G4Isotope *Pu0 = new G4Isotope("Pu0", 94, 240, 240.0538 * g / mole);

  G4Element *Pu239 = new G4Element("Pu239", "Pu239", ncomponents = 2);
  Pu239->AddIsotope(Pu9, abundance = 96.8552 * perCent);
  Pu239->AddIsotope(Pu0, abundance = 3.1448 * perCent);
  G4Element *Am241 = new G4Element("Am241", "Am241", 95, 241.05682 * g / mole);
  density = 19.816 * g / cm3;
  auto fHp_Pu239 = new G4Material("Hp_Pu239", density, ncomponents = 2);
  fHp_Pu239->AddElement(Pu239, 99.85 * perCent);
  fHp_Pu239->AddElement(Am241, 0.15 * perCent);

  //----------------------------------------------------------------------
  // 232Th materials
  //----------------------------------------------------------------------
  density = 9.86 * g / cm3;
  auto ThO2 = new G4Material("ThO2", density, ncomponents = 2);
  ThO2->AddElement(nistMan->FindOrBuildElement("Th"), 1);
  ThO2->AddElement(nistMan->FindOrBuildElement("O"), 2);

  //----------------------------------------------------------------------
  //  CF4
  //----------------------------------------------------------------------
  density = 0.0036625616 * g / cm3;
  auto CF4 = new G4Material("CF4", density, 2, kStateGas);
  CF4->AddElement(nistMan->FindOrBuildElement("C"), 1);
  CF4->AddElement(nistMan->FindOrBuildElement("F"), 4);
  //----------------------------------------------------------------------
  //  Ar
  //----------------------------------------------------------------------
  //----------------------------------------------------------------------
  //   ionized gas 10%CF4+90%Ar
  //----------------------------------------------------------------------
  density = 0.00144366931280571 * g / cm3;
  auto Ionized_gas = new G4Material("Ionized_gas", density, 2, kStateGas,
                                    300. * kelvin, 80375 * pascal);
  Ionized_gas->AddMaterial(nistMan->FindOrBuildMaterial("G4_Ar"), 0.9);
  Ionized_gas->AddMaterial(nistMan->FindOrBuildMaterial("CF4"), 0.1);
}
