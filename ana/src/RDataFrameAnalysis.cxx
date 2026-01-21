/*****************************************************
 * File: RDataFrameAnalysis.cxx
 * Description: Implementation of RDataFrame analysis class
 * Author: Kilo Code
 * Created: 2026/01/21
 ****************************************************/

#include "RDataFrameAnalysis.h"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"
#include "ROOT/TThreadExecutor.hxx"
#include "TCanvas.h"
#include "TCutG.h"
#include "TF1.h"
#include "TFile.h"
#include "TGraph.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TLegend.h"
#include "utils.h"
#include <iostream>
#include <map>
#include <memory>
#include <sstream>

// Constructor
RDataFrameAnalysis::RDataFrameAnalysis(ConfigReader &configReader)
    : m_configReader(configReader), m_chain(nullptr), m_isInitialized(false) {}

// Destructor
RDataFrameAnalysis::~RDataFrameAnalysis() {
  if (m_chain) {
    delete m_chain;
    m_chain = nullptr;
  }
}

bool RDataFrameAnalysis::InitializeTChain() {
  std::cout << "========================================" << std::endl;
  std::cout << "Initializing TChain" << std::endl;
  std::cout << "========================================" << std::endl;

  // Check if configuration is valid
  if (!m_configReader.IsValid()) {
    std::cerr << "Error: Configuration is not valid" << std::endl;
    return false;
  }

  // Create TChain with the root tree name from configuration
  const std::string &rootTreeName = m_configReader.GetRootTreeName();
  m_chain = new TChain(rootTreeName.c_str());

  if (!m_chain) {
    std::cerr << "Error: Failed to create TChain" << std::endl;
    return false;
  }

  // Get file list and root data path from configuration
  const std::vector<std::string> &fileList = m_configReader.GetFileList();
  const std::string &rootDataPath = m_configReader.GetRootDataPath();

  int successCount = 0;
  int failCount = 0;

  // Add files to TChain
  for (const auto &filename : fileList) {
    std::string fullPath = rootDataPath + filename;
    std::cout << "Adding file: " << fullPath << std::endl;

    int result = m_chain->Add(fullPath.c_str());
    if (result > 0) {
      successCount++;
      std::cout << "  -> Success (" << result << " trees added)" << std::endl;
    } else {
      failCount++;
      std::cerr << "  -> Failed to add file" << std::endl;
    }
  }

  std::cout << std::endl;
  std::cout << "Summary: " << successCount << " files added, " << failCount
            << " files failed" << std::endl;
  std::cout << "Total entries in chain: " << m_chain->GetEntries() << std::endl;
  std::cout << std::endl;

  // Check if TChain has entries
  if (m_chain->GetEntries() == 0) {
    std::cerr << "Error: No entries in TChain" << std::endl;
    delete m_chain;
    m_chain = nullptr;
    return false;
  }

  m_isInitialized = true;
  return true;
}

bool RDataFrameAnalysis::RunAnalysis() {
  // Initialize TChain if not already initialized
  if (!m_isInitialized) {
    if (!InitializeTChain()) {
      std::cerr << "Error: Failed to initialize TChain" << std::endl;
      return false;
    }
  }

  // Run the analysis
  try {
    // AnalyzeWithRDataFrame();
    // GetGammaFlash();
    GetThR1();
  } catch (const std::exception &e) {
    std::cerr << "Error during RDataFrame analysis: " << e.what() << std::endl;
    return false;
  }

  return true;
}

TChain *RDataFrameAnalysis::GetTChain() { return m_chain; }

const ConfigReader &RDataFrameAnalysis::GetConfigReader() const {
  return m_configReader;
}

void RDataFrameAnalysis::PrintHeader() {
  std::cout << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << "  ROOT RDataFrame Analysis Tool" << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << std::endl;
}

void RDataFrameAnalysis::PrintFooter() {
  std::cout << std::endl;
  std::cout << "Program completed successfully!" << std::endl;
  std::cout << std::endl;
}

void RDataFrameAnalysis::GetGammaFlash() {
  std::cout << "========================================" << std::endl;
  std::cout << "GetGammaFlash Analysis" << std::endl;
  std::cout << "========================================" << std::endl;

  // Enable multi-threading for RDataFrame
  ROOT::EnableImplicitMT();
  std::cout << "Multi-threading enabled" << std::endl;

  // Create RDataFrame from TChain
  ROOT::RDataFrame df(*m_chain);

  // Get channel IDs to use from configuration
  const std::vector<int> &channelIDs =
      m_configReader.GetFIXMConfig().Global.CHIDUSE;
  std::cout << "Number of channels to analyze: " << channelIDs.size()
            << std::endl;

  // Filter out events where hpn <= noiseCut
  double noiseCut = m_configReader.GetFIXMConfig().Global.NoiseCut;
  auto df_filtered =
      df.Filter([noiseCut](double fhpn) { return fhpn > noiseCut; }, {"fhpn"});

  // Calculate canvas layout
  int nChannels = channelIDs.size();

  // fit
  auto fitrange = m_configReader.GetFIXMConfig().Global.gammaFitRange;
  TF1 *f = new TF1("f1", "gaus", fitrange[0], fitrange[1]);
  f->SetParameter(2, 1.5);

  // Create canvas for each channel
  for (size_t i = 0; i < channelIDs.size(); ++i) {
    int chID = channelIDs[i];
    std::cout << "Processing channel " << chID << "..." << std::endl;

    // Create canvas for this channel
    std::ostringstream canvasName;
    canvasName << "GammaFlash_CH" << chID;
    std::ostringstream canvasTitle;
    canvasTitle << "Gamma Flash Analysis - Channel " << chID;

    TCanvas *c = new TCanvas(canvasName.str().c_str(),
                             canvasTitle.str().c_str(), 1200, 600);
    c->Divide(2, 1);

    // Define columns to extract specific channel data
    auto df_defined = df_filtered.Filter(
        [chID](int channelID) { return channelID == chID; }, {"fChannelID"});

    // Create TH1D: Time distribution
    std::ostringstream h1Name, h1Title;
    h1Name << "h1_time_CH" << chID;
    h1Title << "Time Distribution - Channel " << chID << ";Time (ns);Entries";
    // Fit the histogram and extract mean and sigma
    auto h1 = df_defined.Histo1D(
        {h1Name.str().c_str(), h1Title.str().c_str(), 3600, -1800, 0}, "fTc1");
    h1->Fit(f, "QRN");
    f->SetParameters(f->GetParameters());
    f->SetRange(f->GetParameter(1) - 2.355 * f->GetParameter(2),
                f->GetParameter(1) + 2.355 * f->GetParameter(2));

    // Output fit results
    std::cout << "Channel: " << chID << ", Mean: " << f->GetParameter(1)
              << ",  Sigma: " << f->GetParameter(2) << std::endl;

    // Create TH2D: Time vs Pulse Height
    std::ostringstream h2Name, h2Title;
    h2Name << "h2_time_hp_CH" << chID;
    h2Title << "Time vs Pulse Height - Channel " << chID
            << ";Time (ns);Pulse Height";

    auto h2 = df_defined.Histo2D({h2Name.str().c_str(), h2Title.str().c_str(),
                                  3600, -1800, 0, 2000, 0, 2000},
                                 "fTc1", "fhpn");

    // Draw TH1D on left pad
    c->cd(1);
    h1->SetLineColor(kBlue);
    h1->DrawCopy();
    f->DrawCopy("same");

    // Draw TH2D on right pad
    c->cd(2);
    h2->SetMarkerColor(kRed);
    h2->SetMarkerStyle(20);
    h2->SetMarkerSize(0.5);
    h2->DrawCopy("COLZ");

    c->Update();
    std::cout << "  Created histograms for channel " << chID << std::endl;
  }

  std::cout << std::endl;
  std::cout << "Close plot windows to continue..." << std::endl;
  std::cout << "Press Ctrl+C in terminal to exit..." << std::endl;
  std::cout << std::endl;
}

void RDataFrameAnalysis::GetThR1() {
  std::cout << "========================================" << std::endl;
  std::cout << "GetThR1 Analysis" << std::endl;
  std::cout << "========================================" << std::endl;

  // Enable multi-threading for RDataFrame
  ROOT::EnableImplicitMT();
  std::cout << "Multi-threading enabled" << std::endl;

  // Create RDataFrame from TChain
  ROOT::RDataFrame df(*m_chain);

  // Get channel IDs to use from configuration
  const std::vector<int> &channelIDs =
      m_configReader.GetFIXMConfig().Global.CHIDUSE;
  std::cout << "Number of channels to analyze: " << channelIDs.size()
            << std::endl;

  // Load TCutG cuts from file
  const std::string &xsPath = m_configReader.GetXSPath();
  const std::string &expName = m_configReader.GetExperimentName();
  std::string cutPath = xsPath + expName + "/cutgamma/my_cutg.root";

  TFile *finCut = TFile::Open(cutPath.c_str());
  if (!finCut || finCut->IsZombie()) {
    std::cerr << "Error: Failed to open cut file: " << cutPath << std::endl;
    return;
  }
  std::cout << "Loaded cut file: " << cutPath << std::endl;

  // Load TCutG for each channel
  std::map<int, TCutG *> m_cutg;
  const auto &fixmConfig = m_configReader.GetFIXMConfig();
  for (const auto &[chID, chConfig] : fixmConfig.Channels) {
    TCutG *cutg = nullptr;
    finCut->GetObject(Form("cutgt%d", chID), cutg);
    if (cutg != nullptr) {
      m_cutg[chID] = cutg;
      std::cout << "  Loaded cut for channel " << chID << std::endl;
    } else {
      std::cerr << "  Warning: No cut found for channel " << chID << std::endl;
    }
  }

  // Set up log bins for energy histogram
  constexpr int bpd = 100;
  constexpr int nDec = 10;
  constexpr int nbins = bpd * nDec;
  constexpr double LowEdge = -1.;

  std::vector<double> EnBins(nbins + 1);
  for (Int_t i = 0; i <= nbins; i++) {
    EnBins[i] = pow(10, LowEdge + 1. / bpd * (double)i);
  }

  // Store threshold results
  std::map<int, double> m_Threshold;

  // Process each channel
  for (size_t i = 0; i < channelIDs.size(); ++i) {
    int chID = channelIDs[i];
    std::cout << "Processing channel " << chID << "..." << std::endl;

    // Get channel configuration
    auto it = fixmConfig.Channels.find(chID);
    if (it == fixmConfig.Channels.end()) {
      std::cerr << "Warning: Channel " << chID << " not found in config"
                << std::endl;
      continue;
    }
    const auto &chConfig = it->second;
    double Tg = chConfig.Tg;
    double Length = chConfig.Length;

    // Create canvas for this channel
    std::ostringstream canvasName, canvasTitle;
    canvasName << "Threshold_CH" << chID;
    canvasTitle << "Threshold Analysis - Channel " << chID;

    TCanvas *c = new TCanvas(canvasName.str().c_str(),
                             canvasTitle.str().c_str(), 1200, 600);
    c->Divide(2, 1);

    // Filter by channel ID
    auto df_ch = df.Filter([chID](int channelID) { return channelID == chID; },
                           {"fChannelID"});

    // Define neutron energy: En = calEn(tof, Length) where tof = fT0 - Tg +
    // Length/cspeed
    auto df_withEn =
        df_ch
            .Define(
                "tof",
                [Tg, Length](double fT0) { return fT0 - Tg + Length / cspeed; },
                {"fT0"})
            .Define("En", [Length](double tof) { return calEn(tof, Length); },
                    {"tof"});

    // Filter using TCutG (exclude points inside the cut)
    TCutG *cutg = m_cutg[chID];
    auto df_filtered = df_withEn.Filter(
        [cutg](double fT0, double fhpn) {
          if (cutg == nullptr)
            return true;
          return !cutg->IsInside(fT0, fhpn);
        },
        {"fT0", "fhpn"});

    // Create TH2D: Energy vs Amplitude
    std::ostringstream h2Name, h2Title;
    h2Name << "hEn2d_" << chID;
    h2Title << "Energy vs Amplitude - Channel " << chID
            << ";Neutron Energy (eV);Amplitude(abs)";

    auto h2 = df_filtered.Histo2D({h2Name.str().c_str(), h2Title.str().c_str(),
                                   nbins, EnBins.data(), 2000, 0, 4000},
                                  "En", "fhpn");

    // Project to Y axis to get amplitude distribution
    auto h1 = h2->ProjectionY();

    // Find minimum in range [50, 200]
    int minBin = GetHistogramMinBinInRange(h1, 50, 200);
    double minvalue = h1->GetBinCenter(minBin);

    // Store threshold
    m_Threshold[chID] = minvalue;

    // Draw TH2D on left pad with log scale
    c->cd(1);
    gPad->SetLogz();
    gPad->SetLogx();
    h2->DrawCopy("colz");

    // Draw TH1D on right pad with threshold line
    c->cd(2);
    h1->DrawCopy("hist");

    double x[2] = {minvalue, minvalue};
    double y[2] = {h1->GetMaximum(), 0};
    auto g = new TGraph(2, x, y);
    g->SetLineColor(kBlack);
    g->Draw("l same");

    // Add legend
    auto legd = new TLegend();
    legd->AddEntry(h1, Form("CHID = %d", chID));
    legd->Draw();

    c->Update();
    std::cout << "  Channel " << chID << " threshold: " << minvalue
              << std::endl;
  }

  // Print summary
  std::cout << std::endl;
  std::cout << "CHID = ";
  for (const auto &[chID, threshold] : m_Threshold) {
    std::cout << chID << ", ";
  }
  std::cout << std::endl;
  std::cout << "Threshold = ";
  for (const auto &[chID, threshold] : m_Threshold) {
    std::cout << threshold << ", ";
  }
  std::cout << std::endl;

  // Clean up
  finCut->Close();

  std::cout << std::endl;
  std::cout << "Close plot windows to continue..." << std::endl;
  std::cout << "Press Ctrl+C in terminal to exit..." << std::endl;
  std::cout << std::endl;
}

void RDataFrameAnalysis::AnalyzeWithRDataFrame() {
  // Enable multi-threading for RDataFrame
  ROOT::EnableImplicitMT();

  // Create RDataFrame from TChain
  ROOT::RDataFrame df(*m_chain);

  // Display available columns
  auto columnNames = df.GetColumnNames();
  std::cout << "Available columns:" << std::endl;
  for (const auto &col : columnNames) {
    std::cout << "  - " << col << std::endl;
  }
  std::cout << std::endl;

  // Display various statistics
  DisplayBasicStatistics(df);
  DisplayChannelStatistics(df);
  DisplayTimeRangeStatistics(df);
  DisplayFirstEvents(df, 5);

  // Display plots
  DisplayPlots(df);
}

void RDataFrameAnalysis::DisplayBasicStatistics(ROOT::RDataFrame &df) {
  std::cout << "Basic Statistics:" << std::endl;
  std::cout << "  Total entries: " << *df.Count() << std::endl;

  // Calculate array length statistics
  auto arrayLengthStats = df.Stats("fArrayLength");
  std::cout << std::endl;
  std::cout << "Array Length Statistics:" << std::endl;
  std::cout << "  Mean:   " << arrayLengthStats->GetMean() << std::endl;
  std::cout << "  Min:    " << arrayLengthStats->GetMin() << std::endl;
  std::cout << "  Max:    " << arrayLengthStats->GetMax() << std::endl;
  std::cout << "  StdDev: " << arrayLengthStats->GetRMS() << std::endl;
  std::cout << std::endl;
}

void RDataFrameAnalysis::DisplayChannelStatistics(ROOT::RDataFrame &df) {
  std::cout << "Channel Statistics:" << std::endl;

  // Get channel ID min and max for bin range
  auto channelMin = df.Min("fvChannelID");
  auto channelMax = df.Max("fvChannelID");
  std::cout << "  Min Channel ID: " << *channelMin << std::endl;
  std::cout << "  Max Channel ID: " << *channelMax << std::endl;

  // Create channel distribution histogram
  auto channelStats =
      df.Histo1D({"channelHist", "Channel Distribution;Channel;Entries",
                  (int)*channelMax - (int)*channelMin + 1, *channelMin - 0.5,
                  *channelMax + 0.5},
                 "fvChannelID");

  std::cout << std::endl;
}

void RDataFrameAnalysis::DisplayTimeRangeStatistics(ROOT::RDataFrame &df) {
  auto t0SecMin = df.Min("fT0Sec");
  auto t0SecMax = df.Max("fT0Sec");
  std::cout << "Time Range (seconds):" << std::endl;
  std::cout << "  Min: " << *t0SecMin << std::endl;
  std::cout << "  Max: " << *t0SecMax << std::endl;
  std::cout << std::endl;
}

void RDataFrameAnalysis::DisplayFirstEvents(ROOT::RDataFrame &df,
                                            int numEvents) {
  std::cout << "First " << numEvents << " Events:" << std::endl;
  auto displayEvents = df.Range(0, numEvents);

  auto eventNums = displayEvents.Take<Int_t>("fEventNumber");
  auto arrayLens = displayEvents.Take<Int_t>("fArrayLength");

  for (size_t i = 0; i < eventNums->size(); ++i) {
    std::cout << "  Event " << i << ":" << std::endl;
    std::cout << "    Event Number:    " << (*eventNums)[i] << std::endl;
    std::cout << "    Array Length:    " << (*arrayLens)[i] << std::endl;
  }
  std::cout << std::endl;
}

void RDataFrameAnalysis::DisplayPlots(ROOT::RDataFrame &df) {
  std::cout << "========================================" << std::endl;
  std::cout << "Displaying Plots" << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << "Close plot windows to continue..." << std::endl;

  // Get time range for T0 histogram
  auto t0SecMin = df.Min("fT0Sec");
  auto t0SecMax = df.Max("fT0Sec");

  // Get channel range for channel histogram
  auto channelMin = df.Min("fvChannelID");
  auto channelMax = df.Max("fvChannelID");

  // Create T0 distribution histogram
  auto t0Hist = df.Histo1D({"t0Hist", "T0 Distribution;T0 (seconds);Entries",
                            100, *t0SecMin, *t0SecMax},
                           "fT0Sec");

  // Create channel distribution histogram
  auto channelHist = df.Histo1D(
      {"channelHist", "Channel Distribution;Channel;Entries",
       *channelMax - *channelMin + 1, *channelMin - 0.5, *channelMax + 0.5},
      "fvChannelID");

  // Create canvas with both plots side by side
  TCanvas *c = new TCanvas("c", "Combined Plots - Channel and T0 Distribution",
                           1200, 500);
  c->Divide(2, 1);

  // Left pad: Channel distribution
  c->cd(1);
  channelHist->SetFillColor(kBlue);
  channelHist->SetLineColor(kBlue);
  channelHist->DrawCopy("");

  // Right pad: T0 distribution
  c->cd(2);
  t0Hist->SetFillColor(kRed);
  t0Hist->SetLineColor(kRed);
  t0Hist->DrawCopy("");

  c->Update();

  // Wait for user to close the canvas
  std::cout << "Press Ctrl+C in terminal to exit, or close plot window..."
            << std::endl;
}
