/////////////////////////////////////////////////////////
//                                                     //
//  Oct/2013  E. Nacher  -->  main.cc                  //
//  Practical work for the SWG2013 Workshop            //
//                                                     //
/////////////////////////////////////////////////////////

#include "DetectorConstruction.hh"
#include "ActionInitialization.hh"

#include "G4EmStandardPhysics_option4.hh"
#include "G4DecayPhysics.hh"
#include "G4RadioactiveDecayPhysics.hh"
#include "QGSP_BERT_HP.hh"
#include "QGSP_BIC_AllHP.hh"
#include "G4PhysListFactory.hh"
#include "FTFP_BERT_HP.hh"

#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"

#include "Randomize.hh"
#include "globals.hh"

#include "G4ios.hh"
#include "fstream"
#include "iomanip"
using namespace std;	 	

#include "G4RunManagerFactory.hh"

int main(int argc, char** argv)
{

  CLHEP::HepRandom::setTheEngine(new CLHEP::RanecuEngine());
 //随机数种子
  G4long seed = time(NULL);
  CLHEP::HepRandom::setTheSeed(seed);


  // Detect interactive mode (if no arguments) and define UI session
  //
  G4UIExecutive* ui = nullptr;
  if ( argc == 1 ) {
    ui = new G4UIExecutive(argc, argv);       //输入参数，则argc=2，则ui=0
  }

  auto* runManager = G4RunManagerFactory::CreateRunManager(
      ui ? G4RunManagerType::Serial : G4RunManagerType::Default);

  // set mandatory user classes
  //

  
  auto* physical = new FTFP_BERT_HP;
  runManager->SetUserInitialization(physical);

  DetectorConstruction* detector = new DetectorConstruction;
  runManager->SetUserInitialization(detector);

  runManager->SetUserInitialization(new ActionInitialization(detector)); 

  // Initialize visualization
  //
  G4VisManager* visManager = new G4VisExecutive;
  visManager->Initialize();

  // Get the pointer to the User Interface manager
  G4UImanager* UImanager = G4UImanager::GetUIpointer();

  // Process macro or start UI session
  //
  if ( ! ui ) { 
    // batch mode 批处理
    G4String command = "/control/execute ";
    G4String fileName = argv[1];                   //argv[1]指向在命令行中执行程序名后的第一个字符串 
    UImanager->ApplyCommand(command+fileName);
  }
  else { 
    // interactive mode 交互界面
    UImanager->ApplyCommand("/control/execute init_vis.mac");
    ui->SessionStart();
    delete ui;
  }

  // Job termination
  // Free the store: user actions, physics_list and detector_description are
  // owned and deleted by the run manager, so they should not be deleted 
  // in the main() program !
  
  delete visManager;
  delete runManager;
  return 0;
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo.....
