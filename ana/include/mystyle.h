#ifndef _MYSTYLE_H_
#define _MYSTYLE_H_

#include <TROOT.h>
#include <TStyle.h>
/* constexpr Double_t c_x{1930.};
constexpr Double_t c_y{2300.}; */
inline void mystyle() {

  gStyle->SetOptStat(0);
  gStyle->SetOptTitle(0);
  /*     gStyle->SetPadGridX(kTRUE);
      gStyle->SetPadGridY(kTRUE); */
  gStyle->SetPadTickX(1);
  gStyle->SetPadTickY(1);

  gStyle->SetLineWidth(2);
  gStyle->SetHistLineWidth(2);
  gStyle->SetFrameLineWidth(0);

  gStyle->SetLabelSize(0.05, "XY");
  gStyle->SetLabelOffset(0.003, "XY");
  gStyle->SetMarkerStyle(20);
  gStyle->SetMarkerSize(0.5);
  gStyle->SetLabelFont(22, "xy");
  gStyle->SetTitleFont(22, "xy");
  gStyle->SetTitleSize(0.05, "xy");
  gStyle->SetTitleOffset(1.09, "x");
  gStyle->SetTitleOffset(1.05, "y");
  gStyle->SetLegendFillColor(0);
  gStyle->SetLegendTextSize(0.04);
  gStyle->SetLegendBorderSize(0);
  gStyle->SetPadLeftMargin(0.15);
  gStyle->SetPadRightMargin(0.05);
  gStyle->SetPadBottomMargin(0.15);
  gStyle->SetPadTopMargin(0.06);
  gStyle->SetLegendFont(22);
  gStyle->SetTextSize(0.065);
}

inline void my2dstyle() {
  gStyle->SetOptStat(0);
  gStyle->SetOptTitle(0);
  gStyle->SetPalette(1);
  gStyle->SetOptLogz();
  /*     gStyle->SetOptStat(0);
      gStyle->SetOptTitle(0); */
  gStyle->SetPadTickX(1);
  gStyle->SetPadTickY(1);
  gStyle->SetPadGridX(kFALSE);
  gStyle->SetPadGridY(kFALSE);
  /*     gStyle->SetLineWidth(2); */
  gStyle->SetHistLineWidth(2);
  gStyle->SetFrameLineWidth(1);
  gStyle->SetLabelSize(0.055, "XYZ");
  gStyle->SetLabelOffset(0.005, "XYz");
  gStyle->SetLabelFont(22, "xyz");
  gStyle->SetTitleFont(22, "xyz");
  gStyle->SetTitleSize(0.055, "xyz");
  gStyle->SetTitleOffset(1.08, "x");
  gStyle->SetTitleOffset(1.15, "y");
  gStyle->SetTitleOffset(1.05, "z");
  gStyle->SetLegendFillColor(0);
  gStyle->SetLegendTextSize(0.06);
  gStyle->SetLegendBorderSize(0);
  gStyle->SetLegendFont(22);
  gStyle->SetPadLeftMargin(0.15);
  gStyle->SetPadRightMargin(0.15);
  gStyle->SetPadBottomMargin(0.15);
  gStyle->SetPadTopMargin(0.05);
  gStyle->SetFrameFillStyle(0);
  gStyle->SetTextSize(0.055);
}

inline void mystyle2() {
  gStyle->SetOptStat(0);
  gStyle->SetOptTitle(0);
  //    gStyle->SetPadGridX(kTRUE);
  //    gStyle->SetPadGridY(kTRUE);
  gStyle->SetPadTickX(1);
  gStyle->SetPadTickY(1);

  gStyle->SetLineWidth(2);
  gStyle->SetHistLineWidth(2);
  gStyle->SetFrameLineWidth(1);

  gStyle->SetLabelSize(0.05, "XY");
  gStyle->SetLabelFont(22, "xy");
  gStyle->SetTitleFont(22, "xy");
  gStyle->SetTitleSize(0.05, "xy");
  gStyle->SetTitleOffset(1.1, "x");
  gStyle->SetTitleOffset(1.5, "y");
  gStyle->SetLegendFillColor(10);
  gStyle->SetLegendTextSize(0.07);
  gStyle->SetLegendBorderSize(0);
  gStyle->SetPadLeftMargin(0.15);
  gStyle->SetPadRightMargin(0.05);
  gStyle->SetPadBottomMargin(0.15);
  gStyle->SetPadTopMargin(0.05);
  gStyle->SetLegendFont(22);
  gStyle->SetTextSize(0.07);
}

#endif