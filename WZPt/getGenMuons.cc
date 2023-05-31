#include "TFile.h"
#include "tdrstyle.C"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"

/* 

To RUN:

   root -l -q -b 'getGenMuons.cc("WJets")'
   root -l -q -b 'getGenMuons.cc("DY50")'
   root -l -q -b 'getGenMuons.cc("DY10_50")'

*/

float minPt = 10; 
float minEta = -0.5;
float maxEta = 0.5;
//float xs_WJets = 53940*1000; // From Run 2 LO xs, fb
float xs_WJets = 67350*1000; // From Run 2 NLO xs, fb
float xs_DY50 = 5558*1000; // Run 3 xs, fb
float xs_DY10_50 = 15910*1000; // From Run 2 xs, fb

// WJetsToLNu_TuneCP5_13p6TeV-madgraphMLM-pythia8/Run3Winter22NanoAOD-122X_mcRun3_2021_realistic_v9-v1/NANOAODSIM
TString fn_WJets_nanoAOD = "./data/run3/1f48109d-c7d9-416f-8f6b-71906ce61d45.root";
// DYJetsToLL_M-50_TuneCP5_13p6TeV-madgraphMLM-pythia8/Run3Winter22NanoAOD-122X_mcRun3_2021_realistic_v9_ext2-v1/NANOAODSIM
TString fn_DY50_nanoAOD = "./data/run3/fb32c113-0866-4653-b462-a7c99263ef13.root";
// DYJetsToLL_M-10to50_TuneCP5_13p6TeV-madgraphMLM-pythia8/Run3Winter22NanoAOD-122X_mcRun3_2021_realistic_v9-v1/NANOAODSIM
TString fn_DY10_50_nanoAOD = "./data/run3/2518689b-3bc6-482c-a47d-2bd1d1db451e.root";
// Run 2 WJets pT
TString fn_muon_pt_WJets_run2 = "../muons/data/run2/WJets_CutPtSpect_v2.root";
// Run 2 DY pT
TString fn_muon_pt_DY_run2 = "./muons/data/run2/DY_CutPtSpect_v2.root";

std::chrono::time_point<std::chrono::system_clock> t_old;
std::vector<double> deq;

// copied from "../MCPTree/MCPTree.h"
void progress( int curr, int tot, int period, unsigned int smoothing) {
    if(curr%period == 0) {
        auto now = std::chrono::system_clock::now();
        double dt = ((std::chrono::duration<double>)(now - t_old)).count();
        t_old = now;
        // if (deq.size() >= smoothing) deq.pop_front();                                                                                                                                                            
        if (deq.size() >= smoothing) deq.erase(deq.begin());
        deq.push_back(dt);
        double avgdt = std::accumulate(deq.begin(),deq.end(),0.)/deq.size();
        float prate = (float)period/avgdt;
        float peta = (tot-curr)/prate;
        if (isatty(1)) {
            float pct = (float)curr/(tot*0.01);
            if( ( tot - curr ) <= period ) pct = 100.0;
            printf("\015\033[32m ---> \033[1m\033[31m%4.1f%% \033[34m [%.3f kHz, ETA: %.0f s] \033[0m\033[32m  <---\033[0m\015 ", pct, prate/1000.0, peta);
            if( ( tot - curr ) > period ) fflush(stdout);
            else cout << endl;
        }
    }
}

bool Has_Boson_Ancestor(int ipart, TString processName,
			const TTreeReaderArray<int>& genpart_pdgId,
			const TTreeReaderArray<int>& genpart_motherIdx) {

    bool has_boson_ancestor = false;
    int parent_idx = genpart_motherIdx[ipart];
    int motherID = genpart_pdgId[parent_idx];

    //cout << "###" << endl;
    while (parent_idx != -1) {
      //cout << "motherID: " << genpart_pdgId[parent_idx] << endl;
      if (processName == "WJets" && abs(motherID) == 24) {
	has_boson_ancestor = true;
	break;
      }
      if (processName == "DY50" && abs(motherID) == 23) { 
	// consider both Z and gamma*
	has_boson_ancestor = true;
	break;
      }
      if (processName == "DY10_50" && (abs(motherID) >= 1 && abs(motherID) <= 6)) { 
	// consider both Z and gamma*
	has_boson_ancestor = true;
	break;
      }

      //if ( abs(motherID) > 13 ) break;
      
      parent_idx = genpart_motherIdx[parent_idx];
      motherID = genpart_pdgId[parent_idx];
    }

    //cout << "has_boson_ancestor: " << has_boson_ancestor << ", motherID: " << motherID << endl;
    return has_boson_ancestor;
}

void getGenMuons(TString processName /* WJets or DY */) {

    // Open the CMS nanoAOD file and access the Events tree
    TString fn_nanoAOD = "";
    if (processName == "WJets") fn_nanoAOD = fn_WJets_nanoAOD;
    if (processName == "DY50") fn_nanoAOD = fn_DY50_nanoAOD;
    if (processName == "DY10_50") fn_nanoAOD = fn_DY10_50_nanoAOD;
    
    TFile *file = TFile::Open(fn_nanoAOD);
    TTreeReader reader("Events", file);
    Long64_t nEntries = reader.GetEntries(true);

    // Define the TTreeReaderValue pointers to the Muon_pt, Muon_eta, and Muon_phi branches
    TTreeReaderArray<float> genpart_pt(reader, "GenPart_pt");
    TTreeReaderArray<float> genpart_eta(reader, "GenPart_eta");
    TTreeReaderArray<float> genpart_phi(reader, "GenPart_phi");
    TTreeReaderArray<int> genpart_status(reader, "GenPart_status");
    TTreeReaderArray<int> genpart_pdgId(reader, "GenPart_pdgId");
    TTreeReaderArray<int> genpart_motherIdx(reader, "GenPart_genPartIdxMother");

    // Create histograms to plot the eta and pT distribution of the selected muon
    TH1F* h_eta = new TH1F("h_eta", "Muon Eta Distribution", 50, -3.0, 3.0);
    TH1F* h_phi = new TH1F("h_phi", "Muon Phi Distribution", 50, -4.0, 4.0);
    //TH1F* h_pt = new TH1F("h_pt", "Muon PT Distribution", 40, 0, 100);
    TH1F* h_pt = new TH1F("pt", "Muon PT Distribution", 100, 0, 100);

    // Loop over the events in the tree
    int counter = 0;
    while (reader.Next()) {

	progress(counter, nEntries, 1000, 300);

	// Loop over the generator level particles in the event
	for (int ipart = 0; ipart < genpart_pdgId.GetSize(); ++ipart){

	    // Select only the status 1 muons that decay from the W or Z boson
	    if ( !(abs(genpart_pdgId[ipart]) == 13 && genpart_status[ipart] == 1) ) continue;

	    bool has_boson_ancestor = Has_Boson_Ancestor(ipart, processName, genpart_pdgId, genpart_motherIdx);

	    float pt = genpart_pt[ipart];
	    float eta = genpart_eta[ipart];
	    float phi = genpart_phi[ipart];

	    // If the particle has a W/Z boson ancestor
	    if (has_boson_ancestor) {

		// Get the momentum of the selected muon
		TLorentzVector muon;
		muon.SetPtEtaPhiM(pt, eta, phi, 0.10566);

		// Fill the histograms with the eta and pT of the selected muon
		h_eta->Fill(muon.Eta());
		h_phi->Fill(muon.Phi());

		if (muon.Pt() > minPt && muon.Eta() > minEta && muon.Eta() < maxEta) {

		  h_pt->Fill(muon.Pt());
		  break; // avoid Z->mu mu double counting, e.g. two muons from same event end up in same eta range
		}
	    }

	}
	counter++;
    }

    // Get pT distribution used for Run 2
    TString fn_pt_run2 = "";
    if (processName == "WJets") fn_pt_run2 = fn_muon_pt_WJets_run2;
    if (processName.Contains("DY")) fn_pt_run2 = fn_muon_pt_DY_run2;

    TFile *file_run2 = TFile::Open(fn_pt_run2);
    TH1F* h_pt_run2 = (TH1F*)file_run2->Get("pt");

    setTDRStyle();
   
    // Make a TCanvas, split it in 3 (pT, eta, phi)
    TCanvas* c = new TCanvas("c","",800,600);
    TPad* pad1 = new TPad("pad1", "Pad 1", 0.0, 0.5, 1.0, 1.0);
    TPad* pad2 = new TPad("pad2", "Pad 2", 0.0, 0.0, 0.5, 0.5);
    TPad* pad3 = new TPad("pad3", "Pad 3", 0.5, 0.0, 1.0, 0.5);
    pad1->Draw();
    pad2->Draw();
    pad3->Draw();

    pad1->cd();
    h_pt->Draw("HIST");
    h_pt->GetXaxis()->SetTitle("pT (GeV)");
    //h_pt->Scale(h_pt_run2->Integral()/h_pt->Integral());
    h_pt_run2->Draw("HIST same");
    h_pt_run2->SetLineColor(2);

    //cout << "Efficiency: " << h_pt->Integral()/nEntries*100 << endl;
    //cout << "Integral: " << h_pt->Integral() << endl;
    //cout << "Scale factor: " << xs_WJets/nEntries << endl;
    
    //if (processName == "WJets") h_pt->Scale(xs_WJets/nEntries, "width");
    //if (processName == "DY50") h_pt->Scale(xs_DY50/nEntries*2, "width");
    //if (processName == "DY10_50") h_pt->Scale(xs_DY10_50/nEntries*2, "width");
    if (processName == "WJets") h_pt->Scale(xs_WJets/nEntries);
    if (processName == "DY50") h_pt->Scale(xs_DY50/nEntries);
    if (processName == "DY10_50") h_pt->Scale(xs_DY10_50/nEntries);

    h_pt->GetYaxis()->SetRangeUser(0, h_pt->GetMaximum()*1.5);

    cout << "Process: " << processName << endl;
    cout << "Run 2 distribution XS: " << h_pt_run2->Integral() << ", bin width: " << h_pt_run2->GetBinWidth(1) << ", total: " << h_pt_run2->Integral()*h_pt_run2->GetBinWidth(1) << endl;
    cout << "Run 3 distribution XS: " << h_pt->Integral() << ", bin width: " << h_pt->GetBinWidth(1) << ", total: " << h_pt->Integral()*h_pt->GetBinWidth(1) << endl;

    // Create a TLegend and add entries to it
    TLegend *legend = new TLegend(0.7, 0.7, 0.9, 0.9); // x1,y1,x2,y2 (in normalized coordinates)
    legend->AddEntry(h_pt, "Run 3 sample", "l");
    legend->AddEntry(h_pt_run2, "Run 2 sample", "l");
    legend->Draw("same");

    pad2->cd();
    h_eta->Draw();
    h_eta->GetXaxis()->SetTitle("#eta");

    pad3->cd();
    h_phi->Draw();
    h_phi->GetXaxis()->SetTitle("#phi");

    TString fn_plot = "./plots/GenMuon_pt_eta_phi_" + processName + ".pdf";
    c->SaveAs(fn_plot);

    TString fn_out = "";
    if (processName == "WJets") fn_out = "./plots/WJets_CutPtSpect_run3.root";
    if (processName == "DY50") fn_out = "./plots/DY50_CutPtSpect_run3.root";
    if (processName == "DY10_50") fn_out = "./plots/DY10_50_CutPtSpect_run3.root";

    TFile f_out(fn_out, "recreate");
    h_pt->Write();
    f_out.Close();
}
