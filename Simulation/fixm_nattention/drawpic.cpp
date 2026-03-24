#include "TROOT.h"
#include <ROOT/RDataFrame.hxx>
#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TH2D.h"
#include "THStack.h"
#include "TLegend.h"
#include "TCanvas.h"
#include "mystyle.h"
#include "TGraphErrors.h"
#include "pubfunc.h"
#include "TF1.h"
#include <set>
using namespace std;

void drawpic(){
    gROOT->ForceStyle();
    mystyle();
    auto c = new TCanvas;
    gPad->SetLogx();
    auto froot_out = new TFile("fout.root");
    TH1D* hEnin{};
    froot_out->GetObject("hEnin", hEnin);
    auto legd = new TLegend();
    std::vector<TH1D*> v_h{};
    for (size_t i = 0; i < 8; i++)
    {
        string hname = Form("hEn%d", i);
        string valname = Form("htrans%d", i+1);
        TH1D* h{};
        froot_out->GetObject(hname.data(), h);
        auto htrans = (TH1D*)h->Clone(valname.data());
        htrans->Divide(hEnin);
        htrans->SetLineColor(color[i]);
        legd->AddEntry(htrans, Form("ChannelID = %d", i+1), "lf");
        htrans->SetYTitle("Attenuation");
        htrans->Draw("same");
        v_h.emplace_back(htrans);
    }
    legd->Draw();

    auto fluxattenuation = new TFile("fluxattenuation.root", "recreate");
    for (const auto h : v_h)
    {
        h->Write();
    }
    fluxattenuation->Close();
    
}