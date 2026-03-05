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
  int DetID;                       ///< Detector ID
  double Tg;                       ///< Time gate
  int Threshold;                   ///< Threshold value
  std::vector<double> ThresholdRe; ///< Threshold Re for each energy divide
  std::vector<double>
      ThresholdFitcut; ///< Threshold fit cut for each energy divide
  std::vector<double> ThresholdEDivide; ///< Threshold energy divide
  double Length;                        ///< Length
  std::string SampleType;               ///< Sample type (e.g., "U5")
  int SampleNumber;                     ///< Sample number
  double Mass;                          ///< Mass
  std::string MassUnit;                 ///< Mass unit
  double Radius;                        ///< Radius
  int A;                                ///< Atomic mass number
  double DetEff;                        ///< Detector efficiency
};

/**
 * @brief Bin configuration structure for energy histogram
 */
struct BinConfig {
  int bpd;        ///< Bins per decade
  int nDec;       ///< Number of decades
  double LowEdge; ///< Low edge for log scale
};

/**
 * @brief Global FIXM configuration structure
 */
struct FIXMGlobalConfig {
  std::vector<double> gammaFitRange;  ///< Gamma fit range
  std::vector<double> ThFindminRange; ///< Threshold find minimum range
  std::vector<double> LCalEn;         ///< Calibration energies
  std::vector<double> LCaldT;         ///< Calibration time differences
  double DeadTimeConstant;            ///< Dead-time constant (tau) in ns
  double LengthSet;                   ///< Length set
  double NoiseCut;                    ///< Noise cut threshold
  std::vector<int> CHIDUSE;           ///< Channel IDs to use
  std::vector<double> EnergyDivide;   ///< Energy divide boundaries
  double DL_cell;                     ///< DL_cell value (m)
  BinConfig Bin;        ///< Bin configuration for energy histogram
  int NRebin;           ///< Rebin factor for histograms
  double UFRandomTimes; ///< Monte Carlo sampling factor for UF analysis
  double FPulse;        ///< Pulse frequency (Hz)

  std::vector<std::string> T0list; ///< List of T0 files
  double BeamPower;                ///< Beam power in kW
  double BeamRadius;               ///< Beam radius in mm
  double EnergyCut_Low;            ///< Low energy cut (eV)
  double EnergyCut_U8;             ///< U8 energy cut (eV)
  double EnergyCut_High;           ///< High energy cut (eV)
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
  std::string rootDataPath; ///< Path to ROOT data directory
  std::string expName;      ///< Experiment name
  std::string xsPath;       ///< Path to XS directory (for cuts, etc.)
  std::string paraPath; ///< Path to Para directory (for parameter files, etc.)
  std::string t0Path;   ///< Path to T0 data directory
  std::string dataType; ///< Data type (XS, Flux, etc.)
  std::string originDataPath; ///< Path to Origin data directory
  std::string fluxPath;       ///< Path to Flux directory
  std::string beamDataPath;   ///< Path to Beam data directory

  // U-235 ENDF data files
  std::string endfDataU5NF;   ///< ENDF data file for U-235 neutron fission
  std::string endfDataU5NTOT; ///< ENDF data file for U-235 neutron total
  std::string
      unEndfDataU5NF; ///< Uncorrected ENDF data file for U-235 neutron fission

  // U-238 ENDF data files
  std::string endfDataU8NF;   ///< ENDF data file for U-238 neutron fission
  std::string endfDataU8NTOT; ///< ENDF data file for U-238 neutron total
  std::string
      unEndfDataU8NF; ///< Uncorrected ENDF data file for U-238 neutron fission

  // Li-6 ENDF data files
  std::string endfDataLi6NT;   ///< ENDF data file for Li-6 neutron
  std::string endfDataLi6NTOT; ///< ENDF data file for Li-6 neutron total

  // Other data files
  std::string undetecLiSi; ///< Undetected LiSi data file
  std::string effLiSi;     ///< LiSi efficiency data file
  std::string deltaLData;  ///< Delta L data file

  std::vector<std::string> fileList; ///< List of ROOT files
  std::vector<double> timeList;      ///< List of measurement times (seconds)
  double fPulse;                     ///< Pulse frequency (Hz)
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
   * @brief Get loaded configuration
   * @return Reference to configuration structure
   */
  const AnalysisConfig &GetConfig() const;

  /**
   * @brief Get root data path
   * @return Root data path string
   */
  std::string GetRootDataPath() const;

  /**
   * @brief Get experiment name
   * @return Experiment name string
   */
  std::string GetExperimentName() const;

  /**
   * @brief Get data type
   * @return Data type string
   */
  std::string GetDataType() const;

  /**
   * @brief Get origin data path
   * @return Origin data path string
   */
  std::string GetOriginDataPath() const;

  /**
   * @brief Get flux path
   * @return Flux path string
   */
  std::string GetFluxPath() const;

  /**
   * @brief Get beam data path
   * @return Beam data path string
   */
  std::string GetBeamDataPath() const;

  /**
   * @brief Get file list
   * @return Vector of file paths
   */
  const std::vector<std::string> &GetFileList() const;

  /**
   * @brief Get raw tree name
   * @return Raw tree name string
   */
  std::string GetRawTreeName() const;

  /**
   * @brief Get root tree name
   * @return Root tree name string
   */
  std::string GetRootTreeName() const;

  /**
   * @brief Get XS path (for cuts, etc.)
   * @return XS path string
   */
  std::string GetXSPath() const;

  /**
   * @brief Get data path (for parameter files, etc.)
   * @return Data path string
   */
  std::string GetDataPath() const;

  /**
   * @brief Get ENDF data file for U-235 neutron fission
   * @return Full path to ENDF data file
   */
  std::string GetENDFDataU5NF() const;

  /**
   * @brief Get ENDF data file for U-235 neutron total
   * @return Full path to ENDF data file
   */
  std::string GetENDFDataU5NTOT() const;

  /**
   * @brief Get uncorrected ENDF data file for U-235 neutron fission
   * @return Full path to uncorrected ENDF data file
   */
  std::string GetUnENDFDataU5NF() const;

  /**
   * @brief Get ENDF data file for U-238 neutron fission
   * @return Full path to ENDF data file
   */
  std::string GetENDFDataU8NF() const;

  /**
   * @brief Get ENDF data file for U-238 neutron total
   * @return Full path to ENDF data file
   */
  std::string GetENDFDataU8NTOT() const;

  /**
   * @brief Get uncorrected ENDF data file for U-238 neutron fission
   * @return Full path to uncorrected ENDF data file
   */
  std::string GetUnENDFDataU8NF() const;

  /**
   * @brief Get ENDF data file for Li-6 neutron
   * @return Full path to ENDF data file
   */
  std::string GetENDFDataLi6NT() const;

  /**
   * @brief Get ENDF data file for Li-6 neutron total
   * @return Full path to ENDF data file
   */
  std::string GetENDFDataLi6NTOT() const;

  /**
   * @brief Get undetected LiSi data file
   * @return Full path to undetected LiSi data file
   */
  std::string GetUndetecLiSi() const;

  /**
   * @brief Get LiSi efficiency data file
   * @return Full path to LiSi efficiency data file
   */
  std::string GetEffLiSi() const;

  /**
   * @brief Get delta L data file
   * @return Full path to delta L data file
   */
  std::string GetDeltaLData() const;

  /**
   * @brief Get time list (measurement times in seconds)
   * @return Vector of measurement times
   */
  const std::vector<double> &GetTimeList() const;

  /**
   * @brief Get pulse frequency (Hz)
   * @return Pulse frequency in Hz
   */
  double GetFPulse() const;

  /**
   * @brief Get dead-time constant (tau) in ns
   * @return Dead-time constant in nanoseconds
   */
  double GetTau() const;

  /**
   * @brief Get DL_cell value
   * @return DL_cell value in meters
   */
  double GetDL_cell() const;

  /**
   * @brief Get rebin factor
   * @return Rebin factor
   */
  int GetNRebin() const;

  /**
   * @brief Get T0 path
   * @return T0 path string
   */
  std::string GetT0Path() const;

  /**
   * @brief Get FIXM configuration
   * @return Reference to FIXM configuration
   */
  const FIXMConfig &GetFIXMConfig() const;

  /**
   * @brief Get beam power (kW)
   * @return Beam power in kW
   */
  double GetBeamPower() const;

  /**
   * @brief Get beam radius (mm)
   * @return Beam radius in mm
   */
  double GetBeamRadius() const;

  /**
   * @brief Get low energy cut (eV)
   */
  double GetEnergyCutLow() const;

  /**
   * @brief Get U8 energy cut (eV)
   */
  double GetEnergyCutU8() const;

  /**
   * @brief Get high energy cut (eV)
   */
  double GetEnergyCutHigh() const;

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
