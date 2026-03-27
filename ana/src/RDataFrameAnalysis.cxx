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
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "TVirtualPad.h"
#include "utils.h"
#include <algorithm>
#include <cfloat>
#include <climits>
#include <filesystem>
#include <fstream>
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
  // CalSimTrans uses its own TChain, no need to initialize the experiment
  // TChain
  if (analysisType == "CalSimTrans") {
    try {
      CalSimTrans();
    } catch (const std::exception &e) {
      std::cerr << "Error during CalSimTrans: " << e.what() << std::endl;
      return false;
    }
    return true;
  }

  if (analysisType == "CountT0") {
    try {
      CountT0();
    } catch (const std::exception &e) {
      std::cerr << "Error during CountT0: " << e.what() << std::endl;
      return false;
    }
    return true;
  }

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
    } else if (analysisType == "Coincheck") {
      Coincheck();
    } else if (analysisType == "GetHRateXSUF") {
      GetHRateXSUF();
    } else if (analysisType == "EvalDeltaTc1") {
      EvalDeltaTc1();
    } else if (analysisType == "AnalyzeWithRDataFrame") {
      AnalyzeWithRDataFrame();
    } else {
      std::cerr << "Error: Unknown analysis type: " << analysisType
                << std::endl;
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
  // m_expName 必须先赋值，路径构造依赖它
  m_expName = m_configReader.GetExperimentName();

  if (m_configReader.GetDataType() == "Flux") {
    // Flux 模式：按探测器类型分子目录
    // 路径规则：FluxPath/ExpName/FIXM   FluxPath/ExpName/LiSi
    std::string base = m_configReader.GetFluxPath() + m_expName + "/";
    m_fixmOutputPath = base + "FIXM";
    m_lisiOutputPath = base + "LiSi";
    // 将完整子目录路径放入 m_outputPath，同时清空 m_expName，
    // 以兼容现有代码中 m_outputPath + m_expName + "/Outcome" 的写法
    m_outputPath = m_fixmOutputPath;
    m_expName = "";

    m_lisiConfig = &m_configReader.GetLiSiConfig();
    m_lisiChannelIDs = m_lisiConfig->Global.CHIDUSE;
  } else {
    m_outputPath = m_configReader.GetXSPath();
  }

  // Initialize Delta L cache
  if (!m_endeltaL) {
    const std::string &deltaLPath = m_configReader.GetDeltaLData();
    m_endeltaL = std::make_shared<TGraph>();
    get_graph(deltaLPath.c_str(), m_endeltaL.get());

    const std::vector<double> &LCalEn = m_fixmConfig->Global.LCalEn;
    double SumDL = 0.;
    for (const auto &c : LCalEn) {
      SumDL += m_endeltaL->Eval(c);
    }
    m_avgDL = LCalEn.empty() ? 0. : SumDL / LCalEn.size();
  }
}

void RDataFrameAnalysis::SwitchToLiSi() {
  m_savedConfig = m_fixmConfig;
  m_savedChannelIDs = m_channelIDs;
  m_savedOutputPath = m_outputPath;

  m_fixmConfig = m_lisiConfig;
  m_channelIDs = m_lisiChannelIDs;
  m_outputPath = m_lisiOutputPath;
  std::cout << "\n=== Switched to LiSi configuration ===" << std::endl;
}

void RDataFrameAnalysis::RestoreFromLiSi() {
  m_fixmConfig = m_savedConfig;
  m_channelIDs = m_savedChannelIDs;
  m_outputPath = m_savedOutputPath;
  std::cout << "=== Restored to FIXM configuration ===\n" << std::endl;
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

bool RDataFrameAnalysis::LoadXSENDFData(int nrebin) {
  // 默认实现为返回 true，可在派生类中覆写以加载实验样品截面数据
  return true;
}

bool RDataFrameAnalysis::LoadSTDENDFData(int nrebin) {
  // Clear existing data
  m_xs_nr.clear();
  m_xs_ntot.clear();
  for (auto &pair : m_hxs_nr) {
    if (pair.second)
      delete pair.second;
  }
  m_hxs_nr.clear();

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

  // Convert m_xs_nr TGraphs to TH1D and store in m_hxs_nr
  // Use rebinned bin edges (step=nrebin) to match hrate_re after Rebin(nrebin)
  if (m_nbins > 0 && !m_EnBins.empty()) {
    const int step = (nrebin > 1) ? nrebin : 1;
    const int nbins_reb = m_nbins / step;
    // Build edge array: pick every step-th entry from m_EnBins
    std::vector<double> rebinnedEdges;
    rebinnedEdges.reserve(nbins_reb + 1);
    for (int i = 0; i <= nbins_reb; i++) {
      rebinnedEdges.push_back(m_EnBins[i * step]);
    }

    for (const auto &pair : m_xs_nr) {
      std::string hname = "hxs_nr_" + pair.first;
      TH1D *h = new TH1D(hname.c_str(), pair.first.c_str(), nbins_reb,
                         rebinnedEdges.data());
      h->SetDirectory(nullptr);
      for (int i = 1; i <= nbins_reb; i++) {
        h->SetBinContent(i, pair.second->Eval(h->GetBinCenter(i)));
      }
      m_hxs_nr[pair.first] = h;
    }
    std::cout << "Converted reaction cross section TGraphs to TH1D"
              << " (nrebin=" << step << ", nbins=" << nbins_reb << ")."
              << std::endl;
  } else {
    std::cerr << "Warning: Energy bins not initialized. Cannot create TH1D for "
                 "cross sections. Please call InitializeEnergyBins() first."
              << std::endl;
  }
  LoadXSENDFData(nrebin);
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
  double Lengthgeo = Length - m_avgDL;
  auto endeltaL = m_endeltaL;

  return df.Define("En",
                   [Length, Lengthgeo, endeltaL](double tof) {
                     double En = -1e6;
                     auto curr_length = Length;
                     for (size_t i = 0; i < 3; i++) {
                       En = calEn(tof, curr_length);
                       curr_length = Lengthgeo + endeltaL->Eval(En);
                     }
                     return En;
                   },
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
         Form("Time Distribution - Channel %d;Time (ns);Entries", chID), 1800,
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
         1800, -1800, 0, 2000, 0, 2000},
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

  auto core_logic = [&]() {
    InitializeEnergyBins();

    if (!LoadGammaCuts()) {
      return;
    }

    std::cout << "Number of channels to analyze: " << m_channelIDs.size()
              << std::endl;

    auto df = CreateDataFrame();

    // Get energy divide from configuration
    const std::vector<double> &energyDivide = m_fixmConfig->Global.EnergyDivide;

    // Delta L data is loaded in InitializeCommonConfig
    std::cout << "Average Delta L: " << m_avgDL << std::endl;

    // Load ENDF data
    if (!LoadSTDENDFData()) {
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
      double Lengthgeo = Length - m_avgDL;
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
      auto has_xs_nr = (m_xs_nr.find(sampletype) != m_xs_nr.end());
      TGraph *xs_nr = has_xs_nr ? m_xs_nr.at(sampletype) : nullptr;

      auto df_withXS =
          df_withFactor.Define("factor_xs_nr",
                               [xs_nr](double En, double factor) {
                                 if (!xs_nr)
                                   return 0.0;
                                 double eval = xs_nr->Eval(En);
                                 return (eval > 0) ? factor / eval : 0.0;
                               },
                               {"En", "factor"});

      auto has_xs_ntot = (m_xs_ntot.find(sampletype) != m_xs_ntot.end());
      TGraph *xs_ntot = has_xs_ntot ? m_xs_ntot.at(sampletype) : nullptr;

      auto df_withYield = df_withXS.Define(
          "factor_xs_yield",
          [nd, xs_ntot](double En, double factor_xs_nf) {
            if (!xs_ntot)
              return 0.0;
            double eval = xs_ntot->Eval(En);
            if (eval <= 0.0)
              return 0.0;
            return factor_xs_nf / (1. - exp(-nd * eval)) / eval;
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
  };

  std::cout << "\n=== Running GetReactionRate for FIXM ===" << std::endl;
  core_logic();

  if (m_configReader.GetDataType() == "Flux" && HasLiSiConfig()) {
    SwitchToLiSi();
    core_logic();
    RestoreFromLiSi();
  }
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
         Form("T-Tg Distribution - Channel %d;T-T_{g} (ns);Counts", chID), 1500,
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

  // Initialize common configuration to prevent segmentation fault
  InitializeCommonConfig();

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
  int lcalChID = m_configReader.GetLCalChannel();
  int calDetID = 1;
  auto it_cal = m_fixmConfig->Channels.find(lcalChID);
  if (it_cal != m_fixmConfig->Channels.end()) {
    calDetID = it_cal->second.DetID;
  } else {
    std::cerr << "Warning: LCalChannel " << lcalChID
              << " not found in configuration!" << std::endl;
  }
  std::cout << "Flight path lengths for channels:" << std::endl;
  for (size_t i = 0; i < m_channelIDs.size(); i++) {
    int detID = m_fixmConfig->Channels.find(m_channelIDs[i])->second.DetID;
    std::cout << Form("DetID %d, CHID: %d, FP: %f m", detID, m_channelIDs[i],
                      l1 + (detID - calDetID) * DL_cell)
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

  auto core_logic = [&]() {
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

    std::string xspara_path = m_outputPath + m_expName + "/para/";
    std::string pileup_corr_name = xspara_path + "pileup_corr.root";
    auto frootcorr_out = new TFile(pileup_corr_name.c_str(), "RECREATE");

    std::string hratepileup_name =
        m_outputPath + m_expName + "/Outcome/hratepileup.root";
    auto fhratepileup_out = new TFile(hratepileup_name.c_str(), "RECREATE");

    // Map to store all h_corr_ histograms for plotting
    std::map<int, TH1D *> m_h_corr;

    // Process each channel
    for (int chID : m_channelIDs) {
      double tau = m_configReader.GetTau(chID);
      PrintChannelInfo(chID);

      // Get histogram for this channel
      TH1D *h1 = nullptr;
      file->GetObject(Form("h1_En_%d", chID), h1);
      if (!h1) {
        std::cerr << "  Warning: Channel " << chID << " En histogram not found!"
                  << std::endl;
        continue;
      }
      auto h_corr_ = (TH1D *)h1->Clone(Form("h_pileup_corr_%d", chID));
      h_corr_->SetDirectory(0); // Detach from file to keep it in memory

      TH1D *h1_tof = nullptr;
      file->GetObject(Form("h1_tof_%d", chID), h1_tof);
      if (!h1_tof) {
        std::cerr << "  Warning: Channel " << chID
                  << " tof histogram not found!" << std::endl;
        continue;
      }
      auto ht_corr_ = (TH1D *)h1_tof->Clone(Form("ht_pileup_corr_%d", chID));
      ht_corr_->SetDirectory(0);

      TH1D *h1_Enxs = nullptr;
      file->GetObject(Form("h1_Enxs_%d", chID), h1_Enxs);
      if (!h1_Enxs) {
        std::cerr << "  Warning: Channel " << chID
                  << " Enxs histogram not found!" << std::endl;
        continue;
      }

      TH1D *h1_EnxsNs = nullptr;
      file->GetObject(Form("h1_EnxsNs_%d", chID), h1_EnxsNs);
      if (!h1_EnxsNs) {
        std::cerr << "  Warning: Channel " << chID
                  << " EnxsNs histogram not found!" << std::endl;
        continue;
      }

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
        if (tau * R_pulse < 1.0 && R_pulse > 0.) {
          fcorr = 1.0 / (1.0 - tau * R_pulse);
        }
        h_corr_->SetBinContent(bx, fcorr);
      }

      int nbins_tof = h1_tof->GetNbinsX();
      for (int bx = 1; bx <= nbins_tof; bx++) {
        double bin_content = h1_tof->GetBinContent(bx);
        double bin_width = h1_tof->GetBinWidth(bx); // ns

        double R_pulse = bin_content / (N_pulse * bin_width);

        double fcorr = 1.0;
        if (tau * R_pulse < 1.0 && R_pulse > 0.) {
          fcorr = 1.0 / (1.0 - tau * R_pulse);
        }
        ht_corr_->SetBinContent(bx, fcorr);
      }

      // Apply corrections
      h1->Multiply(h_corr_);
      h1_tof->Multiply(ht_corr_);
      h1_Enxs->Multiply(h_corr_);
      h1_EnxsNs->Multiply(h_corr_);

      // Store histogram for plotting
      m_h_corr[chID] = h_corr_;

      frootcorr_out->cd();
      h_corr_->Write();
      ht_corr_->Write();

      fhratepileup_out->cd();
      h1->Write();
      h1_tof->Write();
      h1_Enxs->Write();
      h1_EnxsNs->Write();
    }
    frootcorr_out->Close();
    fhratepileup_out->Close();

    std::cout << "  Saved pileup correction table to: " << pileup_corr_name
              << std::endl;
    std::cout << "  Saved pileup corrected histograms to: " << hratepileup_name
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
  };

  std::cout << "\n=== Running GetPileupCorr for FIXM ===" << std::endl;
  core_logic();

  if (m_configReader.GetDataType() == "Flux" && HasLiSiConfig()) {
    SwitchToLiSi();
    core_logic();
    RestoreFromLiSi();
  }
}

void RDataFrameAnalysis::CoincheckSingleBunch() {
  PrintSectionHeader("CoincheckSingleBunch Analysis");

  int nrebin = m_configReader.GetNRebin();

  InitializeCommonConfig();

  auto core_logic = [&]() {
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

    // Open hratepileup.root and fluxattenuation.root files
    std::string outcomePath = m_outputPath + m_expName + "/Outcome";
    std::string outparaPath = m_outputPath + m_expName + "/para";
    std::string hratePath = outcomePath + "/hratepileup.root";
    std::string fluxattenPath = outparaPath + "/fluxattenuation.root";
    std::cout << "Opening hrate file: " << hratePath << std::endl;
    std::unique_ptr<TFile> fin_hrate(TFile::Open(hratePath.c_str()));
    if (!fin_hrate || fin_hrate->IsZombie()) {
      std::cerr << "Error: Cannot open " << hratePath << std::endl;
      return;
    }

    std::cout << "Opening flux attenuation file: " << fluxattenPath
              << std::endl;
    std::unique_ptr<TFile> fin_hatten(TFile::Open(fluxattenPath.c_str()));
    if (!fin_hatten || fin_hatten->IsZombie()) {
      std::cerr << "Warning: Cannot open " << fluxattenPath
                << ", flux attenuation correction will be skipped."
                << std::endl;
      fin_hatten.reset(); // 保持 nullptr 状态，后续 if(hatten) 守卫会跳过修正
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
        std::cerr << "  Warning: No histogram for channel " << chID
                  << std::endl;
        continue;
      }

      auto hrate = (TH1D *)h1E->Clone(Form("hrate_%d", chID));

      if (hrate == nullptr) {
        continue;
      }
      TH1D *hatten = nullptr;
      fin_hatten->GetObject(Form("htrans%d", DetID), hatten);

      // Normalize by nd and efficiency
      auto hrate_norm = (TH1D *)hrate->Clone(Form("hrate_norm_%d", chID));
      hrate_norm->Scale(1.0 / nd / chConfig.DetEff);

      m_hrate_norm[chID] = hrate_norm;

      // Rebin
      auto hrate_re = (TH1D *)hrate->Clone(Form("hrate_re_%d", chID));
      if (hatten != nullptr) {
        hrate_re->Divide(hatten);
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
    std::string foutPath = Form("%s/hrate_norm.root", outcomePath.c_str());
    auto *fout_hrate_norm = new TFile(foutPath.c_str(), "recreate");
    for (auto &ele_m : m_hrate_norm) {
      ele_m.second->Write();
    }
    fout_hrate_norm->Close();
    delete fout_hrate_norm;
    std::cout << "Output file saved: " << foutPath << std::endl;

    // 加载 ENDF 截面数据（与 CoincheckDoubleBunch 对齐）
    InitializeEnergyBins();
    if (!LoadSTDENDFData(nrebin)) {
      std::cerr << "Error: Failed to load ENDF data" << std::endl;
      return;
    }

    // Draw — 按样品类型逐一作图
    TCanvas *c = nullptr;
    TLegend *legd = nullptr;

    for (const auto &sample_typeuse : v_sample_typeuse) {
      if (m_hrate_re_norm.empty())
        continue;

      // Canvas 1: 除以 ENDF 截面后画各通道 + 平均值线
      c = new TCanvas();
      gPad->SetLogx();
      gPad->SetLogy();
      legd = new TLegend();
      auto hsum = (TH1D *)m_hrate_re_norm.begin()->second->Clone(
          Form("hsum_%s", sample_typeuse.c_str()));
      hsum->Reset();
      int ihist = 0;
      for (int chID : m_channelIDs) {
        if (m_sampletype[chID] != sample_typeuse)
          continue;
        if (m_hrate_re_norm.find(chID) == m_hrate_re_norm.end())
          continue;

        auto h = m_hrate_re_norm[chID];
        if (m_hxs_nr.find(m_sampletype[chID]) != m_hxs_nr.end() &&
            m_hxs_nr[m_sampletype[chID]] != nullptr) {
          h->Divide(m_hxs_nr[m_sampletype[chID]]);
        }
        h->SetLineColor(color[ihist % 10]);
        hsum->Add(h);
        ihist++;
      }

      auto havg = (TH1D *)hsum->Clone(Form("havg_%s", sample_typeuse.c_str()));
      havg->SetLineColor(kBlack);
      havg->SetLineWidth(2);
      havg->Draw("hist");
      legd->AddEntry(havg, "Average", "l");

      for (int chID : m_channelIDs) {
        if (m_sampletype[chID] != sample_typeuse)
          continue;
        if (m_hrate_re_norm.find(chID) == m_hrate_re_norm.end())
          continue;
        auto h = m_hrate_re_norm[chID];
        h->DrawCopy("same hist");
        legd->AddEntry(h, m_samplename[chID].c_str(), "l");
      }
      if (ihist > 0)
        havg->Scale(1. / (double)ihist);
      legd->Draw();

      // Canvas 2: 各通道 / 平均值（比值）
      c = new TCanvas();
      gPad->SetLogx();
      legd = new TLegend();
      ihist = 0;
      for (int chID : m_channelIDs) {
        if (m_sampletype[chID] != sample_typeuse)
          continue;
        if (m_hrate_re_norm.find(chID) == m_hrate_re_norm.end())
          continue;

        auto hratio = (TH1D *)m_hrate_re_norm[chID]->Clone(
            Form("hratio_%s_%d", sample_typeuse.c_str(), chID));
        hratio->SetYTitle("Ratio");
        hratio->Divide(havg);
        legd->AddEntry(hratio, m_samplename[chID].c_str(), "l");
        m_hratio[chID] = hratio;
        if (ihist == 0) {
          hratio->Draw("hist");
        } else {
          hratio->Draw("hist same");
        }
        ihist++;
      }
      legd->Draw();

      // Canvas 3: 统计误差
      c = new TCanvas();
      gPad->SetLogx();
      legd = new TLegend();
      ihist = 0;
      for (int chID : m_channelIDs) {
        if (m_sampletype[chID] != sample_typeuse)
          continue;
        if (m_herror_rate.find(chID) == m_herror_rate.end())
          continue;

        m_herror_rate[chID]->SetLineColor(color[ihist % 10]);
        m_herror_rate[chID]->SetYTitle("Error");
        if (ihist == 0) {
          m_herror_rate[chID]->DrawCopy("hist");
        } else {
          m_herror_rate[chID]->DrawCopy("hist same");
        }
        legd->AddEntry(m_herror_rate[chID], m_samplename[chID].c_str(), "l");
        ihist++;
      }
      legd->Draw();

      // Canvas 4: 符合误差（实验标准差，与 CoincheckDoubleBunch 一致）
      c = new TCanvas();
      gPad->SetLogx();
      auto gerror_coin = new TGraph();

      int n_channels = 0;
      for (int chID : m_channelIDs) {
        if (m_sampletype[chID] == sample_typeuse &&
            m_hrate_re_norm.find(chID) != m_hrate_re_norm.end())
          n_channels++;
      }

      for (int i = 0; i < havg->GetNbinsX(); i++) {
        auto x = havg->GetBinCenter(i + 1);
        double avg_val = havg->GetBinContent(i + 1);
        if (avg_val <= 0)
          continue;

        double sum_sq_diff = 0;
        for (int chID : m_channelIDs) {
          if (m_sampletype[chID] != sample_typeuse)
            continue;
          if (m_hrate_re_norm.find(chID) == m_hrate_re_norm.end())
            continue;

          double rate_k = m_hrate_re_norm[chID]->GetBinContent(i + 1);
          if (rate_k > 0) {
            double q_k = rate_k / avg_val;
            sum_sq_diff += (q_k - 1.0) * (q_k - 1.0);
          }
        }

        if (n_channels > 1) {
          double s_qk = std::sqrt(sum_sq_diff / (n_channels - 1));
          double s_qbar = s_qk / std::sqrt(n_channels);
          if (s_qbar > 0) {
            gerror_coin->AddPoint(x, s_qbar);
          }
        }
      }

      auto herror_coin =
          (TH1D *)havg->Clone(Form("herror_coin_%s", sample_typeuse.c_str()));
      herror_coin->SetYTitle("Uncertainty");
      gerror_coin->GetYaxis()->SetTitle("Uncertainty");
      herror_coin->Reset();
      graph2hist(gerror_coin, herror_coin);
      herror_coin->Smooth();
      herror_coin->Draw("hist");
      herror_coin->SetLineColor(kRed);

      // 写出符合效率文件（路径与 CoincheckDoubleBunch 对齐：Outcome/UN...）
      std::string out_dat = Form("%s/UN%sCOINEFF.dat", outcomePath.c_str(),
                                 sample_typeuse.c_str());
      std::ofstream ofs(out_dat);
      for (int i = 0; i < herror_coin->GetNbinsX(); i++) {
        ofs << herror_coin->GetBinCenter(i + 1) << "\t"
            << herror_coin->GetBinContent(i + 1) << std::endl;
      }
      ofs.close();
      std::cout << "  Saved coincidence efficiency to: " << out_dat
                << std::endl;

      delete gerror_coin;
    }
  };

  std::cout << "\n=== Running CoincheckSingleBunch for FIXM ===" << std::endl;
  core_logic();

  if (m_configReader.GetDataType() == "Flux" && HasLiSiConfig()) {
    SwitchToLiSi();
    core_logic();
    RestoreFromLiSi();
  }

  PrintClosePrompt();
}

void RDataFrameAnalysis::Coincheck() {
  std::string beamMode = m_configReader.GetBeamMode();
  if (beamMode == "SingleBunch") {
    CoincheckSingleBunch();
  } else if (beamMode == "DoubleBunch") {
    CoincheckDoubleBunch();
  } else {
    std::cerr << "Error: Unknown BeamMode " << beamMode
              << ", cannot decide which Coincheck to run." << std::endl;
  }
}

void RDataFrameAnalysis::CoincheckDoubleBunch() {
  PrintSectionHeader("CoincheckDoubleBunch Analysis");

  int nrebin = m_configReader.GetNRebin();
  InitializeCommonConfig();

  auto core_logic = [&]() {
    const std::string &dataDir = m_configReader.GetDataPath();

    std::cout << "Number of channels: " << m_channelIDs.size() << std::endl;

    std::map<int, double> m_tg;
    std::map<int, double> m_length;
    std::map<int, double> m_thres;
    std::map<int, double> m_mass;
    std::map<int, double> m_radius;
    std::map<int, double> m_A;
    std::map<int, double> m_nd;
    std::map<int, double> m_eff;
    std::map<int, int> m_detID;
    std::map<int, std::vector<double>> m_fitres;
    std::map<int, std::string> m_samplename;
    std::map<int, std::string> m_sampletype;
    std::set<std::string> v_sample_typeuse;

    for (int chID : m_channelIDs) {
      auto it = m_fixmConfig->Channels.find(chID);
      if (it == m_fixmConfig->Channels.end()) {
        continue;
      }
      const auto &chConfig = it->second;

      m_detID[chID] = chConfig.DetID;
      m_tg[chID] = chConfig.Tg;
      m_length[chID] = chConfig.Length;
      m_thres[chID] = chConfig.Threshold;
      int sampleid = chConfig.SampleNumber;
      m_eff[chID] = chConfig.DetEff;
      std::string sampletype = chConfig.SampleType;
      std::string samplename = sampletype + std::to_string(sampleid);

      v_sample_typeuse.insert(sampletype);
      m_sampletype[chID] = sampletype;

      m_fitres[chID] = chConfig.ThresholdEDivide;

      double sample_m = chConfig.Mass;
      double sample_r = chConfig.Radius / 10.; // to cm
      double sample_area = sample_r * sample_r * TMath::Pi();
      double sample_t = sample_m / sample_area; // mg/cm2
      double sample_A = chConfig.A;

      double ArealDensity =
          sample_t / sample_A * Na * barn_to_cm2 * mg_to_g; // atoms/barn
      m_nd[chID] = ArealDensity;
      m_samplename[chID] = samplename;
    }

    // Files
    std::string outcomePath = m_outputPath + m_expName + "/Outcome";
    std::string paraPath = m_outputPath + m_expName + "/para/";
    std::string hratePath = outcomePath + "/hratepileup.root";
    std::string fluxattenPath = paraPath + "/fluxattenuation.root";

    std::unique_ptr<TFile> fin_hrate(TFile::Open(hratePath.c_str()));
    if (!fin_hrate || fin_hrate->IsZombie()) {
      std::cerr << "Error: Cannot open " << hratePath << std::endl;
      return;
    }
    std::unique_ptr<TFile> fin_hatten(TFile::Open(fluxattenPath.c_str()));
    if (!fin_hatten || fin_hatten->IsZombie()) {
      std::cerr << "Warning: Cannot open " << fluxattenPath
                << ", flux attenuation correction will be skipped."
                << std::endl;
      fin_hatten.reset(); // 保持 nullptr 状态，后续 if(htrans) 守卫会跳过修正
    }

    std::string hratexsufPath = outcomePath + "/hratexsuf.root";
    std::unique_ptr<TFile> fin_hratexsuf(TFile::Open(hratexsufPath.c_str()));
    if (!fin_hratexsuf || fin_hratexsuf->IsZombie()) {
      std::cerr << "Error: Cannot open " << hratexsufPath << std::endl;
      return;
    }

    std::map<int, TH1D *> m_hratem;
    std::map<int, TH1D *> m_hrate_norm;
    std::map<int, TH1D *> m_hrate_re;
    std::map<int, TH1D *> m_herror_rate;
    std::map<int, TH1D *> m_hrate_re_norm;
    std::map<int, TH1D *> m_hratio;
    std::map<int, TH1D *> m_hratio_dec;
    std::map<int, TH1D *> h_hflux_atten;

    for (int chID : m_channelIDs) {
      TH1D *h1E = nullptr;
      std::string name = Form("h1_En_%d", chID);
      fin_hrate->GetObject(name.c_str(), h1E);
      if (!h1E)
        continue;

      auto hrate = (TH1D *)h1E->Clone(Form("hrate_%d", chID));

      TH1D *hrate_uf = nullptr;
      fin_hratexsuf->GetObject(Form("h1_En_%d", chID), hrate_uf);
      if (hrate != nullptr && hrate_uf != nullptr) {
        auto hrate_m = (TH1D *)hrate_uf->Clone(Form("hrate_m_%d", chID));
        for (int i = 0; i < hrate_m->FindBin(1e4) + 1; i++) {
          hrate_m->SetBinContent(i + 1, hrate->GetBinContent(i + 1));
        }
        TH1D *htrans = nullptr;
        std::string transname = Form("htrans%d", m_detID[chID]);
        fin_hatten->GetObject(transname.c_str(), htrans);
        if (htrans)
          hrate_m->Divide(htrans);
        m_hratem[chID] = hrate_m;

        auto hrate_norm = (TH1D *)hrate_m->Clone(Form("hrate_norm_%d", chID));
        hrate_norm->Scale(1. / m_nd[chID] / m_eff[chID]);
        m_hrate_norm[chID] = hrate_norm;

        auto hrate_re = (TH1D *)hrate_m->Clone(Form("hrate_re_%d", chID));
        hrate_re->Rebin(nrebin);
        m_hrate_re[chID] = hrate_re;

        auto herror_rate =
            (TH1D *)hrate_re->Clone(Form("herror_rate_%d", chID));
        get_sta_errorhist(hrate_re, herror_rate);
        m_herror_rate[chID] = herror_rate;

        auto hrate_re_norm =
            (TH1D *)hrate_re->Clone(Form("hrate_re_norm_%d", chID));
        hrate_re_norm->Scale(1. / m_nd[chID] / m_eff[chID]);
        m_hrate_re_norm[chID] = hrate_re_norm;
      } else {
        std::cout << "rate calculate no hist id = " << chID << std::endl;
      }
    }

    // get ENDF DATA
    InitializeEnergyBins();
    if (!LoadSTDENDFData(nrebin)) {
      std::cerr << "Error: Failed to load ENDF data" << std::endl;
      return;
    }

    // Write
    std::string foutPath = Form("%s/hrate_norm.root", outcomePath.c_str());
    auto *fout_hrate_norm = new TFile(foutPath.c_str(), "recreate");
    for (auto ele_m : m_hrate_norm) {
      int chid = ele_m.first;
      ele_m.second->Write();
      if (m_hratem.find(chid) != m_hratem.end()) {
        m_hratem[chid]->Write();
      }
    }
    fout_hrate_norm->Close();
    delete fout_hrate_norm;

    // Draw
    TCanvas *c = nullptr;
    TLegend *legd = nullptr;

    for (const auto &sample_typeuse : v_sample_typeuse) {
      if (m_hrate_re_norm.empty())
        continue;

      c = new TCanvas();
      gPad->SetLogx();
      gPad->SetLogy();
      legd = new TLegend();
      auto hsum = (TH1D *)m_hrate_re_norm.begin()->second->Clone(
          Form("hsum_%s", sample_typeuse.c_str()));
      hsum->Reset();
      int ihist = 0;
      for (int chID : m_channelIDs) {
        if (m_sampletype[chID] != sample_typeuse)
          continue;
        if (m_hrate_re_norm.find(chID) == m_hrate_re_norm.end())
          continue;

        auto h = m_hrate_re_norm[chID];
        if (m_hxs_nr.find(m_sampletype[chID]) != m_hxs_nr.end() &&
            m_hxs_nr[m_sampletype[chID]] != nullptr) {
          h->Divide(m_hxs_nr[m_sampletype[chID]]);
        }
        h->SetLineColor(color[ihist % 10]);
        hsum->Add(h);
        ihist++;
      }

      auto havg = (TH1D *)hsum->Clone(Form("havg_%s", sample_typeuse.c_str()));

      havg->SetLineColor(kBlack);
      havg->SetLineWidth(2);
      havg->Draw("hist");
      legd->AddEntry(havg, "Average", "l");

      for (int chID : m_channelIDs) {
        if (m_sampletype[chID] != sample_typeuse)
          continue;
        if (m_hrate_re_norm.find(chID) == m_hrate_re_norm.end())
          continue;

        auto h = m_hrate_re_norm[chID];
        h->DrawCopy("same hist");
        legd->AddEntry(h, m_samplename[chID].c_str(), "l");
      }
      if (ihist > 0)
        havg->Scale(1. / (double)ihist);
      legd->Draw();

      c = new TCanvas();
      gPad->SetLogx();
      legd = new TLegend();
      ihist = 0;
      for (int chID : m_channelIDs) {
        if (m_sampletype[chID] != sample_typeuse)
          continue;
        if (m_hrate_re_norm.find(chID) == m_hrate_re_norm.end())
          continue;

        auto hratio = (TH1D *)m_hrate_re_norm[chID]->Clone(
            Form("hratio_%s_%d", sample_typeuse.c_str(), chID));
        hratio->SetYTitle("Ratio");
        hratio->Divide(havg);
        legd->AddEntry(hratio, m_samplename[chID].c_str(), "l");
        m_hratio[chID] = hratio;
        if (ihist == 0) {
          hratio->Draw("hist");
        } else {
          hratio->Draw("hist same");
        }
        ihist++;
      }
      legd->Draw();

      c = new TCanvas();
      gPad->SetLogx();
      legd = new TLegend();
      ihist = 0;
      for (int chID : m_channelIDs) {
        if (m_sampletype[chID] != sample_typeuse)
          continue;
        if (m_herror_rate.find(chID) == m_herror_rate.end())
          continue;

        m_herror_rate[chID]->SetLineColor(color[ihist % 10]);
        m_herror_rate[chID]->SetYTitle("Error");

        if (ihist == 0) {
          m_herror_rate[chID]->DrawCopy("hist");
        } else {
          m_herror_rate[chID]->DrawCopy("hist same");
        }
        legd->AddEntry(m_herror_rate[chID], m_samplename[chID].c_str(), "l");
        ihist++;
      }
      legd->Draw();

      c = new TCanvas();
      gPad->SetLogx();
      auto gerror_coin = new TGraph();

      int n_channels = m_hrate_re_norm.size();

      for (int i = 0; i < havg->GetNbinsX(); i++) {
        auto x = havg->GetBinCenter(i + 1);
        double avg_val = havg->GetBinContent(i + 1);
        if (avg_val <= 0)
          continue;

        double sum_sq_diff = 0;

        for (int chID : m_channelIDs) {
          if (m_sampletype[chID] != sample_typeuse)
            continue;
          if (m_hrate_re_norm.find(chID) == m_hrate_re_norm.end())
            continue;

          double rate_k = m_hrate_re_norm[chID]->GetBinContent(i + 1);
          if (rate_k > 0) {
            double q_k = rate_k / avg_val;
            // q_bar is theoretically 1 since havg is the average of
            // m_hrate_re_norm
            sum_sq_diff += (q_k - 1.0) * (q_k - 1.0);
          }
        }

        if (n_channels > 1) {
          double s_qk = std::sqrt(sum_sq_diff / (n_channels - 1));
          double s_qbar = s_qk / std::sqrt(n_channels);
          if (s_qbar > 0) {
            gerror_coin->AddPoint(x, s_qbar);
          }
        }
      }
      auto herror_coin =
          (TH1D *)havg->Clone(Form("herror_coin_%s", sample_typeuse.c_str()));
      herror_coin->SetYTitle("Uncertainty");
      gerror_coin->GetYaxis()->SetTitle("Uncertainty");
      herror_coin->Reset();
      graph2hist(gerror_coin, herror_coin);
      herror_coin->Smooth();
      herror_coin->Draw("hist");
      herror_coin->SetLineColor(kRed);

      std::string out_dat = Form("%s/UN%sCOINEFF.dat", outcomePath.c_str(),
                                 sample_typeuse.c_str());
      std::ofstream ofs(out_dat);
      for (int i = 0; i < herror_coin->GetNbinsX(); i++) {
        ofs << herror_coin->GetBinCenter(i + 1) << "\t"
            << herror_coin->GetBinContent(i + 1) << std::endl;
      }
      ofs.close();

      delete gerror_coin;
    }
  };

  std::cout << "\n=== Running CoincheckDoubleBunch for FIXM ===" << std::endl;
  core_logic();

  if (m_configReader.GetDataType() == "Flux" && HasLiSiConfig()) {
    SwitchToLiSi();
    core_logic();
    RestoreFromLiSi();
  }

  PrintClosePrompt();
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

  // ========================================================
  // Proton beam data loading (before per-file loop so fTime can be printed)
  // ========================================================
  const std::string &beamDataPath = m_configReader.GetBeamDataPath();
  int experimentTimeStart = m_configReader.GetExperimentTimeStart();
  int experimentTimeEnd = m_configReader.GetExperimentTimeEnd();
  double BeamPowerW = m_configReader.GetBeamPower() * 1000.0; // kW -> W
  double ProtonEnergy = m_configReader.GetProtonEnergy();     // eV

  // Build the combined TChain first (needed for tMin/tMax scan)
  auto chain = std::make_unique<TChain>("WNSRawTree");
  for (const auto &t0file : T0list) {
    chain->Add((T0Path + t0file).c_str());
  }
  Long64_t nentries = chain->GetEntries();
  if (nentries == 0) {
    std::cerr << "Error: No entries found in T0 files" << std::endl;
    return;
  }

  // Proton data vector: (ns_timestamp, flow)
  std::vector<std::pair<ULong64_t, Long64_t>> protonData;

  if (experimentTimeStart != 0 && experimentTimeEnd != 0 && BeamPowerW > 0 &&
      ProtonEnergy > 0) {
    // Scan chain for global time bounds to filter proton file
    ULong64_t tMin_global = std::numeric_limits<ULong64_t>::max();
    ULong64_t tMax_global = 0;
    {
      ULong64_t tRangeMin = 0, tRangeMax = 0;
      chain->SetBranchStatus("*", 0);
      chain->SetBranchStatus("TimeRange_Min", 1);
      chain->SetBranchStatus("TimeRange_Max", 1);
      chain->SetBranchAddress("TimeRange_Min", &tRangeMin);
      chain->SetBranchAddress("TimeRange_Max", &tRangeMax);
      for (Long64_t i = 0; i < nentries; ++i) {
        chain->GetEntry(i);
        if (tRangeMin > 0 && tRangeMin < tMin_global)
          tMin_global = tRangeMin;
        if (tRangeMax > tMax_global)
          tMax_global = tRangeMax;
      }
      std::cout << "  Time range: [" << tMin_global << ", " << tMax_global
                << "] ns" << std::endl;
    }

    auto nproton_cut = BeamPowerW / echarge / ProtonEnergy / FPulse / 1e7 *
                       m_configReader.GetProtonCutPercent();
    std::cout << "  Number of protons cut: " << nproton_cut << std::endl;
    // Stream-read proton file, only keeping rows in [tMin_global, tMax_global]
    protonData.reserve(1 << 20);
    Long64_t totalLineCount = 0;
    for (int t = experimentTimeStart; t <= experimentTimeEnd;) {
      std::string protonFilePath =
          beamDataPath + "proton_" + std::to_string(t) + ".txt";
      std::cout << "Reading proton data from: " << protonFilePath << std::endl;
      std::ifstream protonFile(protonFilePath);
      if (!protonFile.is_open()) {
        std::cerr << "Warning: Cannot open proton data file: " << protonFilePath
                  << std::endl;
      } else {
        std::string line;
        std::getline(protonFile, line); // skip header
        Long64_t lineCount = 0;
        while (std::getline(protonFile, line)) {
          if (line.empty())
            continue;
          std::istringstream iss(line);
          Long64_t sec;
          ULong64_t nsec;
          Long64_t flow;
          if (iss >> sec >> nsec >> flow) {
            auto t0 = static_cast<ULong64_t>(sec) * 1000000000ULL + nsec;
            if (t0 < tMin_global)
              continue;
            if (t0 > tMax_global)
              break;
            if (flow < nproton_cut)
              continue;
            protonData.emplace_back(t0, flow);
            ++lineCount;
          }
        }
        protonFile.close();
        totalLineCount += lineCount;
        std::cout << "  Loaded " << lineCount << " proton data points"
                  << std::endl;
      }
      int year = t / 100;
      int month = t % 100;
      if (month == 12) {
        year++;
        month = 1;
      } else {
        month++;
      }
      t = year * 100 + month;
    }
    std::sort(protonData.begin(), protonData.end(),
              [](const auto &a, const auto &b) { return a.first < b.first; });
    std::cout << "  Loaded total " << totalLineCount << " proton data points"
              << std::endl;
  } else {
    std::cerr
        << "Warning: ExperimentTimeStart/End/BeamPower/ProtonEnergy not set, "
           "skipping proton fTime output"
        << std::endl;
  }

  // Helper lambda: compute fTime for a given [fileMin, fileMax] from protonData
  auto computeFTime = [&](ULong64_t fileMin, ULong64_t fileMax) -> double {
    if (protonData.empty() || BeamPowerW <= 0 || ProtonEnergy <= 0)
      return -1.0;
    auto lo = std::lower_bound(
        protonData.cbegin(), protonData.cend(),
        std::make_pair(fileMin, std::numeric_limits<Long64_t>::min()),
        [](const auto &e, const auto &v) { return e.first < v.first; });
    Long64_t sumFlow = 0;
    for (auto it = lo; it != protonData.cend() && it->first <= fileMax; ++it)
      sumFlow += it->second;
    return static_cast<double>(sumFlow) * 1e7 /
           (BeamPowerW / echarge / ProtonEnergy);
  };

  // Process each file individually to get measurement time
  std::cout << "========================================" << std::endl;
  std::cout << "Processing Individual T0 Files" << std::endl;
  std::cout << "========================================" << std::endl;

  std::vector<double> fileTimes;
  std::vector<Long64_t> fileT0Counts;
  std::vector<double> fileFTimes; // proton fTime per file entry

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
      fileFTimes.push_back(-1.0);
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
      fileFTimes.push_back(-1.0);
      continue;
    }

    // Restore all branch status before RDataFrame processing for this tree
    tree->SetBranchStatus("*", 1);

    // Create RDataFrame from tree
    ROOT::RDataFrame df(*tree);
    auto count_entries = df.Count();
    auto sumT0_TCM = df.Sum<Int_t>("T0Number_TCM");
    // Filter out invalid entries (TimeRange_Max==0 means no data) before
    // Min/Max
    auto df_valid = df.Filter("TimeRange_Max > 0");
    auto minTRangeMin = df_valid.Min<ULong64_t>("TimeRange_Min");
    auto maxTRangeMax = df_valid.Max<ULong64_t>("TimeRange_Max");

    Long64_t nEntries = *count_entries;
    Long64_t fileT0Count = *sumT0_TCM;
    double fileTime = fileT0Count / FPulse;
    ULong64_t fileMin = *minTRangeMin;
    ULong64_t fileMax = *maxTRangeMax;

    fileTimes.push_back(fileTime);
    fileT0Counts.push_back(fileT0Count);

    double fTime = computeFTime(fileMin, fileMax);
    fileFTimes.push_back(fTime);

    std::cout << "  Entry Number: " << nEntries << std::endl;
    std::cout << "  T0Number_TCM: " << fileT0Count
              << ",  Measurement time (TCM): " << fileTime << " s ("
              << fileTime / 3600.0 << " h)" << std::endl;
    if (fTime >= 0.0) {
      std::cout << "  Proton fTime:             " << fTime << " s ("
                << fTime / 3600.0 << " h)" << std::endl;
    }
    std::cout << std::endl;
  }

  std::cout << "========================================" << std::endl;
  std::cout << std::endl;

  // Restore all branch status before RDataFrame processing.
  // SetBranchStatus("*", 0) was called during the proton tMin/tMax scan above.
  chain->SetBranchStatus("*", 1);

  // chain already built above; now run RDataFrame processing
  std::cout << "Total entries in chain: " << nentries << std::endl;
  std::cout << "Processing with RDataFrame..." << std::endl;

  ROOT::RDataFrame df(*chain);
  auto df_indexed = df.DefineSlotEntry(
      "EntryIndex", [](unsigned int /*slot*/, ULong64_t entry) {
        return static_cast<Double_t>(entry);
      });
  auto df_filetime = df_indexed.Define(
      "fileTime",
      [FPulse](Int_t t0) { return static_cast<double>(t0) / FPulse; },
      {"T0Number_TCM"});

  auto totalT0_TCM = df_filetime.Sum<Int_t>("T0Number_TCM");
  auto h1 = df_filetime.Histo1D(
      {"h1_fileTime", ";Entry Number;Measurement Time (s)",
       static_cast<int>(nentries), 0.0, static_cast<double>(nentries)},
      "EntryIndex", "fileTime");

  Long64_t total_TCM = *totalT0_TCM;
  double totalTime_TCM = total_TCM / FPulse;

  auto h1_ptr = h1.GetPtr();
  h1_ptr->SetLineColor(kBlue);
  h1_ptr->SetStats(0);

  std::cout << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << "Summary" << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << "Total entries: " << nentries << std::endl;
  std::cout << "T0Number_TCM:" << std::endl;
  std::cout << "  Total counts: " << total_TCM << std::endl;
  std::cout << "  Measurement time: " << totalTime_TCM << " seconds"
            << std::endl;
  std::cout << "  Measurement time: " << totalTime_TCM / 3600.0 << " hours"
            << std::endl;

  if (!protonData.empty()) {
    double totalTime_Proton = 0.0;
    for (double fTime : fileFTimes) {
      if (fTime > 0.0) {
        totalTime_Proton += fTime;
      }
    }
    std::cout << "Proton:" << std::endl;
    std::cout << "  Measurement time: " << totalTime_Proton << " seconds"
              << std::endl;
    std::cout << "  Measurement time: " << totalTime_Proton / 3600.0 << " hours"
              << std::endl;
  }

  std::cout << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << std::endl;

  // ========================================================
  // Proton beam data comparison with T0 (TCM)
  // ========================================================
  std::cout << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << "Proton Beam Data Comparison" << std::endl;
  std::cout << "========================================" << std::endl;

  if (protonData.empty()) {
    std::cerr << "No proton data loaded, skipping comparison" << std::endl;
    std::cout << "Close plot window to continue..." << std::endl;
    return;
  }

  auto hProton =
      new TH1D("h_ProtonTime", ";Entry Number;Measurement Time (s)",
               static_cast<int>(nentries), 0.0, static_cast<double>(nentries));
  hProton->SetDirectory(0);
  {
    ULong64_t tRangeMin = 0;
    ULong64_t tRangeMax = 0;
    chain->SetBranchStatus("*", 0);
    chain->SetBranchStatus("TimeRange_Min", 1);
    chain->SetBranchStatus("TimeRange_Max", 1);
    chain->SetBranchAddress("TimeRange_Min", &tRangeMin);
    chain->SetBranchAddress("TimeRange_Max", &tRangeMax);

    std::cout << "  Processing " << nentries << " entries for proton sum..."
              << std::endl;

    // Cursor into protonData: since both protonData and entries are
    // time-ordered, the lower bound of the next entry's TimeRange_Min can only
    // move forward. We maintain lower_cursor so each proton row is visited at
    // most twice.
    auto lower_cursor = protonData.cbegin();

    for (Long64_t iEntry = 0; iEntry < nentries; ++iEntry) {
      chain->GetEntry(iEntry);

      if (tRangeMin == 0 && tRangeMax == 0) {
        hProton->SetBinContent(static_cast<int>(iEntry) + 1, 0.0);
        continue;
      }

      // Advance lower_cursor to first element with t0 >= tRangeMin
      lower_cursor = std::lower_bound(
          lower_cursor, protonData.cend(),
          std::make_pair(tRangeMin, std::numeric_limits<Long64_t>::min()),
          [](const auto &elem, const auto &val) {
            return elem.first < val.first;
          });

      // Sum all Flow03_User where t0 <= tRangeMax
      Long64_t sumFlow = 0;
      for (auto it = lower_cursor;
           it != protonData.cend() && it->first <= tRangeMax; ++it) {
        sumFlow += it->second;
      }

      double fTime = 0.0;
      if (BeamPowerW > 0 && ProtonEnergy > 0) {
        // fTime = sumFlow * 1e7 / BeamPower / echarge / ProtonEnergy
        fTime = static_cast<double>(sumFlow) * 1e7 /
                (BeamPowerW / echarge / ProtonEnergy);
      }

      hProton->SetBinContent(static_cast<int>(iEntry) + 1, fTime);
    }
    std::cout << "  Proton histogram filled" << std::endl;
  }

  // Step 4: Draw comparison Canvas
  auto c2 =
      new TCanvas("c_ProtonComparison", "Proton vs T0 Comparison", 1200, 600);
  c2->Divide(2, 1);

  // Left panel: Overlay TCM measurement time and proton fTime (dual Y-axis if
  // scale differ, but they are both seconds)
  c2->cd(1);
  auto legd_proton = new TLegend(0.55, 0.7, 0.9, 0.9);

  // Clone h1 (fileTime) for this canvas
  auto h1_clone = static_cast<TH1D *>(h1_ptr->Clone("h1_fileTime_clone"));
  h1_clone->SetLineColor(kRed);
  h1_clone->SetStats(0);
  h1_clone->SetYTitle("Measurement Time (s)");
  h1_clone->Draw("hist");
  legd_proton->AddEntry(h1_clone, "by TCM", "l");

  // Draw proton hist on same canvas
  hProton->SetLineColor(kBlue);
  hProton->SetStats(0);
  hProton->Draw("hist same");
  legd_proton->AddEntry(hProton, "by Proton", "l");
  legd_proton->Draw();

  // Right panel: Ratio fileTime / fTime
  c2->cd(2);
  auto hRatioProton = static_cast<TH1D *>(h1_ptr->Clone("hratio_Time_Proton"));
  hRatioProton->SetTitle(";Entry Number;FileTime / fTime");
  // Avoid division by zero
  for (int iBin = 1; iBin <= hRatioProton->GetNbinsX(); ++iBin) {
    double protonVal = hProton->GetBinContent(iBin);
    if (protonVal > 0) {
      hRatioProton->SetBinContent(iBin, hRatioProton->GetBinContent(iBin) /
                                            protonVal);
      hRatioProton->SetBinError(iBin, 0.0);
    } else {
      hRatioProton->SetBinContent(iBin, 0.0);
    }
  }
  hRatioProton->SetLineColor(kBlack);
  hRatioProton->SetStats(0);
  hRatioProton->Draw("hist");

  std::cout << std::endl;
  std::cout << "Proton comparison canvas created." << std::endl;
  std::cout << "Close plot window to continue..." << std::endl;
}

void RDataFrameAnalysis::GetHRateXSUF() {
  PrintSectionHeader("GetHRateXSUF Analysis");

  InitializeCommonConfig();
  InitializeEnergyBins();

  auto core_logic = [&]() {
    // Get ntimes from configuration
    double ntimes = m_fixmConfig->Global.UFRandomTimes;

    std::cout << "Number of channels to analyze: " << m_channelIDs.size()
              << std::endl;
    std::cout << "Monte Carlo sampling factor: " << ntimes << std::endl;

    // Load ENDF data using existing member function
    if (!LoadSTDENDFData()) {
      std::cerr << "Error: Failed to load ENDF data" << std::endl;
      return;
    }

    // Get outcome path from configuration
    std::string outcomePath = m_outputPath + m_expName + "/Outcome";

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

    // Delta L cache is pre-loaded in InitializeCommonConfig
    auto endeltaL = m_endeltaL;
    double avgDL = m_avgDL;

    // Lambda function to calculate hrate (only En)
    auto calhrate = [this, endeltaL, avgDL](TH1D *hrate, TH1D *hrateout,
                                            int chid, double /*ntimes*/) {
      hrateout->Reset();

      // Get channel length
      auto it_ch = m_fixmConfig->Channels.find(chid);
      if (it_ch == m_fixmConfig->Channels.end())
        return;
      double Length = it_ch->second.Length;
      double Lengthgeo = Length - avgDL;

      double EnCutHigh = m_configReader.GetEnergyCutHigh();
      const int nsub = m_fixmConfig->Global.Intergralnsub;

      for (int ibin_out = 1; ibin_out <= hrateout->GetNbinsX(); ibin_out++) {
        double En_lo = hrateout->GetBinLowEdge(ibin_out);
        double En_hi = En_lo + hrateout->GetBinWidth(ibin_out);

        if (En_lo > EnCutHigh)
          continue;

        // En -> TOF (note: low energy corresponds to high TOF)
        double tof_hi = calTOF(En_lo, Length);
        double tof_lo = calTOF(En_hi, Length);

        double sum_weight = 0.0;
        double dtof = (tof_hi - tof_lo) / nsub;

        for (int isub = 0; isub < nsub; isub++) {
          double tof = tof_lo + dtof * (isub + 0.5);

          int bin_in = hrate->FindBin(tof);
          double density =
              hrate->GetBinContent(bin_in) / hrate->GetBinWidth(bin_in);
          if (density <= 0)
            continue;

          double En = -1e6;
          double length = Length;
          for (size_t j = 0; j < 3; j++) {
            En = calEn(tof, length);
            length = Lengthgeo + endeltaL->Eval(En);
          }

          if (En <= EnCutHigh) {
            sum_weight += density * dtof;
          }
        }

        hrateout->SetBinContent(ibin_out, sum_weight);
      }
    };

    // Lambda function to calculate hratexs (without self-shielding correction)
    // Use member variable m_xs_nr instead of local m_xs_nf
    auto calhratexs = [this, &m_sampletype, endeltaL,
                       avgDL](TH1D *hrate, TH1D *hratexs, int chid,
                              double /*ntimes*/) {
      hratexs->Reset();
      auto sampletype = m_sampletype[chid];

      // Get channel length
      auto it_ch = m_fixmConfig->Channels.find(chid);
      if (it_ch == m_fixmConfig->Channels.end())
        return;
      double Length = it_ch->second.Length;
      double Lengthgeo = Length - avgDL;

      // Use member variable m_xs_nr
      auto it = m_xs_nr.find(sampletype);
      if (it == m_xs_nr.end()) {
        std::cerr << "  Warning: No cross section data for sample type "
                  << sampletype << std::endl;
        return;
      }
      auto gENDFNF = it->second;

      double EnCutHigh = m_configReader.GetEnergyCutHigh();
      const int nsub = m_fixmConfig->Global.Intergralnsub;

      for (int ibin_out = 1; ibin_out <= hratexs->GetNbinsX(); ibin_out++) {
        double En_lo = hratexs->GetBinLowEdge(ibin_out);
        double En_hi = En_lo + hratexs->GetBinWidth(ibin_out);

        if (En_lo > EnCutHigh)
          continue;

        // En -> TOF (note: low energy corresponds to high TOF)
        double tof_hi = calTOF(En_lo, Length);
        double tof_lo = calTOF(En_hi, Length);

        double sum_weight = 0.0;
        double dtof = (tof_hi - tof_lo) / nsub;

        for (int isub = 0; isub < nsub; isub++) {
          double tof = tof_lo + dtof * (isub + 0.5);

          int bin_in = hrate->FindBin(tof);
          double density =
              hrate->GetBinContent(bin_in) / hrate->GetBinWidth(bin_in);
          if (density <= 0)
            continue;

          double En = -1e6;
          double length = Length;
          for (size_t j = 0; j < 3; j++) {
            En = calEn(tof, length);
            length = Lengthgeo + endeltaL->Eval(En);
          }

          auto xs_nf = gENDFNF->Eval(En);
          if (En <= EnCutHigh && xs_nf > 0) {
            sum_weight += density * dtof / xs_nf;
          }
        }

        hratexs->SetBinContent(ibin_out, sum_weight);
      }
    };

    // Lambda function to calculate hratexsNs (with self-shielding correction)
    // Use member variables m_xs_nr and m_xs_ntot
    auto calhratexsNs = [this, &m_sampletype, &m_nd, endeltaL,
                         avgDL](TH1D *hrate, TH1D *hratexs, int chid,
                                double /*ntimes*/) {
      hratexs->Reset();
      auto sampletype = m_sampletype[chid];

      // Get channel length
      auto it_ch = m_fixmConfig->Channels.find(chid);
      if (it_ch == m_fixmConfig->Channels.end())
        return;
      double Length = it_ch->second.Length;
      double Lengthgeo = Length - avgDL;

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

      double EnCutHigh = m_configReader.GetEnergyCutHigh();
      const int nsub = m_fixmConfig->Global.Intergralnsub;

      for (int ibin_out = 1; ibin_out <= hratexs->GetNbinsX(); ibin_out++) {
        double En_lo = hratexs->GetBinLowEdge(ibin_out);
        double En_hi = En_lo + hratexs->GetBinWidth(ibin_out);

        if (En_lo > EnCutHigh)
          continue;

        // En -> TOF (note: low energy corresponds to high TOF)
        double tof_hi = calTOF(En_lo, Length);
        double tof_lo = calTOF(En_hi, Length);

        double sum_weight = 0.0;
        double dtof = (tof_hi - tof_lo) / nsub;

        for (int isub = 0; isub < nsub; isub++) {
          double tof = tof_lo + dtof * (isub + 0.5);

          int bin_in = hrate->FindBin(tof);
          double density =
              hrate->GetBinContent(bin_in) / hrate->GetBinWidth(bin_in);
          if (density <= 0)
            continue;

          double En = -1e6;
          double length = Length;
          for (size_t j = 0; j < 3; j++) {
            En = calEn(tof, length);
            length = Lengthgeo + endeltaL->Eval(En);
          }

          auto xs_ntot = gENDFTOT->Eval(En);
          auto xs_nf = gENDFNF->Eval(En);
          auto yield = 1.0 - exp(-nd * xs_ntot);
          if (En <= EnCutHigh && xs_nf > 0 && yield > 0) {
            sum_weight += density * dtof / yield / xs_nf * xs_ntot;
          }
        }

        hratexs->SetBinContent(ibin_out, sum_weight);
      }
    };

    // Maps to store histograms
    std::map<int, TH1D *> m_hrate;
    std::map<int, TH1D *> m_hratexs;
    std::map<int, TH1D *> m_hratexsNs;
    std::map<int, TH1D *> m_herror;

    // Process each channel
    for (int chID : m_channelIDs) {
      PrintChannelInfo(chID);

      // Open UF result file
      std::string ufFilePath = outcomePath + Form("/UF_%d.root", chID);
      TFile *fin = TFile::Open(ufFilePath.c_str());

      if (!fin || fin->IsZombie()) {
        std::cerr << "  Warning: Cannot open file " << ufFilePath << std::endl;
        continue;
      }

      // Get h_finalE histogram
      TH1D *hEn = nullptr;
      TH1D *hT = nullptr;
      TH1D *hError = nullptr;
      fin->GetObject("h_finalE", hEn);
      fin->GetObject("hUFt", hT);
      fin->GetObject("h_UFerroronly", hError);
      if (!hEn) {
        std::cerr << "  Warning: h_finalE not found in " << ufFilePath
                  << std::endl;
        fin->Close();
        continue;
      }
      if (!hT) {
        std::cerr << "  Warning: hUFt not found in " << ufFilePath << std::endl;
        fin->Close();
        continue;
      }
      if (!hError) {
        std::cerr << "  Warning: h_UFerroronly not found in " << ufFilePath
                  << std::endl;
        fin->Close();
        continue;
      }

      auto hErrorClone =
          static_cast<TH1D *>(hError->Clone(Form("h_UFerroronly_%d", chID)));
      hErrorClone->SetDirectory(0);
      m_herror[chID] = hErrorClone;

      // Clone histogram to keep it in memory after file closes
      auto hEnre = static_cast<TH1D *>(hEn->Clone(Form("h1_En_%d", chID)));
      hEnre->SetDirectory(0);

      // Create output histograms
      std::cout << "  Calculating rate (Monte Carlo En)..." << std::endl;

      auto hEnxs = static_cast<TH1D *>(hEn->Clone(Form("h1_Enxs_%d", chID)));
      hEnxs->SetDirectory(0);
      auto hEnxsNs =
          static_cast<TH1D *>(hEn->Clone(Form("h1_EnxsNs_%d", chID)));
      hEnxsNs->SetDirectory(0);

      // Calculate rate histograms
      std::cout << "  Calculating rate (with self-shielding)..." << std::endl;
      calhrate(hT, hEnre, chID, ntimes);
      m_hrate[chID] = hEnre;

      // Calculate rate×xs histograms
      auto SampleType = m_sampletype[chID];
      if (m_xs_nr.find(SampleType) != m_xs_nr.end()) {
        std::cout << "  Calculating rate×xs (without self-shielding)..."
                  << std::endl;
        calhratexs(hT, hEnxs, chID, ntimes);
        m_hratexs[chID] = hEnxs;
        // Uncomment to calculate with self-shielding correction
        // std::cout << "  Calculating rate×xsNs (with self-shielding)..."
        //           << std::endl;
        // calhratexsNs(hT, hEnxsNs, chID, ntimes);
        // m_hratexsNs[chID] = hEnxsNs;
      }

      fin->Close();
    }

    // Save results to output file
    std::string outpath = outcomePath + "/hratexsuf.root";
    auto fout = TFile::Open(outpath.c_str(), "RECREATE");

    for (int chID : m_channelIDs) {
      //
      if (m_hrate.find(chID) != m_hrate.end()) {
        m_hrate[chID]->Write();
      }

      if (m_herror.find(chID) != m_herror.end()) {
        m_herror[chID]->Write();
      }

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

    // variables m_xs_nr and m_xs_ntot will be cleaned up in destructor or when
    // LoadSTDENDFData is called again
  };

  std::cout << "\n=== Running GetHRateXSUF for FIXM ===" << std::endl;
  core_logic();

  if (m_configReader.GetDataType() == "Flux" && HasLiSiConfig()) {
    SwitchToLiSi();
    core_logic();
    RestoreFromLiSi();
  }
}

void RDataFrameAnalysis::EvalDeltaTc1() {
  PrintSectionHeader("EvalDeltaTc1 Analysis");

  InitializeCommonConfig();

  if (!m_chain) {
    std::cerr << "Error: TChain is not initialized" << std::endl;
    return;
  }

  std::cout << "Entries in chain: " << m_chain->GetEntries() << std::endl;

  std::cout << "Number of channels to analyze: " << m_channelIDs.size()
            << std::endl;

  // Create customized TH1D for each channel
  std::map<int, TH1D *> hDeltaTc0Map;
  std::map<int, TH1D *> hDeltaTc1Map;
  std::map<int, TH1D *> hDeltaTc2Map;

  std::map<int, TH2D *> h2Tc0Map;
  std::map<int, TH2D *> h2Tc1Map;
  std::map<int, TH2D *> h2Tc2Map;

  // Create logarithmic bins for Tc (X-axis)
  const int nBinsTc = 1000;
  double minTc = 1.0; // > 0 for log axis
  double maxTc = 4e7;
  std::vector<double> logBinsTc(nBinsTc + 1);
  double logStep = (std::log10(maxTc) - std::log10(minTc)) / nBinsTc;
  for (int i = 0; i <= nBinsTc; ++i) {
    logBinsTc[i] = std::pow(10, std::log10(minTc) + i * logStep);
  }

  for (int chID : m_channelIDs) {
    TH1D *h0 =
        new TH1D(Form("hDeltaTc0_CH%d", chID),
                 Form("fTc0 difference between adjacent hits in same "
                      "fEventNumber - Channel %d; #Delta fTc0 (ns); Entries",
                      chID),
                 2000, 0, 2000);
    h0->SetDirectory(nullptr); // Ensure it persists
    hDeltaTc0Map[chID] = h0;

    TH1D *h1 =
        new TH1D(Form("hDeltaTc1_CH%d", chID),
                 Form("fTc1 difference between adjacent hits in same "
                      "fEventNumber - Channel %d; #Delta fTc1 (ns); Entries",
                      chID),
                 2000, 0, 2000);
    h1->SetDirectory(nullptr);
    hDeltaTc1Map[chID] = h1;

    TH1D *h2 =
        new TH1D(Form("hDeltaTc2_CH%d", chID),
                 Form("fTc2 difference between adjacent hits in same "
                      "fEventNumber - Channel %d; #Delta fTc2 (ns); Entries",
                      chID),
                 2000, 0, 2000);
    h2->SetDirectory(nullptr);
    hDeltaTc2Map[chID] = h2;

    TH2D *h2_0 = new TH2D(
        Form("h2Tc0_CH%d", chID),
        Form("fTc0 vs #Delta fTc0 - Channel %d; fTc0 (ns); #Delta fTc0 (ns)",
             chID),
        nBinsTc, logBinsTc.data(), 500, 0.0, 4000.0);
    h2_0->SetDirectory(nullptr);
    h2Tc0Map[chID] = h2_0;

    TH2D *h2_1 = new TH2D(
        Form("h2Tc1_CH%d", chID),
        Form("fTc1 vs #Delta fTc1 - Channel %d; fTc1 (ns); #Delta fTc1 (ns)",
             chID),
        nBinsTc, logBinsTc.data(), 500, 0.0, 4000.0);
    h2_1->SetDirectory(nullptr);
    h2Tc1Map[chID] = h2_1;

    TH2D *h2_2 = new TH2D(
        Form("h2Tc2_CH%d", chID),
        Form("fTc2 vs #Delta fTc2 - Channel %d; fTc2 (ns); #Delta fTc2 (ns)",
             chID),
        nBinsTc, logBinsTc.data(), 500, 0.0, 4000.0);
    h2_2->SetDirectory(nullptr);
    h2Tc2Map[chID] = h2_2;
  }

  // Use TTreeReader to iterate over the flattened TTree
  TTreeReader reader(m_chain);
  TTreeReaderValue<Int_t> fEventNumber(reader, "fEventNumber");
  TTreeReaderValue<Double_t> fTc0(reader, "fTc0");
  TTreeReaderValue<Double_t> fTc1(reader, "fTc1");
  TTreeReaderValue<Double_t> fTc2(reader, "fTc2");
  TTreeReaderValue<Double_t> fhpn(reader, "fhpn");
  TTreeReaderValue<Int_t> fChannelID(reader, "fChannelID");

  // Get thresholds for each channel
  std::map<int, double> chThresholds;
  for (const auto &pair : m_fixmConfig->Channels) {
    chThresholds[pair.first] = pair.second.Threshold;
  }

  std::map<int, Int_t> prevEventNumber;
  std::map<int, Double_t> prevTc0;
  std::map<int, Double_t> prevTc1;
  std::map<int, Double_t> prevTc2;
  std::map<int, bool> hasPrev;

  for (int chID : m_channelIDs) {
    prevEventNumber[chID] = -1;
    prevTc0[chID] = 0.0;
    prevTc1[chID] = 0.0;
    prevTc2[chID] = 0.0;
    hasPrev[chID] = false;
  }

  Long64_t nEntries = m_chain->GetEntries();
  Long64_t nout = nEntries * 0.1;
  if (nout == 0)
    nout = 1;

  std::cout << "Processing entries..." << std::endl;
  Long64_t count = 0;

  while (reader.Next()) {
    if (count % nout == 0) {
      std::cout << "  Progress: " << count << " / " << nEntries << " ("
                << (100.0 * count / nEntries) << "%)" << std::endl;
    }
    count++;

    // Check threshold constraint
    auto it = chThresholds.find(*fChannelID);
    double threshold = (it != chThresholds.end()) ? it->second : 0.0;
    if (*fhpn <= threshold) {
      continue;
    }

    int chID = *fChannelID;
    if (m_channelIDs.end() ==
        std::find(m_channelIDs.begin(), m_channelIDs.end(), chID)) {
      continue;
    }

    if (hasPrev[chID] && (*fEventNumber == prevEventNumber[chID])) {
      double deltaTc0 = std::abs(*fTc0 - prevTc0[chID]);
      double deltaTc1 = std::abs(*fTc1 - prevTc1[chID]);
      double deltaTc2 = std::abs(*fTc2 - prevTc2[chID]);
      hDeltaTc0Map[chID]->Fill(deltaTc0);
      hDeltaTc1Map[chID]->Fill(deltaTc1);
      hDeltaTc2Map[chID]->Fill(deltaTc2);

      h2Tc0Map[chID]->Fill(*fTc0, deltaTc0);
      h2Tc1Map[chID]->Fill(*fTc1, deltaTc1);
      h2Tc2Map[chID]->Fill(*fTc2, deltaTc2);
    }

    prevEventNumber[chID] = *fEventNumber;
    prevTc0[chID] = *fTc0;
    prevTc1[chID] = *fTc1;
    prevTc2[chID] = *fTc2;
    hasPrev[chID] = true;
  }

  std::cout << "  Progress: " << nEntries << " / " << nEntries << " (100.0%)"
            << std::endl;

  // Draw the histograms on a single main canvas with 3 subpads
  TCanvas *cAll = new TCanvas(
      "cDeltaTc_all", "Delta Tc (Tc0, Tc1, Tc2) - All Channels", 1500, 500);
  cAll->Divide(3, 1);

  auto drawHists = [&](int padIndex, std::map<int, TH1D *> &hMap,
                       const char *title) {
    cAll->cd(padIndex);
    gPad->SetLogy();
    TLegend *legd = new TLegend();
    int ihist = 0;
    for (int chID : m_channelIDs) {
      if (hMap.find(chID) == hMap.end())
        continue;

      hMap[chID]->SetTitle(title);
      hMap[chID]->SetLineColor(color[ihist % 10]);
      TH1 *hdrawn = hMap[chID]->DrawCopy(ihist == 0 ? "hist" : "hist same");
      if (hdrawn) {
        legd->AddEntry(hdrawn, Form("Channel %d", chID), "l");
      }
      ihist++;
    }
    legd->Draw();
  };

  std::cout << "  Drawing Tc0 histograms..." << std::endl;
  drawHists(1, hDeltaTc0Map, "Delta Tc0 - All Channels");
  std::cout << "  Drawing Tc1 histograms..." << std::endl;
  drawHists(2, hDeltaTc1Map, "Delta Tc1 - All Channels");
  std::cout << "  Drawing Tc2 histograms..." << std::endl;
  drawHists(3, hDeltaTc2Map, "Delta Tc2 - All Channels");

  cAll->Update();

  // Draw 2D histograms
  TCanvas *c2DAll =
      new TCanvas("c2DTc_all", "Tc vs Delta Tc - All Channels", 1500, 500);
  c2DAll->Divide(3, 1);

  auto draw2DHists = [&](int padIndex, std::map<int, TH2D *> &hMap,
                         const char *title) {
    c2DAll->cd(padIndex);
    gPad->SetLogx();
    gPad->SetLogz();
    // Only plotting the first available channel due to TH2D drawing complexity
    // when overlaying
    for (int chID : m_channelIDs) {
      if (hMap.find(chID) != hMap.end()) {
        hMap[chID]->SetTitle(title);
        hMap[chID]->DrawCopy("COLZ");
        break; // draw only first valid channel to show the basic 2D spectrum
      }
    }
  };

  std::cout << "  Drawing 2D Tc0 histograms..." << std::endl;
  draw2DHists(1, h2Tc0Map, "Tc0 vs Delta Tc0 (1st CH)");
  std::cout << "  Drawing 2D Tc1 histograms..." << std::endl;
  draw2DHists(2, h2Tc1Map, "Tc1 vs Delta Tc1 (1st CH)");
  std::cout << "  Drawing 2D Tc2 histograms..." << std::endl;
  draw2DHists(3, h2Tc2Map, "Tc2 vs Delta Tc2 (1st CH)");

  c2DAll->Update();

  for (auto &pair : hDeltaTc0Map)
    delete pair.second;
  for (auto &pair : hDeltaTc1Map)
    delete pair.second;
  for (auto &pair : hDeltaTc2Map)
    delete pair.second;
  for (auto &pair : h2Tc0Map)
    delete pair.second;
  for (auto &pair : h2Tc1Map)
    delete pair.second;
  for (auto &pair : h2Tc2Map)
    delete pair.second;

  PrintClosePrompt();
}

void RDataFrameAnalysis::CalSimTrans() {
  PrintSectionHeader("CalSimTrans - Simulation Flux Attenuation");

  // Initialize configuration (energy bins from FIXM config)
  InitializeCommonConfig();
  InitializeEnergyBins();

  // Get simulation path and folders
  const std::string &simPath = m_configReader.GetSimFixmNAttentionPath();
  if (simPath.empty()) {
    std::cerr << "Error: SimFixmNAttentionPath is not configured in "
                 "filepath.json"
              << std::endl;
    return;
  }

  const auto &simFolders = m_fixmConfig->Global.SimFixmNAttentionFolders;
  if (simFolders.empty()) {
    std::cerr << "Error: SimFixmNAttentionFolders is empty in FIXM config"
              << std::endl;
    return;
  }

  // Build file list by scanning OutPut/<folder>/*.root
  TChain simChain("tree");
  int fileCount = 0;
  for (const auto &folder : simFolders) {
    std::string dirPath = simPath + "OutPut/" + folder + "/";
    std::cout << "Scanning directory: " << dirPath << std::endl;

    try {
      for (const auto &entry : std::filesystem::directory_iterator(dirPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".root") {
          std::string filePath = entry.path().string();
          simChain.Add(filePath.c_str());
          fileCount++;
          std::cout << "  Added: " << entry.path().filename().string()
                    << std::endl;
        }
      }
    } catch (const std::filesystem::filesystem_error &e) {
      std::cerr << "Error scanning directory " << dirPath << ": " << e.what()
                << std::endl;
      return;
    }
  }

  std::cout << "Total simulation files added: " << fileCount << std::endl;
  if (fileCount == 0) {
    std::cerr << "Error: No ROOT files found in simulation directories"
              << std::endl;
    return;
  }

  std::cout << "Total entries in chain: " << simChain.GetEntries() << std::endl;

  // Enable multi-threading and create RDataFrame
  EnableMultiThreading();
  ROOT::RDataFrame df(simChain);

  // Build CHIDUSE -> DetID mapping
  // For each channel in CHIDUSE, get its DetID from the channel config
  std::vector<std::pair<int, int>> chDetPairs; // (chID, DetID)
  for (int chID : m_channelIDs) {
    auto it = m_fixmConfig->Channels.find(chID);
    if (it == m_fixmConfig->Channels.end()) {
      std::cerr << "Warning: Channel " << chID << " not found in config"
                << std::endl;
      continue;
    }
    int detID = it->second.DetID;
    chDetPairs.emplace_back(chID, detID);
    std::cout << "Channel " << chID << " -> DetID " << detID << std::endl;
  }

  // Create histograms for each channel (using DetID for filtering)
  std::map<int, ROOT::RDF::RResultPtr<TH1D>> hEnMap;
  for (const auto &[chID, detID] : chDetPairs) {
    std::string hname = Form("hEn%d", chID);
    std::string valname = Form("En%d", chID);
    std::string cmd = Form("vec_sampleNEnergy[vec_ChannelID==%d]", detID);

    auto hEn = df.Define(valname, cmd)
                   .Histo1D({hname.c_str(), ";Neutron Energy (eV);count",
                             m_nbins, m_EnBins.data()},
                            valname);
    hEnMap.emplace(chID, hEn);
  }

  // Create incident energy histogram
  auto hEnin = df.Histo1D(
      {"hEnin", ";Neutron Energy (eV);count", m_nbins, m_EnBins.data()},
      "EnIn");

  // Calculate flux attenuation: htrans = hEn / hEnin
  std::vector<TH1D *> vTrans;
  for (const auto &[chID, detID] : chDetPairs) {
    std::string transName = Form("htrans%d", chID);
    auto htrans = (TH1D *)hEnMap[chID]->Clone(transName.c_str());
    htrans->Divide(hEnin.GetPtr());
    htrans->SetYTitle("Attenuation");
    vTrans.push_back(htrans);
    std::cout << "Calculated attenuation for channel " << chID
              << " (DetID=" << detID << ")" << std::endl;
  }

  // Output to para directory
  std::string outPath = m_outputPath + m_expName + "/para/";
  std::string outFile = outPath + "fluxattenuation.root";

  // Create output directory if it doesn't exist
  std::filesystem::create_directories(outPath);

  auto fout = new TFile(outFile.c_str(), "RECREATE");
  for (auto *h : vTrans) {
    h->Write();
  }
  fout->Close();
  delete fout;

  std::cout << "Output written to: " << outFile << std::endl;

  // Draw all attenuation histograms on a single canvas
  TCanvas *c = new TCanvas("cSimTrans", "Flux Attenuation", 1200, 600);
  gPad->SetLogx();

  auto *legd = new TLegend();
  int colorIdx[] = {kRed,    kBlue,   kGreen + 2, kMagenta, kCyan + 1,
                    kOrange, kViolet, kTeal,      kPink,    kAzure};

  bool first = true;
  int ihist = 0;
  for (size_t i = 0; i < vTrans.size(); i++) {
    int chID = chDetPairs[i].first;
    int detID = chDetPairs[i].second;
    TH1D *h = vTrans[i];
    h->SetLineColor(colorIdx[ihist % 10]);
    h->SetLineWidth(2);
    h->DrawClone(first ? "hist" : "hist same");
    legd->AddEntry(h, Form("CH%d (DetID=%d)", chID, detID), "l");
    first = false;
    ihist++;
  }
  legd->Draw();
  c->Update();

  // Cleanup
  for (auto *h : vTrans) {
    delete h;
  }

  PrintClosePrompt();
}
