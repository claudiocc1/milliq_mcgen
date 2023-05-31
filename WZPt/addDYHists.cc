#include "TFile.h"
#include "tdrstyle.C"


void addDYHists() {

  setTDRStyle();
  // get input hists from DY50 and DY10_50
  TString fn_DY50 = "./plots/DY50_CutPtSpect_run3.root";
  TString fn_DY10_50 = "./plots/DY10_50_CutPtSpect_run3.root";

  TFile *file_DY50 = TFile::Open(fn_DY50);
  TFile *file_DY10_50 = TFile::Open(fn_DY10_50);

  TH1F* h_pt_DY50 = (TH1F*)file_DY50->Get("pt");
  TH1F* h_pt_DY10_50 = (TH1F*)file_DY10_50->Get("pt");

  // add them, make a new pT hist
  TH1F* h_pt_DY = (TH1F*)h_pt_DY50->Clone("h_pt_DY");
  h_pt_DY->Add(h_pt_DY10_50);

  // compare with run 2 distribution
  TFile *file_run2 = TFile::Open("./data/run2/DY_CutPtSpect_v2.root");
  TH1F* h_pt_run2 = (TH1F*)file_run2->Get("pt");
  
  TCanvas* c = new TCanvas("c","",800,600);
  h_pt_DY->Draw("HIST");
  h_pt_DY->GetXaxis()->SetTitle("pT (GeV)");
  h_pt_run2->Draw("HIST same");
  h_pt_run2->SetLineColor(2);

  TLegend *legend = new TLegend(0.7, 0.7, 0.9, 0.9); // x1,y1,x2,y2 (in normalized coordinates)
  legend->AddEntry(h_pt_DY, "Run 3 sample", "l");
  legend->AddEntry(h_pt_run2, "Run 2 sample", "l");
  legend->Draw("same");

  TString fn_plot = "./plots/GenMuon_pt_DY_compare.pdf";
  c->SaveAs(fn_plot);

  // save to a new root file
  TString fn_out = "./plots/DY_CutPtSpect_run3.root";

  TFile f_out(fn_out, "recreate");
  h_pt_DY->SetName("pt");
  h_pt_DY->Write();
  f_out.Close();

}
