//-----------------------------------------------------------------------------------------------------
// this is a demo program to show how to use class DoubleBunchUnfolder to
// unfold a folded TOF-histogram and E-histogram from a doube bunch beam
// experiment. author: yih_csns_ihep date: 13/5/2018 comtact: yih@ihep.ac.cn
//-----------------------------------------------------------------------------------------------------

#include "ConfigReader.h"
#include "DoubleBunchUnfolder.h"
#include "utils.h"

#include "TApplication.h"
#include "TLegend.h"
#include "TMath.h"
#include "TROOT.h"

#include "inttypes.h"
#include "stdio.h"
#include "stdlib.h"
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char **argv) {
  //------------------------------------------
  // define variables
  //------------------------------------------

  double delay = 410.; // the unit of x-axis of histogram must be "ns" !!!
  int terminaltype = 0;

  int min_rebinc = delay / 10;
  if (argc < 5) {
    std::cout << "do not have enough parameters" << std::endl;
    return -1;
  }

  double ScaleFactor{1.};
  double lof{0.};
  bool isprint = false;
  std::string name_infile{};
  int chid{};
  int RunNumT{-1};
  double energycut{9.5e3};
  for (int iarg = 1; iarg < argc; ++iarg) {
    string opt = argv[iarg];
    if (opt == "-inf") {
      name_infile = argv[iarg + 1];
    }
    if (opt == "-noh") {
      chid = atoi(argv[iarg + 1]);
    }
    if (opt == "-run") {
      RunNumT = atoi(argv[iarg + 1]);
    }
    if (opt == "-dis") {
      isprint = atoi(argv[iarg + 1]);
    }
    if (opt == "-fsc") {
      ScaleFactor = atof(argv[iarg + 1]);
    }
    if (opt == "-ecut") {
      energycut = atof(argv[iarg + 1]);
    }
  }

  ConfigReader configReader;
  std::string fpc_path = "./config/filepath.json";
  std::string exp_path = "./config/202512_Flux.json";
  if (!configReader.LoadFilepathConfig(fpc_path)) {
    return -1;
  }
  if (!configReader.LoadExperimentConfig(exp_path)) {
    return -1;
  }

  lof = configReader.GetFIXMConfig().Channels.at(chid).Length;

  std::cout << ", Filename = " << name_infile << ", ChID = " << chid
            << ", FlightPath = " << lof << ", RunNumT = " << RunNumT
            << ", ScaleFactor = " << ScaleFactor << ", IsPrint = " << isprint
            << std::endl;

  TApplication *app = new TApplication("app", &argc, argv);

  int RunNumE = 1; // the total number of E-iteration

  // read En vs Delta L
  auto g_endeltaL = new TGraph();
  get_graph(configReader.GetDeltaLData().c_str(), g_endeltaL);
  auto deltaE_Eval = 10.34;                            // eV
  double lofgeo = lof - g_endeltaL->Eval(deltaE_Eval); // length of flight: m

  // ################################################################
  // ### select the Htype and Etype according to your application ###
  // ################################################################

  //  string Htype = "TOF"; // only unfold T-histogram
  string Htype = "ESPEC"; // unfold both T-histogram and E-histogram
  string etype;
  if (Htype == "ESPEC") {
    etype = "Log"; // the E-histogram has a logrithm coordinate x-axis with
                   // exponetial bin width.
    // etype = "Linear"; // the E-histogram has a linear coordinate x-axie
  }

  TH1D *h_inputT;  // double bunch tof-histogram, the x-axis must has unit of
                   // (ns) !!!
  TH1D *h_foldedT; // double bunch tof-histogram, the x-axis must has unit of
                   // (ns) !!!
  TH1D *h_finalT;  // unfolded tof-histogram after iteration
  TH1D *h_foldedE;
  TH1D *h_SampledE; // unfolded-sampled E-histogram after iteration
  TH1D *h_finalE;   // unfolded-sampled E-histogram after iteration
  TH1D *h_UFerror;  // unfolded-sampled E-histogram after iteration
  TH1D *h_staerror; // unfolded-sampled E-histogram after iteration
  TH1D *h_UFerroronly;

  TGraph *gerrorE;
  TLegend *leg3;
  TCanvas *c;
  TCanvas *c1;
  TGraph *gchi2;
  TGraph *glike;
  TGraph *gerrorT;
  TGraph *gpvalue;

  double *BinErrorsT;
  double *BinErrorsE;
  double *ChiSquare;
  double *Likelihood;
  auto RunNum = new double[RunNumT];
  auto pvalues = new double[RunNumT];

  DoubleBunchUnfolder *unfolder = new DoubleBunchUnfolder();
  unfolder->SetHistogramType(Htype);
  if (Htype == "ESPEC")
    unfolder->SetEtype(etype);
  unfolder->SetDelay(delay);
  unfolder->SetLOFgeo(lofgeo);
  unfolder->SetLOF(lof);
  unfolder->ImportEnvsDelta(g_endeltaL);
  unfolder->SetRunNum(RunNumT, RunNumE);
  unfolder->SetTerminalType(terminaltype);
  unfolder->SetScaleFactor(ScaleFactor);
  //------------------------------------------
  // read Data
  //------------------------------------------
  std::string dataType = configReader.GetDataType();
  std::string outputPath = (dataType == "XS") ? configReader.GetXSPath()
                                              : configReader.GetFluxPath();
  std::string expName = configReader.GetExperimentName();
  std::string outcomePath = outputPath + expName + "/Outcome/";
  std::string Dir_infile = outcomePath;

  TFile *infile = new TFile(TString(Dir_infile + "/" + name_infile), "read");
  if (!infile || infile->IsZombie()) {
    std::cout << name_infile << " not found or failed to open" << std::endl;
    return -1;
  }
  std::string name_htof = "h1_tof";
  name_htof = name_htof + "_" + std::to_string(chid);
  infile->GetObject(name_htof.data(), h_inputT);
  auto tofcut = floor(calTOF(energycut, lof) / min_rebinc) * min_rebinc + delay;
  h_foldedT = new TH1D("h_foldedT", "h_foldedT", tofcut, 0, tofcut);
  for (size_t i = 0; i < h_foldedT->GetNbinsX(); i++) {
    h_foldedT->SetBinContent(i + 1, h_inputT->GetBinContent(i + 1));
  }
  h_foldedT->Rebin(min_rebinc);
  double terminal0 = h_foldedT->GetBinCenter(h_foldedT->FindFirstBinAbove(0));

  if (h_foldedT == nullptr) {
    std::cout << name_htof << " not exit " << std::endl;
    return -1;
  }

  std::string name_hEn = "h1_En";
  name_hEn = name_hEn + "_" + std::to_string(chid);
  infile->GetObject(name_hEn.data(), h_foldedE);
  if (h_foldedE == nullptr) {
    std::cout << name_hEn << " not exit " << std::endl;
    return -1;
  }

  h_finalE = (TH1D *)h_foldedE->Clone("h_finalE");
  h_finalE->Reset();
  h_UFerror = (TH1D *)h_foldedE->Clone("h_UFerror");
  h_UFerror->Reset();
  h_staerror = (TH1D *)h_foldedE->Clone("h_staerror");
  h_staerror->Reset();
  h_UFerroronly =
      (TH1D *)h_foldedE->Clone("h_UFerroronly"); //(only UF error | no staerror)
  h_UFerroronly->Reset();

  //------------------------------------------
  // run unfolder
  //------------------------------------------
  unfolder->ImportHistogramT(h_foldedT); // double bunch histogram, the x-axis
                                         // must has unit of (ns) !!!
  if (Htype == "ESPEC")
    unfolder->ImportHistogramE(h_foldedE); // double bunch histogram, the x-axis
                                           // must has unit of (eV) !!!
  if (terminaltype == 0)
    unfolder->SetTerminalT(
        terminal0); // this function can only be called after ImprotHistogram()!
                    // min value of measureble TOF (ns)
  unfolder->RunUnfolder();
  h_finalT = unfolder->GetUnfoldedHistogramT(RunNumT);

  h_finalT->SetName("hUFt");
  if (Htype == "ESPEC")
    h_SampledE = unfolder->GetUnfoldedSampledE();

  BinErrorsE = unfolder->GetBinErrorsE();

  for (size_t i = 0; i < h_UFerror->GetNbinsX(); i++) {
    auto binc = h_SampledE->GetBinContent(i + 1);
    auto un = 0.;
    if (isinf(BinErrorsE[i]) || isnan(BinErrorsE[i])) {
      BinErrorsE[i] = RunNumT * binc;
    }
    if (binc != 0) {
      un = BinErrorsE[i] / binc;
      h_UFerror->SetBinContent(i + 1, BinErrorsE[i]);
      h_staerror->SetBinContent(i + 1, sqrt(binc));
      auto uferror = sqrt(BinErrorsE[i] * BinErrorsE[i] - binc);
      if (isinf(uferror) || isnan(uferror)) {
        uferror = binc;
      }
      h_UFerroronly->SetBinContent(i + 1, uferror);
    }
    h_finalE->SetBinContent(i + 1, binc);
  }

  std::cout << "\n";
  ChiSquare = unfolder->GetChiSquare();
  Likelihood = unfolder->GetLikelihood();

  int ndf = 0;
  for (int i = 1; i <= h_foldedT->GetNbinsX(); i++) {
    if (h_foldedT->GetBinContent(i) > 0) {
      ndf++;
    }
  }
  std::cout << "Valid Degrees of Freedom (NDF): " << ndf << std::endl;
  std::cout << "Iter\tChiSquare\tLikelihood\tp-value\n";
  for (int i = 0; i < RunNumT; i++) {
    RunNum[i] = i + 1;
    pvalues[i] = TMath::Prob(ChiSquare[i], ndf);
    cout << i << "\t" << ChiSquare[i] << "\t" << Likelihood[i] << "\t"
         << pvalues[i] << endl;
  }
  //------------------------------------------
  // draw histogram
  //------------------------------------------
  if (isprint) {
    c = new TCanvas("c", "c", 1400, 600);
    c->Divide(2, 2);
    c->cd(1);
    gPad->SetGridx();
    gPad->SetGridy();
    gPad->SetLogy();
    h_finalT->SetLineColor(kRed);
    h_finalT->Draw("HIST");
    h_foldedT->SetLineColor(kBlack);
    h_foldedT->Draw("HIST same");

    auto leg0 = new TLegend(0.6, 0.7, 0.9, 0.9);
    leg0->AddEntry(h_foldedT, "Double bunch TOF", "lf");
    leg0->AddEntry(h_finalT, "Unfolded TOF", "lf");
    leg0->Draw();

    c->cd(2);
    gPad->SetGridx();
    gPad->SetGridy();
    gPad->SetLogx();
    /*     gPad->SetLogy(); */

    h_finalE->GetXaxis()->SetRangeUser(1e4, 1e9);
    h_finalE->SetLineColor(kGreen);
    h_finalE->Draw("HIST");
    h_foldedE->SetLineColor(kBlack);
    h_foldedE->Draw("HIST same");
    auto leg1 = new TLegend(0.6, 0.7, 0.9, 0.9);
    leg1->AddEntry(h_foldedE, "Double bunch E", "lf");
    leg1->AddEntry(h_finalE, "UnfoldedSampled E", "lf");
    leg1->Draw();
    c->Update();
    c->Modified();
    c->cd(3);
    gPad->SetGridx();
    gPad->SetGridy();
    BinErrorsT = unfolder->GetBinErrorsT();
    int N_binT = unfolder->GetNbinT();
    double tmin = unfolder->GetTmin();
    double tmax = unfolder->GetTmax();
    double tstep = (tmax - tmin) / N_binT;
    double Tbins[N_binT];
    for (int i = 0; i < N_binT; i++) {
      Tbins[i] = tmin + i * tstep;
    }
    gerrorT = new TGraph(N_binT, Tbins, BinErrorsT);
    gerrorT->SetTitle(";TOF (ns);Error");
    gerrorT->Draw();
    auto leg2 = new TLegend(0.6, 0.8, 0.9, 0.9);
    leg2->AddEntry(gerrorT, "BinErrorsT", "lf");
    leg2->Draw();

    c->cd(4);
    gPad->SetGridx();
    gPad->SetGridy();
    gPad->SetLogx();
    h_UFerror->Draw("hist");
    h_staerror->Draw("same");
    h_staerror->SetLineColor(kRed);
    TLegend *leg3 = new TLegend(0.6, 0.8, 0.9, 0.9);
    leg3->AddEntry(h_UFerror, "BinErrorsE", "lf");
    leg3->AddEntry(h_staerror, "StaErrorsE", "lf");
    leg3->Draw();

    c1 = new TCanvas("c1", "c1", 1800, 400);
    c1->Divide(3, 1);

    c1->cd(1);
    gchi2 = new TGraph(RunNumT, RunNum, ChiSquare);
    gchi2->SetTitle(";Iteration Number;ChiSquare");
    gchi2->SetLineColor(kRed);
    gchi2->Draw("al");
    c1->cd(2);
    glike = new TGraph(RunNumT, RunNum, Likelihood);
    glike->SetTitle(";Iteration Number;Likelihood");
    glike->SetLineColor(kBlue);
    glike->Draw("al");
    c1->cd(3);
    gpvalue = new TGraph(RunNumT, RunNum, pvalues);
    gpvalue->SetTitle(";Iteration Number;p-value");
    gpvalue->SetLineColor(kGreen + 2);
    gpvalue->Draw("al");
  }

  //---------------------------------------------
  // draw the process of convergency of unfolding
  //---------------------------------------------

  if (isprint) {
    app->Run();
  } else {
    std::string DirOut = outcomePath;
    TString name_froot = "UF_" + std::to_string(chid) + ".root";
    auto frootout = TFile::Open(TString(DirOut + name_froot), "recreate");
    h_foldedT->Write();
    h_finalT->Write();
    h_finalE->Write();
    h_UFerror->Write();
    h_UFerroronly->Write();
    frootout->Close();
  }

  //------------------------------------------
  // save h_final and close file
  //------------------------------------------

  //------------------------------------------
  // delete pointer, free memory
  //------------------------------------------

  //------------------------------------------
  // end of code
  //------------------------------------------
  return 0;
} // end of main function
