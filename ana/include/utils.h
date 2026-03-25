/*
 * @Descripttion:
 * @version: 1.0
 * @Author: Qiu Yijia
 * @Date: 2023-07-27 09:17:35
 * @LastEditors: Qiu Yijia
 * @LastEditTime: 2023-11-06 00:30:02
 */

#ifndef _PUBFUNC_H_
#define _PUBFUNC_H_

#include "TFile.h"
#include "TGraph.h"
#include "TH1.h"
#include "TSpline.h"
#include <TCanvas.h>
#include <TChain.h>
#include <TRandom3.h>
#include <TStyle.h>
#include <TSystem.h>
#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <sstream>

constexpr double Na = 6.02214076e23;
constexpr double cspeed = 0.29979;           // speed of light: m/ns
constexpr double Mneutron = 939.5654133;     // mass of neutron: MeV
constexpr double echarge = 1.6021766208e-19; // elementary charge: C

// Unit conversion factors
constexpr double mm_to_cm = 0.1;
constexpr double cm_to_mm = 10.0;
constexpr double eV_to_MeV = 1e-6;
constexpr double MeV_to_eV = 1e6;
constexpr double barn_to_cm2 = 1e-24;
constexpr double cm2_to_barn = 1e24;
constexpr double mg_to_g = 1e-3;

static EColor color[] = {kRed,     kBlue, kBlack,  kGreen, kYellow,
                         kMagenta, kCyan, kOrange, kPink,  kSpring,
                         kViolet,  kTeal, kAzure};

/**
 * @name: calEn
 * @msg: calculate neutron energy : eV
 * @param {double} tof :ns
 * @param {double} length :m
 * @return {*}
 */
inline double calEn(double tof, double length) {
  auto En =
      Mneutron *
      (1. / sqrt(1. - length * length / (cspeed * tof) / (cspeed * tof)) - 1.) *
      1e6;
  return En;
}

/**
 * @name: calTOF
 * @msg: calculate neutron TOF : ns
 * @param {double} energy
 * @param {double} length
 * @return {*}
 */
inline double calTOF(double energy, double length) {
  auto tof = length / cspeed *
             sqrt(1.0 / (1.0 - 1.0 / (1.0 + energy / 1e6 / Mneutron) /
                                   (1.0 + energy / 1e6 / Mneutron)));
  return tof;
}

/**
 * @name: GetHistogramMinBinInRange
 * @msg: Get Histogram Min Bin in Range
 * @param {TH1*} hist
 * @param {double} xmin
 * @param {double} xmax
 * @return minBin
 */
inline int GetHistogramMinBinInRange(TH1 *hist, double xmin, double xmax) {

  // 找到指定区间的bin索引
  int binMin = hist->GetXaxis()->FindBin(xmin);
  int binMax = hist->GetXaxis()->FindBin(xmax);

  // 初始化最小值为区间内的第一个bin的内容
  double minVal = hist->GetBinContent(binMin);
  int minBin = binMin;

  // 在区间内遍历所有bin，找到最小值
  for (int i = binMin + 1; i <= binMax; i++) {
    double binContent = hist->GetBinContent(i);
    if (binContent < minVal) {
      minVal = binContent;
      minBin = i;
    }
  }

  return minBin;
}

/**
 * @name:
 * @msg:
 * @param {TH1*} hist
 * @param {double} xmin
 * @param {double} xmax
 * @return {*}
 */
inline int GetHistogramMaxBinInRange(TH1 *hist, double xmin, double xmax) {

  // 找到指定区间的bin索引
  int binMin = hist->GetXaxis()->FindBin(xmin);
  int binMax = hist->GetXaxis()->FindBin(xmax);

  // 初始化最小值为区间内的第一个bin的内容
  double maxVal = hist->GetBinContent(binMin);
  int maxBin = binMin;

  // 在区间内遍历所有bin，找到最小值
  for (int i = binMin + 1; i <= binMax; i++) {
    double binContent = hist->GetBinContent(i);
    if (binContent > maxVal) {
      maxVal = binContent;
      maxBin = i;
    }
  }

  return maxBin;
}

inline int read_proton(const char *fileName,
                       std::map<TString, double> &map_data) {
  std::ifstream fin_txt(fileName);
  if (!fin_txt.is_open()) {
    std::cerr << "no such file to read proton!" << std::endl;
    return -1;
  }
  // read para
  TString line{};
  line.ReadLine(fin_txt);
  while (!fin_txt.eof()) {
    line.ReadLine(fin_txt);
    TString runfile{};
    ULong64_t nproton{};
    Double_t BeamPower{};
    Double_t ptime{};
    if (line.Sizeof() < 2) {
      break;
    }

    std::stringstream ss(line.Data());
    ss >> runfile >> nproton >> BeamPower >> ptime;

    map_data.insert(std::make_pair(runfile, ptime));
  }
  return 1;
}

/**
 * @name:
 * @msg:
 * @param {char*} filename
 * @param {TGraph*} graph_
 * @param {double} times
 * @return {*}
 */
inline int get_graph(const char *filename, TGraph *graph_, double times = 1.,
                     TString opts = "") {
  std::ifstream fin_txt(filename);
  if (!fin_txt.is_open()) {
    std::cerr << "no such file " << filename << " to read graph para "
              << std::endl;
    return -1;
  }
  // read para
  TString line{};
  line.ReadLine(fin_txt);
  while (!fin_txt.eof()) {
    line.ReadLine(fin_txt);

    Double_t En{}, XS{};

    if (line.Sizeof() < 2) {
      break;
    }
    if (line.Contains("#")) {
      continue;
    }
    if (line.Contains("/")) {
      continue;
    }
    std::stringstream ss(line.Data());
    ss >> En >> XS;
    En = En * times;
    if (opts == "log10") {
      En = log10(En);
    }

    graph_->AddPoint(En, XS);
  }
  return 1;
}

/**
 * @name:
 * @msg: convert TGraph to histogram| if val<0, val=0.
 * @param {TGraph*} gr
 * @param {TH1*} h1
 * @return {*}
 */
inline int graph2hist(TGraph *gr, TH1 *h1, TString opts = "") {
  if (!gr || gr->GetN() == 0)
    return 0;

  double xmin = *std::min_element(gr->GetX(), gr->GetX() + gr->GetN());
  double xmax = *std::max_element(gr->GetX(), gr->GetX() + gr->GetN());

  for (size_t i = 0; i < h1->GetNbinsX(); i++) {
    auto x = h1->GetBinCenter(i + 1);
    if (x < xmin || x > xmax) {
      h1->SetBinContent(i + 1, 0);
      continue;
    }
    auto y = gr->Eval(x, nullptr, opts);
    if (y < 0) {
      y = 0;
    }
    h1->SetBinContent(i + 1, y);
  }
  return 1;
}

inline void get_sta_errorhist(TH1D *hinput, TH1D *herror) {
  herror->Reset();
  for (size_t i = 0; i < hinput->GetNbinsX(); i++) {
    auto bincontent = hinput->GetBinContent(i + 1);
    if (bincontent == 0) {
      herror->SetBinContent(i + 1, 0);
    } else {
      herror->SetBinContent(i + 1, sqrt(1. / bincontent));
    }
  }
}

inline void set_binerror(TH1D *hinput, TH1D *herror) {
  // check
  auto n1 = hinput->GetNbinsX();
  auto n2 = herror->GetNbinsX();
  if (n1 != n2) {
    std::cout << "Nbins are not the same!" << std::endl;
    return;
  }
  for (size_t i = 0; i < n1; i++) {
    auto rerror = herror->GetBinContent(i + 1);
    auto bincontent = hinput->GetBinContent(i + 1);
    // std::cout << rerror << "\t" << bincontent << "\n";
    hinput->SetBinError(i + 1, rerror * bincontent);
  }
}

inline void add_error(TH1D *hinput1, TH1D *hinput2, TH1D *herrortot) {
  herrortot->Reset();
  for (size_t i = 0; i < herrortot->GetNbinsX(); i++) {
    auto error1 = hinput1->GetBinContent(i + 1);
    auto error2 = hinput2->GetBinContent(i + 1);
    auto errortot = sqrt(error1 * error1 + error2 * error2);
    herrortot->SetBinContent(i + 1, errortot);
  }
}

inline void add_error(TH1D *hinput, TH1D *herrortot) {
  for (size_t i = 0; i < herrortot->GetNbinsX(); i++) {
    auto error1 = hinput->GetBinContent(i + 1);
    auto error2 = herrortot->GetBinContent(i + 1);
    if (error1 < 0) {
      error1 = 0;
      std::cout << error1 << std::endl;
    }
    if (error2 < 0) {
      error2 = 0;
      std::cout << error2 << std::endl;
    }
    auto errortot = sqrt(error1 * error1 + error2 * error2);
    herrortot->SetBinContent(i + 1, errortot);
  }
}

inline void get_rerror(TH1D *hinput, TH1D *h_aerror, TH1D *h_rerror) {
  h_rerror->Reset();
  for (size_t i = 0; i < hinput->GetNbinsX(); i++) {
    auto bincontent = hinput->GetBinContent(i + 1);
    if (bincontent > 0) {
      auto aerror = h_aerror->GetBinContent(i + 1);
      auto rerror = aerror / bincontent;
      h_rerror->SetBinContent(i + 1, rerror);
    }
  }
}

inline void get_aerror(TH1D *hinput, TH1D *hr_error, TH1D *ha_error) {
  ha_error->Reset();
  for (size_t i = 0; i < hinput->GetNbinsX(); i++) {
    auto bincontent = hinput->GetBinContent(i + 1);
    if (bincontent == 0) {
      ha_error->SetBinContent(i + 1, 0);
    } else {
      ha_error->SetBinContent(i + 1,
                              hr_error->GetBinContent(i + 1) * bincontent);
    }
  }
}

inline void get_aerror(TH1D *hinput, TH1D *ha_error) {
  ha_error->Reset();
  for (size_t i = 0; i < hinput->GetNbinsX(); i++) {
    auto binerror = hinput->GetBinError(i + 1);
    auto bincontent = hinput->GetBinContent(i + 1);
    if (bincontent > 0) {
      ha_error->SetBinContent(i + 1, binerror);
    }
  }
}

inline void get_nomintergral_error(TH1D *hinput1, TH1D *hinput2, TH1D *herror,
                                   const double *binrange) {
  herror->Reset();
  auto ibin_spec_sta = hinput1->FindBin(binrange[0]);
  auto ibin_spec_sto = hinput1->FindBin(binrange[1]);
  Double_t sum[2]{};
  Double_t error[2]{};

  for (size_t i = ibin_spec_sta; i <= ibin_spec_sto; i++) {
    auto binerror1 = hinput1->GetBinError(i + 1);
    auto binerror2 = hinput2->GetBinError(i + 1);
    auto bincontent1 = hinput1->GetBinContent(i + 1);
    auto bincontent2 = hinput2->GetBinContent(i + 1);
    if (bincontent1 > 0 && bincontent2 > 0) {
      sum[0] += bincontent1;
      sum[1] += bincontent2;
      error[0] = sqrt(error[0] * error[0] + (binerror1) * (binerror1));
      error[1] = sqrt(error[1] * error[1] + (binerror2) * (binerror2));
    }
  }
  auto errortot = sqrt(error[0] * error[0] / (sum[0] * sum[0]) +
                       error[1] * error[1] / (sum[1] * sum[1]));
  for (size_t i = 0; i < herror->GetNbinsX(); i++) {
    herror->SetBinContent(i + 1, errortot);
  }

  std::cout << "norm uncertainty " << errortot << std::endl;
}

inline void getrangehist(TH1D *hinput, TH1D *hout, Double_t xmin,
                         Double_t xmax) {
  hout->Reset();
  for (size_t i = 0; i < hinput->GetNbinsX(); i++) {
    double binCenter = hinput->GetBinLowEdge(i + 1);
    if (binCenter >= xmin &&
        binCenter <= xmax) { // 检查bin的中心是否在指定范围内
      hout->SetBinContent(
          i + 1, hinput->GetBinContent(i + 1)); // 如果在范围内，复制该bin的内容
    }
  }
}

inline void getrangehist(TH1D *hinput, TH1D *hout, int ixmin, int ixmax) {
  hout->Reset();
  for (size_t i = 0; i < hinput->GetNbinsX(); i++) {
    if (i >= ixmin && i <= ixmax) { // 检查bin的中心是否在指定范围内
      hout->SetBinContent(
          i + 1, hinput->GetBinContent(i + 1)); // 如果在范围内，复制该bin的内容
    }
  }
}

inline void movehist(TH1D *hinput, TH1D *hout, Double_t movestep) {
  hout->Reset();
  auto imovestep = movestep / hout->GetBinWidth(1);
  for (size_t i = 0; i < hinput->GetNbinsX() - imovestep; i++) {
    hout->SetBinContent(
        i + 1 + imovestep,
        hinput->GetBinContent(i + 1)); // 如果在范围内，复制该bin的内容
  }
}

inline void smooth_gdata(TGraph *gin, TGraph *gout) {
  int n = gin->GetN();
  double log_x[n], log_y[n];

  for (int i = 0; i < n; ++i) {
    double x, y;
    gin->GetPoint(i, x, y);
    log_x[i] = x;
    log_y[i] = (y > 0) ? log(y) : -5; // 避免 log(0)
  }

  auto gin_log = new TGraph(n, log_x, log_y);

  // 对数空间插值
  auto spline_log = new TSpline3("spline_log", gin_log, "e", 1, 1);

  // 绘制对数插值结果（转换回线性空间）
  for (double x = gin->GetXaxis()->GetXmin(); x <= gin->GetXaxis()->GetXmax();
       x *= pow(10, 0.001)) {
    double y_log = spline_log->Eval(x);
    gout->AddPoint(x, exp(y_log));
  }
  SafeDelete(gin_log);
  SafeDelete(spline_log);
}

inline TH1D *getsimhist(const char *filename, double timesE = 1.) {
  TH1D *h{nullptr};
  std::ifstream fin_txt(filename);
  if (!fin_txt.is_open()) {
    std::cerr << "no such file " << filename << " to read graph para!"
              << std::endl;
    return h;
  }
  // read para
  std::vector<double> vE1{};
  std::vector<double> vE2{};
  std::vector<double> vflux{};

  TString line{};
  line.ReadLine(fin_txt);
  while (!fin_txt.eof()) {
    line.ReadLine(fin_txt);
    Double_t En1{}, En2{}, XS{};

    if (line.Sizeof() < 2) {
      break;
    }
    if (line.Contains("#")) {
      continue;
    }

    std::stringstream ss(line.Data());
    ss >> En1 >> En2 >> XS;
    vE1.emplace_back(En1 * timesE);
    vE2.emplace_back(En2 * timesE);
    vflux.emplace_back(XS);
  }
  TString hname = filename;
  hname.ReplaceAll(".dat", "");
  auto nbins = vE1.size();
  auto xbins = new double[nbins + 1]{};
  for (size_t i = 0; i < vE1.size(); i++) {
    xbins[i] = vE1[i];
  }
  xbins[nbins] = vE2[nbins - 1];
  h = new TH1D(hname, hname, nbins, xbins);
  for (size_t i = 0; i < nbins; i++) {
    h->SetBinContent(i + 1, vflux[i]);
  }

  std::cout << std::endl;
  return h;
}

inline bool ensure_path_exists(const std::string &path,
                               const std::string &label) {
  if (gSystem->AccessPathName(path.c_str())) {
    std::cerr << "[Error] " << label << " not found: " << path << std::endl;
    return false;
  }
  return true;
}

inline std::vector<Double_t> SetLogBins(int nDec, int bpd, Double_t LowEdge) {
  int nbins = nDec * bpd;
  std::vector<Double_t> bins(nbins + 1);
  for (Int_t i = 0; i <= nbins; i++) {
    bins[i] = pow(10, LowEdge + 1. / (double)bpd * (double)i);
  }
  return bins;
}

/**
 * @name: caldT
 * @msg: calculate time difference for flight path calibration : ns
 * @param {double*} x : array with x[0] = energy (eV)
 * @param {double*} par : array with par[0] = length (m)
 * @return {double} time difference (ns)
 */
inline double caldT(double *x, double *par) {
  double energy = x[0];   // variable energy
  double length = par[0]; // parameter length
  double dt = length / cspeed *
                  sqrt(1.0 / (1.0 - 1.0 / (1.0 + energy / 1e6 / Mneutron) /
                                        (1.0 + energy / 1e6 / Mneutron))) -
              length / cspeed;
  return dt;
}
#endif