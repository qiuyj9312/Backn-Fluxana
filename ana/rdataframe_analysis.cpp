/*****************************************************
 * File: rdataframe_analysis.cpp
 * Description: Main entry point for RDataFrame analysis
 * Author: Kilo Code
 * Created: 2026/01/21
 ****************************************************/

#include "ConfigReader.h"
#include "CrossSectionAnalysis.h"
#include "NeutronFluxAnalysis.h"
#include "RDataFrameAnalysis.h"
#include "TApplication.h"
#include <iostream>
#include <vector>

/**
 * @brief Main function
 * @param argc Argument count
 * @param argv Argument values
 * @return 0 on success, 1 on failure
 */
int main(int argc, char **argv) {
  // Create TApplication for ROOT graphics
  TApplication theApp("App", &argc, argv);

  // Print header
  RDataFrameAnalysis::PrintHeader();

  // Default config file path and analysis type
  std::string filepathConfig = "config/filepath.json";
  std::string analysisType = "GetThR1"; // Default analysis type

  // Parse command line arguments (skip TApplication arguments)
  for (int i = 1; i < theApp.Argc(); ++i) {
    std::string arg = theApp.Argv(i);
    if (arg == "--help" || arg == "-h") {
      std::cout << "Usage: " << argv[0] << " [analysis_type]" << std::endl;
      std::cout << std::endl;
      std::cout << "Arguments:" << std::endl;
      std::cout << "  analysis_type  Type of analysis to run (default: GetThR1)"
                << std::endl;
      std::cout << std::endl;
      std::cout << "Available analysis types:" << std::endl;
      std::cout << "  - GetGammaFlash" << std::endl;
      std::cout << "  - GetThR1" << std::endl;
      std::cout << "  - GetPileupCorr" << std::endl;
      std::cout << "  - CountT0" << std::endl;
      std::cout << "  - CalFlux" << std::endl;
      std::cout << "  - CalUncertainty" << std::endl;
      std::cout << "  - AnalyzeWithRDataFrame" << std::endl;
      std::cout << std::endl;
      std::cout << "Examples:" << std::endl;
      std::cout << "  " << argv[0] << std::endl;
      std::cout << "  " << argv[0] << " GetGammaFlash" << std::endl;
      std::cout << "  " << argv[0] << " GetThR1" << std::endl;
      std::cout << "  " << argv[0] << " AnalyzeWithRDataFrame" << std::endl;
      return 0;
    } else {
      analysisType = arg;
    }
  }

  // Step 1: Create ConfigReader instance
  ConfigReader configReader;

  // Step 2: Load filepath.json configuration
  if (!configReader.LoadFilepathConfig(filepathConfig)) {
    std::cerr << "Error: Failed to read filepath configuration" << std::endl;
    return 1;
  }

  // Step 3: Build experiment config file path
  std::string expConfigPath =
      "config/" + configReader.GetExperimentName() + ".json";

  // Step 4: Load experiment configuration file
  if (!configReader.LoadExperimentConfig(expConfigPath)) {
    std::cerr << "Error: Failed to read experiment configuration" << std::endl;
    return 1;
  }

  // Step 5: Print configuration summary
  configReader.PrintSummary();

  // Step 6: 根据分析类型选择合适的分析类
  std::cout << "Running analysis type: " << analysisType << std::endl;

  bool success = false;

  // 截面分析类型列表
  std::vector<std::string> xsAnalysisTypes = {"GetXSSingleBunch"};

  // 检查是否为截面分析
  bool isCrossSectionAnalysis = false;
  for (const auto &type : xsAnalysisTypes) {
    if (analysisType == type) {
      isCrossSectionAnalysis = true;
      break;
    }
  }

  if (isCrossSectionAnalysis) {
    // 使用截面分析类
    std::cout << "Using CrossSectionAnalysis class..." << std::endl;
    CrossSectionAnalysis analysis(configReader);
    success = analysis.RunAnalysis(analysisType);
  } else {
    // 使用中子通量分析类 (包含所有其他分析类型)
    std::cout << "Using NeutronFluxAnalysis class..." << std::endl;
    NeutronFluxAnalysis analysis(configReader);
    success = analysis.RunAnalysis(analysisType);
  }

  if (!success) {
    std::cerr << "Error: Analysis failed" << std::endl;
    return 1;
  }

  // Print footer
  RDataFrameAnalysis::PrintFooter();

  // Run application to keep plots visible
  theApp.Run();

  return 0;
}
