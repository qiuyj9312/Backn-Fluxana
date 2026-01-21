/*****************************************************
 * File: rdataframe_analysis.cpp
 * Description: Main entry point for RDataFrame analysis
 * Author: Kilo Code
 * Created: 2026/01/21
 ****************************************************/

#include "ConfigReader.h"
#include "RDataFrameAnalysis.h"
#include "TApplication.h"
#include <iostream>

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

  // Default config file path
  std::string filepathConfig = "config/filepath.json";

  // Parse command line arguments (skip TApplication arguments)
  for (int i = 1; i < theApp.Argc(); ++i) {
    std::string arg = theApp.Argv(i);
    if (arg == "--help" || arg == "-h") {
      std::cout << "Usage: " << argv[0] << " [config_file]" << std::endl;
      std::cout << std::endl;
      std::cout << "Arguments:" << std::endl;
      std::cout << "  config_file    Path to filepath.json (default: "
                   "config/filepath.json)"
                << std::endl;
      std::cout << std::endl;
      std::cout << "Examples:" << std::endl;
      std::cout << "  " << argv[0] << std::endl;
      std::cout << "  " << argv[0] << " config/filepath.json" << std::endl;
      return 0;
    } else {
      filepathConfig = arg;
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

  // Step 6: Create RDataFrameAnalysis instance with ConfigReader
  RDataFrameAnalysis analysis(configReader);

  // Step 7: Run the analysis
  if (!analysis.RunAnalysis()) {
    std::cerr << "Error: Analysis failed" << std::endl;
    return 1;
  }

  // Print footer
  RDataFrameAnalysis::PrintFooter();

  // Run application to keep plots visible
  theApp.Run();

  return 0;
}
