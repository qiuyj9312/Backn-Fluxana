//-------------------------------------------------------------------------------------------
// this source file defines the functions of class DoubleBunchUnfolder
// author: yih_csns_ihep
// date: 13/5/2018
// contact: yih@ihep.ac.cn
//
// update: Add error calculation for E-histogram.
// The function RunUnfolderEprocess() has a wrong algrithom, so don't use it
// currently. date: 2020-10-4 author: yih@ihep.ac.cn
//-------------------------------------------------------------------------------------------

#include "DoubleBunchUnfolder.h"

#include "TAxis.h"
#include "TH1D.h"
#include "TMath.h"

#include "inttypes.h"
#include "iostream"
#include "math.h"
#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"

#define Mneutron 939565413.3 // mass of neutron
#define cspeed 0.299792458   // speed of light

using namespace std;
using namespace TMath;

//-------------------------------------------------------------------------------
// DoubleBunchUnfolder();
//-------------------------------------------------------------------------------
DoubleBunchUnfolder::DoubleBunchUnfolder() : scaleFactor(1.) { ; }

//-------------------------------------------------------------------------------
// ~DoubleBunchUnfolder();
//-------------------------------------------------------------------------------
DoubleBunchUnfolder::~DoubleBunchUnfolder() {
  delete[] Tcount;
  if (Htype == "ESPEC")
    delete[] Ecount;

  delete[] hTBinNum;
  if (Htype == "ESPEC")
    delete[] hEBinNum;

  // sleep(SleepTime);
  // cout<<"===> Free count and BinNum memory."<<endl;
  // cout<<"===> Free h_unfolded memory......"<<endl;

  for (int i = 0; i < (RunNumT + 1); i++) {
    delete h_unfoldedT[i];
  }
  if (Htype == "ESPEC") {
    for (int i = 0; i < (RunNumE + 1); i++) {
      delete h_unfoldedE[i];
    }
  }
  // cout<<"===> Free hunfolded memory."<<endl;

  delete[] h_unfoldedT;
  if (Htype == "ESPEC") {
    delete[] h_unfoldedE;
    delete h_unfoldedSampledE;
  }
  delete[] BinErrorsT;
  if (Htype == "ESPEC")
    delete[] BinErrorsE;
  delete[] ChiSquare;
  delete[] Likelihood;
  sleep(SleepTime / 2);
  // cout<<"===> Free bin memory and h_unfoldedT/E memory ......"<<endl;
}

//-------------------------------------------------------------------------------
// void SetEtype(string type)
//-------------------------------------------------------------------------------
void DoubleBunchUnfolder::SetEtype(std::string type) {
  Etype = type;
  cout << "===> Etype = " << Etype << endl;
  sleep(SleepTime);
}

//-------------------------------------------------------------------------------
// void SetDelay(double d);
//-------------------------------------------------------------------------------
void DoubleBunchUnfolder::SetDelay(double d) {
  delay = d;
  cout << "Delay = " << delay << endl;
  sleep(SleepTime);
}

//-------------------------------------------------------------------------------
// void SetLOF(double lof);
//-------------------------------------------------------------------------------
void DoubleBunchUnfolder::SetLOF(double lof) {
  LOF = lof;
  cout << "LOF = " << lof << endl;
  sleep(SleepTime);
}

//-------------------------------------------------------------------------------
// void SetRunNum(int n);
//-------------------------------------------------------------------------------
void DoubleBunchUnfolder::SetRunNum(int nt, int ne) {
  RunNumT = nt;
  RunNumE = ne;
  ChiSquare = new double[RunNumT];
  Likelihood = new double[RunNumT];
  cout << "RunNumT = " << RunNumT << ", RunNumE = " << RunNumE << endl;
  sleep(SleepTime);
}

//-------------------------------------------------------------------------------
// void ImportHistogramT(TH1D* h);
//-------------------------------------------------------------------------------
void DoubleBunchUnfolder::ImportHistogramT(TH1D *ht) {

  h_doubleT = ht;
  N_binT = h_doubleT->GetNbinsX();
  Tcount = new double[N_binT]{};
  hTBinNum = new double[N_binT]{};
  BinErrorsT = new double[N_binT]{};
  tmin = h_doubleT->GetXaxis()->GetXmin();
  tmax = h_doubleT->GetXaxis()->GetXmax();
  BinWidthT = (tmax - tmin) / N_binT;
  delay_bin = round(delay / BinWidthT);

  cout << "===> Import the T-histogram successfully!" << endl;
  cout << "NbinsXT = " << N_binT << endl;
  cout << "delay_bin = " << delay_bin << endl;
  cout << "X_minT = " << tmin << ", X_maxT = " << tmax << endl;
  cout << "BinWidthT = " << BinWidthT << endl;
  sleep(SleepTime);
}

//-------------------------------------------------------------------------------
// void ImportHistogramE(TH1D* h);
//-------------------------------------------------------------------------------
void DoubleBunchUnfolder::ImportHistogramE(TH1D *he) {
  if (Etype == "Linear") {
    h_doubleE = he;
    N_binE = h_doubleE->GetNbinsX();
    Ecount = new double[N_binE];
    hEBinNum = new double[N_binE];
    BinErrorsE = new double[N_binE];
    emin = h_doubleE->GetXaxis()->GetXmin();
    emax = h_doubleE->GetXaxis()->GetXmax();
    BinWidthE = (emax - emin) / N_binE;
  }

  if (Etype == "Log") {
    N_binE = he->GetNbinsX();
    Ecount = new double[N_binE];
    hEBinNum = new double[N_binE];
    BinErrorsE = new double[N_binE];
    emin = Log10(he->GetXaxis()->GetXmin());
    emax = Log10(he->GetXaxis()->GetXmax());
    BinWidthE = (emax - emin) / N_binE;
    h_doubleE = new TH1D("h_doubleE", "h_doubleE", N_binE, emin, emax);
    for (int i = 0; i < N_binE; i++) {
      h_doubleE->SetBinContent(i + 1, he->GetBinContent(i + 1));
    }
  }

  cout << "===> Import the E-histogram successfully!" << endl;
  cout << "NbinsXE = " << N_binE << endl;
  cout << "X_minE = " << emin << ", X_maxE = " << emax << endl;
  cout << "BinWidthE = " << BinWidthE << endl;
  sleep(SleepTime);
}
//-------------------------------------------------------------------------------
// void SetTerminalType(int type);
//-------------------------------------------------------------------------------
void DoubleBunchUnfolder::SetTerminalType(int type) {
  TerminalType = type;
  cout << "===> TerminalType = " << TerminalType << endl;
  sleep(SleepTime);
} // end of function

//-------------------------------------------------------------------------------
// void SetTerminalT(double min);
//-------------------------------------------------------------------------------
void DoubleBunchUnfolder::SetTerminalT(double m) {
  TerminalT = m;
  TerminalBinT =
      floor((TerminalT - tmin) / BinWidthT); // bin index begin from 0
  cout << "===> TerminalT-1D = " << TerminalT << endl;
  cout << "===> TerminalBinT-1D = " << TerminalBinT << endl;

  if (Htype == "ESPEC") {
    TerminalE = Nenergy(TerminalT, LOF);

    if (isinf(TerminalE)) {
      TerminalE = emax;
    }

    TerminalBinE = floor((TerminalE) / BinWidthE); // bin index begin from 1
    cout << "===> TerminalE-1D = " << TerminalE << endl;
    cout << "===> TerminalBinE-1D = " << TerminalBinE << endl;
  }

  sleep(SleepTime);
}

//--------------------------------------------
// Ntof(): ns
//--------------------------------------------
double DoubleBunchUnfolder::Ntof(double e, double length) {
  double energy = {0.}; // eV
  if (Etype == "Linear")
    energy = e;
  if (Etype == "Log")
    energy = Power(10., e);
  double tof = 0.;
  tof = length / cspeed *
        Sqrt(1.0 / (1.0 - 1.0 / (1.0 + energy / Mneutron) /
                              (1.0 + energy / Mneutron)));
  return tof;
}

//--------------------------------------------
// Nenergy(): eV
//--------------------------------------------
double DoubleBunchUnfolder::Nenergy(double tof, double length) {
  double energy{0.}; // energy of neutron: eV
  energy =
      Mneutron *
      (1. / Sqrt(1. - length * length / (cspeed * tof) / (cspeed * tof)) - 1.);
  if (Etype == "Linear")
    return energy;
  else
    return Log10(energy);
}

//-------------------------------------------------------------------------------
// void RunUnfolderT();
//-------------------------------------------------------------------------------
void DoubleBunchUnfolder::RunUnfolderTprocess() {

  //---------set basic parameters and variables--------//
  h_unfoldedT = new TH1D *[RunNumT + 1];
  for (int i = 0; i < (RunNumT + 1); i++) {
    char hname[32];
    sprintf(hname, "h_unfoldedT[%d]", i);
    h_unfoldedT[i] = new TH1D(hname, hname, N_binT, tmin, tmax);
  }
  for (int i = 0; i < N_binT; i++) {
    Tcount[i] = h_doubleT->GetBinContent(i + 1);
  }

  double integral = h_doubleT->Integral();
  for (int i = 0; i < N_binT; i++) {
    h_unfoldedT[0]->SetBinContent(i + 1, h_doubleT->GetBinContent(i + 1));
    // h_unfoldedT[0]->SetBinContent(i+1, 1./h_doubleT->Integral());
  }
  for (int i = 0; i < N_binT; i++) {
    hTBinNum[i] = 0.;
  }

  //---------define partial derivation matrix---------------//
  // double PDmatrix0[N_binT][N_binT]; // partial derivation of last iteration
  // double PDmatrix1[N_binT][N_binT]; // partial derivation of new iteration

  double **PDmatrix0 = new double *[N_binT] {};
  double **PDmatrix1 = new double *[N_binT] {};

  for (int i = 0; i < N_binT; i++) {
    PDmatrix0[i] = new double[N_binT]{};
    PDmatrix1[i] = new double[N_binT]{};
  }

  for (int i = 0; i < N_binT; i++)
    for (int j = 0; j < N_binT; j++) {
      if (i == j)
        PDmatrix0[i][j] = 1.;
      if (i != j)
        PDmatrix0[i][j] = 0.;
      PDmatrix1[i][j] = PDmatrix0[i][j];
    }

  //---------iteration cylce--------//
  for (int run = 1; run < (RunNumT + 1); run++) {

    // h_unfoldedT[run-1]->Smooth(1);
    for (int i = 0; i < (N_binT); i++) {
      // cout<<"i = "<<i<<endl;
      if ((i + 1 - TerminalBinT) < 1) {
        hTBinNum[i] = h_unfoldedT[run - 1]->GetBinContent(i + 1);
      } // end of region 1

      if ((i + 1 - TerminalBinT) > 0 &&
          (i + 1 - TerminalBinT) < (delay_bin + 1)) {
        double ni = h_unfoldedT[run - 1]->GetBinContent(i + 1);
        double np = h_unfoldedT[run - 1]->GetBinContent(i + delay_bin + 1);
        // cout<<"ni = "<<ni<<", np = "<<np<<endl;
        double k1 = 0.;
        double k2 = 0.;
        if (TerminalType == 0) {
          k1 = 1.0;
        }
        if (TerminalType == 1) {
          k1 = 0.5;
        }
        if ((ni + np) > 0.) {
          k2 = ni / (ni + np);
        }
        if ((ni + np) <= 0.) {
          k2 = 0.;
        }
        hTBinNum[i] = k1 * Tcount[i] + Tcount[i + delay_bin] * k2;

        if (ni > 0.) {
          double R[N_binT];
          for (int j = 0; j < N_binT; j++) {
            if (j == i)
              R[j] = k1;
            else if (j == (i + delay_bin))
              R[j] = k2;
            else
              R[j] = 0.;
          }
          for (int j = 0; j < N_binT; j++)
            PDmatrix1[i][j] =
                R[j] +
                R[i + delay_bin] * Tcount[i + delay_bin] / ni *
                    PDmatrix0[i][j] -
                R[i + delay_bin] * R[i + delay_bin] * Tcount[i + delay_bin] /
                    ni * (PDmatrix0[i][j] + PDmatrix0[i + delay_bin][j]);
        }
        if (ni <= 0.) {
          for (int j = 0; j < N_binT; j++)
            PDmatrix1[i][j] = 0.;
        }
      } // end of region2

      if (((i + 1 - TerminalBinT) > delay_bin) &&
          ((i + 1) < (N_binT - delay_bin + 1))) {
        double ni = h_unfoldedT[run - 1]->GetBinContent(i + 1);
        double np = h_unfoldedT[run - 1]->GetBinContent(i + delay_bin + 1);
        double nm = h_unfoldedT[run - 1]->GetBinContent(i - delay_bin + 1);
        // cout<<"ni = "<<ni<<", np = "<<np<<", nm = "<<nm<<endl;
        double k1, k2;

        if ((ni + nm) > 0.) {
          k1 = ni / (ni + nm);
        }
        if ((ni + nm) <= 0.) {
          k1 = 0.;
        }
        if ((ni + np) > 0.) {
          k2 = ni / (ni + np);
        }
        if ((ni + np) <= 0.) {
          k2 = 0.;
        }
        hTBinNum[i] = Tcount[i] * k1 + Tcount[i + delay_bin] * k2;
        // cout<<"hBinNum = "<<hBinNum[i]<<endl;

        if (ni > 0.) {
          double R[N_binT];
          for (int j = 0; j < N_binT; j++) {
            if (j == i)
              R[j] = k1;
            else if (j == (i + delay_bin))
              R[j] = k2;
            else
              R[j] = 0.;
          }
          for (int j = 0; j < N_binT; j++) {
            /*cout<<i<<", "<<j<<endl;
            cout<<"i+delay_bin = "<<(i+delay_bin)<<endl;
            cout<<"i-delay_bin = "<<(i-delay_bin)<<endl;
            cout<<"PDmatrix1[i][j] = "<<PDmatrix1[i][j]<<endl;
            cout<<"R[i] = "<<R[j]<<", R[j] = "<<R[j]<<endl;
            cout<<"hTBinNum[i] = "<<hTBinNum[i]<<endl;*/
            PDmatrix1[i][j] =
                R[j] + hTBinNum[i] / ni * PDmatrix0[i][j] -
                R[i] * R[i] * Tcount[i] / ni *
                    (PDmatrix0[i - delay_bin][j] + PDmatrix0[i][j]) -
                R[i + delay_bin] * R[i + delay_bin] * Tcount[i + delay_bin] /
                    ni * (PDmatrix0[i][j] + PDmatrix0[i + delay_bin][j]);
          }
        }
        if (ni <= 0.) {
          for (int j = 0; j < N_binT; j++)
            PDmatrix1[i][j] = 0.;
        }
      } // end of region 3

      if ((i + 1) > (N_binT - delay_bin)) {
        double ni = h_unfoldedT[run - 1]->GetBinContent(i + 1);
        double nm = h_unfoldedT[run - 1]->GetBinContent(i - delay_bin + 1);
        // cout<<"ni = "<<ni<<", nm = "<<nm<<endl;
        double k1;
        double k2 = 0.5;
        if ((ni + nm) > 0.) {
          k1 = ni / (ni + nm);
        }
        if ((ni + nm) <= 0.) {
          k1 = 0.;
        }
        hTBinNum[i] = Tcount[i] * k1 + k2 * Tcount[N_binT - 1];
        // cout<<"hBinNum = "<<hBinNum[i]<<endl;

        if (ni > 0.) {
          double R[N_binT]{};
          for (int j = 0; j < N_binT; j++) {
            if (j == i) {
              R[j] = k1;
              // cout<<"R"<<j<<" = "<<R[j]<<endl;
            } else if (j == (N_binT - 1))
              R[j] = k2;
            else
              R[j] = 0.;
            // cout<<"R"<<j<<" = "<<R[j]<<endl;
          }
          for (int j = 0; j < N_binT; j++) {
            PDmatrix1[i][j] =
                R[j] + R[i] * Tcount[i] / ni * PDmatrix0[i][j] -
                R[i] * R[i] * Tcount[i] / ni *
                    (PDmatrix0[i - delay_bin][j] + PDmatrix0[i][j]);
            if ((i == (N_binT - 1)) && (j == (N_binT - 1)))
              PDmatrix1[i][j] =
                  R[j] + k1 + R[i] * Tcount[i] / ni * PDmatrix0[i][j] -
                  R[i] * R[i] * Tcount[i] / ni *
                      (PDmatrix0[i - delay_bin][j] + PDmatrix0[i][j]);
          }
        }
        if (ni <= 0.) {
          for (int j = 0; j < N_binT; j++)
            PDmatrix1[i][j] = 0.;
        }
      } // end of region 4
    }   // end of calculate ni loop

    // set new ni loop
    for (int i = 0; i < (N_binT); i++) {
      // cout<<"i = "<<i<<endl;
      if (hTBinNum[i] < 0.) {
        hTBinNum[i] = 0.;
      }
      h_unfoldedT[run]->SetBinContent(i + 1, hTBinNum[i]);
    }

    // copy new PDmatrix to old one
    for (int i = 0; i < N_binT; i++)
      for (int j = 0; j < N_binT; j++) {
        PDmatrix0[i][j] = PDmatrix1[i][j];
      }

    // calculate chi2
    double Eprime[N_binT];
    for (int i = 0; i < N_binT; i++) {
      if ((i + 1 - TerminalBinT) < 0) {
        Eprime[i] = hTBinNum[i];
      }
      if ((i + 1 - TerminalBinT) >= 0 &&
          (i + 1 - TerminalBinT) < (delay_bin + 1)) {
        if (TerminalType == 0) {
          Eprime[i] = 0.5 * hTBinNum[i];
        }
        if (TerminalType == 1) {
          Eprime[i] = 0.5 * hTBinNum[TerminalBinT] + 0.5 * hTBinNum[i];
        }
      }
      if (((i + 1 - TerminalBinT) > delay_bin) &&
          ((i + 1) < (N_binT - delay_bin + 1))) {
        // cout<<"i = "<<i<<", TerminalBinT = "<<TerminalBinT<<", delay_bin =
        // "<<delay_bin<<endl; cout<<"index = "<<(i-delay_bin)<<endl;
        Eprime[i] = 0.5 * hTBinNum[i - delay_bin] + 0.5 * hTBinNum[i];
      }
      if ((i + 1) > (N_binT - delay_bin)) {
        Eprime[i] = 0.5 * hTBinNum[i - delay_bin] + 0.5 * hTBinNum[i];
      }
    }

    ChiSquare[run - 1] = 0.;
    Likelihood[run - 1] = 0.;
    for (int i = 0; i < N_binT; i++) {
      if (Tcount[i] > 0) {
        ChiSquare[run - 1] = ChiSquare[run - 1] + (Eprime[i] - Tcount[i]) *
                                                      (Eprime[i] - Tcount[i]) /
                                                      Tcount[i];
        // cout<<"i = "<<i<<", Chi2 = "<<ChiSquare[run-1]<<endl;
      }
      if ((Eprime[i] > 0.) && (Tcount[i] > 0.)) {
        Likelihood[run - 1] = Likelihood[run - 1] +
                              Tcount[i] / integral * Log(Eprime[i] / integral);
        // cout<<"i = "<<i<<", Tcount = "<<Tcount[i]<<", E = "<<Eprime[i]<<", LH
        // = "<<Likelihood[run-1]<<endl;
      }
    }
  } // end of iterative run

  for (int i = 0; i < N_binT; i++) {
    BinErrorsT[i] = 0.;
    for (int j = 0; j < N_binT; j++) {
      // if(i>274)cout<<"R"<<j<<" = "<<PDmatrix1[i][j]<<endl;
      BinErrorsT[i] =
          BinErrorsT[i] + PDmatrix1[i][j] * PDmatrix1[i][j] * Tcount[j];
    }
    BinErrorsT[i] = Sqrt(BinErrorsT[i]);
    // cout<<"BinErrors["<<i<<"] = "<<BinErrorsT[i]<<endl;
  }

  // free PDMatrix memory
  for (int i = 0; i < N_binT; i++) {
    delete[] PDmatrix0[i];
    delete[] PDmatrix1[i];
  }
  delete[] PDmatrix0;
  delete[] PDmatrix1;

} // end of function definition

//-------------------------------------------------------------------------------
// void RunUnfolderE();
//-------------------------------------------------------------------------------
void DoubleBunchUnfolder::RunUnfolderEprocess() {
  //----- calculate Emaxdelaybin -----
  if (TerminalType == 1) {

    TerminalE = emax;
    TerminalBinE = N_binE;

    Emaxdelay = Nenergy(Ntof(TerminalE, LOF) + delay, LOF);
    Emaxdelaybin =
        floor((Emaxdelay - emin) / BinWidthE); // bin index begin from 1
    Emaxdeltabin = TerminalBinE - Emaxdelaybin;
    cout << "tof_Emax = " << Ntof(TerminalE, LOF) << endl;
    cout << "Emaxdelay = " << Emaxdelay << endl;
    cout << "Emaxdelaybin = " << Emaxdelaybin << endl;
    cout << "Emaxdeltabin = " << Emaxdeltabin << endl;
    sleep(SleepTime);
  }

  if (TerminalType == 0) {
    Emaxdelay = Nenergy(Ntof(TerminalE, LOF) + delay, LOF);
    Emaxdelaybin =
        floor((Emaxdelay - emin) / BinWidthE); // bin index begin from 1
    Emaxdeltabin = TerminalBinE - Emaxdelaybin;
    cout << "tof_Emax = " << Ntof(TerminalE, LOF) << endl;
    cout << "Emaxdelay = " << Emaxdelay << endl;
    cout << "Emaxdelaybin = " << Emaxdelaybin << endl;
    cout << "Emaxdeltabin = " << Emaxdeltabin << endl;
    sleep(SleepTime);
  }

  Emindelay = Nenergy(Ntof(emin, LOF) - delay, LOF);
  Emindelaybin = ceil((Emindelay - emin) / BinWidthE); // bin index begin from 1
  Emindeltabin = Emindelaybin;
  cout << "tof_Emin = " << Ntof(emin, LOF) << endl;
  cout << "Emindelay = " << Emindelay << endl;
  cout << "Emindelaybin = " << Emindelaybin << endl;
  cout << "Emindeltabin = " << Emindeltabin << endl;
  sleep(SleepTime);

  //----- set basic parameters and variables -----

  h_unfoldedE = new TH1D *[RunNumE + 1];
  for (int i = 0; i < (RunNumE + 1); i++) {
    char hname[32];
    sprintf(hname, "h_unfoldedE[%d]", i);
    h_unfoldedE[i] = new TH1D(hname, hname, N_binE, emin, emax);
  }
  for (int i = 0; i < N_binE; i++) {
    Ecount[i] = h_doubleE->GetBinContent(i + 1);
  }

  double sum = h_unfoldedT[RunNumT]->Integral() * scaleFactor;
  h_unfoldedSampledE = new TH1D("h_SampledE", "h_SampledE", N_binE, emin, emax);
  for (int i = 0; i < round(sum); i++) {
    // cout<<Nenergy(h_unfoldedT[RunNumT]->GetRandom(), LOF)<<endl;
    auto tof = h_unfoldedT[RunNumT]->GetRandom();
    double En{-1e6};
    auto length = LOF;
    for (size_t i = 0; i < 3; i++) {
      En = Nenergy(tof, length);
      length = LOF_geo + g_endeltaL->Eval(En);
    }
    h_unfoldedSampledE->Fill(En, 1. / scaleFactor);
  }

  for (int i = 0; i < N_binE; i++) {
    h_unfoldedE[0]->SetBinContent(i + 1,
                                  h_unfoldedSampledE->GetBinContent(i + 1));
    // h_unfolded[0]->SetBinContent(i+1, sum/N_bin);
  }
  for (int i = 0; i < N_binE; i++) {
    hEBinNum[i] = 0.;
  }

  //-----------------------------------
  // iteration cylce
  //-----------------------------------
  for (int run = 1; run < (RunNumE + 1); run++) {
    // h_unfolded[run-1]->Smooth(3);
    for (int i = (N_binE - 1); i >= 0; i--) {
      // cout<<"i = "<<i<<endl;
      if ((i + 1) > TerminalBinE) {
        hEBinNum[i] = h_unfoldedE[run - 1]->GetBinContent(i + 1);
      }

      if ((i + 1) <= TerminalBinE && (i + 1) > Emaxdelaybin) {
        double Ei = emin + BinWidthE * i;
        double Eidelay = Nenergy(Ntof(Ei, LOF) + delay, LOF);
        int deltai =
            round((Ei - Eidelay) / BinWidthE); // bin index begin from 1
        double ni = h_unfoldedE[run - 1]->GetBinContent(i + 1);
        double nm = h_unfoldedE[run - 1]->GetBinContent(i - deltai + 1);
        double np = h_unfoldedE[run - 1]->GetBinContent(TerminalBinE);
        // cout<<"ni = "<<ni<<", np = "<<np<<endl;
        double k1 = 0.;
        double k2 = 0.;
        if (TerminalType == 0) {
          k2 = 1.0;
        }
        if ((TerminalType == 1) && ((ni + np) <= 0.)) {
          k2 = 0.;
        }
        if ((TerminalType == 1) && ((ni + np) > 0.)) {
          k2 = ni / (ni + np);
        }
        if ((ni + nm) > 0.) {
          k1 = ni / (ni + nm);
        }
        if ((ni + nm) <= 0.) {
          k1 = 0.;
        }
        // cout<<"i = "<<i<<", deltai = "<<deltai<<endl;
        hEBinNum[i] = Ecount[i - deltai] * k1 + k2 * Ecount[i];
      }

      if (((i + 1) > Emindelaybin) && ((i + 1) <= Emaxdelaybin)) {
        double Ei = emin + BinWidthE * i;
        double Eidelayp = Nenergy(Ntof(Ei, LOF) - delay, LOF);
        double Eidelaym = Nenergy(Ntof(Ei, LOF) + delay, LOF);
        int deltaip =
            round((Eidelayp - Ei) / BinWidthE); // bin index begin from 1
        int deltaim =
            round((Ei - Eidelaym) / BinWidthE); // bin index begin from 1
        double ni = h_unfoldedE[run - 1]->GetBinContent(i + 1);
        double np = h_unfoldedE[run - 1]->GetBinContent(i + deltaip + 1);
        double nm = h_unfoldedE[run - 1]->GetBinContent(i - deltaim + 1);
        // cout<<"ni = "<<ni<<", np = "<<np<<", nm = "<<nm<<endl;
        double k1 = 0.;
        double k2 = 0.;
        if ((ni + nm) > 0.) {
          k1 = ni / (ni + nm);
        }
        if ((ni + nm) <= 0.) {
          k1 = 0.;
        }
        if ((ni + np) > 0.) {
          k2 = ni / (ni + np);
        }
        if ((ni + np) <= 0.) {
          k2 = 0.;
        }
        hEBinNum[i] = Ecount[i] * k2 + Ecount[i - deltaim] * k1;
        // cout<<"hBinNum = "<<hBinNum[i]<<endl;
      }

      if ((i + 1) <= Emindelaybin) {
        double Ei = emin + BinWidthE * i;
        double Eidelay = Nenergy(Ntof(Ei, LOF) - delay, LOF);
        int deltai =
            round((Eidelay - Ei) / BinWidthE); // bin index begin from 1
        double ni = h_unfoldedE[run - 1]->GetBinContent(i + 1);
        double np = h_unfoldedE[run - 1]->GetBinContent(i + deltai + 1);
        double nm = h_unfoldedE[run - 1]->GetBinContent(1);
        // cout<<"ni = "<<ni<<", np = "<<np<<endl;
        double k1 = 0.;
        ;
        double k2 = 0.5;
        if ((ni + np) > 0.) {
          k1 = ni / (ni + np);
        }
        if ((ni + np) <= 0.) {
          k1 = 0.;
        }
        if ((ni + nm) > 0.) {
          k2 = ni / (ni + nm);
        }
        if ((ni + nm) <= 0.) {
          k2 = 0.;
        }
        hEBinNum[i] = Ecount[i] * k1 + k2 * Ecount[0];
        // cout<<"hBinNum = "<<hBinNum[i]<<endl;
      }
    } // end of calculating ni loop

    for (int i = 0; i < (N_binE); i++) {
      // cout<<"i = "<<i<<endl;
      if (hEBinNum[i] < 0.) {
        hEBinNum[i] = 0.;
      }
      h_unfoldedE[run]->SetBinContent(i + 1, hEBinNum[i]);
    } // end of set new ni for cycle
  }   // end of iterative run

} // end of function dfinition

//-------------------------------------------------------------------------------
// TH1D* GetUnfoldedHistogramT(int i)
//-------------------------------------------------------------------------------
TH1D *DoubleBunchUnfolder::GetUnfoldedHistogramT(int i) {

  if (i > RunNumT) {
    cout << "***error:The input number over the max RunNum!***" << endl;
    sleep(SleepTime);
    cout << "Return the unfolded histogram of the last iteration." << endl;
    sleep(SleepTime);
    cout << "===> Return h_unfoldedT[" << RunNumT << "]" << endl;
    sleep(SleepTime);
    return h_unfoldedT[RunNumT];
  } else {
    // cout<<"===> Return h_unfoldedT["<<i<<"]"<<endl;
    sleep(SleepTime);
    return h_unfoldedT[i];
  }
} // end of function definition

//-------------------------------------------------------------------------------
// TH1D* GetUnfoldedHistogramE(int i)
//-------------------------------------------------------------------------------
TH1D *DoubleBunchUnfolder::GetUnfoldedHistogramE(int index) {

  TH1D *hret = nullptr;
  if (index > RunNumE) {
    cout << "***error:The input number over the max RunNum!***" << endl;
    sleep(SleepTime);
    cout << "Return the unfolded histogram of the last iteration." << endl;
    sleep(SleepTime);
    cout << "===> Return h_unfoldedE[" << RunNumE << "]" << endl;
    sleep(SleepTime);
    if (Etype == "Linear") {
      hret = h_unfoldedE[RunNumE];
    }
    if (Etype == "Log") {
      double step = (emax - emin) / N_binE;
      double *bins = new double[N_binE + 1];
      for (int i = 0; i < (N_binE + 1); i++) {
        bins[i] = Power(10., emin) * Power(10., step * i);
      }
      hret = new TH1D("h_finalE", "h_finalE", N_binE, bins);
      for (int i = 0; i < N_binE; i++) {
        hret->SetBinContent(i + 1, h_unfoldedE[RunNumE]->GetBinContent(i + 1));
      }
    }
    return hret;
  } else {
    cout << "===> Return h_unfoldedE[" << index << "]" << endl;
    sleep(SleepTime);
    if (Etype == "Linear") {
      hret = h_unfoldedE[index];
    }
    if (Etype == "Log") {
      double step = (emax - emin) / N_binE;
      double *bins = new double[N_binE + 1];
      for (int i = 0; i < (N_binE + 1); i++) {
        bins[i] = Power(10., emin) * Power(10., step * i);
      }
      hret = new TH1D("h_finalE", "h_finalE", N_binE, bins);
      for (int i = 0; i < N_binE; i++) {
        hret->SetBinContent(i + 1, h_unfoldedE[index]->GetBinContent(i + 1));
      }
    }
    return hret;
  }
} // end of function definition

//-------------------------------------------------------------------------------
// 	TH1D* GetUnfoldedSampledE(int i)
//-------------------------------------------------------------------------------
TH1D *DoubleBunchUnfolder::GetUnfoldedSampledE() {
  if (Etype == "Linear") {
    return h_unfoldedSampledE;
  } else { // Changed from if (Etype == "Log")
    double step = (emax - emin) / N_binE;
    double *bins = new double[N_binE + 1];
    for (int i = 0; i < (N_binE + 1); i++) {
      bins[i] = Power(10., emin) * Power(10., step * i);
    }
    TH1D *hret = new TH1D("h_SampledLogE", "h_SampledLogE", N_binE, bins);
    for (int i = 0; i < N_binE; i++) {
      hret->SetBinContent(i + 1, h_unfoldedSampledE->GetBinContent(i + 1));
    }
    return hret;
  }
}

//-------------------------------------------------------------------------------
// void RunUnfolder();
//-------------------------------------------------------------------------------
void DoubleBunchUnfolder::RunUnfolder() {
  cout << "===> Unfolder-1D is running, please wait ......" << endl;
  RunUnfolderTprocess();
  if (Htype == "ESPEC")
    RunUnfolderEprocess();
  sleep(SleepTime);
}

//-------------------------------------------------------------------------------
// double* GetBinErrorsE();
//-------------------------------------------------------------------------------
double *DoubleBunchUnfolder::GetBinErrorsE() {
  /*
    double rtIndex0, rtIndex1; // E-histogram's bins coordinate relative to
    T-histogram int index0, index1; for(int i=0; i<(N_binE); i++){ rtIndex0 =
    (Ntof(emin+i*BinWidthE, LOF)-tmin)/BinWidthT; rtIndex1 =
    (Ntof(emin+(i+1)*BinWidthE, LOF)-tmin)/BinWidthT; index0 =
    (int)round(rtIndex0); index1 = (int)round(rtIndex1);
      //cout<<"i = "<<i<<", index0 = "<<index0<<", index1 = "<<index1<<endl;
      double error2 = BinErrorsT[index1]*BinErrorsT[index1];
      int binCount = 1;
      for(int ie=(index1+1); ie<(index0); ie++){
        error2 = error2 + BinErrorsT[ie]*BinErrorsT[ie];
        binCount++;
      }
      //error = error*(rtIndex0-rtIndex1)/binCount;
      double error = sqrt(error2);
      //cout<<"error = "<<error<<endl;
      BinErrorsE[i] = error;
      //cout<<"BinErrorsE["<<i<<"] = "<<BinErrorsE[i]<<endl;
    } // end of ibin loop
    return BinErrorsE;
  */
  double rtIndex0,
      rtIndex1; // E-histogram's bins coordinate relative to T-histogram
  int index0, index1;

  for (int i = 0; i < (N_binE); i++) {
    rtIndex0 = (Ntof(emin + i * BinWidthE, LOF) - tmin) / BinWidthT;
    rtIndex1 = (Ntof(emin + (i + 1) * BinWidthE, LOF) - tmin) / BinWidthT;
    index0 = (int)ceil(rtIndex0);
    index1 = (int)floor(rtIndex1);

    /*   if(i==0){        cout<<"i = "<<Ntof(emin+i*BinWidthE, LOF) <<", tmin =
    "<< tmin <<endl;

          cout<<"i = "<<i <<", index1 = "<< index1 << ", rtIndex1= " << rtIndex1
    <<", index0 = "<<index0 << ", rtIndex0= " << rtIndex0 << "\t"<<endl;
    } */
    if (index1 >= N_binT) {
      BinErrorsE[i] = 0;
      continue;
    }

    double error2 = BinErrorsT[index1] * BinErrorsT[index1];
    int binCount = 1;

    for (int ie = index1; ie < index0; ie++) {
      double w(1.);
      if (index0 - index1 < 2) {
        w = rtIndex0 - rtIndex1;
      } else if (ie == index1) {
        w = 1 - (rtIndex1 - index1);
      } else if (ie == index0 - 1) {
        w = rtIndex0 - ((float)index0 - 1.);
      }

      error2 = error2 + BinErrorsT[ie] * BinErrorsT[ie] * w;
      /*       cout<<"i = "<<i <<", index1 = "<< index1 << ", rtIndex1= " <<
       * rtIndex1 <<", index0 = "<<index0 << ", rtIndex0= " << rtIndex0 << "\t"
       * << BinErrorsT[ie]<<endl;
       */
      binCount++;
    }

    // cout << "\n";
    // error = error*(rtIndex0-rtIndex1)/binCount;
    double error = sqrt(error2);

    // cout<<"error = "<<error<<endl;
    BinErrorsE[i] = error;
    // cout<<"BinErrorsE["<<i<<"] = "<<BinErrorsE[i]<<endl;
  } // end of ibin loop
  return BinErrorsE;

} // end of function
