/*****************************************************
 * File: ConfigReader.h
 * Description: Configuration reader class for loading and managing
 *              experiment configuration from JSON files
 * Author: Kilo Code
 * Created: 2026/01/21
 ****************************************************/

#ifndef CONFIG_READER_H
#define CONFIG_READER_H

#include "nlohmann/json.hpp"
#include <map>
#include <string>
#include <vector>

using json = nlohmann::json;

/**
 * @brief Channel configuration structure for FIXM
 */
struct ChannelConfig {
  int DetID;                            ///< Detector ID
  double Tg;                            ///< Time gate
  int Threshold;                        ///< Threshold value
  double ThresholdFitcut;               ///< Threshold fit cut
  std::vector<double> ThresholdEDivide; ///< Threshold energy divide
  double Length;                        ///< Length
  std::string SampleType;               ///< Sample type (e.g., "U5", "U8")
  int SampleNumber;                     ///< Sample number
  double Mass;                          ///< Mass
  double Radius;                        ///< Radius
  int A;                                ///< Atomic mass number
  double DetEff;                        ///< Detector efficiency
};

/**
 * @brief Global FIXM configuration structure
 */
struct FIXMGlobalConfig {
  std::vector<double> gammaFitRange; ///< Gamma fit range
  std::vector<double> LCalEn;        ///< Calibration energies
  std::vector<double> LCaldT;        ///< Calibration time differences
  double LengthSet;                  ///< Length set
  double NoiseCut;                   ///< Noise cut threshold
  std::vector<int> CHIDUSE;          ///< Channel IDs to use
  std::vector<double> EnergyDivide;  ///< Energy divide boundaries
};

/**
 * @brief FIXM configuration structure
 */
struct FIXMConfig {
  FIXMGlobalConfig Global;               ///< Global FIXM settings
  std::map<int, ChannelConfig> Channels; ///< Channel configurations by ID
};

/**
 * @brief Configuration structure for analysis
 */
struct AnalysisConfig {
  std::string rootDataPath;          ///< Path to ROOT data directory
  std::string expName;               ///< Experiment name
  std::string xsPath;                ///< Path to XS directory (for cuts, etc.)
  std::vector<std::string> fileList; ///< List of ROOT files
  std::string rawTreeName;           ///< Raw tree name (e.g., "EventBranch")
  std::string rootTreeName;          ///< Root tree name (e.g., "TTree")
  FIXMConfig fixmConfig;             ///< FIXM configuration
};

/**
 * @brief Configuration reader class
 *
 * This class is responsible for loading, validating, and providing access to
 * experiment configuration parameters from JSON files.
 */
class ConfigReader {
public:
  /**
   * @brief Constructor
   */
  ConfigReader();

  /**
   * @brief Destructor
   */
  ~ConfigReader();

  /**
   * @brief Load configuration from filepath.json
   * @param configPath Path to filepath.json
   * @return true on success, false on failure
   */
  bool LoadFilepathConfig(const std::string &configPath);

  /**
   * @brief Load experiment configuration file
   * @param configPath Path to experiment config file
   * @return true on success, false on failure
   */
  bool LoadExperimentConfig(const std::string &configPath);

  /**
   * @brief Get the loaded configuration
   * @return Reference to the configuration structure
   */
  const AnalysisConfig &GetConfig() const;

  /**
   * @brief Get the root data path
   * @return Root data path string
   */
  std::string GetRootDataPath() const;

  /**
   * @brief Get the experiment name
   * @return Experiment name string
   */
  std::string GetExperimentName() const;

  /**
   * @brief Get the file list
   * @return Vector of file paths
   */
  const std::vector<std::string> &GetFileList() const;

  /**
   * @brief Get the raw tree name
   * @return Raw tree name string
   */
  std::string GetRawTreeName() const;

  /**
   * @brief Get the root tree name
   * @return Root tree name string
   */
  std::string GetRootTreeName() const;

  /**
   * @brief Get the XS path (for cuts, etc.)
   * @return XS path string
   */
  std::string GetXSPath() const;

  /**
   * @brief Get the FIXM configuration
   * @return Reference to FIXM configuration
   */
  const FIXMConfig &GetFIXMConfig() const;

  /**
   * @brief Check if configuration is valid
   * @return true if configuration is valid, false otherwise
   */
  bool IsValid() const;

  /**
   * @brief Print configuration summary
   */
  void PrintSummary() const;

private:
  AnalysisConfig m_config; ///< Configuration data
  bool m_isValid;          ///< Configuration validity flag

  /**
   * @brief Read and parse a JSON file
   * @param filepath Path to JSON file
   * @param outJson Output JSON object
   * @return true on success, false on failure
   */
  bool ReadJSONFile(const std::string &filepath, json &outJson) const;

  /**
   * @brief Read FIXM configuration from JSON object
   * @param fixmJson JSON object containing FIXM configuration
   * @param fixmConfig Output FIXM configuration structure
   * @return true on success, false on failure
   */
  bool ReadFIXMConfig(const json &fixmJson, FIXMConfig &fixmConfig) const;

  /**
   * @brief Validate configuration data
   * @return true if configuration is valid, false otherwise
   */
  bool ValidateConfig() const;
};

#endif // CONFIG_READER_H
