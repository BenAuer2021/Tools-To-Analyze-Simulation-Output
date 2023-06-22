#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TH2D.h>
#include <TH1D.h>
#include <iostream>
#include <TMath.h>
#include <fstream>
#include <TImage.h>
#include <TLegend.h>
#include <TCanvas.h>
#include <TPDF.h>
#include <sstream>

//####################################### Code To generate scatter, primary, and total projection in interfile format (.hdr .sin) #####################
//##############################################+ energy spectra generated from a root file generated from GATE #######################
//## Launch root -> type root in a terminal prompt
//## Compilation: .L Projection_macro.cpp++
//## Usage: Projection_macros("ROOT_FILE_NAME")
//## Example: Projection_macros("Brightview_Main_LEHR_Tc99m_BoneScan_pos1_100_files")
//## Developed in Jun 2023 by Benjamin Auer, PhD from the Brigham and Women's Hospital and Harvard Medical School, Boston, MA, USA / Email: bauer@bwh.harvard.edu
//########################################################################################################################

UInt_t nProjection_per_head = 32; //60
Float_t angleStep = 5.6250; // 3 degree
Float_t anglePitch = 180; //H-Mode
Float_t crystalSize_transaxial = 538.2; //mm
Float_t crystalSize_axial = 397.8; //mm
UInt_t nPix_transaxial = 230; //pixel size 0.234 cm
UInt_t nPix_axial = 170;
UInt_t nHead = 2;
Float_t Shift_Oz = 0; //in mm Head Shift for Bone scan
// Energy Window for Tc-99m 20%, 10%, 10%
Float_t Photopeak_E_min=0.126, Photopeak_E_max=0.154;
Float_t Scatter_Upper_E_min=0.112, Scatter_Upper_E_max=0.126;
Float_t Scatter_Lower_E_min=0.154, Scatter_Lower_E_max=0.168;

void Draw_TH2(vector<TH2D*> Histo, TString name, TString XName, TString YName, TString Option, bool norm){//normalisé ou non
    
    TCanvas *c1 = new TCanvas("c1","histograms with bars",700,800);
    
    int const NbreHisto=Histo.size();
    if (norm) {
        for (int i=0; i<NbreHisto; i++) {
            double Normalisation=Histo[i]->Integral();
            Histo[i]->Scale(1.0/Normalisation);
        }
    }
    int const NumberOfColor=256;
    for (int i=0; i<NbreHisto; i++) {
        Histo[i]->SetStats(0);
        Histo[i]->GetYaxis()->SetTitle(YName);
        Histo[i]->GetYaxis()->SetLabelSize(0.02);
        Histo[i]->GetYaxis()->SetLabelColor(13);
        
        Histo[i]->GetXaxis()->SetTitle(XName);
        Histo[i]->GetXaxis()->SetLabelSize(0.02);
        Histo[i]->GetXaxis()->SetLabelColor(13);
        
        Histo[i]->GetZaxis()->SetLabelSize(0.02);
        Histo[i]->GetZaxis()->SetLabelColor(13);
        Histo[i]->SetContour(NumberOfColor);
        Histo[i]->Draw(Option);
        
        if (i==0) {
            c1->Print(name+".pdf(");
        }
        if ((i!=0)&&(i!=NbreHisto-1)) {
            c1->Print(name+".pdf");
        }
        if (i==NbreHisto-1) {
            c1->Print(name+".pdf)");
        }
    }
}


void writeInterfileImage(unsigned int*** projection, unsigned int nPix_transaxial, unsigned int nPix_axial, unsigned int nProjection_per_head, unsigned int nHead, const std::string& outputFilename) {
    // Open the output image file
    std::string foutputFilename = outputFilename + "x" + std::to_string(nPix_axial) + "x" + std::to_string(nPix_transaxial) + "x" + std::to_string(nProjection_per_head * nHead) + "_PixSize_" + std::to_string(crystalSize_axial / nPix_axial) + "x" + std::to_string(crystalSize_transaxial / nPix_transaxial) + "mm.img";

    std::ofstream imageFile(foutputFilename, std::ios::binary);

    // Write the image dimensions as ImageJ header
    unsigned int dimensions[3] = { nPix_transaxial, nPix_axial, nProjection_per_head * nHead };
    //imageFile.write(reinterpret_cast<const char*>(dimensions), sizeof(dimensions)); // Dimension written in file (offset should be set to 12 bit when opening the file in Amide/ImageJ

    // Write the projection data to the image file and calculate the total number of elements
    unsigned long long totalElements = 0;
    for (unsigned int projectionID = 0; projectionID < nProjection_per_head * nHead; ++projectionID) {
        for (unsigned int transaxialID = 0; transaxialID < nPix_transaxial; ++transaxialID) {
            for (unsigned int axialID = 0; axialID < nPix_axial; ++axialID) {
                unsigned int value = projection[projectionID][transaxialID][axialID];
                imageFile.write(reinterpret_cast<const char*>(&value), sizeof(value));
                totalElements += value;
            }
        }
    }

    // Close the image file
    imageFile.close();

    // Print the total number of elements
    std::cout << "In: " << outputFilename << "--> Total number of counts over projections: " << totalElements << std::endl;
}


//**** In this version, the function get_Projection_ID takes four additional parameters:
// total_nb_projection_per_head, HeadID, rotangle, and angleStep.
// These parameters are used to calculate the ProjectionID based on the provided formula.
UInt_t get_Projection_ID(Int_t nProjection_per_head, UInt_t HeadID, Float_t rotangle, Float_t angleStep) {
    // Compute the angle ID based on the rotation angle and step size
    Int_t angID = TMath::Nint(rotangle / angleStep);
    
    // Compute the projection ID based on the HeadID and number of projections per head
    UInt_t ProjectionID = HeadID * nProjection_per_head + angID;
    
    return ProjectionID;
}

//**** Compute Pixel_ID for a given projection
UInt_t get_Pixel_ID(UInt_t HeadID, UInt_t nPix_transaxial, UInt_t nPix_axial, Float_t rotangle, Float_t anglePitch, Float_t crystalSize_transaxial, Float_t crystalSize_axial, Float_t detPosX, Float_t detPosY, Float_t detPosZ) {
    // Compute size of each pixel in the transaxial and axial directions
    Float_t transaxial_sizePix = crystalSize_transaxial / nPix_transaxial;
    Float_t axial_sizePix = crystalSize_axial / nPix_axial;
    
    // Compute axial pixel ID
    UInt_t axialID = static_cast<UInt_t>((detPosZ + nPix_axial / 2 * axial_sizePix) / axial_sizePix);
    
    // Compute head position angle in GATE referential
    Float_t ang = HeadID * anglePitch + rotangle; // anglePitch: angular shift for each head
    
    // Compute transaxial position
    Float_t sina = TMath::Sin(-ang * TMath::Pi() / 180);
    Float_t cosa = TMath::Cos(-ang * TMath::Pi() / 180);
    Float_t trs_pos = detPosX * sina + detPosY * cosa;
    
    // Compute transaxial pixel ID
    UInt_t transaxialID = static_cast<UInt_t>((trs_pos + nPix_transaxial / 2 * transaxial_sizePix) / transaxial_sizePix);
    // Compute the final pixel ID
    UInt_t pixelID = axialID * nPix_transaxial + transaxialID;
    
    return pixelID;
}

void DrawHistograms(TH1D* hist1, TH1D* hist2, TH1D* hist3, const char* legendName1, const char* legendName2, const char* legendName3, const std::string& outputName) {
   // Create a TCanvas to display the histograms
   TCanvas* canvas = new TCanvas("canvas", "Histograms", 800, 600);

    // Set different line colors for each histogram
    hist1->SetLineColor(kRed);
    hist2->SetLineColor(kBlue);
    hist3->SetLineColor(kGreen);
    
   // Draw the histograms on the canvas
   hist1->Draw();
   hist2->Draw("SAME");
   hist3->Draw("SAME");

   // Create a TLegend to display the names of the histograms
   TLegend* legend = new TLegend(0.7, 0.5, 0.9, 0.7);
   legend->AddEntry(hist1, legendName1, "l");
   legend->AddEntry(hist2, legendName2, "l");
   legend->AddEntry(hist3, legendName3, "l");
   legend->Draw();

   // Update the canvas
   canvas->Update();

   // Print the canvas as a PDF
   canvas->Print(Form("%s.pdf", outputName.c_str()));
}



int Projection_macros(const char* filename) {
    
   // Disable warning messages
    gErrorIgnoreLevel = kWarning;
    
  // Open the ROOT file
  std::stringstream ss;
  ss << filename << ".root";
  std::string fileName = ss.str();
  TFile* file = TFile::Open(fileName.c_str());
  if (!file) {
      std::cout << "Error opening file: " << fileName << std::endl;
      return 0;
  }

  TTree *Singles = dynamic_cast<TTree*>(file->Get("Singles"));
  if (!Singles) {
    std::cout << "Error opening file: " << fileName << std::endl;
    file->Close();
    return 0;
  }

  TObjArray *SinglesList=Singles->GetListOfBranches();
  int nentries_Branch = SinglesList->GetEntries();
  TH1D *total_nb_primaries = (TH1D*)gDirectory->Get("total_nb_primaries");

  // Create histograms to store the projection
  unsigned int nProjection=nHead*nProjection_per_head+1;
  vector<TH2D*> hProjection;
  hProjection.resize(nProjection+1);
    
    for (unsigned int iProjection=0; iProjection<nProjection+1; iProjection++) {
        hProjection[iProjection] = new TH2D(("hProjection_Projection" + std::to_string(iProjection)).c_str(), "Projection 2D", 170, 0, 170, 230, 0, 230);
    }
    
    TH1D* hSpectra_photopeak_total = new TH1D("hSpectra_photopeak_total", "Energy Spectra Photopeak-Total", 600, 0, 600);
    TH1D* hSpectra_photopeak_primary = new TH1D("hSpectra_photopeak_primary", "Energy Spectra Photopeak-Primary", 600, 0, 600);
    TH1D* hSpectra_photopeak_scatter = new TH1D("hSpectra_photopeak_scatter", "Energy Spectra Photopeak-Scatter", 600, 0, 600);
    TH1D* hSpectra_total = new TH1D("hSpectra_total", "Energy Spectra-Total", 600, 0, 600);
    TH1D* hSpectra_primary = new TH1D("hSpectra_primary", "Energy Spectra-Primary", 600, 0, 600);
    TH1D* hSpectra_scatter = new TH1D("hSpectra_scatter", "Energy Spectra-Scatter", 600, 0, 600);
    
  // Variables to hold data from the TTree
  Int_t runID, eventID, sourceID, pixelID, headID, crystalID, RayleighCrystal, RayleighPhantom, comptonCrystal, comptonPhantom;
  Float_t axialPos, globalPosX, globalPosY, globalPosZ, rotationAngle, energy;

  // Set branch addresses
  Singles->SetBranchAddress("RayleighCrystal", &RayleighCrystal);
  Singles->SetBranchAddress("RayleighPhantom", &RayleighPhantom);
  //Singles->SetBranchAddress("RayleighVolName", &RayleighVolName);

  Singles->SetBranchAddress("comptonCrystal", &comptonCrystal);
  Singles->SetBranchAddress("comptonPhantom", &comptonPhantom);
  //Singles->SetBranchAddress("ComptonVolName", &ComptonVolName);

  Singles->SetBranchAddress("axialPos", &axialPos);
  Singles->SetBranchAddress("crystalID", &crystalID);
  Singles->SetBranchAddress("eventID", &eventID);
  Singles->SetBranchAddress("globalPosX", &globalPosX);
  Singles->SetBranchAddress("globalPosY", &globalPosY);
  Singles->SetBranchAddress("globalPosZ", &globalPosZ);
  Singles->SetBranchAddress("energy", &energy);
  Singles->SetBranchAddress("headID", &headID); // Head ID 0 default x negative, 1 x positive
  Singles->SetBranchAddress("pixelID", &pixelID);
  Singles->SetBranchAddress("rotationAngle", &rotationAngle);
  Singles->SetBranchAddress("runID", &runID); // Projection ID, then Two Heads
    
  UInt_t Projection_ID, Pixel_ID;
    
    // Allocate memory for the Projection array
    unsigned int*** Projection_Photopeak = new unsigned int**[nProjection_per_head * nHead];
    unsigned int*** Projection_UpperScatterWindow = new unsigned int**[nProjection_per_head * nHead];
    unsigned int*** Projection_LowerScatterWindow = new unsigned int**[nProjection_per_head * nHead];
    unsigned int*** Projection_Photopeak_ScatterFromGate = new unsigned int**[nProjection_per_head * nHead];
    unsigned int*** Projection_Photopeak_Primary = new unsigned int**[nProjection_per_head * nHead];
    unsigned int*** Projection_Scat_TEW  = new unsigned int**[nProjection_per_head * nHead];
    for (unsigned int i = 0; i < nProjection_per_head * nHead; ++i) {
        Projection_Photopeak[i] = new unsigned int*[nPix_transaxial];
        Projection_UpperScatterWindow[i] = new unsigned int*[nPix_transaxial];
        Projection_LowerScatterWindow[i] = new unsigned int*[nPix_transaxial];
        Projection_Photopeak_ScatterFromGate[i] = new unsigned int*[nPix_transaxial];
        Projection_Photopeak_Primary[i] = new unsigned int*[nPix_transaxial];
        Projection_Scat_TEW[i] = new unsigned int*[nPix_transaxial];
        for (unsigned int j = 0; j < nPix_transaxial; ++j) {
            Projection_Photopeak[i][j] = new unsigned int[nPix_axial]();
            Projection_UpperScatterWindow[i][j] = new unsigned int[nPix_axial]();
            Projection_LowerScatterWindow[i][j] = new unsigned int[nPix_axial]();
            Projection_Photopeak_ScatterFromGate[i][j] = new unsigned int[nPix_axial]();
            Projection_Photopeak_Primary[i][j] = new unsigned int[nPix_axial]();
            Projection_Scat_TEW[i][j] = new unsigned int[nPix_axial]();
        }
    }
        
  unsigned int transaxialID_main;
  unsigned int axialID_main;
  // Loop over the TTree entries
  Long64_t nEntries = Singles->GetEntries();
  Long64_t progressBarStep = nEntries / 20; // 5% steps
    
  for (Long64_t iEntry = 0; iEntry < nEntries; ++iEntry) {
    Singles->GetEntry(iEntry);

    // VERBOSE
    /*printf("---- START --- \n");
    //printf("Projection View over (360 degree): %d\n", runID);
    //printf("eventID: %d\n", eventID);
    //printf("sourceID: %d\n", sourceID);
    //printf("rotationAngle: %f\n", rotationAngle);
    //printf("energy: %f\n", energy);
    //printf("pixelID: %d\n", pixelID);
    //printf("headID: %d\n", headID);
    //printf("crystalID: %d\n", crystalID);
    //printf("RayleighCrystal: %d\n", RayleighCrystal);
    //printf("RayleighPhantom: %d\n", RayleighPhantom);
    //printf("ComptonCrystal: %d\n", comptonCrystal);
    //printf("ComptonPhantom: %d\n", comptonPhantom);
    //printf("axialPos: %f\n", axialPos);
    //printf("globalPosX: %f\n", globalPosX);
    //printf("globalPosY: %f\n", globalPosY);
    //printf("globalPosZ: %f\n", globalPosZ);
    printf("---- END --- \n");*/
    
      if(energy<=Photopeak_E_max && energy>=Photopeak_E_min){ // Photopeak window
        Projection_ID = get_Projection_ID(nProjection_per_head, headID, rotationAngle, angleStep);
        Pixel_ID = get_Pixel_ID(headID, nPix_transaxial, nPix_axial, rotationAngle, anglePitch, crystalSize_transaxial, crystalSize_axial, globalPosX, globalPosY, globalPosZ-Shift_Oz);
        axialID_main = Pixel_ID / nPix_transaxial;
        transaxialID_main = Pixel_ID % nPix_transaxial;
        
        // VERBOSE
        // cout << " ProjectionID: " << Projection_ID << "/" << nProjection_per_head * nHead << " axialID_main: " << axialID_main << "/" << nPix_axial << " transaxialID_main: " << transaxialID_main << "/" << nPix_transaxial << endl;
          
          if ((Projection_ID<nProjection_per_head*nHead) && (transaxialID_main<nPix_transaxial) && (axialID_main<nPix_axial)){
           Projection_Photopeak[Projection_ID][transaxialID_main][axialID_main]++;
           hProjection[Projection_ID]->Fill(axialID_main, transaxialID_main);
           hProjection[nHead*nProjection_per_head]->Fill(axialID_main, transaxialID_main);
          }
          if((comptonPhantom!=0)||(RayleighPhantom!=0)){
              Projection_Photopeak_ScatterFromGate[Projection_ID][transaxialID_main][axialID_main]++;
              hSpectra_photopeak_scatter->Fill(energy*1000);
          }
          if((comptonPhantom==0)&&(RayleighPhantom==0)){
              Projection_Photopeak_Primary[Projection_ID][transaxialID_main][axialID_main]++;
              hSpectra_photopeak_primary->Fill(energy*1000);
          }
          hSpectra_photopeak_total->Fill(energy*1000);
      }
      if(energy<Scatter_Lower_E_max && energy>Scatter_Lower_E_min){ // Lower scatter window
        Projection_ID = get_Projection_ID(nProjection_per_head, headID, rotationAngle, angleStep);
        Pixel_ID = get_Pixel_ID(headID, nPix_transaxial, nPix_axial, rotationAngle, anglePitch, crystalSize_transaxial, crystalSize_axial, globalPosX, globalPosY, globalPosZ-Shift_Oz);
        axialID_main = Pixel_ID / nPix_transaxial;
        transaxialID_main = Pixel_ID % nPix_transaxial;
        if ((Projection_ID<nProjection_per_head*nHead) && (transaxialID_main<nPix_transaxial) && (axialID_main<nPix_axial)){
          Projection_LowerScatterWindow[Projection_ID][transaxialID_main][axialID_main]++;
          hProjection[Projection_ID]->Fill(axialID_main, transaxialID_main);
          hProjection[nHead*nProjection_per_head]->Fill(axialID_main, transaxialID_main);
        }
      }
      if(energy<Scatter_Upper_E_max && energy>Scatter_Upper_E_min){ // Upper scatter window
        Projection_ID = get_Projection_ID(nProjection_per_head, headID, rotationAngle, angleStep);
        Pixel_ID = get_Pixel_ID(headID, nPix_transaxial, nPix_axial, rotationAngle, anglePitch, crystalSize_transaxial, crystalSize_axial, globalPosX, globalPosY, globalPosZ-Shift_Oz);
        axialID_main = Pixel_ID / nPix_transaxial;
        transaxialID_main = Pixel_ID % nPix_transaxial;
        if ((Projection_ID<nProjection_per_head*nHead) && (transaxialID_main<nPix_transaxial) && (axialID_main<nPix_axial)){
         Projection_UpperScatterWindow[Projection_ID][transaxialID_main][axialID_main]++;
         hProjection[Projection_ID]->Fill(axialID_main, transaxialID_main);
         hProjection[nHead*nProjection_per_head]->Fill(axialID_main, transaxialID_main);
        }
      }
      if((comptonPhantom!=0)||(RayleighPhantom!=0)){
          hSpectra_scatter->Fill(energy*1000);
      }
      if((comptonPhantom==0)&&(RayleighPhantom==0)){
          hSpectra_primary->Fill(energy*1000);
      }
      hSpectra_total->Fill(energy*1000);
      
      if (iEntry % progressBarStep == 0) {
          float progress = static_cast<float>(iEntry) / nEntries * 100;
          std::cout << "Reading File in Progress: " << std::fixed << std::setprecision(1) << progress << "%" << std::endl;
      }
  }
  // Complete the progress bar
    std::cout << "------> Reading of the file completed" << std::endl;
    
  // Calculating the scatter projection via the triple energy window method
    float Length_Photopeak = Photopeak_E_max-Photopeak_E_min;
    float Length_TEW_Low = Scatter_Lower_E_max-Scatter_Lower_E_min;
    float Length_TEW_Up = Scatter_Upper_E_max-Scatter_Upper_E_min;
    
    for (unsigned int iHead=0; iHead<nProjection_per_head * nHead; iHead++) {
        for (unsigned int iY=0; iY<nPix_transaxial; iY++) {
            for (unsigned int iX=0; iX<nPix_axial; iX++) {
                Projection_Scat_TEW[iHead][iY][iX]=(Projection_LowerScatterWindow[iHead][iY][iX]/Length_TEW_Low + Projection_UpperScatterWindow[iHead][iY][iX]/Length_TEW_Up)*Length_Photopeak/2.0;
            }
        }
    }

  // Write the Projection array as an Interfile image
    writeInterfileImage(Projection_Photopeak, nPix_transaxial, nPix_axial, nProjection_per_head, nHead, std::string(filename) + "_Projection_Photopeak");
    writeInterfileImage(Projection_UpperScatterWindow, nPix_transaxial, nPix_axial, nProjection_per_head, nHead, std::string(filename) + "_Projection_UpperScatterWindow");
    writeInterfileImage(Projection_LowerScatterWindow, nPix_transaxial, nPix_axial, nProjection_per_head, nHead, std::string(filename) + "_Projection_LowerScatterWindow");
    writeInterfileImage(Projection_Photopeak_Primary, nPix_transaxial, nPix_axial, nProjection_per_head, nHead, std::string(filename) + "_Projection_Photopeak_Primary");
    writeInterfileImage(Projection_Photopeak_ScatterFromGate, nPix_transaxial, nPix_axial, nProjection_per_head, nHead, std::string(filename) + "_Projection_Photopeak_ScatterFromGate");
    writeInterfileImage(Projection_Scat_TEW, nPix_transaxial, nPix_axial, nProjection_per_head, nHead, std::string(filename) + "_Projection_Photopeak_ScatterViaTEW");

    // Deallocate the Projection array
    for (unsigned int i = 0; i < nProjection_per_head * nHead; ++i) {
        for (unsigned int j = 0; j < nPix_transaxial; ++j) {
            delete[] Projection_Photopeak[i][j];
            delete[] Projection_UpperScatterWindow[i][j];
            delete[] Projection_LowerScatterWindow[i][j];
            delete[] Projection_Photopeak_Primary[i][j];
            delete[] Projection_Photopeak_ScatterFromGate[i][j];
        }
        delete[] Projection_Photopeak[i];
        delete[] Projection_UpperScatterWindow[i];
        delete[] Projection_LowerScatterWindow[i];
        delete[] Projection_Photopeak_Primary[i];
        delete[] Projection_Photopeak_ScatterFromGate[i];
    }
    delete[] Projection_Photopeak;
    delete[] Projection_UpperScatterWindow;
    delete[] Projection_LowerScatterWindow;
    delete[] Projection_Photopeak_Primary;
    delete[] Projection_Photopeak_ScatterFromGate;
    
    Draw_TH2(hProjection, std::string(filename)+"Photopeak_Projection2D", "Axial (pixels)", "Transaxial (pixels)", "COL4Z", false);

    DrawHistograms(hSpectra_photopeak_total, hSpectra_photopeak_primary, hSpectra_photopeak_scatter, "Total", "Primary", "Scatter", std::string(filename) + "PhotopeakSpectra");
    DrawHistograms(hSpectra_total, hSpectra_primary, hSpectra_scatter, "Total", "Primary", "Scatter", std::string(filename) + "FullSpectra");
    
    return 0;
}
