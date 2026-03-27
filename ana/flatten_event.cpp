/*****************************************************
 * File: flatten_event.cxx
 * Description: Convert WNSEvent to flattened RDataFrame Tree
 * Author: Kilo Code
 * Created: 2025/01/20
 ****************************************************/

#include "TBranch.h"
#include "TFile.h"
#include "TSystem.h"
#include "TTree.h"
#include "WNSEvent.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using json = nlohmann::json;

std::string ReadTreeNameFromJSON(const std::string &configPath) {
  std::ifstream file(configPath);
  if (!file.is_open()) {
    std::cerr << "Warning: Cannot open JSON file: " << configPath
              << ", using default tree name" << std::endl;
    return "EventBranch";
  }

  try {
    json configJson;
    file >> configJson;

    if (configJson.contains("Files") &&
        configJson["Files"].contains("RawTreeName")) {
      return configJson["Files"]["RawTreeName"].get<std::string>();
    }
  } catch (const json::parse_error &e) {
    std::cerr << "Warning: JSON parse error in " << configPath
              << ", using default tree name: " << e.what() << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "Warning: Failed to read JSON file " << configPath
              << ", using default tree name: " << e.what() << std::endl;
  }

  return "EventBranch";
}

void ConvertToFlatTree(const char *inputFile, const char *outputFile,
                       const char *treeName = "WNSRawTree") {
  std::cout << "========================================" << std::endl;
  std::cout << "Converting WNSEvent to Flat Tree" << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << "Input file:  " << inputFile << std::endl;
  std::cout << "Output file: " << outputFile << std::endl;
  std::cout << "Tree name:   " << treeName << std::endl;
  std::cout << std::endl;

  // 打开输入文件
  TFile *inFile = TFile::Open(inputFile, "READ");
  if (!inFile || inFile->IsZombie()) {
    std::cerr << "Error: Cannot open input file: " << inputFile << std::endl;
    return;
  }

  // 获取输入 Tree
  TTree *inTree = (TTree *)inFile->Get(treeName);
  if (!inTree) {
    std::cerr << "Error: Cannot find '" << treeName << "' in input file"
              << std::endl;
    inFile->Close();
    return;
  }

  // 读取原始 WNSEvent
  WNSEvent *event{};
  inTree->SetBranchAddress("EventBranch", &event);

  Long64_t nEntries = inTree->GetEntries();
  std::cout << "Total entries: " << nEntries << std::endl;
  std::cout << std::endl;

  // 创建输出文件和 Tree
  TFile *outFile = new TFile(outputFile, "RECREATE");
  TTree *outTree = new TTree("tree", "Flattened WNS Event Tree");

  // 定义扁平化变量（通道级完全扁平化）
  Int_t fEventNumber;
  ULong64_t fT0Sec;
  ULong64_t fT0NanoSec;
  Int_t fArrayLength;
  Int_t fChannelID;
  Double_t fTc0;
  Double_t fTc1;
  Double_t fTc2;
  Double_t fhpn;

  // 创建分支
  outTree->Branch("fEventNumber", &fEventNumber, "fEventNumber/I");
  outTree->Branch("fT0Sec", &fT0Sec, "fT0Sec/l");
  outTree->Branch("fT0NanoSec", &fT0NanoSec, "fT0NanoSec/l");
  outTree->Branch("fArrayLength", &fArrayLength, "fArrayLength/I");
  outTree->Branch("fChannelID", &fChannelID, "fChannelID/I");
  outTree->Branch("fTc0", &fTc0, "fTc0/D");
  outTree->Branch("fTc1", &fTc1, "fTc1/D");
  outTree->Branch("fTc2", &fTc2, "fTc2/D");
  outTree->Branch("fhpn", &fhpn, "fhpn/D");

  // 循环读取并转换
  std::cout << "Processing entries..." << std::endl;
  Long64_t nout = (Long64_t)nEntries * 0.05;
  for (Long64_t i = 0; i < nEntries; i++) {
    if (i % nout == 0) {
      std::cout << "  Progress: " << i << " / " << nEntries << " ("
                << (100.0 * i / nEntries) << "%)" << std::endl;
    }

    inTree->GetEntry(i);

    // 提取 Header 信息
    fEventNumber = event->GetHeader()->GetEventNumber();
    fT0Sec = event->GetHeader()->GetT0Sec();
    fT0NanoSec = event->GetHeader()->GetT0NanoSec();

    // 提取数据
    fArrayLength = event->GetArrayLength();
    const std::vector<Int_t> &vChannelID = event->GetChannelID();
    const std::vector<Double_t> &vT0 = event->GetvT0();
    const std::vector<Double_t> &vTc1 = event->GetvTc1();
    const std::vector<Double_t> &vTc2 = event->GetvTc2();
    const std::vector<Double_t> &vhpn = event->Getvhpn();

    // 通道级完全扁平化：每个通道一行
    for (Int_t j = 0; j < fArrayLength; j++) {
      fChannelID = vChannelID[j];
      fTc0 = vT0[j];
      fTc1 = vTc1[j];
      fTc2 = vTc2[j];
      fhpn = vhpn[j];
      outTree->Fill();
    }
  }

  std::cout << "  Progress: " << nEntries << " / " << nEntries << " (100.0%)"
            << std::endl;
  std::cout << std::endl;

  // 写入文件
  std::cout << "Writing output file..." << std::endl;
  outFile->Write();
  outFile->Close();
  inFile->Close();

  std::cout << "Conversion completed successfully!" << std::endl;
  std::cout << "Output file: " << outputFile << std::endl;
  std::cout << std::endl;

  // 显示输出 Tree 的结构
  std::cout << "========================================" << std::endl;
  std::cout << "Output Tree Structure" << std::endl;
  std::cout << "========================================" << std::endl;
  TFile *checkFile = TFile::Open(outputFile, "READ");
  TTree *checkTree = (TTree *)checkFile->Get("tree");
  if (checkTree) {
    checkTree->Print();
    std::cout << std::endl;
    std::cout << "Total entries: " << checkTree->GetEntries() << std::endl;
  }
  checkFile->Close();
}

int main(int argc, char **argv) {
  std::cout << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << "  WNSEvent Flattening Tool" << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << std::endl;

  if (argc < 3) {
    std::cout << "Usage: " << argv[0]
              << " <input_file.root> <output_file.root> [tree_name] "
                 "[config_file.json]"
              << std::endl;
    std::cout << std::endl;
    std::cout << "Arguments:" << std::endl;
    std::cout
        << "  input_file.root   - Input ROOT file containing WNSEvent tree"
        << std::endl;
    std::cout << "  output_file.root  - Output ROOT file with flattened tree"
              << std::endl;
    std::cout << "  tree_name         - Name of the input tree (default: read "
                 "from JSON)"
              << std::endl;
    std::cout << "  config_file.json  - JSON config file to read tree name "
                 "from (default: config/filepath.json)"
              << std::endl;
    std::cout << std::endl;
    std::cout << "Example:" << std::endl;
    std::cout << "  " << argv[0] << " Data/input.root Data/output_flat.root"
              << std::endl;
    std::cout << "  " << argv[0]
              << " Data/input.root Data/output_flat.root WNSRawTree"
              << std::endl;
    std::cout << "  " << argv[0]
              << " Data/input.root Data/output_flat.root WNSRawTree "
                 "config/2025_232Th.json"
              << std::endl;
    std::cout << std::endl;
    return 1;
  }

  const char *inputFile = argv[1];
  const char *outputFile = argv[2];

  // Determine tree name: command line argument > JSON config > default
  std::string treeName;
  if (argc >= 4 && std::string(argv[3]).find(".json") == std::string::npos) {
    // Third argument is a tree name (not a JSON file)
    treeName = argv[3];
  } else {
    // Read from JSON config
    std::string configPath =
        (argc >= 4 && std::string(argv[3]).find(".json") != std::string::npos)
            ? argv[3]
            : "config/filepath.json";
    treeName = ReadTreeNameFromJSON(configPath);
    std::cout << "Reading tree name from config: " << configPath << std::endl;
  }

  std::cout << "Using tree name: " << treeName << std::endl;
  std::cout << std::endl;

  ConvertToFlatTree(inputFile, outputFile, treeName.c_str());

  return 0;
}
