#include <cmath>
#include <utility>
#include <iostream>
#include <string>
#include <cstdlib>

#include "TFile.h"
#include "TRandom.h"
#include "TH1D.h"

#include "../decayMCP/MCPTree/MCPTree.h"

/********************* 
  PRODUCTION MODES
  1. Heavy Flavor
  2. W
  3. Drell-Yan
  4. Decays in flight
*********************/

const float MINBIAS_XSEC = (78.42e-3) * 1e12; //from "cross_section" : https://cms-pdmv.cern.ch/mcm/public/restapi/requests/get/PPD-2022CommissionGS-00001

float MUON_ETAMIN = 0.11 - 0.20;
float MUON_ETAMAX = 0.11 + 0.20;
float MUON_PHIMIN = -0.15;
float MUON_PHIMAX = 0.45;
float MUON_PTMIN = 13.0;

TH1D *h_pt=0, *h_up=0, *h_dn=0;

string GetName(int mode){
    if(mode == 1) return "QCD (bc)";
    else if(mode == 2) return "W -> lv";
    else if(mode == 3) return "Drell-Yan";
    else if(mode == 4) return "QCD(non-bc)";
    else return "UNKNOWN";
}

int init(int production_mode, MCPTree &outtree){
    TFile *f;

    bool run3 = true;
    
    if(production_mode == 1){
        // f = new TFile("data/QCD_CutPtSpect_v2.root");
        // h_pt = (TH1D*)f->Get("pt");
        if (run3) {
	  f = new TFile("data/run3/bc-incl-to-mu.root");
	} else {
	  f = new TFile("data/run2/bc-incl-to-mu.root");
	}
        h_pt = (TH1D*)f->Get("central");
        h_up = (TH1D*)f->Get("up");
        h_dn = (TH1D*)f->Get("down");

        for(int i=1; i<=h_pt->GetNbinsX(); i++){
            if(h_pt->GetXaxis()->GetBinUpEdge(i) <= MUON_PTMIN){
                h_pt->SetBinContent(i, 0);
                h_up->SetBinContent(i, 0);
                h_dn->SetBinContent(i, 0);
            }else
                break;
        }
        outtree.xsec = h_pt->Integral("width") * 2;  // times 2 since it is single-b xsec, but produced as bbbar
        // This includes eta [-1, 1];
        outtree.xsec *= (MUON_ETAMAX - MUON_ETAMIN) / (2*1.0);
        outtree.xsec *= (MUON_PHIMAX - MUON_PHIMIN) / (2*3.14159265);

    }else if(production_mode == 2){
        if (run3) {
	  f = new TFile("data/run3/WJets_CutPtSpect_run3.root");
	} else {
	  f = new TFile("data/run2/WJets_CutPtSpect_v2.root");
	}
        h_pt = (TH1D*)f->Get("pt");
        outtree.xsec = h_pt->Integral() / 1000;
        // This includes eta [-0.5, 0.5];
        outtree.xsec *= (MUON_ETAMAX - MUON_ETAMIN) / (2*0.5);
        outtree.xsec *= (MUON_PHIMAX - MUON_PHIMIN) / (2*3.14159265);
    }else if(production_mode == 3){
        if (run3) {
	  f = new TFile("data/run3/DY_CutPtSpect_run3.root");
	} else {
	  f = new TFile("data/run2/DY_CutPtSpect_v2.root");
	}
        h_pt = (TH1D*)f->Get("pt");
        outtree.xsec = h_pt->Integral() / 1000;
        // This includes eta [-0.5, 0.5];
        outtree.xsec *= (MUON_ETAMAX - MUON_ETAMIN) / (2*0.5);
        outtree.xsec *= (MUON_PHIMAX - MUON_PHIMIN) / (2*3.14159265);
    }else if(production_mode == 4){
        if (run3) {
	  f = new TFile("../mesonPt/pt_dists_run3.root");
	} else {
	  f = new TFile("../mesonPt/pt_dists.root");
	}
        h_pt = (TH1D*)f->Get("h_mu_nonbc");
        // zero out bins below pT threshold
        for(int i=1; i<=h_pt->GetNbinsX(); i++){
            if(h_pt->GetXaxis()->GetBinUpEdge(i) <= MUON_PTMIN)
                h_pt->SetBinContent(i, 0);
            else
                break;
        }
        // bins in this histogram are "muons per minbias event per 50 MeV bin"
        // so the integral is "muons per minbias event"
        // scale by the minbias xsec to get the xsec for producing muons
        outtree.xsec = h_pt->Integral() * MINBIAS_XSEC;
        // xsecs stored for eta [-2,2], so multiply by 0.5 to get to [-1,1].
        // Correction factor of 1.13 to account for non-uniformity in eta (from MC)
        outtree.xsec *= 0.5 * 1.13;
        outtree.xsec *= (MUON_ETAMAX - MUON_ETAMIN) / (2*1.0);
        outtree.xsec *= (MUON_PHIMAX - MUON_PHIMIN) / (2*3.14159265);
    }else{
        return -1;
    }

    h_pt->SetDirectory(0);
    if(h_up) h_up->SetDirectory(0);
    if(h_dn) h_dn->SetDirectory(0);

    for(int i=1; i<=h_pt->GetNbinsX(); i++)
        if(h_pt->GetBinContent(i) < 0)
            h_pt->SetBinContent(i, 0);

    f->Close();
    delete f;
    return 0;
}

int main(int argc, char **argv){

    char opt;
    bool help = false;
    int production_mode = -1;
    float m_muon = 0.105;
    int n_events = 1000, evt_offset = 0;
    uint n_events_total = 0;
    string output_name = "";
    while((opt = getopt(argc, argv, ":hd:o:m:n:N:e:p")) != -1) {
        switch(opt){
        case 'h':
            help = true;
            break;
        case 'd':
            production_mode = atoi(optarg);
            break;
        case 'o':
            output_name = string(optarg);
            break;
        case 'n':
            n_events = atoi(optarg);
            break;
        case 'N':
            n_events_total = atoi(optarg);
            break;
        case 'e':
            evt_offset = atoi(optarg);
            break;
        case '?':
            std::cout << "\nWARNING: unrecognized option " << argv[optind-1] << std::endl;
            break;
        case ':':
            std::cout << "\nERROR: option " << argv[optind-1] << " requires a value\n";
            help = true;
            break;
        }
    }

    if(n_events_total == 0)
        n_events_total = n_events;

    if(help || production_mode < 0 || output_name=="" || n_events <= 0 || n_events_total < n_events || evt_offset < 0){
        std::cout << "\nusage:\n";
        std::cout << "    " << argv[0] << " -d production_mode -o outfile [-n n_events=1000] [-N n_events_total=n_events] [-e evtnum_offset=0]\n\n";
        std::cout << "--- PRODUCTION MODES ---" << std::endl;
        std::cout << "--- 1. QCD (b- or c-hadron)" << std::endl;
        std::cout << "--- 2. W -> lv" << std::endl;
        std::cout << "--- 3. Drell-Yan" << std::endl;
        std::cout << "--- 4. QCD (non-b- or c-hadron, usually pi+-, K+-, K0L, or tau)" << std::endl;
        std::cout << "\n";
        return 1;
    }    
    
    MCPTree outtree;

    gRandom->SetSeed(0);
    
    TFile *fout = new TFile(output_name.c_str(), "RECREATE");
    outtree.Init();

    outtree.n_events_total = n_events_total;
    outtree.decay_mode = production_mode;
    outtree.BR_q1 = 1.0;
    outtree.weight = 1.0;
    outtree.weight_up = 1.0;
    outtree.weight_dn = 1.0;
    outtree.parent_pdgId = 0;
    outtree.mCP_etamin = MUON_ETAMIN;
    outtree.mCP_etamax = MUON_ETAMAX;
    outtree.mCP_phimin = MUON_PHIMIN;
    outtree.mCP_phimax = MUON_PHIMAX;

    init(production_mode, outtree);

    std::cout << "\n";
    std::cout << "**********************************************" << std::endl;
    std::cout << "*   Muon Generator   *" << std::endl;
    std::cout << "**********************************************" << std::endl;
    std::cout << "  Doing decay mode: " << GetName(production_mode) << std::endl;
    std::cout << "         xsec (pb): " << outtree.xsec << std::endl;
    std::cout << "----------------------------------------------" << std::endl;
    std::cout << "       muon eta min: " << MUON_ETAMIN << std::endl;
    std::cout << "       muon eta max: " << MUON_ETAMAX << std::endl;
    std::cout << " muon phi min (q>0): " << MUON_PHIMIN << std::endl;
    std::cout << " muon phi max (q>0): " << MUON_PHIMAX << std::endl;
    std::cout << "        muon pt min: " << MUON_PTMIN << std::endl;
    std::cout << "----------------------------------------------" << std::endl;
    

    outtree.tree()->SetBranchStatus("filter_eff", 0); // turn off and fill later once we're done
    outtree.p4_p->SetM(0.105);
    unsigned long n_attempts = 0;
    for(uint i=0; i<n_events; i++){
        outtree.progress(i, n_events, 200);
        outtree.event = i + evt_offset;
        bool is_good = false;
        float eta = gRandom->Uniform(MUON_ETAMIN, MUON_ETAMAX);
        float phi = gRandom->Uniform(MUON_PHIMIN, MUON_PHIMAX);
        float pt = 0;
        for( ; pt < MUON_PTMIN; n_attempts++)
            pt = h_pt->GetRandom();
        outtree.p4_p->SetPt(pt);
        outtree.p4_p->SetEta(eta);
        outtree.p4_p->SetPhi(phi);        
        if(h_up) outtree.weight_up = h_up->GetBinContent(h_pt->FindBin(pt)) / h_pt->GetBinContent(h_pt->FindBin(pt));
        if(h_dn) outtree.weight_dn = h_dn->GetBinContent(h_pt->FindBin(pt)) / h_pt->GetBinContent(h_pt->FindBin(pt));
        outtree.Fill();
    }

    // fill the filter_eff branch
    outtree.filter_eff = (float)n_events / n_attempts;
    std::cout << "    Total attempted events: " << n_attempts << std::endl;
    std::cout << "Computed filter efficiency: " << outtree.filter_eff << std::endl;
    outtree.tree()->SetBranchStatus("filter_eff", 1);
    TBranch *beff = outtree.tree()->GetBranch("filter_eff");
    for(uint i=0; i<n_events; i++){
        outtree.tree()->GetEntry(i);
        beff->Fill();
    }
    std::cout << "\n";

    outtree.Write(fout);

    fout->Close();
    return 0;
}
