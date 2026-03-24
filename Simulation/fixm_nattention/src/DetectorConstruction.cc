/*
 * @Begin: *********************************
 * @Author: Liuchangqi
 * @Company: LZU
 * @Date: 2020-03-23 08:31:16
 * @LastEditTime: 2024-01-09 20:45:25
 * @Email: liuchq16@lzu.edu.cn
 * @Descripttion:
 * @End:   *********************************
 */

#include "DetectorConstruction.hh"
#include "G4Box.hh"
#include "G4Element.hh"
#include "G4ElementTable.hh"
#include "G4GenericMessenger.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4MaterialTable.hh"
#include "G4NistManager.hh"
#include "G4OpticalSurface.hh"
#include "G4PVPlacement.hh"
#include "G4PhysicalConstants.hh"
#include "G4Polycone.hh"
#include "G4RotationMatrix.hh"
#include "G4RunManager.hh"
#include "G4Sphere.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4Tubs.hh"
#include "G4UnitsTable.hh"
#include "G4UserLimits.hh"
#include "G4VisAttributes.hh"

using namespace CLHEP;

DetectorConstruction::DetectorConstruction()
    : G4VUserDetectorConstruction(), fCheckOverlaps(true), materials(nullptr),
      World_phys(nullptr) {
  materials = MaterialsDef::GetInstance();
}

DetectorConstruction::~DetectorConstruction() {
  // delete materials;
}

G4Material *DetectorConstruction::FindMaterial(G4String name) {
  G4Material *material = materials->GetMaterial(name);
  return material;
}

G4VPhysicalVolume *DetectorConstruction::Construct() {

  //
  // ------ Vis --------
  //
  G4VisAttributes *transgrey =
      new G4VisAttributes(G4Colour(0.64, 0.7, 0.7, 0.5));
  transgrey->SetForceSolid(true);
  G4VisAttributes *transblue =
      new G4VisAttributes(G4Colour(0.01, 0.98, 0.9, 0.3));
  transblue->SetForceSolid(true);
  G4VisAttributes *transyellow =
      new G4VisAttributes(G4Colour(0.8, 0.9, 0.1, 0.9));
  transyellow->SetForceSolid(true);
  G4VisAttributes *transgreen =
      new G4VisAttributes(G4Colour(0.2, 0.98, 0.1, 0.9));
  transgreen->SetForceSolid(true);
  G4VisAttributes *transred = new G4VisAttributes(G4Colour(1.0, 0.0, 0.0));
  transred->SetForceSolid(true);

  //====================================================
  //          Detector geometry 探测器几何构建
  //====================================================
  // Option to switch on/off checking of volumes overlaps

  //
  // ===== World =====
  //

  G4double World_SizeX = 1.0 * m;
  G4double World_SizeY = 1.0 * m;
  G4double World_SizeZ = 1.0 * m;

  auto World_box = new G4Box("world_box", World_SizeX / 2.0, World_SizeY / 2.0,
                             World_SizeZ / 2.0);
  auto World_log = new G4LogicalVolume(World_box, FindMaterial("G4_AIR"),
                                       "world", 0, 0, 0); // Vacuum,G4_AIR
  World_log->SetVisAttributes(G4VisAttributes::GetInvisible());
  World_phys = new G4PVPlacement(0, G4ThreeVector(), World_log, "world", 0,
                                 false, 0, fCheckOverlaps);

  //--------------------------------------------------------------
  //              detector
  //--------------------------------------------------------------
  //--------------------------------cylindrical Al_Alloy
  // shell----------------------

  G4double innerRadiusOfTheTubeShell = 290. / 2 * mm;
  G4double outerRadiusOfTheTubeShell = 300. / 2 * mm;
  G4double hightOfTheTubeShell = 298. * mm;
  G4double startAngleOfTheTubeShell = 0. * deg;
  G4double spanningAngleOfTheTubeShell = 360. * deg;

  auto tubShell =
      new G4Tubs("tub_Shell", innerRadiusOfTheTubeShell,
                 outerRadiusOfTheTubeShell, hightOfTheTubeShell / 2.,
                 startAngleOfTheTubeShell, spanningAngleOfTheTubeShell);
  auto log_tubShell = new G4LogicalVolume(
      tubShell, FindMaterial("Al_Alloy5052"), "log_tubShell");

  G4double tubShell_Pos_x = 0. * mm;
  G4double tubShell_Pos_y = 0. * mm;
  G4double tubShell_Pos_z = 0. * mm;

  new G4PVPlacement(
      0, G4ThreeVector(tubShell_Pos_x, tubShell_Pos_y, tubShell_Pos_z),
      log_tubShell, "phys_tubShell", World_log, false, 0, fCheckOverlaps);

  //--------------------------------window shell----------------------

  G4double innerRadiusOfwindowShell = 80. / 2 * mm;
  G4double outerRadiusOfwindowShell = outerRadiusOfTheTubeShell * mm;
  G4double hightOfwindowShell = 5. * mm;
  G4double startAngleOfThewindowShell = 0. * deg;
  G4double spanningAngleOfwindowShell = 360. * deg;

  auto tub_windowShell =
      new G4Tubs("tub_windowShell", innerRadiusOfwindowShell,
                 outerRadiusOfwindowShell, hightOfwindowShell / 2.,
                 startAngleOfThewindowShell, spanningAngleOfwindowShell);
  auto log_tub_windowShell = new G4LogicalVolume(
      tub_windowShell, FindMaterial("Al_Alloy5052"), "log_tub_windowShell");

  G4double tub_windowShell_Pos_x = 0. * mm;
  G4double tub_windowShell_Pos_y = 0. * mm;
  G4double tub_windowShell_Pos_z =
      hightOfTheTubeShell / 2. + hightOfwindowShell / 2.;

  new G4PVPlacement(0,
                    G4ThreeVector(tub_windowShell_Pos_x, tub_windowShell_Pos_y,
                                  tub_windowShell_Pos_z),
                    log_tub_windowShell, "phys_tub_windowShell", World_log,
                    false, 0, fCheckOverlaps);
  new G4PVPlacement(0,
                    G4ThreeVector(tub_windowShell_Pos_x, tub_windowShell_Pos_y,
                                  -tub_windowShell_Pos_z),
                    log_tub_windowShell, "phys_tub_windowShell", World_log,
                    false, 1, fCheckOverlaps);

  //--------------------------------window_KAPTON----------------------

  G4double innerRadiusOfwindow_KAPTON = 0.;
  G4double outerRadiusOfwindow_KAPTON = innerRadiusOfwindowShell;
  G4double hightOfwindow_KAPTON = 100. * um;
  G4double startAngleOfThewindow_KAPTON = 0. * deg;
  G4double spanningAngleOfwindow_KAPTON = 360. * deg;

  auto tub_window_KAPTON =
      new G4Tubs("tub_window_KAPTON", innerRadiusOfwindow_KAPTON,
                 outerRadiusOfwindow_KAPTON, hightOfwindow_KAPTON / 2.,
                 startAngleOfThewindow_KAPTON, spanningAngleOfwindow_KAPTON);
  auto log_tub_window_KAPTON = new G4LogicalVolume(
      tub_window_KAPTON, FindMaterial("G4_KAPTON"), "log_tub_window_KAPTON");

  G4double tub_window_KAPTON_Pos_x = tub_windowShell_Pos_x;
  G4double tub_window_KAPTON_Pos_y = tub_windowShell_Pos_y;
  G4double tub_window_KAPTON_Pos_z =
      hightOfTheTubeShell / 2. + hightOfwindow_KAPTON / 2.;

  new G4PVPlacement(0,
                    G4ThreeVector(tub_window_KAPTON_Pos_x,
                                  tub_window_KAPTON_Pos_y,
                                  tub_window_KAPTON_Pos_z),
                    log_tub_window_KAPTON, "phys_tub_window_KAPTON", World_log,
                    false, 0, fCheckOverlaps);
  new G4PVPlacement(0,
                    G4ThreeVector(tub_window_KAPTON_Pos_x,
                                  tub_window_KAPTON_Pos_y,
                                  -tub_window_KAPTON_Pos_z),
                    log_tub_window_KAPTON, "phys_tub_window_KAPTON", World_log,
                    false, 1, fCheckOverlaps);

  //--------------------------------window_Al----------------------

  G4double innerRadiusOfwindow_Al = 0.;
  G4double outerRadiusOfwindow_Al = innerRadiusOfwindowShell;
  G4double hightOfwindow_Al = 20. * um;
  G4double startAngleOfThewindow_Al = 0. * deg;
  G4double spanningAngleOfwindow_Al = 360. * deg;

  auto tub_window_Al =
      new G4Tubs("tub_window_Al", innerRadiusOfwindow_Al,
                 outerRadiusOfwindow_Al, hightOfwindow_Al / 2.,
                 startAngleOfThewindow_Al, spanningAngleOfwindow_Al);
  auto log_tub_window_Al = new G4LogicalVolume(
      tub_window_Al, FindMaterial("G4_Al"), "log_tub_window_Al");

  G4double tub_window_Al_Pos_x = tub_windowShell_Pos_x;
  G4double tub_window_Al_Pos_y = tub_windowShell_Pos_y;
  G4double tub_window_Al_Pos_z =
      hightOfTheTubeShell / 2. + hightOfwindow_Al + hightOfwindow_Al / 2.;

  new G4PVPlacement(0,
                    G4ThreeVector(tub_window_Al_Pos_x, tub_window_Al_Pos_y,
                                  tub_window_Al_Pos_z),
                    log_tub_window_Al, "phys_tub_window_Al", World_log, false,
                    0, fCheckOverlaps);
  new G4PVPlacement(0,
                    G4ThreeVector(tub_window_Al_Pos_x, tub_window_Al_Pos_y,
                                  -tub_window_Al_Pos_z),
                    log_tub_window_Al, "phys_tub_window_Al", World_log, false,
                    1, fCheckOverlaps);

  //--------------------------------Chamber----------------------

  G4double innerRadiusOfTheTubeChamber = 0. * mm;
  G4double outerRadiusOfTheTubeChamber = innerRadiusOfTheTubeShell;
  G4double hightOfTheTubeChamber = hightOfTheTubeShell;
  G4double startAngleOfTheTubeChamber = 0. * deg;
  G4double spanningAngleOfTheTubeChamber = 360. * deg;

  auto tubChamber =
      new G4Tubs("tubChamber", innerRadiusOfTheTubeChamber,
                 outerRadiusOfTheTubeChamber, hightOfTheTubeChamber / 2.,
                 startAngleOfTheTubeChamber, spanningAngleOfTheTubeChamber);
  auto log_tubChamber = new G4LogicalVolume(
      tubChamber, FindMaterial("Ionized_gas"), "log_tubChamber");

  G4double tubChamber_Pos_x = tubShell_Pos_x;
  G4double tubChamber_Pos_y = tubShell_Pos_y;
  G4double tubChamber_Pos_z = tubShell_Pos_z;

  new G4PVPlacement(
      0, G4ThreeVector(tubChamber_Pos_x, tubChamber_Pos_y, tubChamber_Pos_z),
      log_tubChamber, "phys_tubChamber", World_log, false, 0, fCheckOverlaps);

  //--------------------------------Cu clamp ring----------------------

  G4double innerRadiusOfTheTubeClamp_Ring = 80. / 2 * mm;
  G4double outerRadiusOfTheTubeClamp_Ring = 110. / 2 * mm;
  ;
  G4double hightOfTheTubeClamp_Ring = 1.5 * mm;
  ;
  G4double startAngleOfTheTubeClamp_Ring = startAngleOfTheTubeChamber;
  G4double spanningAngleOfTheTubeClamp_Ring = spanningAngleOfTheTubeChamber;

  auto tubClamp_Ring = new G4Tubs(
      "tubClamp_Ring", innerRadiusOfTheTubeClamp_Ring,
      outerRadiusOfTheTubeClamp_Ring, hightOfTheTubeClamp_Ring / 2.,
      startAngleOfTheTubeClamp_Ring, spanningAngleOfTheTubeClamp_Ring);
  auto log_tubClamp_Ring = new G4LogicalVolume(
      tubClamp_Ring, FindMaterial("G4_Cu"), "log_tubClamp_Ring");

  //--------------------------------Teflon insulation ring----------------------

  G4double innerRadiusOfTheTubeInsulation_Ring = innerRadiusOfTheTubeClamp_Ring;
  G4double outerRadiusOfTheTubeInsulation_Ring = outerRadiusOfTheTubeClamp_Ring;
  G4double hightOfTheTubeInsulation_Ring = 5. * mm;
  G4double startAngleOfTheTubeInsulation_Ring = startAngleOfTheTubeChamber;
  G4double spanningAngleOfTheTubeInsulation_Ring =
      spanningAngleOfTheTubeChamber;

  auto tub_Insulation_Ring = new G4Tubs(
      "tub_Insulation_Ring", innerRadiusOfTheTubeInsulation_Ring,
      outerRadiusOfTheTubeInsulation_Ring, hightOfTheTubeInsulation_Ring / 2.,
      startAngleOfTheTubeInsulation_Ring,
      spanningAngleOfTheTubeInsulation_Ring);
  auto log_tubInsulation_Ring = new G4LogicalVolume(
      tub_Insulation_Ring, FindMaterial("G4_TEFLON"), "log_tubInsulation_Ring");

  //--------------------------------Teflon Support_Ring----------------------

  G4double innerRadiusOfTheTubeSupport_Ring = 1. * mm;
  G4double outerRadiusOfTheTubeSupport_Ring = 3. * mm;
  G4double hightOfTheTubeSupport_Ring = 10 * mm;
  G4double startAngleOfTheTubeSupport_Ring = startAngleOfTheTubeChamber;
  G4double spanningAngleOfTheTubeSupport_Ring = spanningAngleOfTheTubeChamber;

  auto tub_Support_Ring = new G4Tubs(
      "tub_Support_Ring", innerRadiusOfTheTubeSupport_Ring,
      outerRadiusOfTheTubeSupport_Ring, hightOfTheTubeSupport_Ring / 2.,
      startAngleOfTheTubeSupport_Ring, spanningAngleOfTheTubeSupport_Ring);
  auto log_tubSupport_Ring = new G4LogicalVolume(
      tub_Support_Ring, FindMaterial("G4_TEFLON"), "log_tubSupport_Ring");

  G4double initialdistance = 3 * cm + hightOfTheTubeSupport_Ring / 2.;

  G4double tubClamp_Ring_Pos_x = tubChamber_Pos_x;
  G4double tubClamp_Ring_Pos_y = tubChamber_Pos_y;
  G4double tubClamp_Ring_Pos_z = hightOfTheTubeChamber / 2. -
                                 hightOfTheTubeClamp_Ring / 2. -
                                 initialdistance;

  G4double tubInsulation_Ring_Pos_x = tubChamber_Pos_x;
  G4double tubInsulation_Ring_Pos_y = tubChamber_Pos_y;
  G4double tubInsulation_Ring_Pos_z =
      hightOfTheTubeChamber / 2. - hightOfTheTubeClamp_Ring -
      hightOfTheTubeInsulation_Ring / 2. - initialdistance;

  G4double tubSupport_Ring_Pos_R = 95 / 2. * mm;
  G4double tubSupport_Ring_Pos_theta = 0 * degree;
  G4double tubSupport_Ring_Pos_z =
      hightOfTheTubeChamber / 2. - hightOfTheTubeClamp_Ring * 2 -
      hightOfTheTubeInsulation_Ring - hightOfTheTubeSupport_Ring / 2. -
      initialdistance;

  //--------------------------------Placement---------------------//
  // distance between cells
  G4double distancebetweencells = hightOfTheTubeSupport_Ring +
                                  2 * hightOfTheTubeClamp_Ring +
                                  hightOfTheTubeInsulation_Ring;

  for (size_t i = 0; i < 8; i++) {
    new G4PVPlacement(
        0,
        G4ThreeVector(tubClamp_Ring_Pos_x, tubClamp_Ring_Pos_y,
                      tubClamp_Ring_Pos_z - distancebetweencells * i),
        log_tubClamp_Ring, "phys_tubClamp_Ring", log_tubChamber, false, 2 * i,
        fCheckOverlaps);

    new G4PVPlacement(
        0,
        G4ThreeVector(tubClamp_Ring_Pos_x, tubClamp_Ring_Pos_y,
                      tubClamp_Ring_Pos_z - hightOfTheTubeInsulation_Ring -
                          hightOfTheTubeClamp_Ring - distancebetweencells * i),
        log_tubClamp_Ring, "phys_tubClamp_Ring", log_tubChamber, false,
        2 * i + 1, fCheckOverlaps);

    new G4PVPlacement(
        0,
        G4ThreeVector(tubInsulation_Ring_Pos_x, tubInsulation_Ring_Pos_y,
                      tubInsulation_Ring_Pos_z - distancebetweencells * i),
        log_tubInsulation_Ring, "phys_tubInsulation_Ring", log_tubChamber,
        false, i, fCheckOverlaps);

    for (size_t j = 0; j < 3; j++) {
      new G4PVPlacement(
          0,
          G4ThreeVector(tubSupport_Ring_Pos_R *
                            cos(tubSupport_Ring_Pos_theta + j * 120 * degree),
                        tubSupport_Ring_Pos_R *
                            sin(tubSupport_Ring_Pos_theta + j * 120 * degree),
                        tubSupport_Ring_Pos_z - distancebetweencells * i),
          log_tubSupport_Ring, "phys_tubSupport_Ring ", log_tubChamber, false,
          j + i * 3, fCheckOverlaps);
    }
  }

  //--------------------------------Anode----------------------

  G4double innerRadiusOfTheTubeAnode = 0. * mm;
  G4double outerRadiusOfTheTubeAnode = innerRadiusOfTheTubeClamp_Ring;
  G4double hightOfTheTubeAnode = 100. * um;
  G4double startAngleOfTheTubeAnode = startAngleOfTheTubeInsulation_Ring;
  G4double spanningAngleOfTheTubeAnode = spanningAngleOfTheTubeInsulation_Ring;

  auto tub_Anode =
      new G4Tubs("tub_Anode", innerRadiusOfTheTubeAnode,
                 outerRadiusOfTheTubeAnode, hightOfTheTubeAnode / 2.,
                 startAngleOfTheTubeAnode, spanningAngleOfTheTubeAnode);
  auto log_tubAnode =
      new G4LogicalVolume(tub_Anode, FindMaterial("G4_Al"), "log_tubAnode");

  G4double tubAnode_Pos_x = tubChamber_Pos_x;
  G4double tubAnode_Pos_y = tubChamber_Pos_y;
  G4double tubAnode_Pos_z = hightOfTheTubeChamber / 2. -
                            hightOfTheTubeClamp_Ring -
                            hightOfTheTubeInsulation_Ring - initialdistance;

  for (size_t i = 0; i < 8; i++) {
    new G4PVPlacement(0,
                      G4ThreeVector(tubAnode_Pos_x, tubAnode_Pos_y,
                                    tubAnode_Pos_z - distancebetweencells * i),
                      log_tubAnode, "phys_tubAnode", log_tubChamber, false, i,
                      fCheckOverlaps);
  }
  //--------------------------------------------------------------
  //              cathode  && sample
  //--------------------------------------------------------------
  G4double a_Hight_tub_cathode[8]{100. * um, 100. * um, 100. * um, 100. * um,
                                  100. * um, 100. * um, 100. * um, 100. * um};
  G4String a_Material_cathode[8]{"G4_Al", "G4_Al", "G4_Al", "G4_Al",
                                 "G4_Al", "G4_Al", "G4_Al", "G4_Al"};
  G4double a_Hight_tub_sample[8]{
      853.96 * nm, 853.96 * nm, 422.0511471 * nm, 465.3535506 * nm,
      206.3 * nm,  190.3 * nm,  200.1 * nm,       225.5 * nm};
  G4double a_Radius_tub_sample[8]{24.87 * mm, 24.88 * mm, 24.88 * mm,
                                  24.88 * mm, 24.88 * mm, 24.88 * mm,
                                  24.88 * mm, 24.88 * mm};
  G4String a_Material_sample[8]{"Depleted_U", "Depleted_U", "Er_U235",
                                "Er_U235",    "ThO2",       "ThO2",
                                "ThO2",       "ThO2"};

  for (int i = 0; i < 8; i++) {
    //--------------------------------------------------------------
    //              cathode
    //--------------------------------------------------------------
    G4double innerRadius_tub_cathode = 0. * mm;
    G4double outerRadius_tub_cathode = outerRadiusOfTheTubeAnode;
    G4double Hight_tub_cathode = a_Hight_tub_cathode[i];
    G4double startAngle_tub_cathode = 0. * degree;
    G4double spanningAngle_tub_cathode = 360. * degree;
    G4String GeoVName = "tub_cathode_" + std::to_string(i);
    auto tub_cathode =
        new G4Tubs(GeoVName, innerRadius_tub_cathode, outerRadius_tub_cathode,
                   Hight_tub_cathode / 2., startAngle_tub_cathode,
                   spanningAngle_tub_cathode);
    G4String LogVName = "log_cathode_" + std::to_string(i);
    G4String MaterialCName_cathode = a_Material_cathode[i];
    auto log_cathode = new G4LogicalVolume(
        tub_cathode, FindMaterial(MaterialCName_cathode), LogVName);
    G4double Pos_x_cathode = tubChamber_Pos_x;
    G4double Pos_y_cathode = tubChamber_Pos_y;
    G4double Pos_z_cathode = hightOfTheTubeChamber / 2. -
                             hightOfTheTubeClamp_Ring - initialdistance -
                             distancebetweencells * i;
    G4String PhyVName = "phys_cathode_" + std::to_string(i);
    new G4PVPlacement(
        0, G4ThreeVector(Pos_x_cathode, Pos_y_cathode, Pos_z_cathode),
        log_cathode, PhyVName, log_tubChamber, false, 0, fCheckOverlaps);
    //--------------------------------------------------------------
    //              sample
    //--------------------------------------------------------------
    G4String MaterialCName_sample = a_Material_sample[i];
    if (MaterialCName_cathode == "") {
      continue;
    }

    G4double innerRadius_tub_sample = 0. * mm;
    G4double outerRadius_tub_sample = a_Radius_tub_sample[i];
    G4double Hight_tub_sample = a_Hight_tub_sample[i];
    G4double startAngle_tub_sample = 0. * degree;
    G4double spanningAngle_tub_sample = 360. * degree;
    GeoVName = "tub_sample_" + std::to_string(i);
    LogVName = "log_sample_" + std::to_string(i);
    PhyVName = "phys_sample_" + std::to_string(i);
    m_sample.emplace(PhyVName, i);
    auto tub_sample = new G4Tubs(
        GeoVName, innerRadius_tub_sample, outerRadius_tub_sample,
        Hight_tub_sample / 2., startAngle_tub_sample, spanningAngle_tub_sample);
    auto log_sample = new G4LogicalVolume(
        tub_sample, FindMaterial(MaterialCName_sample), LogVName);

    G4double Pos_x_sample = tubChamber_Pos_x;
    G4double Pos_y_sample = tubChamber_Pos_y;
    G4double Pos_z_sample =
        Pos_z_cathode - Hight_tub_cathode / 2. - Hight_tub_sample / 2;

    new G4PVPlacement(
        0, G4ThreeVector(Pos_x_sample, Pos_y_sample, Pos_z_sample), log_sample,
        PhyVName, log_tubChamber, false, 0, fCheckOverlaps);
  }

  //--------------------------------------------------------------
  //              Ti windows
  //--------------------------------------------------------------
  /*     G4double innerRadiusOfTheTiWindows = 0. *mm;
      G4double outerRadiusOfTheTiWindows = innerRadiusOfTheTubeShell;
      G4double hightOfTheTiWindowr = 100*um;
      G4double startAngleOfTheTiWindow = 0. * deg;
      G4double spanningAngleOfTheTiWindow = 360. * deg;

      auto tub_TiWindows = new G4Tubs("tub_TiWindows",
                                 innerRadiusOfTheTiWindows,
                                 outerRadiusOfTheTiWindows,
                                 hightOfTheTiWindowr/2.,
                                 startAngleOfTheTiWindow,
                                 spanningAngleOfTheTiWindow);
      auto log_TiWindows = new G4LogicalVolume(tub_TiWindows,
     FindMaterial("G4_Ti"), "log_TiWindows"); G4double tubTiWindows_Pos_x =
     tubChamber_Pos_x; G4double tubTiWindows_Pos_y = tubChamber_Pos_y; G4double
     tubTiWindows_Pos_z = hightOfTheTubeChamber/2. + 20 *cm; new
     G4PVPlacement(0, G4ThreeVector(tubTiWindows_Pos_x, tubTiWindows_Pos_y,
     tubTiWindows_Pos_z), log_TiWindows, "phys_TiWindows", World_log, false, 0,
                        fCheckOverlaps); */

  //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
  //
  // Always return the physical World
  //

  return World_phys;
}
