/*****************************************************
 * File: CrossSectionAnalysis.cxx
 * Description: Implementation of cross section analysis class
 * Author: Kilo Code
 * Created: 2026/02/08
 ****************************************************/

#include "CrossSectionAnalysis.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TGraph.h"
#include "TH1D.h"
#include "TLegend.h"
#include "TMath.h"
#include "utils.h"
#include <algorithm>
#include <iostream>
#include <memory>

// Constructor
CrossSectionAnalysis::CrossSectionAnalysis(ConfigReader &configReader)
    : RDataFrameAnalysis(configReader) {}

// Destructor
CrossSectionAnalysis::~CrossSectionAnalysis() {}

// RunAnalysis implementation
bool CrossSectionAnalysis::RunAnalysis(const std::string &analysisType) {
  if (!m_isInitialized && !InitializeTChain()) {
    std::cerr << "Error: Failed to initialize TChain" << std::endl;
    return false;
  }

  if (analysisType == "GetXSSingleBunch") {
    GetXSSingleBunch();
    return true;
  } else {
    // Call base class implementation for all other analysis types
    return RDataFrameAnalysis::RunAnalysis(analysisType);
  }
}

// GetXSSingleBunch implementation (moved from RDataFrameAnalysis)
void CrossSectionAnalysis::GetXSSingleBunch() {
  PrintSectionHeader("GetXSSingleBunch Analysis");

  EnableMultiThreading();
  InitializeCommonConfig();
  InitializeEnergyBins();

  const std::string &dataDir = m_configReader.GetDataPath();

  std::cout << "Bin configuration: bpd=" << m_bpd << ", nDec=" << m_nDec
            << ", nbins=" << m_nbins << std::endl;

  // Get energy divide from configuration
  const std::vector<double> &energyDivide = m_fixmConfig->Global.EnergyDivide;
  std::cout << "Number of channels: " << m_channelIDs.size() << std::endl;

  // Calculate effective radius and area
  double r_sample =
      m_fixmConfig->Channels.begin()->second.Radius; // r sample at 0
  double S = r_sample * r_sample * mm_to_cm * mm_to_cm * TMath::Pi();
  std::cout << "Effective area: " << S << " cm^2" << std::endl;

  // Build maps for channel data
  std::map<int, double> m_nd;
  std::map<int, double> m_eff;
  std::map<int, int> m_detID;
  std::map<int, std::string> m_samplename;
  std::map<int, TH1D *> m_hrate;

  // Load hratepileup.root file
  std::string outcomePath = m_outputPath + m_expName + "/Outcome";
  std::string hratePath = outcomePath + "/hratepileup.root";
  std::cout << "Opening hrate file: " << hratePath << std::endl;

  std::unique_ptr<TFile> fin_hrate(TFile::Open(hratePath.c_str()));
  if (!fin_hrate || fin_hrate->IsZombie()) {
    std::cerr << "Error: Cannot open " << hratePath << std::endl;
    return;
  }

  // Load flux attenuation file
  std::string fluxattenPath =
      m_outputPath + m_expName + "/para/fluxattenuation.root";
  std::cout << "Opening flux attenuation file: " << fluxattenPath << std::endl;

  std::unique_ptr<TFile> fin_hatten(TFile::Open(fluxattenPath.c_str()));
  if (!fin_hatten || fin_hatten->IsZombie()) {
    std::cerr << "Error: Cannot open " << fluxattenPath << std::endl;
    return;
  }

  // Process channel data
  for (int chID : m_channelIDs) {
    auto it = m_fixmConfig->Channels.find(chID);
    if (it == m_fixmConfig->Channels.end()) {
      std::cerr << "Warning: Channel " << chID << " not found in config"
                << std::endl;
      continue;
    }
    const auto &chConfig = it->second;

    // Calculate areal density
    double sample_r = chConfig.Radius * mm_to_cm; // to cm
    double sample_area = sample_r * sample_r * TMath::Pi();
    double sample_t = chConfig.Mass / sample_area; // mg/cm2
    double ArealDensity =
        sample_t / chConfig.A * Na * barn_to_cm2 * mg_to_g; // atoms/barn
    double nd = ArealDensity;

    m_nd[chID] = nd;
    m_eff[chID] = chConfig.DetEff;
    m_detID[chID] = chConfig.DetID;
    std::string samplename =
        chConfig.SampleType + std::to_string(chConfig.SampleNumber);
    m_samplename[chID] = samplename;

    std::cout << Form(
                     "ChID = %d, Mass = %f %s, Radius = %f, Area = %f, nd = %e",
                     chID, chConfig.Mass, chConfig.MassUnit.c_str(), sample_r,
                     sample_area, nd)
              << std::endl;

    // Get histograms from hratepileup.root
    TH1D *horiginin = nullptr;
    fin_hrate->GetObject(Form("h1_En_%d", chID), horiginin);
    TH1D *hatten = nullptr;
    fin_hatten->GetObject(Form("htrans%d", chID), hatten);

    if (horiginin != nullptr) {
      // Clone to avoid modifying the file's histogram
      TH1D *h = (TH1D *)horiginin->Clone(Form("h_corr_%d", chID));
      h->SetDirectory(0);

      // Scale by efficiency and divide by attenuation
      h->Scale(1.0 / m_eff[chID]);
      if (hatten != nullptr) {
        h->Divide(hatten);
      }
      m_hrate[chID] = h;
    } else {
      std::cerr << Form("Warning: No histogram for channel %d in hratepileup.root",
                        chID)
                << std::endl;
    }
  }

  // Load ENDF data
  std::string name_ENDFU5 = m_configReader.GetENDFDataU5NF();
  auto gENDFU5 = new TGraph();
  get_graph(name_ENDFU5.c_str(), gENDFU5, MeV_to_eV);
  auto hENDFU5 = (TH1D *)m_hrate.begin()->second->Clone("hENDFU5");
  hENDFU5->Reset();
  graph2hist(gENDFU5, hENDFU5);

  std::cout << "Loaded ENDF data files" << std::endl;

  // Lambda function to get rate sum
  auto gethratesum = [this, &m_hrate, &m_samplename](TH1D *hratesum,
                                                     const char *sampletype) {
    for (const auto &c : m_hrate) {
      auto chid = c.first;
      std::string sampelname = m_samplename[chid];
      if (sampelname.find(sampletype) != std::string::npos) {
        hratesum->Add(m_hrate[chid]);
      }
    }
  };

  // Create histograms for statistics
  auto hsta_rerror_U5 =
      (TH1D *)m_hrate.begin()->second->Clone("hsta_rerror_U5");
  hsta_rerror_U5->Reset();
  auto hratesum_U5 = (TH1D *)m_hrate.begin()->second->Clone("hratesum_U5");
  hratesum_U5->Reset();

  std::map<std::string, TH1D *> m_hsta_rerror;
  m_hsta_rerror["U5"] = hsta_rerror_U5;
  std::map<std::string, TH1D *> m_hratesum;
  m_hratesum["U5"] = hratesum_U5;

  auto hsta_rerror_Th =
      (TH1D *)m_hrate.begin()->second->Clone("hsta_rerror_Th");
  hsta_rerror_Th->Reset();
  auto hratesum_Th = (TH1D *)m_hrate.begin()->second->Clone("hratesum_Th");
  hratesum_Th->Reset();
  m_hsta_rerror["Th"] = hsta_rerror_Th;
  m_hratesum["Th"] = hratesum_Th;

  // Get sum and statistical error
  for (const auto &c : m_hratesum) {
    auto sampletype = c.first;
    auto hratesum = c.second;
    double ndtot = 0.0;

    std::cout << "\nProcessing sample type: " << sampletype << std::endl;

    for (const auto &c1 : m_hrate) {
      auto chid = c1.first;
      std::string samplename = m_samplename[chid];

      // Check if sample name contains the sample type (e.g., "Th1" contains
      // "Th")
      if (samplename.find(sampletype) != std::string::npos) {
        std::cout << "  Matching channel " << chid << " (sample: " << samplename
                  << "), nd = " << m_nd[chid] << std::endl;
        ndtot = ndtot + m_nd[chid];
        hratesum->Add(m_hrate[chid]);
      }
    }
    for (int i = 0; i < hratesum->GetNbinsX(); i++) {
      if (i + 1 > hratesum->FindBin(m_configReader.GetEnergyCutHigh())) {
        hratesum->SetBinContent(i + 1, 0);
      }
    }
    get_sta_errorhist(hratesum, m_hsta_rerror[sampletype]);

    // Debug: Check scaling factors
    std::cout << "Sample type: " << sampletype << ", ndtot = " << ndtot
              << ", S = " << S << ", scale factor = " << (1. / ndtot / S)
              << std::endl;
    std::cout << "  hratesum before scale - integral: " << hratesum->Integral()
              << std::endl;

    // Only scale if ndtot and S are valid
    if (ndtot > 0 && S > 0) {
      hratesum->Scale(1. / ndtot / S);
      std::cout << "  hratesum after scale - integral: " << hratesum->Integral()
                << std::endl;
    } else {
      std::cerr << "  ERROR: Cannot scale - invalid ndtot or S!" << std::endl;
    }
  }

  // Debug: Check input histograms before calculation
  std::cout << "\n=== Debug: Checking histograms before calculation ==="
            << std::endl;
  std::cout << "m_hratesum[\"Th\"] - entries: "
            << m_hratesum["Th"]->GetEntries()
            << ", integral: " << m_hratesum["Th"]->Integral()
            << ", mean: " << m_hratesum["Th"]->GetMean() << std::endl;
  std::cout << "m_hratesum[\"U5\"] - entries: "
            << m_hratesum["U5"]->GetEntries()
            << ", integral: " << m_hratesum["U5"]->Integral()
            << ", mean: " << m_hratesum["U5"]->GetMean() << std::endl;
  std::cout << "hENDFU5 - entries: " << hENDFU5->GetEntries()
            << ", integral: " << hENDFU5->Integral()
            << ", mean: " << hENDFU5->GetMean() << std::endl;

  // Divide by ENDF cross section and normalize
  auto hxserror = (TH1D *)m_hsta_rerror["Th"]->Clone("hxserror");
  add_error(hxserror, m_hsta_rerror["U5"]);
  auto hxs = (TH1D *)m_hratesum["Th"]->Clone("hxs");
  hxs->Divide(m_hratesum["U5"]);
  hxs->Multiply(hENDFU5);
  set_binerror(hxs, hxserror);

  // Debug: Check result after calculation
  std::cout << "\nhxs after calculation - entries: " << hxs->GetEntries()
            << ", integral: " << hxs->Integral() << ", mean: " << hxs->GetMean()
            << std::endl;

  // Save results to file FIRST
  std::string outputPath = outcomePath + "/XSSimple.root";
  std::unique_ptr<TFile> fout(TFile::Open(outputPath.c_str(), "recreate"));
  if (!fout || fout->IsZombie()) {
    std::cerr << "Error: Cannot create output file: " << outputPath
              << std::endl;
    return;
  }
  hxs->Write();
  fout->Close();
  std::cout << "Output file saved: " << outputPath << std::endl;

  // NOW create canvas and draw (after file is saved)
  TCanvas *c = new TCanvas("cxs", "Cross Section Comparison", 1200, 800);
  c->SetLogx();
  c->SetLogy();

  // Set histogram properties
  hxs->SetYTitle("Cross Section (barns)");
  hxs->SetXTitle("Energy (eV)");
  hxs->SetLineColor(kRed);
  hxs->SetLineWidth(2);
  hxs->SetStats(0); // Don't show statistics box

  // Determine Y-axis range based on both histograms
  double ymin = 1e-10; // Small default for log scale
  double ymax = 10.0;  // Default maximum

  std::cout << "\nSetting Y-axis range: [" << ymin << ", " << ymax << "]"
            << std::endl;

  hxs->Draw("hist");

  // Add legend
  auto leg = new TLegend(0.7, 0.7, 0.9, 0.9);
  leg->AddEntry(hxs, "This Work", "l");
  leg->Draw();

  c->Update();

  std::cout << std::endl;
  std::cout << "Close plot windows to continue..." << std::endl;
  std::cout << "Press Ctrl+C in terminal to exit..." << std::endl;
  std::cout << std::endl;
}

bool CrossSectionAnalysis::LoadXSENDFData(int nrebin) {
  if (m_nbins <= 0 || m_EnBins.empty()) {
    std::cerr << "Warning: Energy bins not initialized. Cannot create TH1D for "
                 "cross sections. Please call InitializeEnergyBins() first."
              << std::endl;
    return false;
  }

  const int step = (nrebin > 1) ? nrebin : 1;
  const int nbins_reb = m_nbins / step;
  std::vector<double> rebinnedEdges;
  rebinnedEdges.reserve(nbins_reb + 1);
  for (int i = 0; i <= nbins_reb; i++) {
    rebinnedEdges.push_back(m_EnBins[i * step]);
  }

  std::vector<std::string> loadedSampleTypes;

  for (const auto &[chID, chConfig] : m_fixmConfig->Channels) {
    std::string sampleType = chConfig.SampleType;

    // Skip if already loaded
    if (std::find(loadedSampleTypes.begin(), loadedSampleTypes.end(),
                  sampleType) != loadedSampleTypes.end()) {
      continue;
    }

    // Skip if already loaded in standard data (e.g. 235U, 6Li)
    if (m_xs_nr.find(sampleType) != m_xs_nr.end()) {
      loadedSampleTypes.push_back(sampleType);
      std::cout << "Data for sample " << sampleType << " already loaded from standard ENDF files." << std::endl;
      continue;
    }

    std::string filepath = m_outputPath + m_expName + "/para/XSData/" +
                           sampleType + "/ENDFB-VIII.1.txt";
    auto gENDF = new TGraph();
    get_graph(filepath.c_str(), gENDF, MeV_to_eV);

    if (gENDF->GetN() > 0) {
      m_xs_nr[sampleType] = gENDF;

      std::string hname = "hxs_nr_" + sampleType;
      TH1D *h =
          new TH1D(hname.c_str(), sampleType.c_str(), nbins_reb, rebinnedEdges.data());
      h->SetDirectory(nullptr); // Ensure histogram persists beyond current
                                // directory closing
      for (int i = 1; i <= nbins_reb; i++) {
        h->SetBinContent(i, gENDF->Eval(h->GetBinCenter(i)));
      }
      m_hxs_nr[sampleType] = h;
      loadedSampleTypes.push_back(sampleType);

      std::cout << "Loaded ENDF data for sample: " << sampleType << " from "
                << filepath << " (nrebin=" << step << ", nbins=" << nbins_reb << ")." << std::endl;
    } else {
      delete gENDF;
      std::cerr << "Warning: Failed to load or empty ENDF data from "
                << filepath << std::endl;
    }
  }

  std::cout << "Converted sample reaction cross section TGraphs to TH1D."
            << std::endl;

  return true;
}

void CrossSectionAnalysis::CalUncertainty() {
  PrintSectionHeader("CalUncertainty Analysis (Cross Section)");

  std::cout << "TODO: Implement cross section uncertainty calculation"
            << std::endl;
  std::cout << "This method should calculate uncertainties for:" << std::endl;
  std::cout << "  - Statistical uncertainty" << std::endl;
  std::cout << "  - Systematic uncertainty from flux normalization"
            << std::endl;
  std::cout << "  - Systematic uncertainty from detector efficiency"
            << std::endl;
  std::cout << "  - Systematic uncertainty from background subtraction"
            << std::endl;
  std::cout << "  - Total uncertainty" << std::endl;

  PrintClosePrompt();
}
