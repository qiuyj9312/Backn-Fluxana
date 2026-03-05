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
#include "TVirtualPad.h"
#include "utils.h"
#include <cfloat>
#include <climits>
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

bool RDataFrameAnalysis::RunAnalysis(const std::string &analysisType) {
  // Initialize TChain if not already initialized
  if (!m_isInitialized) {
    if (!InitializeTChain()) {
      std::cerr << "Error: Failed to initialize TChain" << std::endl;
      return false;
    }
  }

  // Run the analysis based on the specified type
  try {
    if (analysisType == "GetGammaFlash") {
      GetGammaFlash();
    } else if (analysisType == "GetThR1") {
      GetThR1();
    } else if (analysisType == "GetThR2") {
      GetThR2();
    } else if (analysisType == "GetReactionRate") {
      GetReactionRate();
    } else if (analysisType == "GetDtForCalL") {
      GetDtForCalL();
    } else if (analysisType == "CalFlightPath") {
      CalFlightPath();
    } else if (analysisType == "GetPileupCorr") {
      GetPileupCorr();
    } else if (analysisType == "CoincheckSimple") {
      CoincheckSimple();
    } else if (analysisType == "CoincheckSingleBunchSimple") {
      CoincheckSingleBunchSimple();
    } else if (analysisType == "CountT0") {
      CountT0();
    } else if (analysisType == "GetHRateXSUF") {
      GetHRateXSUF();
    } else if (analysisType == "AnalyzeWithRDataFrame") {
      AnalyzeWithRDataFrame();
    } else {
      std::cerr << "Error: Unknown analysis type: " << analysisType
                << std::endl;
      std::cerr << "Available analysis types:" << std::endl;
      std::cerr << "  - GetGammaFlash" << std::endl;
      std::cerr << "  - GetThR1" << std::endl;
      std::cerr << "  - GetThR2" << std::endl;
      std::cerr << "  - GetReactionRate" << std::endl;
      std::cerr << "  - GetDtForCalL" << std::endl;
      std::cerr << "  - CalFlightPath" << std::endl;
      std::cerr << "  - GetPileupCorr" << std::endl;
      std::cerr << "  - CoincheckSimple" << std::endl;
      std::cerr << "  - CountT0" << std::endl;
      std::cerr << "  - GetHRateXSUF" << std::endl;
      std::cerr << "  - AnalyzeWithRDataFrame" << std::endl;
      return false;
    }
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

// ========== 辅助方法实现 ==========

// 初始化辅助方法
void RDataFrameAnalysis::InitializeCommonConfig() {
  m_fixmConfig = &m_configReader.GetFIXMConfig();
  m_channelIDs = m_fixmConfig->Global.CHIDUSE;
  if (m_configReader.GetDataType() == "Flux") {
    m_outputPath = m_configReader.GetFluxPath();
  } else {
    m_outputPath = m_configReader.GetXSPath();
  }
  m_expName = m_configReader.GetExperimentName();
}

void RDataFrameAnalysis::InitializeEnergyBins() {
  const auto &binConfig = m_fixmConfig->Global.Bin;
  m_bpd = binConfig.bpd;
  m_nDec = binConfig.nDec;
  m_nbins = m_bpd * m_nDec;
  m_lowEdge = binConfig.LowEdge;
  m_EnBins = SetLogBins(m_nDec, m_bpd, m_lowEdge);
}

bool RDataFrameAnalysis::LoadGammaCuts() {
  m_cutgMap.clear();
  for (const auto &[chID, chConfig] : m_fixmConfig->Channels) {
    std::string cutfilename = Form("/cutgamma/ch%d.root", chID);
    std::string cutPath = m_outputPath + m_expName + cutfilename;
    TFile *finCut = TFile::Open(cutPath.c_str());

    if (!finCut || finCut->IsZombie()) {
      std::cerr << "Error: Failed to open cut file: " << cutPath << std::endl;
      return false;
    }
    std::cout << "Loaded cut file: " << cutPath << std::endl;

    TCutG *cutg = nullptr;
    finCut->GetObject("CUTG", cutg);
    if (cutg != nullptr) {
      m_cutgMap[chID] = cutg;
      std::cout << "  Loaded cut for channel " << chID << std::endl;
    } else {
      std::cerr << "  Warning: No cut found for channel " << chID << std::endl;
    }
  }
  return true;
}

bool RDataFrameAnalysis::LoadENDFData() {
  // Clear existing data
  m_xs_nr.clear();
  m_xs_ntot.clear();

  // Load U-235 fission cross section
  auto gENDFU5NF = new TGraph();
  get_graph(m_configReader.GetENDFDataU5NF().c_str(), gENDFU5NF, MeV_to_eV);
  m_xs_nr["235U"] = gENDFU5NF;

  // Load U-235 total cross section
  auto gENDFU5NTOT = new TGraph();
  get_graph(m_configReader.GetENDFDataU5NTOT().c_str(), gENDFU5NTOT, MeV_to_eV);
  m_xs_ntot["235U"] = gENDFU5NTOT;

  // Load U-238 fission cross section
  auto gENDFU8NF = new TGraph();
  get_graph(m_configReader.GetENDFDataU8NF().c_str(), gENDFU8NF, MeV_to_eV);
  m_xs_nr["238U"] = gENDFU8NF;

  // Load U-238 total cross section
  auto gENDFU8NTOT = new TGraph();
  get_graph(m_configReader.GetENDFDataU8NTOT().c_str(), gENDFU8NTOT, MeV_to_eV);
  m_xs_ntot["238U"] = gENDFU8NTOT;

  // Load Li-6 cross section
  auto gENDFLi6NT = new TGraph();
  get_graph(m_configReader.GetENDFDataLi6NT().c_str(), gENDFLi6NT, MeV_to_eV);
  m_xs_nr["6Li"] = gENDFLi6NT;

  // Load Li-6 total cross section
  auto gENDFLi6NTOT = new TGraph();
  get_graph(m_configReader.GetENDFDataLi6NTOT().c_str(), gENDFLi6NTOT,
            MeV_to_eV);
  m_xs_ntot["6Li"] = gENDFLi6NTOT;

  std::cout << "Loaded ENDF data files:" << std::endl;
  std::cout << "  235U fission cross section: "
            << m_configReader.GetENDFDataU5NF() << std::endl;
  std::cout << "  235U total cross section: "
            << m_configReader.GetENDFDataU5NTOT() << std::endl;
  std::cout << "  238U fission cross section: "
            << m_configReader.GetENDFDataU8NF() << std::endl;
  std::cout << "  238U total cross section: "
            << m_configReader.GetENDFDataU8NTOT() << std::endl;
  std::cout << "  6Li n,t cross section: " << m_configReader.GetENDFDataLi6NT()
            << std::endl;
  std::cout << "  6Li total cross section: "
            << m_configReader.GetENDFDataLi6NTOT() << std::endl;

  return true;
}

// 输出辅助方法
void RDataFrameAnalysis::PrintSectionHeader(const std::string &title) {
  std::cout << SEPARATOR << std::endl;
  std::cout << title << std::endl;
  std::cout << SEPARATOR << std::endl;
}

void RDataFrameAnalysis::PrintClosePrompt() {
  std::cout << std::endl;
  std::cout << CLOSE_PROMPT << std::endl;
  std::cout << EXIT_PROMPT << std::endl;
  std::cout << std::endl;
}

void RDataFrameAnalysis::PrintChannelInfo(int chID) {
  std::cout << "Processing channel " << chID << "..." << std::endl;
}

// RDataFrame 辅助方法
void RDataFrameAnalysis::EnableMultiThreading() {
  ROOT::EnableImplicitMT();
  std::cout << "Multi-threading enabled" << std::endl;
}

ROOT::RDataFrame RDataFrameAnalysis::CreateDataFrame() {
  return ROOT::RDataFrame(*m_chain);
}

// 过滤器辅助方法
ROOT::RDF::RNode RDataFrameAnalysis::FilterByChannel(ROOT::RDF::RNode df,
                                                     int chID) {
  return df.Filter([chID](int channelID) { return channelID == chID; },
                   {"fChannelID"});
}

ROOT::RDF::RNode RDataFrameAnalysis::FilterByThreshold(ROOT::RDF::RNode df,
                                                       double threshold) {
  return df.Filter([threshold](double fhpn) { return fhpn > threshold; },
                   {"fhpn"});
}

ROOT::RDF::RNode RDataFrameAnalysis::FilterByGammaCut(ROOT::RDF::RNode df,
                                                      int chID) {
  auto it = m_cutgMap.find(chID);
  if (it == m_cutgMap.end()) {
    return df; // 没有 cut,返回原始 dataframe
  }

  TCutG *cutg = it->second;
  return df.Filter(
      [cutg](double fTc1, double fhpn) {
        if (cutg == nullptr)
          return true;
        return !cutg->IsInside(fTc1, fhpn);
      },
      {"fTc1", "fhpn"});
}

// 列定义辅助方法
ROOT::RDF::RNode RDataFrameAnalysis::DefineTOF(ROOT::RDF::RNode df, double Tg,
                                               double Length) {
  return df.Define(
      "tof", [Tg, Length](double fTc1) { return fTc1 - Tg + Length / cspeed; },
      {"fTc1"});
}

ROOT::RDF::RNode RDataFrameAnalysis::DefineEnergy(ROOT::RDF::RNode df,
                                                  double Length) {
  return df.Define("En", [Length](double tof) { return calEn(tof, Length); },
                   {"tof"});
}

ROOT::RDF::RNode RDataFrameAnalysis::DefineTOFAndEnergy(ROOT::RDF::RNode df,
                                                        double Tg,
                                                        double Length) {
  auto df_tof = DefineTOF(df, Tg, Length);
  return DefineEnergy(df_tof, Length);
}

// 画布辅助方法
std::string RDataFrameAnalysis::MakeCanvasName(const std::string &prefix,
                                               int chID) {
  std::ostringstream oss;
  oss << prefix << "_CH" << chID;
  return oss.str();
}

std::string RDataFrameAnalysis::MakeCanvasTitle(const std::string &title,
                                                int chID) {
  std::ostringstream oss;
  oss << title << " - Channel " << chID;
  return oss.str();
}

void RDataFrameAnalysis::GetGammaFlash() {
  PrintSectionHeader("GetGammaFlash Analysis");

  EnableMultiThreading();
  InitializeCommonConfig();

  auto df = CreateDataFrame();

  std::cout << "Number of channels to analyze: " << m_channelIDs.size()
            << std::endl;

  // Filter out events where hpn <= noiseCut
  double noiseCut = m_fixmConfig->Global.NoiseCut;
  auto df_filtered = FilterByThreshold(df, noiseCut);

  // Fit function
  auto fitrange = m_fixmConfig->Global.gammaFitRange;
  TF1 *f = new TF1("f1", "gaus", fitrange[0], fitrange[1]);
  f->SetParameter(2, 1.5);

  // Create canvas for each channel
  for (int chID : m_channelIDs) {
    PrintChannelInfo(chID);

    // Create canvas
    TCanvas *c = new TCanvas(
        MakeCanvasName("GammaFlash", chID).c_str(),
        MakeCanvasTitle("Gamma Flash Analysis", chID).c_str(), 1200, 600);
    c->Divide(2, 1);

    // Filter by channel
    auto df_ch = FilterByChannel(df_filtered, chID);

    // Create TH1D: Time distribution
    auto h1 = df_ch.Histo1D(
        {Form("h1_time_CH%d", chID),
         Form("Time Distribution - Channel %d;Time (ns);Entries", chID), 3600,
         -1800, 0},
        "fTc1");

    // Fit the histogram and extract mean and sigma
    h1->Fit(f, "QRN");
    f->SetParameters(f->GetParameters());
    f->SetRange(f->GetParameter(1) - 2.355 * f->GetParameter(2),
                f->GetParameter(1) + 2.355 * f->GetParameter(2));

    // Output fit results
    std::cout << "Channel: " << chID << ", Mean: " << f->GetParameter(1)
              << ",  Sigma: " << f->GetParameter(2) << std::endl;

    // Create TH2D: Time vs Pulse Height
    auto h2 = df_ch.Histo2D(
        {Form("h2_time_hp_CH%d", chID),
         Form("Time vs Pulse Height - Channel %d;Time (ns);Pulse Height", chID),
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

  PrintClosePrompt();
}

void RDataFrameAnalysis::GetThR1() {
  PrintSectionHeader("GetThR1 Analysis");

  EnableMultiThreading();
  InitializeCommonConfig();
  InitializeEnergyBins();

  if (!LoadGammaCuts()) {
    return;
  }

  std::cout << "Number of channels to analyze: " << m_channelIDs.size()
            << std::endl;

  auto df = CreateDataFrame();
  std::map<int, double> m_Threshold;

  // Process each channel
  for (int chID : m_channelIDs) {
    PrintChannelInfo(chID);

    // Get channel configuration
    auto it = m_fixmConfig->Channels.find(chID);
    if (it == m_fixmConfig->Channels.end()) {
      std::cerr << "Warning: Channel " << chID << " not found in config"
                << std::endl;
      continue;
    }
    const auto &chConfig = it->second;

    // Create canvas
    TCanvas *c = new TCanvas(
        MakeCanvasName("Threshold", chID).c_str(),
        MakeCanvasTitle("Threshold Analysis", chID).c_str(), 1200, 600);
    c->Divide(2, 1);

    // Apply filters and define columns
    auto df_processed = FilterByChannel(df, chID);
    df_processed =
        DefineTOFAndEnergy(df_processed, chConfig.Tg, chConfig.Length);
    df_processed = FilterByGammaCut(df_processed, chID);

    // Create TH2D: Energy vs Amplitude
    auto h2 =
        df_processed.Histo2D({Form("hEn2d_%d", chID),
                              Form("Energy vs Amplitude - Channel %d;Neutron "
                                   "Energy (eV);Amplitude(abs)",
                                   chID),
                              m_nbins, m_EnBins.data(), 2000, 0, 4000},
                             "En", "fhpn");

    // Project to Y axis to get amplitude distribution
    auto h1 = h2->ProjectionY();

    // Find minimum in range from configuration
    const auto &thFindminRange = m_fixmConfig->Global.ThFindminRange;
    int minBin =
        GetHistogramMinBinInRange(h1, thFindminRange[0], thFindminRange[1]);
    double minvalue = h1->GetBinCenter(minBin);
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

  PrintClosePrompt();
}

void RDataFrameAnalysis::GetThR2() {
  PrintSectionHeader("GetThR2 Analysis");

  EnableMultiThreading();
  InitializeCommonConfig();
  InitializeEnergyBins();

  if (!LoadGammaCuts()) {
    return;
  }

  std::cout << "Number of channels to analyze: " << m_channelIDs.size()
            << std::endl;

  auto df = CreateDataFrame();

  // Get energy divide from configuration
  const std::vector<double> &energyDivide = m_fixmConfig->Global.EnergyDivide;

  // Create fit functions for each channel
  std::map<int, TF1 *> m_fitfunc;
  for (int chID : m_channelIDs) {
    TF1 *f = new TF1(Form("f_%d", chID), "expo");
    m_fitfunc[chID] = f;
  }

  // Process each channel
  for (int chID : m_channelIDs) {
    PrintChannelInfo(chID);

    // Get channel configuration
    auto it = m_fixmConfig->Channels.find(chID);
    if (it == m_fixmConfig->Channels.end()) {
      std::cerr << "Warning: Channel " << chID << " not found in config"
                << std::endl;
      continue;
    }
    const auto &chConfig = it->second;
    double Threshold = chConfig.Threshold;
    const std::vector<double> &ThresholdRe = chConfig.ThresholdRe;
    const std::vector<double> &ThresholdFitcut = chConfig.ThresholdFitcut;

    // Create canvas
    TCanvas *c = new TCanvas(
        MakeCanvasName("ThR2", chID).c_str(),
        MakeCanvasTitle("Threshold R2 Analysis", chID).c_str(), 1200, 800);
    c->Divide(2, 2);

    // Apply filters and define columns
    auto df_processed = FilterByChannel(df, chID);
    df_processed =
        DefineTOFAndEnergy(df_processed, chConfig.Tg, chConfig.Length);
    // Note: FilterByGammaCut is commented out in original code (returns true)
    // df_processed = FilterByGammaCut(df_processed, chID);

    // Create TH2D: Energy vs Amplitude
    auto h2 =
        df_processed.Histo2D({Form("hEn2d_%d", chID),
                              Form("Energy vs Amplitude - Channel %d;Neutron "
                                   "Energy (eV);Amplitude(abs)",
                                   chID),
                              m_nbins, m_EnBins.data(), 2000, 0, 4000},
                             "En", "fhpn");

    // Draw TH2D on pad 1 with log scale
    c->cd(1);
    gPad->SetLogz();
    gPad->SetLogx();
    h2->DrawCopy("colz");

    // Add legend
    auto legd = new TLegend();
    legd->AddEntry(h2.GetPtr(), Form("CHID = %d", chID));
    legd->Draw();

    // Process each energy divide part
    for (size_t j = 0; j < energyDivide.size(); j++) {
      // Get threshold values for this energy range
      double thresholdRe =
          (j < ThresholdRe.size()) ? ThresholdRe[j] : Threshold;
      double thresholdFitcut =
          (j < ThresholdFitcut.size()) ? ThresholdFitcut[j] : thresholdRe;

      // Project to Y axis to get amplitude distribution
      TH1D *hd;
      if (j < energyDivide.size() - 1) {
        hd = h2->ProjectionY(Form("hd_%d_%zu", chID, j + 1),
                             h2->GetXaxis()->FindBin(energyDivide[j]) - 1,
                             h2->GetXaxis()->FindBin(energyDivide[j + 1]));
      } else {
        hd = h2->ProjectionY(Form("hd_%d_%zu", chID, j + 1),
                             h2->GetXaxis()->FindBin(energyDivide[j]) - 1,
                             h2->GetNbinsX());
      }
      hd->SetTitle(
          Form("Amplitude Distribution Part %zu;Amplitude;Counts", j + 1));

      // Draw histogram on pad (j+2)
      c->cd(j + 2);
      gPad->SetLogy();
      hd->Draw("hist");

      // Find max bin in range [thresholdRe, 2000]
      auto maxBin = GetHistogramMaxBinInRange(hd, thresholdRe, 2000);
      auto peakVal = hd->GetBinCenter(maxBin);
      auto maxVal = hd->GetBinContent(maxBin);
      auto binCut = hd->FindBin(thresholdRe);
      auto minVal = hd->GetBinContent(binCut);

      // Fit with exponential in range [thresholdRe, peakVal]
      TF1 *f = m_fitfunc[chID];
      f->SetRange(thresholdRe, peakVal);
      hd->Fit(f, "R0Q");

      // Calculate mean x from fit
      auto meanX = (log((maxVal + minVal) / 2.) - f->GetParameter(0)) /
                   f->GetParameter(1);

      std::cout << Form("%f\t%f\t%f", thresholdRe, peakVal, meanX) << std::endl;

      // Fit again in range [thresholdRe, thresholdFitcut]
      f->SetRange(thresholdRe, thresholdFitcut);
      hd->Fit(f, "R0Q");

      // Create function histogram
      auto hfun = (TH1D *)hd->Clone(Form("hfun_%d_%zu", chID, j + 1));
      auto f11 = (TF1 *)f->Clone(Form("f_%d_%zu", chID, j));
      f11->DrawCopy("same");

      // Fill hfun with function values
      for (int k = 0; k < hfun->GetNbinsX(); k++) {
        hfun->SetBinContent(k + 1, f11->Eval(hfun->GetBinCenter(k + 1)));
      }

      // Calculate integrals
      auto intg1 = hfun->Integral(0, binCut - 1);
      auto intg2 = hd->Integral(binCut, hd->GetNbinsX());
      auto ratio = intg2 / (intg1 + intg2);

      std::cout << Form("%d\t%zu\tpara1\t%f\tpara2\t%f\tratio\t%f", chID, j,
                        f11->GetParameter(0), f11->GetParameter(1), ratio)
                << std::endl;

      // Add legend
      legd = new TLegend();
      legd->AddEntry(hd, Form("Part %zu", j + 1));
      legd->Draw();
    }

    c->Update();
    std::cout << "  Created histograms for channel " << chID << std::endl;
  }

  PrintClosePrompt();
}

void RDataFrameAnalysis::GetReactionRate() {
  PrintSectionHeader("GetReactionRate Analysis");

  EnableMultiThreading();
  InitializeCommonConfig();
  InitializeEnergyBins();

  if (!LoadGammaCuts()) {
    return;
  }

  std::cout << "Number of channels to analyze: " << m_channelIDs.size()
            << std::endl;

  auto df = CreateDataFrame();

  // Get energy divide from configuration
  const std::vector<double> &energyDivide = m_fixmConfig->Global.EnergyDivide;

  // Load Delta L data
  const std::string &deltaLPath = m_configReader.GetDeltaLData();
  auto g_endeltaL = new TGraph();
  get_graph(deltaLPath.c_str(), g_endeltaL);
  std::cout << "Loaded Delta L data: " << deltaLPath << std::endl;

  // Calculate average Delta L
  const std::vector<double> &LCalEn =
      m_configReader.GetFIXMConfig().Global.LCalEn;
  double SumDL = 0.;
  for (const auto &c : LCalEn) {
    SumDL = SumDL + g_endeltaL->Eval(c);
  }
  double AvgDL = SumDL / LCalEn.size();
  std::cout << "Average Delta L: " << AvgDL << std::endl;

  // Load ENDF data
  if (!LoadENDFData()) {
    std::cerr << "Error: Failed to load ENDF data" << std::endl;
    return;
  }

  // Open output file for writing
  std::string outcomePath = m_outputPath + m_expName + "/Outcome";
  std::string outpath = outcomePath + "/hrate.root";

  auto f = new TFile(outpath.c_str(), "recreate");

  // Process each channel: create histograms, process data, and store results
  for (int chID : m_channelIDs) {
    auto it = m_fixmConfig->Channels.find(chID);
    if (it == m_fixmConfig->Channels.end()) {
      std::cerr << "Warning: Channel " << chID << " not found in config"
                << std::endl;
      continue;
    }
    const auto &chConfig = it->second;
    double Tg = chConfig.Tg;
    double Length = chConfig.Length;
    double Lengthgeo = Length - AvgDL;
    double Threshold = chConfig.Threshold;
    const std::vector<double> &ThresholdEDivide = chConfig.ThresholdEDivide;
    std::string sampletype = chConfig.SampleType;

    PrintChannelInfo(chID);

    // Calculate areal density
    double sample_r = chConfig.Radius * mm_to_cm; // to cm
    double sample_area = sample_r * sample_r * TMath::Pi();
    double sample_t = chConfig.Mass / sample_area; // mg/cm2
    double ArealDensity =
        sample_t / chConfig.A * Na * barn_to_cm2 * mg_to_g; // atoms/barn
    double nd = ArealDensity;

    // Apply filters and define columns
    auto df_processed = FilterByChannel(df, chID);
    df_processed = FilterByGammaCut(df_processed, chID);
    df_processed = DefineTOFAndEnergy(df_processed, Tg, Length);

    // Fill 2D histograms using RDataFrame
    auto h2t = df_processed.Histo2D(
        {Form("h2_tof_%d", chID), Form("h2dt%d;TOF(ns);Amplitude(abs)", chID),
         m_nbins, m_EnBins.data(), 2000, 0, 2000},
        "tof", "fhpn");
    auto h2E = df_processed.Histo2D(
        {Form("h2_En_%d", chID),
         Form("h2dEn%d;Neutron Energy(eV);Amplitude(abs)", chID), m_nbins,
         m_EnBins.data(), 2000, 0, 2000},
        "En", "fhpn");

    // Filter by threshold and apply energy divide factors
    auto df_threshold = df_processed.Filter(
        [Threshold](double fhpn) { return fhpn > Threshold; }, {"fhpn"});

    auto df_withFactor = df_threshold.Define(
        "factor",
        [energyDivide, ThresholdEDivide](double En) {
          double factor = 1.;
          if (En >= energyDivide[0] && En < energyDivide[1]) {
            factor = 1. / ThresholdEDivide[0];
          } else if (En >= energyDivide[1] && En < energyDivide[2]) {
            factor = 1. / ThresholdEDivide[1];
          } else if (En >= energyDivide[2]) {
            factor = 1. / ThresholdEDivide[2];
          }
          return factor;
        },
        {"En"});

    // Fill 1D histograms using RDataFrame
    auto h1t = df_withFactor.Histo1D({Form("h1_tof_%d", chID),
                                      Form("htrate%d;TOF(ns);Counts", chID),
                                      10004000, 0, 10004000},
                                     "tof", "factor");
    auto h1E = df_withFactor.Histo1D(
        {Form("h1_En_%d", chID),
         Form("hEnrate%d;Neutron Energy(eV);Counts", chID), m_nbins,
         m_EnBins.data()},
        "En", "factor");

    // Calculate cross section and yield
    auto df_withXS = df_withFactor.Define(
        "factor_xs_nr",
        [sampletype, xs_nr = m_xs_nr.at(sampletype)](double En, double factor) {
          return factor / xs_nr->Eval(En);
        },
        {"En", "factor"});
    // m_h1ExsNs[fChannelID]->Fill(En, factors / yield / xs_nf * xs_ntot);
    auto df_withYield = df_withXS.Define(
        "factor_xs_yield",
        [nd, sampletype,
         xs_ntot = m_xs_ntot.at(sampletype)](double En, double factor_xs_nf) {
          return factor_xs_nf / (1. - exp(-nd * xs_ntot->Eval(En))) /
                 xs_ntot->Eval(En);
        },
        {"En", "factor_xs_nr"});

    // Fill cross section histograms using RDataFrame
    auto h1Exs = df_withYield.Histo1D(
        {Form("h1_Enxs_%d", chID),
         Form("hEnxsrate%d;Neutron Energy(eV);Counts/barn", chID), m_nbins,
         m_EnBins.data()},
        "En", "factor_xs_nr");

    auto h1ExsNs = df_withYield.Histo1D(
        {Form("h1_EnxsNs_%d", chID),
         Form("h1EnxsNsrate%d;Neutron Energy(eV);Neutrons", chID), m_nbins,
         m_EnBins.data()},
        "En", "factor_xs_yield");

    // Write histograms to file directly
    h2t->Write();
    h2E->Write();
    h1t->Write();
    h1E->Write();
    h1Exs->Write();
    h1ExsNs->Write();

    std::cout << "  Processed channel " << chID << std::endl;
  }

  // Close output file
  f->Close();

  std::cout << std::endl;
  std::cout << "Output file saved: " << outpath << std::endl;
  std::cout << std::endl;
}

void RDataFrameAnalysis::GetDtForCalL() {
  PrintSectionHeader("GetDtForCalL Analysis");

  EnableMultiThreading();
  InitializeCommonConfig();

  if (!LoadGammaCuts()) {
    return;
  }

  std::cout << "Number of channels to analyze: " << m_channelIDs.size()
            << std::endl;

  auto df = CreateDataFrame();

  // Get LengthSet and LCalEn from configuration
  double LengthSet = m_fixmConfig->Global.LengthSet;
  const std::vector<double> &LCalEn = m_fixmConfig->Global.LCalEn;

  std::cout << "LengthSet: " << LengthSet << std::endl;
  std::cout << "Number of calibration energies: " << LCalEn.size() << std::endl;

  // Process each channel
  for (int chID : m_channelIDs) {
    PrintChannelInfo(chID);

    // Get channel configuration
    auto it = m_fixmConfig->Channels.find(chID);
    if (it == m_fixmConfig->Channels.end()) {
      std::cerr << "Warning: Channel " << chID << " not found in config"
                << std::endl;
      continue;
    }
    const auto &chConfig = it->second;

    // Create canvas
    TCanvas *c = new TCanvas(
        MakeCanvasName("DtForCalL", chID).c_str(),
        MakeCanvasTitle("Dt For Calibration Length", chID).c_str(), 1200, 600);

    // Apply filters
    auto df_processed = FilterByChannel(df, chID);
    df_processed = FilterByThreshold(df_processed, chConfig.Threshold);
    df_processed = FilterByGammaCut(df_processed, chID);

    // Create TH1D: Time difference (T-Tg) distribution
    auto h1 = df_processed.Histo1D(
        {Form("hDt_CH%d", chID),
         Form("T-Tg Distribution - Channel %d;T-T_g (ns);Counts", chID), 1500,
         5e5, 2e6},
        "fTc1");

    // Create TGraph for calibration points
    auto g1 = new TGraph();
    g1->SetMarkerStyle(20);
    g1->SetMarkerColor(kRed);
    g1->SetMarkerSize(2);

    // Add calibration points: caldT(LCalEn[i], LengthSet) vs histogram maximum
    for (size_t j = 0; j < LCalEn.size(); j++) {
      double energy = LCalEn[j];
      double x[1] = {energy};
      double par[1] = {LengthSet};
      double dt = caldT(x, par);
      g1->AddPoint(dt, h1->GetMaximum());
    }

    // Draw histogram
    h1->DrawCopy();

    // Draw TGraph on top
    g1->Draw("p same");

    c->Update();
    std::cout << "  Created histogram and graph for channel " << chID
              << std::endl;
  }

  PrintClosePrompt();
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
  auto channelHist =
      df.Histo1D({"channelHist", "Channel Distribution;Channel;Entries",
                  (int)*channelMax - (int)*channelMin + 1, *channelMin - 0.5,
                  *channelMax + 0.5},
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

void RDataFrameAnalysis::CalFlightPath() {
  PrintSectionHeader("CalFlightPath Analysis");

  // Get DL_cell from configuration
  double DL_cell = m_configReader.GetDL_cell();
  std::cout << "DL_cell: " << DL_cell << " m" << std::endl;

  // Get calibration parameters from configuration
  const std::vector<double> &LCalEn = m_fixmConfig->Global.LCalEn;
  const std::vector<double> &LCaldT = m_fixmConfig->Global.LCaldT;
  double LengthSet = m_fixmConfig->Global.LengthSet;

  std::cout << "Number of calibration energies: " << LCalEn.size() << std::endl;
  std::cout << "LengthSet: " << LengthSet << std::endl;

  // Create canvas for fitting
  TCanvas *c = new TCanvas("c_fit", "Flight Path Fit", 800, 600);

  // Create TF1 for fitting
  TF1 *f1 = new TF1("f1", caldT, 7, 22, 1);

  // Create TGraph from calibration data
  TGraph *g = new TGraph(LCalEn.size(), LCalEn.data(), LCaldT.data());
  g->SetTitle("Flight Path Fit;Neutron Energy (eV); T-T_g (ns)");
  g->SetMarkerSize(1.5);
  g->SetMarkerStyle(20);

  // Fit the graph
  f1->SetParameter(0, LengthSet);
  g->Draw("ap");
  g->Fit(f1, "R");

  // Get fitted length
  double l1 = f1->GetParameter(0);
  std::cout << Form("Fit result is %f, ", l1) << std::endl;

  // Calculate and print flight path lengths for each channel
  std::cout << "Flight path lengths for channels:" << std::endl;
  for (size_t i = 0; i < m_channelIDs.size(); i++) {
    std::cout << Form("Channel %d: %f", m_channelIDs[i], l1 + i * DL_cell)
              << std::endl;
  }
  std::cout << std::endl;

  // Load ENDF data files
  const std::string &dataDir = m_configReader.GetDataPath();
  std::string name_ENDF_con =
      dataDir + "YGL_RPI_Convolution_ENDF_BVIII_U5_1keV.dat";
  std::string name_ENDF = m_configReader.GetENDFDataU5NF();
  std::string name_ENDF_fil = dataDir + "U235ENDF80_standards_Filter.dat";

  // Create canvas for ENDF data
  c = new TCanvas("c_endf", "ENDF Data", 800, 600);

  TGraph *gENDF_con = new TGraph(name_ENDF_con.data());
  TGraph *gENDF = new TGraph();
  get_graph(name_ENDF.data(), gENDF, MeV_to_eV);
  TGraph *gENDF_fil = new TGraph(name_ENDF_fil.data());

  gENDF_con->Draw("al");
  gENDF->SetLineColor(kBlue);
  gENDF->Draw("same l");
  gENDF_fil->SetLineColor(kRed);
  gENDF_fil->Draw("same l");

  // Add legend
  TLegend *legd = new TLegend();
  legd->AddEntry(gENDF_con, "ENDF Convolution", "lf");
  legd->AddEntry(gENDF_fil, "ENDF_Filter", "lf");
  legd->AddEntry(gENDF, "ENDF", "lf");
  legd->Draw();

  PrintClosePrompt();
}

void RDataFrameAnalysis::GetPileupCorr() {
  PrintSectionHeader("GetPileupCorr Analysis");

  EnableMultiThreading();
  InitializeCommonConfig();

  // Get FPulse (pulse frequency in Hz) from ConfigReader
  double FPulse = m_configReader.GetFPulse();
  if (FPulse <= 0) {
    std::cerr << "Error: Invalid pulse frequency: " << FPulse << " Hz"
              << std::endl;
    return;
  }
  std::cout << "Pulse frequency: " << FPulse << " Hz" << std::endl;

  // Get total time from timelist
  double T_total = 0.0;
  const auto &timeList = m_configReader.GetTimeList();
  for (const auto &time : timeList) {
    T_total += time;
  }

  // If no timelist or invalid total time, abort
  if (T_total <= 0) {
    std::cerr << "Error: Invalid total time from timelist: " << T_total
              << " seconds" << std::endl;
    return;
  }
  std::cout << "Total live time: " << T_total << " seconds" << std::endl;

  if (m_channelIDs.empty()) {
    std::cerr << "Error: No channels configured in CHIDUSE" << std::endl;
    return;
  }
  std::cout << "Number of channels: " << m_channelIDs.size() << std::endl;

  // Build channel-to-length map
  std::map<int, double> channel_length;
  for (const auto &[chID, chConfig] : m_fixmConfig->Channels) {
    channel_length[chID] = chConfig.Length;
  }

  // Total pulses emitted during the run
  double N_pulse = T_total * FPulse;
  if (N_pulse <= 0) {
    std::cerr << "Error: Invalid pulse count" << std::endl;
    return;
  }
  std::cout << "Total pulses: " << N_pulse << std::endl;

  // Open hrate.root file
  std::string hrate_path = m_outputPath + m_expName + "/Outcome/hrate.root";
  std::cout << "Opening file: " << hrate_path << std::endl;

  std::unique_ptr<TFile> file(TFile::Open(hrate_path.c_str()));
  if (!file || file->IsZombie()) {
    std::cerr << "Error: Cannot open " << hrate_path << std::endl;
    return;
  }

  // Dead-time constant (ns)
  double tau = m_configReader.GetTau();

  std::string xspara_path = m_outputPath + m_expName + "/para/";
  std::string pileup_corr_name = xspara_path + "pileup_corr.root";
  auto frootcorr_out = new TFile(pileup_corr_name.c_str(), "RECREATE");

  // Map to store all h_corr_ histograms for plotting
  std::map<int, TH1D *> m_h_corr;

  // Process each channel
  for (int chID : m_channelIDs) {
    PrintChannelInfo(chID);

    // Get histogram for this channel
    TH1D *h1 = nullptr;
    file->GetObject(Form("h1_En_%d", chID), h1);
    if (!h1) {
      std::cerr << "  Warning: Channel " << chID << " histogram not found!"
                << std::endl;
      continue;
    }
    auto h_corr_ = (TH1D *)h1->Clone(Form("h_pileup_corr_%d", chID));
    h_corr_->SetDirectory(0); // Detach from file to keep it in memory

    // Get flight length for this channel
    double flight_length = 0.0;
    auto it_len = channel_length.find(chID);
    if (it_len != channel_length.end()) {
      flight_length = it_len->second;
    }
    if (flight_length <= 0) {
      std::cerr << "  Warning: Invalid flight length for channel " << chID
                << std::endl;
      continue;
    }

    std::vector<double> v_En;
    std::vector<double> v_rcorr;
    int nbins = h1->GetNbinsX();

    // Process each bin
    for (int bx = 1; bx <= nbins; bx++) {
      double bin_center = h1->GetXaxis()->GetBinCenter(bx);
      double bin_content = h1->GetBinContent(bx);

      double E_left = h1->GetXaxis()->GetBinLowEdge(bx);
      double E_right = h1->GetXaxis()->GetBinUpEdge(bx);

      // Calculate TOF bin width in ns
      double tof_left = calTOF(E_left, flight_length);
      double tof_right = calTOF(E_right, flight_length);
      double bin_width = fabs(tof_right - tof_left); // ns

      // Calculate pulse rate per ns
      double R_pulse = bin_content / (N_pulse * bin_width);

      // Calculate pileup correction factor
      double fcorr = 1.0;
      if (tau * R_pulse < 0.99 && R_pulse > 0.) {
        fcorr = 1.0 / (1.0 - tau * R_pulse);
      }
      h_corr_->SetBinContent(bx, fcorr);
    }

    // Store histogram for plotting
    m_h_corr[chID] = h_corr_;

    frootcorr_out->cd();
    h_corr_->Write();
  }
  frootcorr_out->Close();

  std::cout << "  Saved pileup correction table to: " << pileup_corr_name
            << std::endl;

  // Create canvas and plot all h_corr_ histograms
  if (!m_h_corr.empty()) {
    TCanvas *c_corr =
        new TCanvas("c_pileup_corr", "Pileup Correction Factors", 1200, 800);
    c_corr->SetLogx();
    c_corr->SetGridx();
    c_corr->SetGridy();

    // Define color palette for different channels
    std::vector<int> colors = {kRed,      kBlue,       kGreen + 2, kMagenta,
                               kCyan + 1, kOrange + 7, kViolet,    kTeal - 5,
                               kPink + 1, kSpring - 5};

    TLegend *leg = new TLegend(0.75, 0.6, 0.95, 0.9);
    leg->SetBorderSize(1);
    leg->SetFillStyle(0);

    bool first = true;
    int colorIdx = 0;

    for (auto &[chID, h_corr] : m_h_corr) {
      // Set histogram style
      int color = colors[colorIdx % colors.size()];
      h_corr->SetLineColor(color);
      h_corr->SetLineWidth(2);
      h_corr->SetStats(0);

      // Set axis titles (only for first histogram)
      if (first) {
        h_corr->SetTitle(
            "Pileup Correction Factors;Energy (eV);Correction Factor");
        h_corr->GetYaxis()->SetRangeUser(0.9, 1.5);
        h_corr->Draw("HIST");
        first = false;
      } else {
        h_corr->Draw("HIST SAME");
      }

      // Add to legend
      leg->AddEntry(h_corr, Form("Channel %d", chID), "l");

      colorIdx++;
    }

    leg->Draw();
    c_corr->Update();

    std::cout << std::endl;
    std::cout << "Pileup correction factors plotted for " << m_h_corr.size()
              << " channels" << std::endl;
    std::cout << "Close plot window to continue..." << std::endl;
    std::cout << std::endl;
  }
}

void RDataFrameAnalysis::CoincheckSingleBunchSimple() {
  PrintSectionHeader("CoincheckSingleBunchSimple Analysis");

  int nrebin = m_configReader.GetNRebin();

  InitializeCommonConfig();

  const std::string &dataDir = m_configReader.GetDataPath();

  std::cout << "Number of channels: " << m_channelIDs.size() << std::endl;

  // Build maps for channel data
  std::map<int, std::string> m_samplename;
  std::map<int, std::string> m_sampletype;
  std::set<std::string> v_sample_typeuse;

  // Maps for histograms
  std::map<int, TH1D *> m_hratem;
  std::map<int, TH1D *> m_hrate_norm;
  std::map<int, TH1D *> m_hrate_re;
  std::map<int, TH1D *> m_herror_rate;
  std::map<int, TH1D *> m_hrate_re_norm;
  std::map<int, TH1D *> m_hratio;
  std::map<int, TH1D *> m_hratio_dec;

  // Open hrate.root and fluxattenuation.root files
  std::string outcomePath = m_outputPath + m_expName + "/Outcome";
  std::string outparaPath = m_outputPath + m_expName + "/para";
  std::string hratePath = outcomePath + "/hrate.root";
  std::string fluxattenPath = outparaPath + "/fluxattenuation.root";
  std::string pileupcorrPath = outparaPath + "/pileup_corr.root";

  std::cout << "Opening hrate file: " << hratePath << std::endl;
  std::unique_ptr<TFile> fin_hrate(TFile::Open(hratePath.c_str()));
  if (!fin_hrate || fin_hrate->IsZombie()) {
    std::cerr << "Error: Cannot open " << hratePath << std::endl;
    return;
  }

  std::cout << "Opening flux attenuation file: " << fluxattenPath << std::endl;
  std::unique_ptr<TFile> fin_hatten(TFile::Open(fluxattenPath.c_str()));
  if (!fin_hatten || fin_hatten->IsZombie()) {
    std::cerr << "Error: Cannot open " << fluxattenPath << std::endl;
    return;
  }

  std::cout << "Opening pileup correction file: " << pileupcorrPath
            << std::endl;
  std::unique_ptr<TFile> fin_pileupcorr(TFile::Open(pileupcorrPath.c_str()));
  if (!fin_pileupcorr || fin_pileupcorr->IsZombie()) {
    std::cerr << "Error: Cannot open " << pileupcorrPath << std::endl;
    return;
  }

  // Process each channel: build configuration maps and process histograms
  for (int chID : m_channelIDs) {
    std::cout << "Processing channel " << chID << "..." << std::endl;

    // Get channel configuration
    auto it = m_fixmConfig->Channels.find(chID);
    if (it == m_fixmConfig->Channels.end()) {
      std::cerr << "  Warning: Channel " << chID << " not found in config"
                << std::endl;
      continue;
    }
    const auto &chConfig = it->second;

    // Build configuration maps
    m_sampletype[chID] = chConfig.SampleType;
    auto DetID = chConfig.DetID;
    std::string samplename =
        chConfig.SampleType + std::to_string(chConfig.SampleNumber);
    v_sample_typeuse.insert(chConfig.SampleType);
    m_samplename[chID] = samplename;

    // Calculate areal density
    double sample_r = chConfig.Radius * mm_to_cm; // to cm
    double sample_area = sample_r * sample_r * TMath::Pi();
    double sample_t = chConfig.Mass / sample_area; // mg/cm2
    double ArealDensity =
        sample_t / chConfig.A * Na * barn_to_cm2 * mg_to_g; // atoms/barn
    double nd = ArealDensity;

    // Get hrate histogram
    TH1D *h1E = nullptr;
    fin_hrate->GetObject(Form("h1_En_%d", chID), h1E);
    if (!h1E) {
      std::cerr << "  Warning: No histogram for channel " << chID << std::endl;
      continue;
    }

    auto hrate = (TH1D *)h1E->Clone(Form("hrate_%d", chID));

    if (hrate == nullptr) {
      continue;
    }
    TH1D *hatten = nullptr;
    fin_hatten->GetObject(Form("htrans%d", DetID), hatten);
    TH1D *hpileupcorr = nullptr;
    fin_pileupcorr->GetObject(Form("h_corr_%d", chID), hpileupcorr);

    // Normalize by nd and efficiency
    auto hrate_norm = (TH1D *)hrate->Clone(Form("hrate_norm_%d", chID));
    hrate_norm->Scale(1.0 / nd / chConfig.DetEff);

    m_hrate_norm[chID] = hrate_norm;

    // Rebin
    auto hrate_re = (TH1D *)hrate->Clone(Form("hrate_re_%d", chID));
    if (hatten != nullptr) {
      hrate_re->Divide(hatten);
    }
    if (hpileupcorr != nullptr) {
      hrate_re->Multiply(hpileupcorr);
    }
    hrate_re->Rebin(nrebin);
    m_hrate_re[chID] = hrate_re;

    // Calculate statistical error
    auto herror_rate = (TH1D *)hrate_re->Clone(Form("herror_rate_%d", chID));
    get_sta_errorhist(hrate_re, herror_rate);
    m_herror_rate[chID] = herror_rate;

    // Normalize re-binned histogram
    auto hrate_re_norm =
        (TH1D *)hrate_re->Clone(Form("hrate_re_norm_%d", chID));
    hrate_re_norm->Scale(1.0 / nd / chConfig.DetEff);
    m_hrate_re_norm[chID] = hrate_re_norm;
  }

  // Write normalized histograms to file
  std::string hrate_norm_path = outcomePath + "/hrate_norm.root";
  std::unique_ptr<TFile> fout_hrate_norm(
      TFile::Open(hrate_norm_path.c_str(), "recreate"));
  if (fout_hrate_norm && !fout_hrate_norm->IsZombie()) {
    for (auto &ele_m : m_hrate_norm) {
      ele_m.second->Write();
    }
    fout_hrate_norm->Close();
    std::cout << "Output file saved: " << hrate_norm_path << std::endl;
  }

  // Process each sample type
  for (const auto &sample_typeuse : v_sample_typeuse) {
    std::cout << "Processing sample type: " << sample_typeuse << std::endl;

    // Canvas 1: Divide by ENDF cross section
    TCanvas *c1 = new TCanvas;
    gPad->SetLogx();
    gPad->SetLogy();
    auto legd1 = new TLegend;

    auto hsum = (TH1D *)m_hrate_re_norm.begin()->second->Clone(
        Form("hsum_%s", sample_typeuse.c_str()));
    hsum->Reset();
    int ihist = 0;

    for (int chID : m_channelIDs) {
      if (m_sampletype[chID] != sample_typeuse) {
        continue;
      }
      auto h = m_hrate_re_norm[chID];
      h->SetLineColor(color[ihist]);
      legd1->AddEntry(h, m_samplename[chID].c_str(), "lf");
      hsum->Add(h);
      // Fix: First histogram uses "hist" to initialize canvas, subsequent ones
      // use "hist same"
      if (ihist == 0) {
        h->Draw("hist");
      } else {
        h->Draw("hist same");
      }
      ihist++;
    }

    // Only create and draw havg if we have histograms
    // Define havg at this scope level so it's accessible in subsequent canvases
    TH1D *havg = nullptr;
    if (ihist > 0) {
      havg = (TH1D *)hsum->Clone(Form("havg_%s", sample_typeuse.c_str()));
      havg->Scale(1.0 / (double)ihist);
      legd1->Draw();
    }

    // Canvas 2: Ratio to average
    TCanvas *c2 = new TCanvas;
    gPad->SetLogx();
    auto legd2 = new TLegend();

    ihist = 0;
    // Only process Canvas 2 if we have histograms from Canvas 1
    if (havg != nullptr) {
      for (int chID : m_channelIDs) {
        if (m_sampletype[chID] != sample_typeuse) {
          continue;
        }
        auto hratio = (TH1D *)m_hrate_re_norm[chID]->Clone(
            Form("hratio_%s_%d", sample_typeuse.c_str(), chID));
        hratio->SetYTitle("Reaction Rate / Average Reaction Rate");
        hratio->Divide(havg);
        legd2->AddEntry(hratio, m_samplename[chID].c_str(), "lf");
        m_hratio[chID] = hratio;
        // Fix: First histogram uses "hist" to initialize canvas, subsequent
        // ones use "hist same"
        if (ihist == 0) {
          hratio->Draw("hist");
        } else {
          hratio->Draw("hist same");
        }
        ihist++;
      }
      if (ihist > 0) {
        legd2->Draw();
      }
    }

    // Canvas 3: Statistical error
    TCanvas *c3 = new TCanvas;
    gPad->SetLogx();
    auto legd3 = new TLegend();
    ihist = 0;

    ihist = 0;
    for (int chID : m_channelIDs) {
      if (m_sampletype[chID] != sample_typeuse) {
        continue;
      }
      m_herror_rate[chID]->SetLineColor(color[ihist]);
      // Fix: First histogram uses "hist" to initialize canvas, subsequent ones
      // use "hist same"
      if (ihist == 0) {
        m_herror_rate[chID]->Draw("hist");
      } else {
        m_herror_rate[chID]->Draw("hist same");
      }
      legd3->AddEntry(m_herror_rate[chID], m_samplename[chID].c_str(), "lf");
      ihist++;
    }
    legd3->Draw();

    // Canvas 4: Decoupled error
    TCanvas *c4 = new TCanvas;
    gPad->SetLogx();
    auto legd4 = new TLegend();
    ihist = 0;

    for (int chID : m_channelIDs) {
      if (m_sampletype[chID] != sample_typeuse) {
        continue;
      }

      auto hrr = (TH1D *)m_hratio[chID]->Clone(Form("hratio_%d", chID));
      hrr->Reset();

      for (size_t i = 0; i < hrr->GetNbinsX(); i++) {
        auto bincontent = m_hratio[chID]->GetBinContent(i + 1);
        if (bincontent <= 0) {
          continue;
        }
        auto bin_un = m_herror_rate[chID]->GetBinContent(i + 1);
        auto debc = abs(bincontent - 1);
        Double_t cre = 0.;

        if (debc > bin_un) {
          cre = sqrt(debc * debc - bin_un * bin_un);
          if (std::signbit(bincontent - 1)) {
            cre = -cre;
          }
        } else {
          if (std::signbit(bincontent - 1)) {
            cre = -debc;
          }
        }

        hrr->SetBinContent(i + 1, cre);
      }

      m_hratio_dec[chID] = hrr;
      // Fix: First histogram uses "hist" to initialize canvas, subsequent ones
      // use "hist same"
      if (ihist == 0) {
        hrr->Draw("hist");
      } else {
        hrr->Draw("hist same");
      }
      ihist++;
      legd4->AddEntry(hrr, m_samplename[chID].c_str(), "lf");
    }
    // Only draw legend if we have histograms
    if (ihist > 0) {
      legd4->Draw();
    }

    // Canvas 5: Coincidence error
    TCanvas *c5 = new TCanvas;
    gPad->SetLogx();
    auto gerror_coin = new TGraph();
    TH1D *herror_coin = nullptr;

    // Only process Canvas 5 if we have havg
    if (havg != nullptr) {
      for (size_t i = 0; i < havg->GetNbinsX(); i++) {
        double max_y = -DBL_MAX;
        double min_y = DBL_MAX;
        auto x = havg->GetBinCenter(i + 1);

        for (int chID : m_channelIDs) {
          if (m_sampletype[chID] != sample_typeuse) {
            continue;
          }

          auto content = m_hratio_dec[chID]->GetBinContent(i + 1);

          if (max_y < content) {
            max_y = content;
          }
          if (min_y > content) {
            min_y = content;
          }
        }

        auto dec = max_y - min_y;
        if (dec > 0 && dec < 0.2) {
          gerror_coin->AddPoint(x, dec);
        }
      }

      herror_coin =
          (TH1D *)havg->Clone(Form("herror_coin_%s", sample_typeuse.c_str()));
      herror_coin->SetYTitle("Uncertainty");
      herror_coin->Reset();
      graph2hist(gerror_coin, herror_coin);
      herror_coin->Smooth();
      herror_coin->Draw("hist");
      herror_coin->SetLineColor(kRed);
    }

    // Write coincidence efficiency to file (only if we have herror_coin)
    if (herror_coin != nullptr) {
      std::string output_dat =
          outparaPath + "/" + sample_typeuse + "COINEFF.dat";
      std::ofstream ofs(output_dat);
      if (ofs.is_open()) {
        for (size_t i = 0; i < herror_coin->GetNbinsX(); i++) {
          ofs << herror_coin->GetBinCenter(i + 1) << "\t"
              << herror_coin->GetBinContent(i + 1) << std::endl;
        }
        ofs.close();
        std::cout << "  Saved coincidence efficiency to: " << output_dat
                  << std::endl;
      }
    }
  }

  std::cout << std::endl;
  std::cout << "Close plot windows to continue..." << std::endl;
  std::cout << "Press Ctrl+C in terminal to exit..." << std::endl;
  std::cout << std::endl;
}

void RDataFrameAnalysis::CoincheckSimple() {
  PrintSectionHeader("CoincheckSimple Analysis");

  int nrebin = m_configReader.GetNRebin();

  InitializeCommonConfig();

  const std::string &dataDir = m_configReader.GetDataPath();

  std::cout << "Number of channels: " << m_channelIDs.size() << std::endl;

  // Build maps for channel data
  std::map<int, std::string> m_samplename;
  std::map<int, std::string> m_sampletype;
  std::set<std::string> v_sample_typeuse;

  // Maps for histograms
  std::map<int, TH1D *> m_hratem;
  std::map<int, TH1D *> m_hrate_norm;
  std::map<int, TH1D *> m_hrate_re;
  std::map<int, TH1D *> m_herror_rate;
  std::map<int, TH1D *> m_hrate_re_norm;
  std::map<int, TH1D *> m_hratio;
  std::map<int, TH1D *> m_hratio_dec;

  // Open hrate.root and fluxattenuation.root files
  std::string outcomePath = m_outputPath + m_expName + "/Outcome";
  std::string hratePath = outcomePath + "/hrate.root";
  std::string fluxattenPath = outcomePath + "/fluxattenuation.root";

  std::cout << "Opening hrate file: " << hratePath << std::endl;
  std::unique_ptr<TFile> fin_hrate(TFile::Open(hratePath.c_str()));
  if (!fin_hrate || fin_hrate->IsZombie()) {
    std::cerr << "Error: Cannot open " << hratePath << std::endl;
    return;
  }

  std::cout << "Opening flux attenuation file: " << fluxattenPath << std::endl;
  std::unique_ptr<TFile> fin_hatten(TFile::Open(fluxattenPath.c_str()));
  if (!fin_hatten || fin_hatten->IsZombie()) {
    std::cerr << "Error: Cannot open " << fluxattenPath << std::endl;
    return;
  }

  // Process each channel: build configuration maps and process histograms
  for (int chID : m_channelIDs) {
    std::cout << "Processing channel " << chID << "..." << std::endl;

    // Get channel configuration
    auto it = m_fixmConfig->Channels.find(chID);
    if (it == m_fixmConfig->Channels.end()) {
      std::cerr << "  Warning: Channel " << chID << " not found in config"
                << std::endl;
      continue;
    }
    const auto &chConfig = it->second;

    // Build configuration maps
    m_sampletype[chID] = chConfig.SampleType;

    std::string samplename =
        chConfig.SampleType + std::to_string(chConfig.SampleNumber);
    v_sample_typeuse.insert(chConfig.SampleType);
    m_samplename[chID] = samplename;

    // Calculate areal density
    double sample_r = chConfig.Radius * mm_to_cm; // to cm
    double sample_area = sample_r * sample_r * TMath::Pi();
    double sample_t = chConfig.Mass / sample_area; // mg/cm2
    double ArealDensity =
        sample_t / chConfig.A * Na * barn_to_cm2 * mg_to_g; // atoms/barn
    double nd = ArealDensity;

    // Get hrate histogram
    TH1D *h1E = nullptr;
    fin_hrate->GetObject(Form("h1_En_%d", chID), h1E);
    if (!h1E) {
      std::cerr << "  Warning: No histogram for channel " << chID << std::endl;
      continue;
    }

    auto hrate = (TH1D *)h1E->Clone(Form("hrate_%d", chID));

    // Open UF file for this channel
    std::string ufPath = outcomePath + "/UF_" + std::to_string(chID) + ".root";
    std::unique_ptr<TFile> fin_uf(TFile::Open(ufPath.c_str()));
    if (!fin_uf || fin_uf->IsZombie()) {
      continue;
    }

    TH1D *hrate_uf = nullptr;
    fin_uf->GetObject("h_finalE", hrate_uf);

    if (hrate != nullptr) {

      // Normalize by nd and efficiency
      auto hrate_norm = (TH1D *)hrate->Clone(Form("hrate_norm_%d", chID));
      hrate_norm->Scale(1.0 / nd / chConfig.DetEff);
      m_hrate_norm[chID] = hrate_norm;

      // Rebin
      auto hrate_re = (TH1D *)hrate->Clone(Form("hrate_re_%d", chID));
      hrate_re->Rebin(nrebin);
      m_hrate_re[chID] = hrate_re;

      // Calculate statistical error
      auto herror_rate = (TH1D *)hrate_re->Clone(Form("herror_rate_%d", chID));
      get_sta_errorhist(hrate_re, herror_rate);
      m_herror_rate[chID] = herror_rate;

      // Normalize re-binned histogram
      auto hrate_re_norm =
          (TH1D *)hrate_re->Clone(Form("hrate_re_norm_%d", chID));
      hrate_re_norm->Scale(1.0 / nd / chConfig.DetEff);
      m_hrate_re_norm[chID] = hrate_re_norm;
    } else {
      std::cout << "  Warning: rate calculation failed for channel " << chID
                << std::endl;
    }
  }

  // Load ENDF data
  std::string name_ENDFU5NF = dataDir + m_configReader.GetENDFDataU5NF();

  auto gENDFU5 = new TGraph();
  get_graph(name_ENDFU5NF.c_str(), gENDFU5, MeV_to_eV);
  auto hENDFU5 = (TH1D *)m_hrate_re_norm.begin()->second->Clone("hENDFU5");
  hENDFU5->Reset();
  graph2hist(gENDFU5, hENDFU5);

  std::map<std::string, TH1D *> m_hxs;
  m_hxs["U5"] = hENDFU5;

  std::cout << "Loaded ENDF data files" << std::endl;

  // Write normalized histograms to file
  std::string hrate_norm_path = outcomePath + "/hrate_norm.root";
  std::unique_ptr<TFile> fout_hrate_norm(
      TFile::Open(hrate_norm_path.c_str(), "recreate"));
  if (fout_hrate_norm && !fout_hrate_norm->IsZombie()) {
    for (auto &ele_m : m_hrate_norm) {
      ele_m.second->Write();
    }
    fout_hrate_norm->Close();
    std::cout << "Output file saved: " << hrate_norm_path << std::endl;
  }

  // Process each sample type
  for (const auto &sample_typeuse : v_sample_typeuse) {
    std::cout << "Processing sample type: " << sample_typeuse << std::endl;

    // Canvas 1: Divide by ENDF cross section
    TCanvas *c1 = new TCanvas;
    gPad->SetLogx();
    gPad->SetLogy();
    auto legd1 = new TLegend;

    auto hsum = (TH1D *)m_hrate_re_norm.begin()->second->Clone(
        Form("hsum_%s", sample_typeuse.c_str()));
    hsum->Reset();
    int ihist = 0;

    for (int chID : m_channelIDs) {
      if (m_sampletype[chID] != sample_typeuse) {
        continue;
      }

      auto h = m_hrate_re_norm[chID];
      h->Divide(m_hxs[m_sampletype[chID]]);
      h->SetLineColor(color[ihist]);
      legd1->AddEntry(h, m_samplename[chID].c_str(), "lf");
      hsum->Add(h);
      // Fix: First histogram uses "hist" to initialize canvas, subsequent ones
      // use "hist same"
      if (ihist == 0) {
        h->Draw("hist");
      } else {
        h->Draw("hist same");
      }
      ihist++;
    }

    // Only create and draw havg if we have histograms
    // Define havg at this scope level so it's accessible in subsequent canvases
    TH1D *havg = nullptr;
    if (ihist > 0) {
      havg = (TH1D *)hsum->Clone(Form("havg_%s", sample_typeuse.c_str()));
      havg->Scale(1.0 / (double)ihist);
      legd1->Draw();
    }

    // Canvas 2: Ratio to average
    TCanvas *c2 = new TCanvas;
    gPad->SetLogx();
    auto legd2 = new TLegend();

    ihist = 0;
    // Only process Canvas 2 if we have histograms from Canvas 1
    if (havg != nullptr) {
      for (int chID : m_channelIDs) {
        if (m_sampletype[chID] != sample_typeuse) {
          continue;
        }

        auto hratio = (TH1D *)m_hrate_re_norm[chID]->Clone(
            Form("hratio_%s_%d", sample_typeuse.c_str(), chID));
        hratio->SetYTitle("Ratio");
        hratio->Divide(havg);
        legd2->AddEntry(hratio, m_samplename[chID].c_str(), "lf");
        m_hratio[chID] = hratio;
        // Fix: First histogram uses "hist" to initialize canvas, subsequent
        // ones use "hist same"
        if (ihist == 0) {
          hratio->Draw("hist");
        } else {
          hratio->Draw("hist same");
        }
        ihist++;
      }
      if (ihist > 0) {
        legd2->Draw();
      }
    }

    // Canvas 3: Statistical error
    TCanvas *c3 = new TCanvas;
    gPad->SetLogx();
    auto legd3 = new TLegend();
    ihist = 0;

    ihist = 0;
    for (int chID : m_channelIDs) {
      if (m_sampletype[chID] != sample_typeuse) {
        continue;
      }

      m_herror_rate[chID]->SetLineColor(color[ihist]);
      // Fix: First histogram uses "hist" to initialize canvas, subsequent ones
      // use "hist same"
      if (ihist == 0) {
        m_herror_rate[chID]->Draw("hist");
      } else {
        m_herror_rate[chID]->Draw("hist same");
      }
      legd3->AddEntry(m_herror_rate[chID], m_samplename[chID].c_str(), "lf");
      ihist++;
    }
    legd3->Draw();

    // Canvas 4: Decoupled error
    TCanvas *c4 = new TCanvas;
    gPad->SetLogx();
    auto legd4 = new TLegend();

    for (int chID : m_channelIDs) {
      if (m_sampletype[chID] != sample_typeuse) {
        continue;
      }

      auto hrr = (TH1D *)m_hratio[chID]->Clone(Form("hratio_%d", chID));
      hrr->Reset();

      for (size_t i = 0; i < hrr->GetNbinsX(); i++) {
        auto bincontent = m_hratio[chID]->GetBinContent(i + 1);
        if (bincontent <= 0) {
          continue;
        }
        auto bin_un = m_herror_rate[chID]->GetBinContent(i + 1);
        auto debc = abs(bincontent - 1);
        Double_t cre = 0.;

        if (debc > bin_un) {
          cre = sqrt(debc * debc - bin_un * bin_un);
          if (std::signbit(bincontent - 1)) {
            cre = -cre;
          }
        } else {
          if (std::signbit(bincontent - 1)) {
            cre = -debc;
          }
        }

        hrr->SetBinContent(i + 1, cre);
      }

      m_hratio_dec[chID] = hrr;
      // Fix: First histogram uses "hist" to initialize canvas, subsequent ones
      // use "hist same"
      if (ihist == 0) {
        hrr->Draw("hist");
      } else {
        hrr->Draw("hist same");
      }
      ihist++;
      legd4->AddEntry(hrr, m_samplename[chID].c_str(), "lf");
    }
    legd4->Draw();

    // Canvas 5: Coincidence error
    TCanvas *c5 = new TCanvas;
    gPad->SetLogx();
    auto gerror_coin = new TGraph();
    TH1D *herror_coin = nullptr;

    // Only process Canvas 5 if we have havg
    if (havg != nullptr) {
      for (size_t i = 0; i < havg->GetNbinsX(); i++) {
        double max_y = -DBL_MAX;
        double min_y = DBL_MAX;
        auto x = havg->GetBinCenter(i + 1);

        for (int chID : m_channelIDs) {
          if (m_sampletype[chID] != sample_typeuse) {
            continue;
          }

          auto content = m_hratio_dec[chID]->GetBinContent(i + 1);

          if (max_y < content) {
            max_y = content;
          }
          if (min_y > content) {
            min_y = content;
          }
        }

        auto dec = max_y - min_y;
        if (dec > 0 && dec < 0.2) {
          gerror_coin->AddPoint(x, dec);
        }
      }

      herror_coin =
          (TH1D *)havg->Clone(Form("herror_coin_%s", sample_typeuse.c_str()));
      herror_coin->SetYTitle("Uncertainty");
      herror_coin->Reset();
      graph2hist(gerror_coin, herror_coin);
      herror_coin->Smooth();
      herror_coin->Draw("hist");
      herror_coin->SetLineColor(kRed);
    }

    // Write coincidence efficiency to file (only if we have herror_coin)
    if (herror_coin != nullptr) {
      std::string output_dat =
          outcomePath + "/UN" + sample_typeuse + "COINEFF.dat";
      std::ofstream ofs(output_dat);
      if (ofs.is_open()) {
        for (size_t i = 0; i < herror_coin->GetNbinsX(); i++) {
          ofs << herror_coin->GetBinCenter(i + 1) << "\t"
              << herror_coin->GetBinContent(i + 1) << std::endl;
        }
        ofs.close();
        std::cout << "  Saved coincidence efficiency to: " << output_dat
                  << std::endl;
      }
    }
  }

  std::cout << std::endl;
  std::cout << "Close plot windows to continue..." << std::endl;
  std::cout << "Press Ctrl+C in terminal to exit..." << std::endl;
  std::cout << std::endl;
}

void RDataFrameAnalysis::CountT0() {
  PrintSectionHeader("CountT0 Analysis");

  // Initialize common configuration
  InitializeCommonConfig();

  // Get configuration
  const std::string &T0Path = m_configReader.GetT0Path();

  // Get FPulse from experiment configuration
  double FPulse = m_fixmConfig->Global.FPulse;

  std::cout << "T0 Path: " << T0Path << std::endl;
  std::cout << "Experiment Name: " << m_expName << std::endl;
  std::cout << "Pulse Frequency: " << FPulse << " Hz" << std::endl;

  if (FPulse <= 0) {
    std::cerr << "Error: Invalid FPulse value: " << FPulse << std::endl;
    return;
  }

  // Get T0 file list from configuration
  const auto &T0list = m_fixmConfig->Global.T0list;

  if (T0list.empty()) {
    std::cerr << "Error: T0list is empty in configuration" << std::endl;
    return;
  }

  std::cout << "Number of T0 files: " << T0list.size() << std::endl;
  std::cout << std::endl;

  // Process each file individually to get measurement time
  std::cout << "========================================" << std::endl;
  std::cout << "Processing Individual T0 Files" << std::endl;
  std::cout << "========================================" << std::endl;

  std::vector<double> fileTimes;
  std::vector<Long64_t> fileT0Counts;

  for (size_t i = 0; i < T0list.size(); i++) {
    const auto &t0file = T0list[i];
    std::string fullPath = T0Path + t0file;

    std::cout << "File " << (i + 1) << "/" << T0list.size() << ": " << t0file
              << std::endl;

    // Open file
    std::unique_ptr<TFile> fin(TFile::Open(fullPath.c_str()));
    if (!fin || fin->IsZombie()) {
      std::cerr << "  Error: Failed to open " << fullPath << std::endl;
      fileTimes.push_back(0.0);
      fileT0Counts.push_back(0);
      continue;
    }

    // Get tree
    TTree *tree = nullptr;
    fin->GetObject("WNSRawTree", tree);
    if (!tree) {
      std::cerr << "  Error: Tree WNSRawTree not found in " << fullPath
                << std::endl;
      fileTimes.push_back(0.0);
      fileT0Counts.push_back(0);
      continue;
    }

    // Create RDataFrame from tree
    ROOT::RDataFrame df(*tree);

    // Sum T0Number_ALL for this file
    auto sumT0_ALL = df.Sum<Int_t>("T0Number_ALL");
    Long64_t fileT0Count = *sumT0_ALL;

    // Calculate measurement time
    double fileTime = fileT0Count / FPulse;

    fileTimes.push_back(fileTime);
    fileT0Counts.push_back(fileT0Count);

    std::cout << "  T0Number_ALL: " << fileT0Count << std::endl;
    std::cout << "  Measurement time: " << fileTime << " s ("
              << fileTime / 3600.0 << " h)" << std::endl;
    std::cout << std::endl;
  }

  std::cout << "========================================" << std::endl;
  std::cout << std::endl;

  // Create TChain to combine all T0 files for histogram generation
  std::cout << "Creating combined TChain for histogram generation..."
            << std::endl;
  auto chain = std::make_unique<TChain>("WNSRawTree");
  for (const auto &t0file : T0list) {
    std::string fullPath = T0Path + t0file;
    chain->Add(fullPath.c_str());
  }

  Long64_t nentries = chain->GetEntries();
  if (nentries == 0) {
    std::cerr << "Error: No entries found in T0 files" << std::endl;
    return;
  }

  std::cout << "Total entries in chain: " << nentries << std::endl;
  std::cout << "Processing with RDataFrame..." << std::endl;

  // Create RDataFrame from TChain
  ROOT::RDataFrame df(*chain);

  // Define entry index column for histogram x-axis
  auto df_indexed = df.DefineSlotEntry(
      "EntryIndex", [](unsigned int /*slot*/, ULong64_t entry) {
        return static_cast<Double_t>(entry);
      });

  // Calculate total counts using Sum
  auto totalT0_TCM = df_indexed.Sum<Int_t>("T0Number_TCM");
  auto totalT0_ALL = df_indexed.Sum<Int_t>("T0Number_ALL");

  // Create histograms using Histo1D
  // For TCM
  auto h1 = df_indexed.Histo1D({"h1_T0_TCM", ";Entry Number;T0Number",
                                static_cast<int>(nentries), 0.0,
                                static_cast<double>(nentries)},
                               "EntryIndex", "T0Number_TCM");

  // For ALL
  auto h2 = df_indexed.Histo1D({"h2_T0_ALL", ";Entry Number;T0Number",
                                static_cast<int>(nentries), 0.0,
                                static_cast<double>(nentries)},
                               "EntryIndex", "T0Number_ALL");

  // Trigger computation and get results
  Long64_t total_TCM = *totalT0_TCM;
  Long64_t total_ALL = *totalT0_ALL;

  // Calculate measurement time
  double totalTime_TCM = total_TCM / FPulse;
  double totalTime_ALL = total_ALL / FPulse;

  // Create canvas for comparison plots
  auto c = new TCanvas("c_T0_comparison", "T0 Comparison", 1200, 600);
  c->Divide(2, 1);

  // Left panel: Overlay of TCM and ALL
  c->cd(1);
  auto legd = new TLegend(0.7, 0.7, 0.9, 0.9);

  // Get histogram pointers and configure
  auto h1_ptr = h1.GetPtr();
  h1_ptr->SetLineColor(kBlue);
  h1_ptr->SetStats(0);
  h1_ptr->DrawCopy("hist");

  auto h2_ptr = h2.GetPtr();
  h2_ptr->SetLineColor(kRed);
  h2_ptr->SetStats(0);
  h2_ptr->DrawCopy("hist same");

  legd->AddEntry(h1_ptr, "T0Number_TCM", "l");
  legd->AddEntry(h2_ptr, "T0Number_ALL", "l");
  legd->Draw();

  // Right panel: Ratio TCM/ALL
  c->cd(2);
  auto hratio = static_cast<TH1D *>(h1_ptr->Clone("hratio_TCM_ALL"));
  hratio->SetTitle(";Entry Number;T0Number_TCM / T0Number_ALL");
  hratio->Divide(h2_ptr);
  hratio->SetLineColor(kBlack);
  hratio->SetStats(0);
  hratio->Draw("hist");

  // Print summary
  std::cout << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << "Summary" << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << "Total entries: " << nentries << std::endl;
  std::cout << std::endl;
  std::cout << "T0Number_TCM:" << std::endl;
  std::cout << "  Total counts: " << total_TCM << std::endl;
  std::cout << "  Measurement time: " << totalTime_TCM << " seconds"
            << std::endl;
  std::cout << "  Measurement time: " << totalTime_TCM / 3600.0 << " hours"
            << std::endl;
  std::cout << std::endl;
  std::cout << "T0Number_ALL:" << std::endl;
  std::cout << "  Total counts: " << total_ALL << std::endl;
  std::cout << "  Measurement time: " << totalTime_ALL << " seconds"
            << std::endl;
  std::cout << "  Measurement time: " << totalTime_ALL / 3600.0 << " hours"
            << std::endl;
  std::cout << std::endl;
  std::cout << "Ratio (TCM/ALL): " << static_cast<double>(total_TCM) / total_ALL
            << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << std::endl;

  std::cout << "Close plot window to continue..." << std::endl;
}

void RDataFrameAnalysis::GetHRateXSUF() {
  PrintSectionHeader("GetHRateXSUF Analysis");

  InitializeCommonConfig();

  // Get ntimes from configuration
  double ntimes = m_fixmConfig->Global.UFRandomTimes;

  std::cout << "Number of channels to analyze: " << m_channelIDs.size()
            << std::endl;
  std::cout << "Monte Carlo sampling factor: " << ntimes << std::endl;

  // Load ENDF data using existing member function
  if (!LoadENDFData()) {
    std::cerr << "Error: Failed to load ENDF data" << std::endl;
    return;
  }

  // Create maps for sample information
  std::map<int, std::string> m_sampletype;
  std::map<int, double> m_nd;

  // Fill sample information from configuration
  for (const auto &[chID, chConfig] : m_fixmConfig->Channels) {
    m_sampletype[chID] = chConfig.SampleType;

    // Calculate areal density
    double sample_r = chConfig.Radius * mm_to_cm; // to cm
    double sample_area = sample_r * sample_r * TMath::Pi();
    double sample_t = chConfig.Mass / sample_area; // mg/cm2
    double ArealDensity =
        sample_t / chConfig.A * Na * barn_to_cm2 * mg_to_g; // atoms/barn
    m_nd[chID] = ArealDensity;
  }

  // Lambda function to calculate hratexs (without self-shielding correction)
  // Use member variable m_xs_nr instead of local m_xs_nf
  auto calhratexs = [this, &m_sampletype](TH1D *hrate, TH1D *hratexs, int chid,
                                          double ntimes) {
    hratexs->Reset();
    size_t nloop = static_cast<size_t>(hrate->Integral() * ntimes);
    auto sampletype = m_sampletype[chid];

    // Use member variable m_xs_nr
    auto it = m_xs_nr.find(sampletype);
    if (it == m_xs_nr.end()) {
      std::cerr << "  Warning: No cross section data for sample type "
                << sampletype << std::endl;
      return;
    }
    auto gENDFNF = it->second;

    for (size_t i = 0; i < nloop; i++) {
      auto En = hrate->GetRandom();
      auto xs_nf = gENDFNF->Eval(En);
      if (En <= m_configReader.GetEnergyCutHigh() && xs_nf > 0) {
        hratexs->Fill(En, 1.0 / xs_nf);
      }
    }
    hratexs->Scale(1.0 / ntimes);
  };

  // Lambda function to calculate hratexsNs (with self-shielding correction)
  // Use member variables m_xs_nr and m_xs_ntot
  auto calhratexsNs = [this, &m_sampletype, &m_nd](TH1D *hrate, TH1D *hratexs,
                                                   int chid, double ntimes) {
    hratexs->Reset();
    size_t nloop = static_cast<size_t>(hrate->Integral() * ntimes);
    auto sampletype = m_sampletype[chid];

    // Use member variables
    auto it_nf = m_xs_nr.find(sampletype);
    auto it_ntot = m_xs_ntot.find(sampletype);
    if (it_nf == m_xs_nr.end() || it_ntot == m_xs_ntot.end()) {
      std::cerr << "  Warning: No cross section data for sample type "
                << sampletype << std::endl;
      return;
    }
    auto gENDFNF = it_nf->second;
    auto gENDFTOT = it_ntot->second;
    auto nd = m_nd[chid];

    for (size_t i = 0; i < nloop; i++) {
      auto En = hrate->GetRandom();
      auto xs_ntot = gENDFTOT->Eval(En);
      auto xs_nf = gENDFNF->Eval(En);
      auto yield = 1.0 - exp(-nd * xs_ntot);
      if (En <= m_configReader.GetEnergyCutHigh() && xs_nf > 0 && yield > 0) {
        hratexs->Fill(En, 1.0 / yield / xs_nf * xs_ntot);
      }
    }
    hratexs->Scale(1.0 / ntimes);
  };

  // Maps to store histograms
  std::map<int, TH1D *> m_hrate;
  std::map<int, TH1D *> m_hratexs;
  std::map<int, TH1D *> m_hratexsNs;

  // Path to UF result files - use member variables
  std::string outcome_base = m_outputPath + m_expName + "/Outcome/FIXM";
  std::cout << "Reading UF result files from: " << outcome_base << std::endl;

  // Process each channel
  for (int chID : m_channelIDs) {
    PrintChannelInfo(chID);

    // Open UF result file
    std::string ufFilePath = outcome_base + Form("/UF_%d.root", chID);
    TFile *fin = TFile::Open(ufFilePath.c_str());

    if (!fin || fin->IsZombie()) {
      std::cerr << "  Warning: Cannot open file " << ufFilePath << std::endl;
      continue;
    }

    // Get h_finalE histogram
    TH1D *hEn = nullptr;
    fin->GetObject("h_finalE", hEn);
    if (!hEn) {
      std::cerr << "  Warning: h_finalE not found in " << ufFilePath
                << std::endl;
      fin->Close();
      continue;
    }

    // Clone histogram to keep it in memory after file closes
    hEn = static_cast<TH1D *>(hEn->Clone(Form("h_finalE_%d", chID)));
    hEn->SetDirectory(0);
    m_hrate[chID] = hEn;

    // Create output histograms
    auto hEnxs = static_cast<TH1D *>(hEn->Clone(Form("h1_Enxs_%d", chID)));
    hEnxs->SetDirectory(0);
    auto hEnxsNs = static_cast<TH1D *>(hEn->Clone(Form("h1_EnxsNs_%d", chID)));
    hEnxsNs->SetDirectory(0);

    // Calculate rate×xs histograms
    std::cout << "  Calculating rate×xs (without self-shielding)..."
              << std::endl;
    calhratexs(hEn, hEnxs, chID, ntimes);
    m_hratexs[chID] = hEnxs;

    // Uncomment to calculate with self-shielding correction
    std::cout << "  Calculating rate×xs (with self-shielding)..." << std::endl;
    calhratexsNs(hEn, hEnxsNs, chID, ntimes);
    m_hratexsNs[chID] = hEnxsNs;

    fin->Close();
  }

  // Save results to output file
  std::string outpath = outcome_base + "/hratexsuf.root";
  auto fout = TFile::Open(outpath.c_str(), "RECREATE");

  for (int chID : m_channelIDs) {
    if (m_hratexs.find(chID) != m_hratexs.end()) {
      m_hratexs[chID]->Write();
    }
    // Uncomment to save self-shielding corrected histograms
    if (m_hratexsNs.find(chID) != m_hratexsNs.end()) {
      m_hratexsNs[chID]->Write();
    }
  }

  fout->Close();

  std::cout << std::endl;
  std::cout << "Output file saved: " << outpath << std::endl;
  std::cout << std::endl;

  // Note: No need to delete ENDF TGraphs as they are managed by member
  // variables m_xs_nr and m_xs_ntot will be cleaned up in destructor or when
  // LoadENDFData is called again
}
