/*****************************************************
 * File: RDataFrameAnalysis.h
 * Description: Header file for RDataFrame analysis class
 * Author: Kilo Code
 * Created: 2026/01/21
 ****************************************************/

#ifndef RDATAFRAME_ANALYSIS_H
#define RDATAFRAME_ANALYSIS_H

#include "ConfigReader.h"
#include "ROOT/RDataFrame.hxx"
#include "TChain.h"
#include <memory>
#include <string>

/**
 * @brief RDataFrame analysis class
 *
 * This class manages the RDataFrame analysis workflow, including:
 * - Configuration loading via ConfigReader
 * - TChain initialization and management
 * - Data analysis using RDataFrame
 * - Visualization and statistics
 */
class RDataFrameAnalysis {
public:
  /**
   * @brief Constructor
   * @param configReader Reference to ConfigReader instance
   */
  explicit RDataFrameAnalysis(ConfigReader &configReader);

  /**
   * @brief Destructor
   */
  ~RDataFrameAnalysis();

  /**
   * @brief Initialize TChain with configuration
   * @return true on success, false on failure
   */
  bool InitializeTChain();

  /**
   * @brief Run the complete analysis workflow
   * @return true on success, false on failure
   */
  bool RunAnalysis();

  /**
   * @brief Get the TChain pointer
   * @return Pointer to TChain (may be nullptr if not initialized)
   */
  TChain *GetTChain();

  /**
   * @brief Get the configuration reader
   * @return Reference to ConfigReader
   */
  const ConfigReader &GetConfigReader() const;

  /**
   * @brief Print analysis header
   */
  static void PrintHeader();

  /**
   * @brief Print analysis footer
   */
  static void PrintFooter();

private:
  /**
   * @brief Analyze data using RDataFrame
   */
  void AnalyzeWithRDataFrame();

  /**
   * @brief Display basic statistics
   * @param df RDataFrame reference
   */
  void DisplayBasicStatistics(ROOT::RDataFrame &df);

  /**
   * @brief Display channel statistics
   * @param df RDataFrame reference
   */
  void DisplayChannelStatistics(ROOT::RDataFrame &df);

  /**
   * @brief Display time range statistics
   * @param df RDataFrame reference
   */
  void DisplayTimeRangeStatistics(ROOT::RDataFrame &df);

  /**
   * @brief Display first few events
   * @param df RDataFrame reference
   * @param numEvents Number of events to display
   */
  void DisplayFirstEvents(ROOT::RDataFrame &df, int numEvents = 5);

  /**
   * @brief Create and display plots
   * @param df RDataFrame reference
   */
  void DisplayPlots(ROOT::RDataFrame &df);

  void GetGammaFlash();
  void GetThR1();

  ConfigReader &m_configReader; ///< Reference to configuration reader
  TChain *m_chain;              ///< TChain pointer for data access
  bool m_isInitialized;         ///< Initialization status flag
};

#endif // RDATAFRAME_ANALYSIS_H
