//-----------------------------------------------------------------------------
// this header file defines the class of DoubleBunchUnfolder.
// For LinearE and logE histogram.
// inludes private variables and public functions.
// author: yih_csns_ihep
// date: 26/10/2018
// contact: yih@ihep.ac.cn
//
// update: Add error calculation for E-histograms
// date: 2020-10-4
// author: yih@ihep.ac.cn
//-----------------------------------------------------------------------------

#ifndef DOUBLEBUNCHUNFOLDER_H
#define DOUBLEBUNCHUNFOLDER_H

#include "TGraph.h"
#include "TH1D.h"
#include "TMath.h"
#include "inttypes.h"
#include "math.h"
#include "stdio.h"
#include "stdlib.h"

#include <string>

class DoubleBunchUnfolder {

private:
  std::string Etype =
      "Linear"; // type of E-histogram, option: [Linear](default), [Log]
  std::string Htype = "TOF"; // type of histogram need to be unfold, option
                             // [TOF](default), [ESPEC]

  int N_binT;       // total bin number of the tof-histogram
  int N_binE;       // total bin number of the E-histogram
  double tmin;      // hte min x-axis value of the tof-histogram
  double tmax;      // the max x-axis value of the tof-histogram
  double emin;      // hte min x-axis value of the E-histogram
  double emax;      // the max x-axis value of the E-histogram
  double BinWidthT; // the width of bin of tof-histogram
  double BinWidthE; // the width of bin of E-histogram
  double delay =
      410.;       // the time delay between the two bunch. defualt value 410ns
  int delay_bin;  // the bin number corresponds to tof-delay, delay_bin =
                  // round(delay*N_binT/(tmax-tmin))
  double LOF;     // length of flight: m
  double LOF_geo; // length of flight: m
  int SleepTime = 0;

  double Emaxdelay;
  int Emaxdelaybin;
  int Emaxdeltabin;
  double Emindelay;
  int Emindelaybin;
  int Emindeltabin;

  double scaleFactor;

  int TerminalType =
      1; // set the type of xmax point of x-axis of energy-histogram.
         // [0]: energy xmax point is the maximum energy value that can be
         // measured; [1]; energy xmax point is not maximum energy value that
         // can be measured, defualt value 1;

  double TerminalT = 0.; // the min value of measureble tof
  int TerminalBinT = 0;  // the bin number corresponding to min TOF terminal
  double TerminalE;      // the max value of measureble energy
  int TerminalBinE;      // the bin number corresponding to mas terminal

  int RunNumT = 30; // number of interation unfolding cycle, default value 30
  int RunNumE = 1;  // number of interation unfolding cycle, default value 1
  double *Tcount;   // save the channel count of h_unfolded[0] histogram, the
                    // original double bunch histogram
  double *hTBinNum; // a double array to save count of every channel of unfolded
                    // histogram for each step of iteration
  double *Ecount;   // save the channel count of h_unfolded[0] histogram, the
                    // original double bunch histogram
  double *hEBinNum; // a double array to save count of every channel of unfolded
                    // histogram for each step of iteration

  TH1D *h_doubleT; // the double bunch folded tof-histogram
  TH1D *
      *h_unfoldedT; // the unfolded tof-histogram array for each iteration step
  TH1D *h_doubleE;  // the double bunch folded E-histogram
  TH1D **h_unfoldedE; // the unfolded E-histogram array for each iteration step
  TH1D *h_unfoldedSampledE;
  TGraph *g_endeltaL;

  void RunUnfolderTprocess();
  void RunUnfolderEprocess();

  double *BinErrorsT;
  double *BinErrorsE;
  double *ChiSquare;
  double *Likelihood;

public:
  DoubleBunchUnfolder();
  ~DoubleBunchUnfolder();
  void SetEtype(std::string type);
  void SetDelay(double d);
  void SetLOF(double lof);
  void SetRunNum(int nt, int ne);
  void SetScaleFactor(double _scaleFactor) { scaleFactor = _scaleFactor; };
  void SetTerminalType(int type);
  void SetTerminalT(double min); // the value of min tof-terminal.
  void SetHistogramType(std::string htype) { Htype = htype; }

  void ImportHistogramT(TH1D *ht); // import the double bunch tof-histogram
  void ImportHistogramE(TH1D *he); // import the double bunch E-histogram
  void ImportEnvsDelta(TGraph *gr) { g_endeltaL = gr; };
  void SetLOFgeo(double lofgeo) { LOF_geo = lofgeo; };

  void RunUnfolder();

  TH1D *GetUnfoldedHistogramT(
      int i); // get the unfolded tof-histogram of ith sep iteration
  TH1D *GetUnfoldedHistogramE(
      int i); // get the unfolded E-histogram of ith step iteration
  TH1D *GetUnfoldedSampledE(); // get the unfolded-sampled E-histogram

  double Ntof(double energy, double length);
  double Nenergy(double tof, double length);

  double *GetBinErrorsT() { return BinErrorsT; }
  double *GetBinErrorsE();
  double GetTmin() { return tmin; }
  double GetTmax() { return tmax; }
  double GetEmin() { return emin; }
  double GetEmax() { return emax; }
  int GetNbinT() { return N_binT; }
  int GetNbinE() { return N_binE; }
  double *GetChiSquare() { return ChiSquare; }
  double *GetLikelihood() { return Likelihood; }

}; // end of class define.

#endif
