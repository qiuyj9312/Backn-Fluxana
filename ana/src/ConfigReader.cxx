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

  // Extract DataType
  if (!filepathJson.contains("DataType")) {
    std::cerr << "Error: 'DataType' key not found in filepath.json" << std::endl;
    m_isValid = false;
    return false;
  }
  m_config.dataType = filepathJson["DataType"].get<std::string>();
  std::cout << "Data type: " << m_config.dataType << std::endl;

  // Extract DetectorType
  if (!filepathJson.contains("DetectorType")) {
    std::cerr << "Error: 'DetectorType' key not found in filepath.json" << std::endl;
    m_isValid = false;
    return false;
  }
  m_config.detectorType = filepathJson["DetectorType"].get<std::string>();
  std::cout << "Detector type: " << m_config.detectorType << std::endl;

  // Extract OriginData
  if (!filepathJson.contains("OriginData")) {
    std::cerr << "Error: 'OriginData' key not found in filepath.json"
              << std::endl;
    m_isValid = false;
    return false;
  }
  m_config.originDataPath = filepathJson["OriginData"].get<std::string>();
  std::cout << "OriginData path: " << m_config.originDataPath << std::endl;

  // Extract FluxPath
  if (!filepathJson.contains("FluxPath")) {
    std::cerr << "Error: 'FluxPath' key not found in filepath.json"
              << std::endl;
    m_isValid = false;
    return false;
  }
  m_config.fluxPath = filepathJson["FluxPath"].get<std::string>();
  std::cout << "Flux path: " << m_config.fluxPath << std::endl;

  // Extract BeamDataPath
  if (!filepathJson.contains("BeamDataPath")) {
    std::cerr << "Error: 'BeamDataPath' key not found in filepath.json"
              << std::endl;
    m_isValid = false;
    return false;
  }
  m_config.beamDataPath = filepathJson["BeamDataPath"].get<std::string>();
  std::cout << "BeamData path: " << m_config.beamDataPath << std::endl;

  // Extract XSPath
  if (!filepathJson.contains("XSPath")) {
    std::cerr << "Error: 'XSPath' key not found in filepath.json" << std::endl;
    m_isValid = false;
    return false;
  }
  m_config.xsPath = filepathJson["XSPath"].get<std::string>();
  std::cout << "XS path: " << m_config.xsPath << std::endl;

  // Extract ParaPath
  if (!filepathJson.contains("ParaPath")) {
    std::cerr << "Error: 'ParaPath' key not found in filepath.json"
              << std::endl;
    m_isValid = false;
    return false;
  }
  m_config.paraPath = filepathJson["ParaPath"].get<std::string>();
  std::cout << "Para path: " << m_config.paraPath << std::endl;

  // Extract T0Path
  if (!filepathJson.contains("T0Path")) {
    std::cerr << "Error: 'T0Path' key not found in filepath.json" << std::endl;
    m_isValid = false;
    return false;
  }
  m_config.t0Path = filepathJson["T0Path"].get<std::string>();
  std::cout << "T0 path: " << m_config.t0Path << std::endl;

  // Extract ENDF data file paths
  // U-235 data
  if (filepathJson.contains("ENDFDATA235UNF")) {
    m_config.endfDataU5NF =
        m_config.paraPath + filepathJson["ENDFDATA235UNF"].get<std::string>();
    std::cout << "ENDF U-235 NF: " << m_config.endfDataU5NF << std::endl;
  }
  if (filepathJson.contains("ENDFDATA235UNTOT")) {
    m_config.endfDataU5NTOT =
        m_config.paraPath + filepathJson["ENDFDATA235UNTOT"].get<std::string>();
    std::cout << "ENDF U-235 NTOT: " << m_config.endfDataU5NTOT << std::endl;
  }
  if (filepathJson.contains("UNENDFDATA235UNF")) {
    m_config.unEndfDataU5NF =
        m_config.paraPath + filepathJson["UNENDFDATA235UNF"].get<std::string>();
    std::cout << "UN-ENDF U-235 NF: " << m_config.unEndfDataU5NF << std::endl;
  }

  // U-238 data
  if (filepathJson.contains("ENDFDATA238UNF")) {
    m_config.endfDataU8NF =
        m_config.paraPath + filepathJson["ENDFDATA238UNF"].get<std::string>();
    std::cout << "ENDF U-238 NF: " << m_config.endfDataU8NF << std::endl;
  }
  if (filepathJson.contains("ENDFDATA238UNTOT")) {
    m_config.endfDataU8NTOT =
        m_config.paraPath + filepathJson["ENDFDATA238UNTOT"].get<std::string>();
    std::cout << "ENDF U-238 NTOT: " << m_config.endfDataU8NTOT << std::endl;
  }
  if (filepathJson.contains("UNENDFDATA238UNF")) {
    m_config.unEndfDataU8NF =
        m_config.paraPath + filepathJson["UNENDFDATA238UNF"].get<std::string>();
    std::cout << "UN-ENDF U-238 NF: " << m_config.unEndfDataU8NF << std::endl;
  }

  // Li-6 data
  if (filepathJson.contains("ENDFDATALi6NT")) {
    m_config.endfDataLi6NT =
        m_config.paraPath + filepathJson["ENDFDATALi6NT"].get<std::string>();
    std::cout << "ENDF Li-6 NT: " << m_config.endfDataLi6NT << std::endl;
  }
  if (filepathJson.contains("ENDFDATALi6NTOT")) {
    m_config.endfDataLi6NTOT =
        m_config.paraPath + filepathJson["ENDFDATALi6NTOT"].get<std::string>();
    std::cout << "ENDF Li-6 NTOT: " << m_config.endfDataLi6NTOT << std::endl;
  }

  // Other data files
  if (filepathJson.contains("UNDETECLiSi")) {
    m_config.undetecLiSi =
        m_config.paraPath + filepathJson["UNDETECLiSi"].get<std::string>();
    std::cout << "UNDETEC LiSi: " << m_config.undetecLiSi << std::endl;
  }
  if (filepathJson.contains("EFFLiSi")) {
    m_config.effLiSi =
        m_config.paraPath + filepathJson["EFFLiSi"].get<std::string>();
    std::cout << "EFF LiSi: " << m_config.effLiSi << std::endl;
  }
  if (filepathJson.contains("DELTALDATA")) {
    m_config.deltaLData =
        m_config.paraPath + filepathJson["DELTALDATA"].get<std::string>();
    std::cout << "Delta L data: " << m_config.deltaLData << std::endl;
  }

  std::cout << std::endl;
  // Don't validate here - fileList will be loaded in LoadExperimentConfig()
  m_isValid = false; // Will be set to true after LoadExperimentConfig()
  return true;
}

bool ConfigReader::ReadFIXMConfig(const json &fixmJson,
                                  FIXMConfig &fixmConfig) const {
  // Helper lambda to extract value from either direct number or {value, unit}
  // object
  auto getValue = [](const json &j) -> double {
    if (j.is_object() && j.contains("value")) {
      return j["value"].get<double>();
    }
    return j.get<double>();
  };
  // Helper lambda to extract unit from {value, unit} object
  auto getUnit = [](const json &j) -> std::string {
    if (j.is_object() && j.contains("unit")) {
      return j["unit"].get<std::string>();
    }
    return "";
  };

  // Helper lambda to extract vector of values
  auto getValueVector = [&getValue](const json &j) -> std::vector<double> {
    std::vector<double> result;
    for (const auto &item : j) {
      result.push_back(getValue(item));
    }
    return result;
  };

  // Read Global FIXM configuration
  if (fixmJson.contains("Global")) {
    auto globalJson = fixmJson["Global"];

    if (globalJson.contains("gammaFitRange")) {
      fixmConfig.Global.gammaFitRange =
          getValueVector(globalJson["gammaFitRange"]);
    }
    if (globalJson.contains("ThFindminRange")) {
      fixmConfig.Global.ThFindminRange =
          getValueVector(globalJson["ThFindminRange"]);
    }
    if (globalJson.contains("LCalEn")) {
      fixmConfig.Global.LCalEn = getValueVector(globalJson["LCalEn"]);
    }
    if (globalJson.contains("LCaldT")) {
      fixmConfig.Global.LCaldT = getValueVector(globalJson["LCaldT"]);
    }
    if (globalJson.contains("LengthSet")) {
      fixmConfig.Global.LengthSet = getValue(globalJson["LengthSet"]);
    }
    if (globalJson.contains("NoiseCut")) {
      fixmConfig.Global.NoiseCut = getValue(globalJson["NoiseCut"]);
    }
    if (globalJson.contains("CHIDUSE")) {
      fixmConfig.Global.CHIDUSE = globalJson["CHIDUSE"].get<std::vector<int>>();
    }
    if (globalJson.contains("EnergyDivide")) {
      fixmConfig.Global.EnergyDivide =
          getValueVector(globalJson["EnergyDivide"]);
    }
    if (globalJson.contains("DL_cell")) {
      fixmConfig.Global.DL_cell = getValue(globalJson["DL_cell"]);
    } else {
      // Default value if not specified
      fixmConfig.Global.DL_cell = 0.019;
      std::cout << "Warning: DL_cell not found, using default: 0.019 m"
                << std::endl;
    }
    if (globalJson.contains("Bin")) {
      auto binJson = globalJson["Bin"];
      if (binJson.contains("bpd")) {
        fixmConfig.Global.Bin.bpd = binJson["bpd"].get<int>();
      }
      if (binJson.contains("nDec")) {
        fixmConfig.Global.Bin.nDec = binJson["nDec"].get<int>();
      }
      if (binJson.contains("LowEdge")) {
        fixmConfig.Global.Bin.LowEdge = binJson["LowEdge"].get<double>();
      }
    }
    if (globalJson.contains("NRebin")) {
      auto getValue = [](const json &j) -> double {
        if (j.is_object() && j.contains("value")) {
          return j["value"].get<double>();
        }
        return j.get<double>();
      };
      fixmConfig.Global.NRebin =
          static_cast<int>(getValue(globalJson["NRebin"]));
    } else {
      fixmConfig.Global.NRebin = 1;
    }

    if (globalJson.contains("Intergralnsub")) {
      auto getValue = [](const json &j) -> double {
        if (j.is_object() && j.contains("value")) {
          return j["value"].get<double>();
        }
        return j.get<double>();
      };
      fixmConfig.Global.Intergralnsub =
          static_cast<int>(getValue(globalJson["Intergralnsub"]));
    } else {
      fixmConfig.Global.Intergralnsub = 20; // Default value
      std::cout << "Warning: Intergralnsub not found, using default: 20"
                << std::endl;
    }

    if (globalJson.contains("UFRandomTimes")) {
      auto getValue = [](const json &j) -> double {
        if (j.is_object() && j.contains("value")) {
          return j["value"].get<double>();
        }
        return j.get<double>();
      };
      fixmConfig.Global.UFRandomTimes = getValue(globalJson["UFRandomTimes"]);
    } else {
      fixmConfig.Global.UFRandomTimes = 10.0; // Default value
      std::cout << "Warning: UFRandomTimes not found, using default: 10.0"
                << std::endl;
    }
    if (globalJson.contains("EnergyCut_Low")) {
      fixmConfig.Global.EnergyCut_Low = getValue(globalJson["EnergyCut_Low"]);
    } else {
      fixmConfig.Global.EnergyCut_Low = 1e4; // 10 keV default
    }

    if (globalJson.contains("EnergyCut_U8")) {
      fixmConfig.Global.EnergyCut_U8 = getValue(globalJson["EnergyCut_U8"]);
    } else {
      fixmConfig.Global.EnergyCut_U8 = 1e6; // 1 MeV default
    }

    if (globalJson.contains("EnergyCut_High")) {
      fixmConfig.Global.EnergyCut_High = getValue(globalJson["EnergyCut_High"]);
    } else {
      fixmConfig.Global.EnergyCut_High = 3e8; // 300 MeV default
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
      chConfig.Tg = getValue(channelJson["Tg"]);
      
      if (channelJson.contains("DeadTimeConstant")) {
        chConfig.DeadTimeConstant = getValue(channelJson["DeadTimeConstant"]);
      } else {
        chConfig.DeadTimeConstant = 80.0;
        std::cout << "Warning: DeadTimeConstant not found near channel " << channelId 
                  << ", using default: 80.0 ns" << std::endl;
      }

      chConfig.Threshold = getValue(channelJson["Threshold"]);

      // Parse ThresholdRe array
      if (channelJson.contains("ThresholdRe")) {
        chConfig.ThresholdRe = getValueVector(channelJson["ThresholdRe"]);
      }

      // Parse ThresholdFitcut array
      if (channelJson.contains("ThresholdFitcut")) {
        chConfig.ThresholdFitcut =
            getValueVector(channelJson["ThresholdFitcut"]);
      }

      chConfig.ThresholdEDivide =
          channelJson["ThresholdEDivide"].get<std::vector<double>>();
      chConfig.Length = getValue(channelJson["Length"]);
      chConfig.SampleType = channelJson["SampleType"].get<std::string>();
      chConfig.SampleNumber = channelJson["SampleNumber"].get<int>();
      chConfig.Mass = getValue(channelJson["Mass"]);
      chConfig.MassUnit = getUnit(channelJson["Mass"]);
      chConfig.Radius = getValue(channelJson["Radius"]);
      chConfig.A = getValue(channelJson["A"]);
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

  // Extract FPulse from ExperimentCondition
  if (expJson.contains("ExperimentCondition") &&
      expJson["ExperimentCondition"].contains("FPulse") &&
      expJson["ExperimentCondition"]["FPulse"].contains("value")) {
    m_config.fPulse =
        expJson["ExperimentCondition"]["FPulse"]["value"].get<double>();
    m_config.fixmConfig.Global.FPulse = m_config.fPulse; // Store in both places
    std::cout << "Pulse frequency: " << m_config.fPulse << " Hz" << std::endl;
  } else {
    std::cerr
        << "Warning: FPulse not found in ExperimentCondition, setting to 0"
        << std::endl;
    m_config.fPulse = 0.0;
    m_config.fixmConfig.Global.FPulse = 0.0;
  }

  // Extract ExperimentTime from ExperimentCondition
  if (expJson.contains("ExperimentCondition") &&
      expJson["ExperimentCondition"].contains("ExperimentTime") &&
      expJson["ExperimentCondition"]["ExperimentTime"].contains("value")) {
    m_config.experimentTime =
        expJson["ExperimentCondition"]["ExperimentTime"]["value"].get<int>();
    std::cout << "Experiment time: " << m_config.experimentTime << " (yyyymm)"
              << std::endl;
  } else {
    std::cerr << "Warning: ExperimentTime not found in ExperimentCondition, "
                 "setting to 0"
              << std::endl;
    m_config.experimentTime = 0;
  }

  // Extract BeamMode from ExperimentCondition
  if (expJson.contains("ExperimentCondition") &&
      expJson["ExperimentCondition"].contains("BeamMode")) {
    m_config.beamMode =
        expJson["ExperimentCondition"]["BeamMode"].get<std::string>();
    std::cout << "Beam mode: " << m_config.beamMode << std::endl;
  } else {
    m_config.beamMode = "SingleBunch";
    std::cout << "Warning: BeamMode not found, using default: SingleBunch"
              << std::endl;
  }

  // Extract BeamConfig from ExperimentCondition
  if (expJson.contains("ExperimentCondition") &&
      expJson["ExperimentCondition"].contains("BeamConfig")) {
    m_config.beamConfig =
        expJson["ExperimentCondition"]["BeamConfig"].get<std::string>();
    std::cout << "Beam config: " << m_config.beamConfig << std::endl;
  } else {
    m_config.beamConfig = "";
    std::cout << "Warning: BeamConfig not found" << std::endl;
  }

  // Extract EndStation from ExperimentCondition
  if (expJson.contains("ExperimentCondition") &&
      expJson["ExperimentCondition"].contains("EndStation")) {
    m_config.endStation =
        expJson["ExperimentCondition"]["EndStation"].get<std::string>();
    std::cout << "End station: " << m_config.endStation << std::endl;
  } else {
    m_config.endStation = "";
    std::cout << "Warning: EndStation not found" << std::endl;
  }

  // Extract BeamWindow Cd thickness from ExperimentCondition
  if (expJson.contains("ExperimentCondition") &&
      expJson["ExperimentCondition"].contains("BeamWindow") &&
      expJson["ExperimentCondition"]["BeamWindow"].contains("Cd") &&
      expJson["ExperimentCondition"]["BeamWindow"]["Cd"].contains("thickness") &&
      expJson["ExperimentCondition"]["BeamWindow"]["Cd"]["thickness"].contains("value")) {
    m_config.beamWindowCdThickness =
        expJson["ExperimentCondition"]["BeamWindow"]["Cd"]["thickness"]["value"].get<double>();
    std::cout << "Beam window Cd thickness: " << m_config.beamWindowCdThickness << " mm" << std::endl;
  } else {
    m_config.beamWindowCdThickness = 0.0;
    std::cout << "Warning: BeamWindow Cd thickness not found, setting to 0" << std::endl;
  }

  // Extract ProtonEnergy from ExperimentCondition
  if (expJson.contains("ExperimentCondition") &&
      expJson["ExperimentCondition"].contains("ProtonEnergy") &&
      expJson["ExperimentCondition"]["ProtonEnergy"].contains("value")) {
    m_config.protonEnergy =
        expJson["ExperimentCondition"]["ProtonEnergy"]["value"].get<double>();
    std::cout << "Proton energy: " << m_config.protonEnergy << " eV"
              << std::endl;
  } else {
    m_config.protonEnergy = 0.0;
    std::cout << "Warning: ProtonEnergy not found, setting to 0" << std::endl;
  }

  // Extract BeamPower from ExperimentCondition
  if (expJson.contains("ExperimentCondition") &&
      expJson["ExperimentCondition"].contains("BeamPower") &&
      expJson["ExperimentCondition"]["BeamPower"].contains("value")) {
    m_config.beamPower =
        expJson["ExperimentCondition"]["BeamPower"]["value"].get<double>();
    std::cout << "Beam power: " << m_config.beamPower << " kW" << std::endl;
  } else {
    m_config.beamPower = 95.0;
    std::cout << "Warning: BeamPower not found in ExperimentCondition, using "
                 "default: 95.0 kW"
              << std::endl;
  }

  // Extract BeamRadius from ExperimentCondition
  if (expJson.contains("ExperimentCondition") &&
      expJson["ExperimentCondition"].contains("BeamRadius") &&
      expJson["ExperimentCondition"]["BeamRadius"].contains("value")) {
    m_config.beamRadius =
        expJson["ExperimentCondition"]["BeamRadius"]["value"].get<double>();
    std::cout << "Beam radius: " << m_config.beamRadius << " mm" << std::endl;
  } else {
    m_config.beamRadius = 30.0;
    std::cout << "Warning: BeamRadius not found in ExperimentCondition, using "
                 "default: 30.0 mm"
              << std::endl;
  }

  // Extract ProtonCutPercent from ExperimentCondition
  if (expJson.contains("ExperimentCondition") &&
      expJson["ExperimentCondition"].contains("ProtonCutPercent") &&
      expJson["ExperimentCondition"]["ProtonCutPercent"].contains("value")) {
    m_config.protonCutPercent =
        expJson["ExperimentCondition"]["ProtonCutPercent"]["value"]
            .get<double>();
    std::cout << "Proton cut percent: " << m_config.protonCutPercent
              << std::endl;
  } else {
    m_config.protonCutPercent = 0.1; // Default 10%
    std::cout << "Warning: ProtonCutPercent not found, using default: 0.1"
              << std::endl;
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

  // Extract timelist if available
  if (expJson["Files"].contains("timelist")) {
    // Helper lambda to extract value from either direct number or {value,
    // unit} object
    auto getValue = [](const json &j) -> double {
      if (j.is_object() && j.contains("value")) {
        return j["value"].get<double>();
      }
      return j.get<double>();
    };

    // Extract values from timelist
    m_config.timeList.clear();
    for (const auto &item : expJson["Files"]["timelist"]) {
      m_config.timeList.push_back(getValue(item));
    }

    std::cout << "Number of time entries: " << m_config.timeList.size()
              << std::endl;

    // Calculate total time
    double totalTime = 0.0;
    for (const auto &time : m_config.timeList) {
      totalTime += time;
    }
    std::cout << "Total measurement time: " << totalTime << " seconds"
              << std::endl;
  } else {
    std::cerr << "Warning: timelist not found in Files section" << std::endl;
  }

  // Extract T0list if available
  if (expJson["Files"].contains("T0list")) {
    m_config.fixmConfig.Global.T0list =
        expJson["Files"]["T0list"].get<std::vector<std::string>>();
    std::cout << "Number of T0 files: "
              << m_config.fixmConfig.Global.T0list.size() << std::endl;
  } else {
    std::cerr << "Warning: T0list not found in Files section" << std::endl;
  }

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

  // Read LiSi configuration
  if (expJson.contains("LiSi")) {
    std::cout << std::endl;
    std::cout << "Reading LiSi configuration..." << std::endl;
    if (!ReadFIXMConfig(expJson["LiSi"], m_config.lisiConfig)) {
      std::cerr << "Warning: Failed to read LiSi configuration" << std::endl;
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

std::string ConfigReader::GetDataType() const { return m_config.dataType; }

std::string ConfigReader::GetDetectorType() const { return m_config.detectorType; }

std::string ConfigReader::GetOriginDataPath() const {
  return m_config.originDataPath;
}

std::string ConfigReader::GetFluxPath() const { return m_config.fluxPath; }

std::string ConfigReader::GetBeamDataPath() const {
  return m_config.beamDataPath;
}

int ConfigReader::GetExperimentTime() const { return m_config.experimentTime; }

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

std::string ConfigReader::GetDataPath() const { return m_config.paraPath; }

std::string ConfigReader::GetENDFDataU5NF() const {
  return m_config.endfDataU5NF;
}
std::string ConfigReader::GetENDFDataU5NTOT() const {
  return m_config.endfDataU5NTOT;
}

std::string ConfigReader::GetUnENDFDataU5NF() const {
  return m_config.unEndfDataU5NF;
}

std::string ConfigReader::GetENDFDataU8NF() const {
  return m_config.endfDataU8NF;
}

std::string ConfigReader::GetENDFDataU8NTOT() const {
  return m_config.endfDataU8NTOT;
}

std::string ConfigReader::GetUnENDFDataU8NF() const {
  return m_config.unEndfDataU8NF;
}

std::string ConfigReader::GetENDFDataLi6NT() const {
  return m_config.endfDataLi6NT;
}

std::string ConfigReader::GetENDFDataLi6NTOT() const {
  return m_config.endfDataLi6NTOT;
}

std::string ConfigReader::GetUndetecLiSi() const {
  return m_config.undetecLiSi;
}

std::string ConfigReader::GetEffLiSi() const { return m_config.effLiSi; }

std::string ConfigReader::GetDeltaLData() const { return m_config.deltaLData; }
const std::vector<double> &ConfigReader::GetTimeList() const {
  return m_config.timeList;
}

double ConfigReader::GetFPulse() const { return m_config.fPulse; }

double ConfigReader::GetTau(int channelId) const {
  auto it = m_config.fixmConfig.Channels.find(channelId);
  if (it != m_config.fixmConfig.Channels.end()) {
    return it->second.DeadTimeConstant;
  }
  std::cerr << "Warning: Channel " << channelId << " not found, using default tau 80.0 ns" << std::endl;
  return 80.0;
}

double ConfigReader::GetDL_cell() const {
  return m_config.fixmConfig.Global.DL_cell;
}

int ConfigReader::GetNRebin() const {
  return m_config.fixmConfig.Global.NRebin;
}

std::string ConfigReader::GetT0Path() const { return m_config.t0Path; }

const FIXMConfig &ConfigReader::GetFIXMConfig() const {
  return m_config.fixmConfig;
}

const FIXMConfig &ConfigReader::GetLiSiConfig() const {
  return m_config.lisiConfig;
}

std::string ConfigReader::GetBeamMode() const { return m_config.beamMode; }

double ConfigReader::GetProtonEnergy() const { return m_config.protonEnergy; }

double ConfigReader::GetProtonCutPercent() const {
  return m_config.protonCutPercent;
}

double ConfigReader::GetBeamPower() const { return m_config.beamPower; }

std::string ConfigReader::GetBeamConfig() const { return m_config.beamConfig; }

std::string ConfigReader::GetEndStation() const { return m_config.endStation; }

double ConfigReader::GetBeamWindowCdThickness() const { return m_config.beamWindowCdThickness; }

double ConfigReader::GetBeamRadius() const { return m_config.beamRadius; }

double ConfigReader::GetEnergyCutLow() const {
  return m_config.fixmConfig.Global.EnergyCut_Low;
}

double ConfigReader::GetEnergyCutU8() const {
  return m_config.fixmConfig.Global.EnergyCut_U8;
}

double ConfigReader::GetEnergyCutHigh() const {
  return m_config.fixmConfig.Global.EnergyCut_High;
}

int ConfigReader::GetIntergralnsub() const {
  return m_config.fixmConfig.Global.Intergralnsub;
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
