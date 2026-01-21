/*****************************************************
 * File: ConfigReader.cxx
 * Description: Implementation of configuration reader class
 * Author: Kilo Code
 * Created: 2026/01/21
 ****************************************************/

#include "ConfigReader.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

using json = nlohmann::json;

// Constructor
ConfigReader::ConfigReader() : m_isValid(false) {
  // Initialize default values
  m_config.rawTreeName = "EventBranch";
  m_config.rootTreeName = "tree";
}

// Destructor
ConfigReader::~ConfigReader() {}

bool ConfigReader::ReadJSONFile(const std::string &filepath,
                                json &outJson) const {
  std::ifstream file(filepath);
  if (!file.is_open()) {
    std::cerr << "Error: Cannot open JSON file: " << filepath << std::endl;
    return false;
  }

  try {
    file >> outJson;
    return true;
  } catch (const json::parse_error &e) {
    std::cerr << "Error: JSON parse error in file " << filepath << ": "
              << e.what() << std::endl;
    return false;
  } catch (const std::exception &e) {
    std::cerr << "Error: Failed to read JSON file " << filepath << ": "
              << e.what() << std::endl;
    return false;
  }
}

bool ConfigReader::LoadFilepathConfig(const std::string &configPath) {
  std::cout << "========================================" << std::endl;
  std::cout << "Reading filepath configuration" << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << "Config file: " << configPath << std::endl;

  json filepathJson;
  if (!ReadJSONFile(configPath, filepathJson)) {
    m_isValid = false;
    return false;
  }

  // Extract RootData path
  if (!filepathJson.contains("RootData")) {
    std::cerr << "Error: 'RootData' key not found in filepath.json"
              << std::endl;
    m_isValid = false;
    return false;
  }
  m_config.rootDataPath = filepathJson["RootData"].get<std::string>();
  std::cout << "RootData path: " << m_config.rootDataPath << std::endl;

  // Extract ExpName
  if (!filepathJson.contains("ExpName")) {
    std::cerr << "Error: 'ExpName' key not found in filepath.json" << std::endl;
    m_isValid = false;
    return false;
  }
  m_config.expName = filepathJson["ExpName"].get<std::string>();
  std::cout << "Experiment name: " << m_config.expName << std::endl;

  // Extract XSPath
  if (!filepathJson.contains("XSPath")) {
    std::cerr << "Error: 'XSPath' key not found in filepath.json" << std::endl;
    m_isValid = false;
    return false;
  }
  m_config.xsPath = filepathJson["XSPath"].get<std::string>();
  std::cout << "XS path: " << m_config.xsPath << std::endl;

  std::cout << std::endl;
  // Don't validate here - fileList will be loaded in LoadExperimentConfig()
  m_isValid = false; // Will be set to true after LoadExperimentConfig()
  return true;
}

bool ConfigReader::ReadFIXMConfig(const json &fixmJson,
                                  FIXMConfig &fixmConfig) const {
  // Read Global FIXM configuration
  if (fixmJson.contains("Global")) {
    auto globalJson = fixmJson["Global"];

    if (globalJson.contains("gammaFitRange")) {
      fixmConfig.Global.gammaFitRange =
          globalJson["gammaFitRange"].get<std::vector<double>>();
    }
    if (globalJson.contains("LCalEn")) {
      fixmConfig.Global.LCalEn =
          globalJson["LCalEn"].get<std::vector<double>>();
    }
    if (globalJson.contains("LCaldT")) {
      fixmConfig.Global.LCaldT =
          globalJson["LCaldT"].get<std::vector<double>>();
    }
    if (globalJson.contains("LengthSet")) {
      fixmConfig.Global.LengthSet = globalJson["LengthSet"].get<double>();
    }
    if (globalJson.contains("NoiseCut")) {
      fixmConfig.Global.NoiseCut = globalJson["NoiseCut"].get<double>();
    }
    if (globalJson.contains("CHIDUSE")) {
      fixmConfig.Global.CHIDUSE = globalJson["CHIDUSE"].get<std::vector<int>>();
    }
    if (globalJson.contains("EnergyDivide")) {
      fixmConfig.Global.EnergyDivide =
          globalJson["EnergyDivide"].get<std::vector<double>>();
    }

    std::cout << "Global FIXM configuration loaded" << std::endl;
  }

  // Read Channel configurations
  if (fixmJson.contains("Channels")) {
    auto channelsJson = fixmJson["Channels"];
    for (auto it = channelsJson.begin(); it != channelsJson.end(); ++it) {
      int channelId = std::stoi(it.key());
      auto channelJson = it.value();

      ChannelConfig chConfig;
      chConfig.DetID = channelJson["DetID"].get<int>();
      chConfig.Tg = channelJson["Tg"].get<double>();
      chConfig.Threshold = channelJson["Threshold"].get<int>();
      chConfig.ThresholdFitcut = channelJson["ThresholdFitcut"].get<double>();
      chConfig.ThresholdEDivide =
          channelJson["ThresholdEDivide"].get<std::vector<double>>();
      chConfig.Length = channelJson["Length"].get<double>();
      chConfig.SampleType = channelJson["SampleType"].get<std::string>();
      chConfig.SampleNumber = channelJson["SampleNumber"].get<int>();
      chConfig.Mass = channelJson["Mass"].get<double>();
      chConfig.Radius = channelJson["Radius"].get<double>();
      chConfig.A = channelJson["A"].get<int>();
      chConfig.DetEff = channelJson["DetEff"].get<double>();

      fixmConfig.Channels[channelId] = chConfig;
    }
    std::cout << "Loaded " << fixmConfig.Channels.size()
              << " channel configurations" << std::endl;
  }

  return true;
}

bool ConfigReader::LoadExperimentConfig(const std::string &configPath) {
  std::cout << "========================================" << std::endl;
  std::cout << "Reading experiment configuration" << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << "Config file: " << configPath << std::endl;

  json expJson;
  if (!ReadJSONFile(configPath, expJson)) {
    m_isValid = false;
    return false;
  }

  // Extract tree names from Files section
  if (expJson.contains("Files")) {
    auto filesJson = expJson["Files"];

    // RawTreeName (for original data)
    if (filesJson.contains("RawTreeName")) {
      m_config.rawTreeName = filesJson["RawTreeName"].get<std::string>();
      std::cout << "Raw tree name: " << m_config.rawTreeName << std::endl;
    } else {
      m_config.rawTreeName = "EventBranch";
      std::cout << "Warning: RawTreeName not found, using default: "
                << m_config.rawTreeName << std::endl;
    }

    // RootTreeName (for flattened data)
    if (filesJson.contains("RootTreeName")) {
      m_config.rootTreeName = filesJson["RootTreeName"].get<std::string>();
      std::cout << "Root tree name: " << m_config.rootTreeName << std::endl;
    } else {
      m_config.rootTreeName = "tree";
      std::cout << "Warning: RootTreeName not found, using default: "
                << m_config.rootTreeName << std::endl;
    }
  }

  // Extract file list
  if (!expJson.contains("Files") || !expJson["Files"].contains("filelist")) {
    std::cerr << "Error: 'Files.filelist' key not found in experiment config"
              << std::endl;
    m_isValid = false;
    return false;
  }

  m_config.fileList =
      expJson["Files"]["filelist"].get<std::vector<std::string>>();
  std::cout << "Number of files: " << m_config.fileList.size() << std::endl;

  // Print file list
  for (size_t i = 0; i < m_config.fileList.size(); ++i) {
    std::cout << "  [" << i << "] " << m_config.fileList[i] << std::endl;
  }

  // Read FIXM configuration
  if (expJson.contains("FIXM")) {
    std::cout << std::endl;
    std::cout << "Reading FIXM configuration..." << std::endl;
    if (!ReadFIXMConfig(expJson["FIXM"], m_config.fixmConfig)) {
      std::cerr << "Warning: Failed to read FIXM configuration" << std::endl;
    }
  }

  std::cout << std::endl;
  m_isValid = ValidateConfig();
  return m_isValid;
}

const AnalysisConfig &ConfigReader::GetConfig() const { return m_config; }

std::string ConfigReader::GetRootDataPath() const {
  return m_config.rootDataPath;
}

std::string ConfigReader::GetExperimentName() const { return m_config.expName; }

const std::vector<std::string> &ConfigReader::GetFileList() const {
  return m_config.fileList;
}

std::string ConfigReader::GetRawTreeName() const {
  return m_config.rawTreeName;
}

std::string ConfigReader::GetRootTreeName() const {
  return m_config.rootTreeName;
}

std::string ConfigReader::GetXSPath() const { return m_config.xsPath; }

const FIXMConfig &ConfigReader::GetFIXMConfig() const {
  return m_config.fixmConfig;
}

bool ConfigReader::IsValid() const { return m_isValid; }

bool ConfigReader::ValidateConfig() const {
  // Check if essential fields are populated
  if (m_config.rootDataPath.empty()) {
    std::cerr << "Error: RootData path is empty" << std::endl;
    return false;
  }

  if (m_config.expName.empty()) {
    std::cerr << "Error: Experiment name is empty" << std::endl;
    return false;
  }

  if (m_config.xsPath.empty()) {
    std::cerr << "Error: XS path is empty" << std::endl;
    return false;
  }

  if (m_config.fileList.empty()) {
    std::cerr << "Error: File list is empty" << std::endl;
    return false;
  }

  if (m_config.rootTreeName.empty()) {
    std::cerr << "Error: Root tree name is empty" << std::endl;
    return false;
  }

  return true;
}

void ConfigReader::PrintSummary() const {
  std::cout << "========================================" << std::endl;
  std::cout << "Configuration Summary" << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << "Root Data Path: " << m_config.rootDataPath << std::endl;
  std::cout << "Experiment Name: " << m_config.expName << std::endl;
  std::cout << "XS Path: " << m_config.xsPath << std::endl;
  std::cout << "Raw Tree Name: " << m_config.rawTreeName << std::endl;
  std::cout << "Root Tree Name: " << m_config.rootTreeName << std::endl;
  std::cout << "Number of Files: " << m_config.fileList.size() << std::endl;
  std::cout << "Number of Channels: " << m_config.fixmConfig.Channels.size()
            << std::endl;
  std::cout << "Configuration Valid: " << (m_isValid ? "Yes" : "No")
            << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << std::endl;
}
