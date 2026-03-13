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
      std::cout << "Usage: " << argv[0] << " <analysis_type>" << std::endl;
      std::cout << "Use scripts/run_analysis.py for interactive help and full "
                   "type list."
                << std::endl;
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

  // 根据 DataType 选择分析类
  const std::string &dataType = configReader.GetDataType();
  std::cout << "DataType: " << dataType << std::endl;

  if (dataType == "XS") {
    // 使用截面分析类
    std::cout << "Using CrossSectionAnalysis class..." << std::endl;
    CrossSectionAnalysis analysis(configReader);
    success = analysis.RunAnalysis(analysisType);
  } else {
    // 使用中子通量分析类 (Flux 及其他)
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
