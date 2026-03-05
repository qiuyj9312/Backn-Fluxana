/*****************************************************
 * File: NeutronFluxAnalysis.cxx
 * Description: Implementation of neutron flux analysis class
 * Author: Kilo Code
 * Created: 2026/02/08
 ****************************************************/

#include "NeutronFluxAnalysis.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TGraph.h"
#include "TH1D.h"
#include "THStack.h"
#include "TLegend.h"
#include "TString.h"
#include "utils.h"
#include <fstream>
#include <iostream>
#include <map>
#include <set>

// Constructor
NeutronFluxAnalysis::NeutronFluxAnalysis(ConfigReader &configReader)
    : RDataFrameAnalysis(configReader) {}

// Destructor
NeutronFluxAnalysis::~NeutronFluxAnalysis() {}

// RunAnalysis implementation - adds flux-specific analysis types
bool NeutronFluxAnalysis::RunAnalysis(const std::string &analysisType) {
  // Handle flux-specific analysis types
  if (analysisType == "CalFlux") {
    CalFlux();
    return true;
  } else if (analysisType == "CalUncertainty") {
    CalUncertainty();
    return true;
  }

  // Delegate to base class for other analysis types
  return RDataFrameAnalysis::RunAnalysis(analysisType);
}

void NeutronFluxAnalysis::CalUncertainty() {
  PrintSectionHeader("CalUncertainty Analysis");
  InitializeCommonConfig();

  // 获取配置
  const auto &fixmConfig = m_configReader.GetFIXMConfig();
  const auto &channelIDs = fixmConfig.Global.CHIDUSE;
  std::string outcomePath = m_xsPath + m_expName + "/Outcome/FIXM";

  // 初始化数据结构
  UncertaintyData data;
  for (const auto &[chID, chConfig] : fixmConfig.Channels) {
    TString sampletype = chConfig.SampleType.c_str();
    data.sampletype[chID] = sampletype;
    data.samplename[chID] = sampletype + Form("%d", chConfig.SampleNumber);
    data.sampletype_set.insert(sampletype);
  }

  // 执行不确定度计算流程
  LoadRateHistograms(data, outcomePath, channelIDs);
  CalculateStatisticalUncertainty(data);
  CalculateUnfoldingUncertainty(data, outcomePath, channelIDs);
  LoadDetectorUncertainty(data, outcomePath);
  LoadENDFUncertainty(data);
  CalculateTotalUncertainty(data, channelIDs);
  WriteResults(data, outcomePath);
  DrawUncertaintyPlots(data);

  PrintClosePrompt();
  std::cout << "\nCalUncertainty analysis completed!" << std::endl;
}

// ============================================================================
// 辅助函数实现
// ============================================================================

void NeutronFluxAnalysis::LoadRateHistograms(
    UncertaintyData &data, const std::string &outcomePath,
    const std::vector<int> &channelIDs) {
  std::cout << "Loading rate histograms from hrate.root..." << std::endl;

  auto fin_hrate = TFile::Open(Form("%s/hrate.root", outcomePath.c_str()));
  if (!fin_hrate || fin_hrate->IsZombie()) {
    std::cerr << "Error: Cannot open hrate.root" << std::endl;
    return;
  }

  for (int chid : channelIDs) {
    TH1D *hrate{nullptr}, *hrate_uf{nullptr};
    fin_hrate->GetObject(Form("h1_En_%d", chid), hrate);

    auto fin_uf = TFile::Open(Form("%s/UF_%d.root", outcomePath.c_str(), chid));
    if (!fin_uf || fin_uf->IsZombie()) {
      std::cerr << "Warning: Cannot open UF_" << chid << ".root" << std::endl;
      continue;
    }
    fin_uf->GetObject("h_finalE", hrate_uf);

    if (hrate && hrate_uf) {
      auto hrate_m = (TH1D *)hrate_uf->Clone(Form("hrate_m_%d", chid));
      // 复制低能区数据
      int lowEBin = hrate_m->FindBin(m_configReader.GetEnergyCutLow());
      for (int i = 0; i <= lowEBin; i++) {
        hrate_m->SetBinContent(i + 1, hrate->GetBinContent(i + 1));
      }
      data.hratem[chid] = hrate_m;
      std::cout << "  Loaded rate histogram for channel " << chid << std::endl;
    } else {
      std::cerr << "Warning: Rate histogram not found for channel " << chid
                << std::endl;
    }
  }
}

void NeutronFluxAnalysis::CalculateStatisticalUncertainty(
    UncertaintyData &data) {
  std::cout << "\nCalculating statistical uncertainty..." << std::endl;

  for (const auto &[chid, hrate_m] : data.hratem) {
    auto herror = (TH1D *)hrate_m->Clone(Form("herror_rate_%d", chid));
    get_sta_errorhist(hrate_m, herror);
    data.error_rate[chid] = herror;
  }
}

void NeutronFluxAnalysis::CalculateUnfoldingUncertainty(
    UncertaintyData &data, const std::string &outcomePath,
    const std::vector<int> &channelIDs) {
  std::cout << "Calculating unfolding uncertainty..." << std::endl;

  for (const auto &[chid, hrate_m] : data.hratem) {
    auto fin_uf = TFile::Open(Form("%s/UF_%d.root", outcomePath.c_str(), chid));
    if (!fin_uf || fin_uf->IsZombie())
      continue;

    TH1D *haerror_uf{nullptr};
    fin_uf->GetObject("h_UFerroronly", haerror_uf);

    if (haerror_uf) {
      auto herror = (TH1D *)haerror_uf->Clone(Form("herror_uf_%d", chid));
      herror->Divide(hrate_m);
      // 低能区置零
      int lowEBin = herror->FindBin(m_configReader.GetEnergyCutLow());
      for (int i = 0; i <= lowEBin; i++) {
        herror->SetBinContent(i + 1, 0);
      }
      data.error_uf[chid] = herror;
    } else {
      std::cerr << "Warning: UF error histogram not found for channel " << chid
                << std::endl;
    }
  }
}

void NeutronFluxAnalysis::LoadDetectorUncertainty(
    UncertaintyData &data, const std::string &outcomePath) {
  std::cout << "Loading detector consistency uncertainty..." << std::endl;

  auto htemplate = data.error_rate.begin()->second;

  for (const auto &sampletype : data.sampletype_set) {
    auto filename =
        Form("%s/UN%sCOINEFF.dat", outcomePath.c_str(), sampletype.Data());
    auto graph = new TGraph();

    if (get_graph(filename, graph) >= 0) {
      auto hist =
          (TH1D *)htemplate->Clone(Form("herror_coin_%s", sampletype.Data()));
      hist->Reset();
      graph2hist(graph, hist);
      data.error_coin[sampletype] = hist;
    } else {
      std::cerr << "Warning: Cannot load " << filename << std::endl;
    }
  }
}

void NeutronFluxAnalysis::LoadENDFUncertainty(UncertaintyData &data) {
  std::cout << "Loading ENDF cross-section uncertainty..." << std::endl;

  auto htemplate = data.error_rate.begin()->second;

  for (const auto &sampletype : data.sampletype_set) {
    std::string endfFile;
    if (sampletype == "U5") {
      endfFile = m_configReader.GetUnENDFDataU5NF();
    } else if (sampletype == "U8") {
      endfFile = m_configReader.GetUnENDFDataU8NF();
    } else {
      std::cerr << "Warning: No ENDF uncertainty file for " << sampletype
                << std::endl;
      continue;
    }

    auto graph = new TGraph();
    if (get_graph(endfFile.c_str(), graph) >= 0) {
      auto hist =
          (TH1D *)htemplate->Clone(Form("herror_ENDF_%s", sampletype.Data()));
      hist->Reset();
      graph2hist(graph, hist);
      data.error_ENDF[sampletype] = hist;
    } else {
      std::cerr << "Warning: Cannot load " << endfFile << std::endl;
    }
  }
}

void NeutronFluxAnalysis::CalculateTotalUncertainty(
    UncertaintyData &data, const std::vector<int> &channelIDs) {
  std::cout << "\nCalculating total uncertainty..." << std::endl;

  for (const auto &sampletype : data.sampletype_set) {
    auto htemplate = data.error_ENDF[sampletype];

    // 初始化总不确定度直方图
    auto hrate_tot =
        (TH1D *)htemplate->Clone(Form("hrate_tot_%s", sampletype.Data()));
    auto herror_tot =
        (TH1D *)htemplate->Clone(Form("herror_tot_%s", sampletype.Data()));
    auto herror_rate_tot =
        (TH1D *)htemplate->Clone(Form("herror_rate_%s", sampletype.Data()));
    auto herror_uf_tot =
        (TH1D *)htemplate->Clone(Form("herror_uf_%s", sampletype.Data()));
    hrate_tot->Reset();
    herror_tot->Reset();
    herror_rate_tot->Reset();
    herror_uf_tot->Reset();

    // 对该样品类型的所有通道求和
    for (int chid : channelIDs) {
      if (sampletype == data.sampletype[chid]) {
        hrate_tot->Add(data.hratem[chid]);
        auto haerror_uf =
            (TH1D *)data.error_uf[chid]->Clone(Form("haerror_uf%d", chid));
        haerror_uf->Multiply(data.hratem[chid]);
        add_error(haerror_uf, herror_uf_tot);
      }
    }

    herror_uf_tot->Divide(hrate_tot);
    get_sta_errorhist(hrate_tot, herror_rate_tot);

    // 合并所有误差源
    add_error(herror_rate_tot, herror_tot);
    add_error(herror_uf_tot, herror_tot);
    add_error(data.error_ENDF[sampletype], herror_tot);
    add_error(data.error_coin[sampletype], herror_tot);

    data.tot_error_rate[sampletype] = herror_rate_tot;
    data.tot_error_uf[sampletype] = herror_uf_tot;
    data.tot_error[sampletype] = herror_tot;
    data.tot_hrate[sampletype] = hrate_tot;

    std::cout << "  Calculated total uncertainty for " << sampletype
              << std::endl;
  }
}

void NeutronFluxAnalysis::WriteResults(const UncertaintyData &data,
                                       const std::string &outcomePath) {
  // 写入 ROOT 文件
  std::cout << "\nWriting results to herror.root..." << std::endl;
  auto fout =
      new TFile(Form("%s/herror.root", outcomePath.c_str()), "recreate");

  for (const auto &[chid, hist] : data.error_uf) {
    hist->Write();
  }
  for (const auto &[chid, hist] : data.error_rate) {
    hist->Write();
  }
  for (const auto &sampletype : data.sampletype_set) {
    data.error_coin.at(sampletype)->Write();
    data.error_ENDF.at(sampletype)->Write();
    data.tot_error_rate.at(sampletype)->Write();
    data.tot_error_uf.at(sampletype)->Write();
    data.tot_error.at(sampletype)->Write();
    data.tot_hrate.at(sampletype)->Write();
  }

  fout->Close();
  std::cout << "Results written to: " << outcomePath << "/herror.root"
            << std::endl;

  // 写入文本文件
  std::cout << "\nWriting text files..." << std::endl;
  for (const auto &sampletype : data.sampletype_set) {
    auto h = data.tot_hrate.at(sampletype);
    auto herror = data.tot_error.at(sampletype);

    std::ofstream ofs(
        Form("%s/%s.dat", outcomePath.c_str(), sampletype.Data()));
    ofs << "Energy(MeV)\tdN/dlogE(neutrons/cm2/s)\tUncertainty(neutrons/cm2/"
           "s)\t"
           "Relative_Uncertainty(%)"
        << std::endl;

    for (int i = 0; i < h->GetNbinsX(); i++) {
      double En = h->GetBinCenter(i + 1) * eV_to_MeV;
      double Flux = h->GetBinContent(i + 1);
      double error = herror->GetBinContent(i + 1);
      double rel_error = (Flux > 0) ? (error / Flux * 100) : 0;

      ofs << En << "\t" << Flux << "\t" << error * Flux << "\t" << rel_error
          << std::endl;
    }
    ofs.close();
    std::cout << "  Written: " << outcomePath << "/" << sampletype << ".dat"
              << std::endl;
  }
}

void NeutronFluxAnalysis::DrawUncertaintyPlots(const UncertaintyData &data) {
  std::cout << "\nDrawing uncertainty plots..." << std::endl;

  for (const auto &sampletype : data.sampletype_set) {
    auto c =
        new TCanvas(Form("c_%s", sampletype.Data()),
                    Form("Uncertainty - %s", sampletype.Data()), 1200, 800);
    auto legd = new TLegend(0.7, 0.6, 0.9, 0.9);
    gPad->SetLogx();

    auto hstack = new THStack(Form("hs_%s", sampletype.Data()),
                              "; Neutron Energy (eV); Uncertainty");

    data.tot_error_rate.at(sampletype)->SetLineColor(color[0]);
    hstack->Add(data.tot_error_rate.at(sampletype));
    legd->AddEntry(data.tot_error_rate.at(sampletype), "Counting statistics",
                   "lf");

    data.tot_error_uf.at(sampletype)->SetLineColor(color[1]);
    hstack->Add(data.tot_error_uf.at(sampletype));
    legd->AddEntry(data.tot_error_uf.at(sampletype), "Double-bunch unfolding",
                   "lf");

    data.error_ENDF.at(sampletype)->SetLineColor(color[2]);
    hstack->Add(data.error_ENDF.at(sampletype));
    legd->AddEntry(data.error_ENDF.at(sampletype), "Cross-section", "lf");

    data.error_coin.at(sampletype)->SetLineColor(color[3]);
    hstack->Add(data.error_coin.at(sampletype));
    legd->AddEntry(data.error_coin.at(sampletype), "Detector consistency",
                   "lf");

    data.tot_error.at(sampletype)->SetLineColor(color[4]);
    hstack->Add(data.tot_error.at(sampletype));
    legd->AddEntry(data.tot_error.at(sampletype), "Total", "lf");

    hstack->Draw("nostack");
    legd->Draw();
    c->Update();
  }
}

// ============================================================================
// CalFlux 实现
// ============================================================================

void NeutronFluxAnalysis::CalFlux() {
  PrintSectionHeader("CalFlux Analysis");
  InitializeCommonConfig();

  // 获取配置
  const auto &fixmConfig = m_configReader.GetFIXMConfig();
  const auto &channelIDs = fixmConfig.Global.CHIDUSE;
  std::string outcomePath = m_xsPath + m_expName + "/Outcome/FIXM";
  int bpd = fixmConfig.Global.Bin.bpd;

  // 获取实验时间
  double expTime = 0.;
  const auto &timeList = m_configReader.GetTimeList();
  for (const auto &t : timeList) {
    expTime += t;
  }
  std::cout << "Total experiment time: " << expTime << " s" << std::endl;

  // 获取束流功率
  double beamPower = m_configReader.GetBeamPower(); // kW
  std::cout << "Beam power: " << beamPower << " kW" << std::endl;

  // 计算有效面积
  double r_beam = m_configReader.GetBeamRadius(); // mm
  double r_sample =
      fixmConfig.Channels.begin()->second.Radius;       // 第一个样品半径
  double r_eff = std::min(r_beam, r_sample) * mm_to_cm; // 转换为 cm
  double effectiveArea = r_eff * r_eff * TMath::Pi();
  std::cout << "Effective area: " << effectiveArea << " cm^2" << std::endl;

  // 初始化数据结构
  FluxData data;
  for (const auto &[chID, chConfig] : fixmConfig.Channels) {
    TString sampletype = chConfig.SampleType.c_str();
    data.sampletype[chID] = sampletype;
    data.samplename[chID] = sampletype + Form("%d", chConfig.SampleNumber);
    data.sampletype_set.insert(sampletype);
    data.detID[chID] = chConfig.DetID;

    // 计算面积密度
    double sample_r = chConfig.Radius * mm_to_cm; // 转换为 cm
    double sample_area = sample_r * sample_r * TMath::Pi();
    double sample_t = chConfig.Mass / sample_area; // mg/cm2
    double ArealDensity =
        sample_t / chConfig.A * Na * barn_to_cm2 * mg_to_g; // atoms/barn
    data.nd[chID] = ArealDensity;
  }

  // 执行通量计算流程
  LoadFluxInputData(data, outcomePath, channelIDs);
  CalculateFluxByType(data, channelIDs, expTime, beamPower, effectiveArea, bpd);
  LoadFluxUncertainty(data, outcomePath);
  WriteFluxResults(data, outcomePath);
  DrawFluxPlots(data);

  PrintClosePrompt();
  std::cout << "\nCalFlux analysis completed!" << std::endl;
}

// ============================================================================
// CalFlux 辅助函数实现
// ============================================================================

void NeutronFluxAnalysis::LoadFluxInputData(
    FluxData &data, const std::string &outcomePath,
    const std::vector<int> &channelIDs) {
  std::cout << "\nLoading flux input data..." << std::endl;

  // 打开输入文件
  auto fhratexs = TFile::Open(Form("%s/hrate.root", outcomePath.c_str()));
  auto fhratexs_uf =
      TFile::Open(Form("%s/hratexsuf.root", outcomePath.c_str()));
  auto fin_hatten =
      TFile::Open(Form("%s/fluxattenuation.root", outcomePath.c_str()));

  if (!fhratexs || fhratexs->IsZombie()) {
    std::cerr << "Error: Cannot open hrate.root" << std::endl;
    return;
  }
  if (!fhratexs_uf || fhratexs_uf->IsZombie()) {
    std::cerr << "Error: Cannot open hratexsuf.root" << std::endl;
    return;
  }
  if (!fin_hatten || fin_hatten->IsZombie()) {
    std::cerr << "Error: Cannot open fluxattenuation.root" << std::endl;
    return;
  }

  const auto &fixmConfig = m_configReader.GetFIXMConfig();

  // 处理每个通道
  for (int chid : channelIDs) {
    TH1D *hratexs{nullptr}, *hratexs_uf{nullptr}, *hatten{nullptr};

    // 获取反应率直方图
    fhratexs->GetObject(Form("h1_Enxs_%d", chid), hratexs);
    fhratexs_uf->GetObject(Form("h1_Enxs_%d", chid), hratexs_uf);

    // 获取衰减修正
    TString attenName = Form("htrans%d", data.detID[chid]);
    fin_hatten->GetObject(attenName, hatten);

    if (hratexs && hratexs_uf && hatten) {
      // 合并低能区和高能区数据
      auto hmerge = (TH1D *)hratexs_uf->Clone(Form("h1_Enxs_m%d", chid));
      int lowEBin = hmerge->FindBin(m_configReader.GetEnergyCutLow());
      for (int i = 0; i <= lowEBin; i++) {
        hmerge->SetBinContent(i + 1, hratexs->GetBinContent(i + 1));
      }

      // 应用探测器效率和衰减修正
      auto it = fixmConfig.Channels.find(chid);
      if (it != fixmConfig.Channels.end()) {
        double eff = it->second.DetEff;
        hmerge->Scale(1. / eff);
        hmerge->Divide(hatten);
        data.hratexs[chid] = hmerge;
        std::cout << "  Loaded and processed data for channel " << chid
                  << std::endl;
      }
    } else {
      std::cerr << "Warning: Missing data for channel " << chid << std::endl;
    }
  }
}

void NeutronFluxAnalysis::CalculateFluxByType(
    FluxData &data, const std::vector<int> &channelIDs, double expTime,
    double beamPower, double effectiveArea, int bpd) {
  std::cout << "\nCalculating flux by sample type..." << std::endl;

  // 为每种样品类型初始化通量直方图
  for (const auto &sampletype : data.sampletype_set) {
    auto hflux_tot = (TH1D *)data.hratexs.begin()->second->Clone(
        Form("hflux_m1_%s", sampletype.Data()));
    hflux_tot->Reset();
    data.hflux_tot[sampletype] = hflux_tot;
  }

  // 按样品类型聚合通量
  for (const auto &[sampletype, hflux_tot] : data.hflux_tot) {
    double ndtot = 0.;

    for (int chid : channelIDs) {
      if (sampletype == data.sampletype[chid]) {
        hflux_tot->Add(data.hratexs[chid]);
        ndtot += data.nd[chid];
        std::cout << "  " << data.samplename[chid] << "\t" << data.nd[chid]
                  << std::endl;
      }
    }

    // 归一化
    hflux_tot->Scale(1. / ndtot / expTime * bpd / effectiveArea);
    hflux_tot->Scale(100. / beamPower); // 归一化到 100 kW
    hflux_tot->SetYTitle("dN/dlog10E_{n}(neutrons/cm^{2}/s)");

    std::cout << "  Calculated flux for " << sampletype << std::endl;
  }
}

void NeutronFluxAnalysis::LoadFluxUncertainty(FluxData &data,
                                              const std::string &outcomePath) {
  std::cout << "\nLoading flux uncertainties..." << std::endl;

  auto ferror = TFile::Open(Form("%s/herror.root", outcomePath.c_str()));
  if (!ferror || ferror->IsZombie()) {
    std::cerr << "Warning: Cannot open herror.root" << std::endl;
    return;
  }

  for (const auto &sampletype : data.sampletype_set) {
    TString hname = Form("herror_tot_%s", sampletype.Data());
    TH1D *h{nullptr};
    ferror->GetObject(hname, h);

    if (h) {
      data.herror[sampletype] = h;
      std::cout << "  Loaded uncertainty for " << sampletype << std::endl;
    } else {
      std::cerr << "  Warning: Cannot find " << hname << " in herror.root"
                << std::endl;
    }
  }
}

void NeutronFluxAnalysis::WriteFluxResults(const FluxData &data,
                                           const std::string &outcomePath) {
  std::cout << "\nWriting flux results..." << std::endl;

  // 写入 ROOT 文件
  auto fout =
      TFile::Open(Form("%s/Flux.root", outcomePath.c_str()), "recreate");

  for (const auto &[sampletype, hflux_tot] : data.hflux_tot) {
    // 设置误差
    auto it = data.herror.find(sampletype);
    if (it != data.herror.end() && it->second) {
      set_binerror(hflux_tot, it->second);
      it->second->Write();
    } else {
      std::cout << "  Warning: Skip setting error for " << sampletype
                << " due to missing histogram" << std::endl;
    }
    hflux_tot->Write();
  }
  fout->Close();
  std::cout << "Results written to: " << outcomePath << "/Flux.root"
            << std::endl;

  // 写入文本文件
  std::cout << "\nWriting text files..." << std::endl;
  for (const auto &[sampletype, hflux_tot] : data.hflux_tot) {
    std::ofstream ofs(
        Form("%s/%s_Flux.dat", outcomePath.c_str(), sampletype.Data()));
    ofs << "Energy(MeV)\tdN/dlogE(neutrons/cm2/s)\tUncertainty(neutrons/cm2/"
           "s)\tRelative_Uncertainty(%)"
        << std::endl;

    for (int i = 0; i < hflux_tot->GetNbinsX(); i++) {
      double En = hflux_tot->GetBinCenter(i + 1) * eV_to_MeV;
      double value = hflux_tot->GetBinContent(i + 1);
      double error = hflux_tot->GetBinError(i + 1);
      double rerror = (value != 0) ? (error / value * 100) : 0.;
      ofs << En << "\t" << value << "\t" << error << "\t" << rerror
          << std::endl;
    }
    ofs.close();
    std::cout << "  Written: " << outcomePath << "/" << sampletype
              << "_Flux.dat" << std::endl;
  }

  // 输出积分通量
  int bpd = m_configReader.GetFIXMConfig().Global.Bin.bpd;
  for (const auto &[sampletype, hflux_tot] : data.hflux_tot) {
    std::cout << "Integral flux for " << sampletype << ": "
              << hflux_tot->Integral() / bpd << " neutrons/cm^2/s" << std::endl;
  }
}

void NeutronFluxAnalysis::DrawFluxPlots(const FluxData &data) {
  std::cout << "\nDrawing flux plots..." << std::endl;

  auto c = new TCanvas("c_flux", "Neutron Flux", 1200, 800);
  auto legd = new TLegend(0.7, 0.6, 0.9, 0.9);

  gPad->SetLogx();
  gPad->SetLogy();

  int colorIdx = 0;

  for (const auto &[sampletype, hflux_tot] : data.hflux_tot) {
    TString legname = sampletype + " m1";

    // 对 U8 应用低能截断
    if (sampletype == "U8") {
      for (int i = 0; i < hflux_tot->FindBin(m_configReader.GetEnergyCutU8());
           i++) {
        hflux_tot->SetBinContent(i + 1, 0);
      }
    }

    hflux_tot->SetLineColor(color[colorIdx % 13]);
    hflux_tot->Draw("same hist");
    legd->AddEntry(hflux_tot, legname, "lf");
    colorIdx++;
  }

  legd->Draw();
  c->Update();
}
