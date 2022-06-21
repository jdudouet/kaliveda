#include "KVTestIDGridDialog.h"
#include "Riostream.h"
#include "KVIDGridManager.h"
#include "TGFileDialog.h"
#include "TPad.h"
#include <TGMsgBox.h>
#include <TTimer.h>
#include <TClass.h>
#include <TROOT.h>
#include <TGLayout.h>
#include <TCanvas.h>
#include "KVConfig.h"
#include "KVNucleus.h"
#include "TStyle.h"
#include "TBox.h"
#include "TLine.h"
#include "KVNuclearChart.h"
#include "KVIdentificationResult.h"
#include "KVIDZAFromZGrid.h"

TString KVTestIDGridDialog::fNameData = "htemp";
TString KVTestIDGridDialog::fNameZreal = "hzreal";
TString KVTestIDGridDialog::fNameZvsE = "hzvse";
Int_t KVTestIDGridDialog::hzrealbins = 500;
Int_t KVTestIDGridDialog::hzrealxmin = 0;
Int_t KVTestIDGridDialog::hzrealxmax = 100;
Int_t KVTestIDGridDialog::hzvsexbins = 500;
Double_t KVTestIDGridDialog::hzvsexmin = 0;
Double_t KVTestIDGridDialog::hzvsexmax = 5000;
Int_t KVTestIDGridDialog::hzvseybins = 500;
Int_t KVTestIDGridDialog::hzvseymin = 0;
Int_t KVTestIDGridDialog::hzvseymax = 100;

ClassImp(KVTestIDGridDialog)

KVTestIDGridDialog::KVTestIDGridDialog(const TGWindow* p,
                                       const TGWindow* main, UInt_t w,
                                       UInt_t h, KVIDGraph* g, TH2* data_histo)
{
   if (data_histo) {
      fNameData.Form("%s", data_histo->GetName());
      fNameZreal.Form("PID:H=%s_G=%s", data_histo->GetName(), g->GetName());
      fNameZvsE.Form("PIDvsEres:H=%s_G=%s", data_histo->GetName(), g->GetName());
      hzvsexmax = data_histo->GetXaxis()->GetXmax();
   }
   // set PID limits from grid
   g->GetIdentifiers()->Sort(); // make sure lines are in order of increasing PID
   hzrealxmin = ((KVIDentifier*)g->GetIdentifiers()->First())->GetPID() - 1.0;
   hzrealxmax = ((KVIDentifier*)g->GetIdentifiers()->Last())->GetPID() + 1.0;
   hzvseymin = hzrealxmin;
   hzvseymax = hzrealxmax;

   if (g->HasMassIDCapability() && !g->InheritsFrom("KVIDZAFromZGrid")) {// KLUDGE! replaces if(!g->IsOnlyZId()) {
      hzrealbins = 150 * (hzrealxmax - hzrealxmin);
      hzvseybins = hzrealbins;
   }

   //Dialog box for testing identification grid
   fMain = new TGTransientFrame(p, main, w, h);
   fMain->Connect("CloseWindow()", "KVTestIDGridDialog", this,
                  "DoClose()");
   fMain->DontCallClose();      // to avoid double deletions.

   fSelectedGrid = g;
   // use hierarchical cleaning
   fMain->SetCleanup(kDeepCleanup);

   /******* frame for name of data histo ********/
   fHdataFrame = new TGHorizontalFrame(fMain);
   fHdataNameLabel = new TGLabel(fHdataFrame, "Name of data histo (TH2F)");
   fHdataNameEntry = new TGTextEntry(fNameData, fHdataFrame);
   fHdataNameEntry->SetWidth(150);
   fHdataFrame->AddFrame(fHdataNameEntry,
                         new TGLayoutHints(kLHintsRight, 5, 5, 2, 2));
   fHdataFrame->AddFrame(fHdataNameLabel,
                         new TGLayoutHints(kLHintsRight, 5, 5, 2, 2));

   /******* frame for Zreal histo **************/
   fHzrealFrame = new TGGroupFrame(fMain, "ID test histo");
   fHzrealNameFrame = new TGHorizontalFrame(fHzrealFrame);
   fHzrealNameLabel =
      new TGLabel(fHzrealNameFrame, "Name of PID histo (TH1F)");
   fHzrealNameEntry = new TGTextEntry(fNameZreal, fHzrealNameFrame);
   fHzrealNameEntry->SetWidth(150);
   fHzrealNameFrame->AddFrame(fHzrealNameEntry,
                              new TGLayoutHints(kLHintsRight, 5, 5, 2, 2));
   fHzrealNameFrame->AddFrame(fHzrealNameLabel,
                              new TGLayoutHints(kLHintsRight, 5, 5, 2, 2));
   fHzrealBinsFrame = new TGHorizontalFrame(fHzrealFrame);
   fHzrealBinsLabel = new TGLabel(fHzrealBinsFrame, "nbins");
   fHzrealBinsEntry =
      new TGNumberEntry(fHzrealBinsFrame, hzrealbins, 4, 0,
                        TGNumberFormat::kNESInteger,
                        TGNumberFormat::kNEAPositive);
   fHzrealXminLabel = new TGLabel(fHzrealBinsFrame, "xmin");
   fHzrealXminEntry =
      new TGNumberEntry(fHzrealBinsFrame, hzrealxmin, 4, 0,
                        TGNumberFormat::kNESInteger,
                        TGNumberFormat::kNEAPositive);
   fHzrealXmaxLabel = new TGLabel(fHzrealBinsFrame, "xmax");
   fHzrealXmaxEntry =
      new TGNumberEntry(fHzrealBinsFrame, hzrealxmax, 4, 0,
                        TGNumberFormat::kNESInteger,
                        TGNumberFormat::kNEAPositive);
   fHzrealBinsFrame->AddFrame(fHzrealXmaxEntry,
                              new TGLayoutHints(kLHintsRight, 5, 5, 2, 2));
   fHzrealBinsFrame->AddFrame(fHzrealXmaxLabel,
                              new TGLayoutHints(kLHintsRight, 5, 5, 2, 2));
   fHzrealBinsFrame->AddFrame(fHzrealXminEntry,
                              new TGLayoutHints(kLHintsRight, 5, 5, 2, 2));
   fHzrealBinsFrame->AddFrame(fHzrealXminLabel,
                              new TGLayoutHints(kLHintsRight, 5, 5, 2, 2));
   fHzrealBinsFrame->AddFrame(fHzrealBinsEntry,
                              new TGLayoutHints(kLHintsRight, 5, 5, 2, 2));
   fHzrealBinsFrame->AddFrame(fHzrealBinsLabel,
                              new TGLayoutHints(kLHintsRight, 5, 5, 2, 2));
   fHzrealFrame->AddFrame(fHzrealNameFrame,
                          new TGLayoutHints(kLHintsNormal, 5, 5, 2, 2));
   fHzrealFrame->AddFrame(fHzrealBinsFrame,
                          new TGLayoutHints(kLHintsNormal, 5, 5, 2, 2));

   /******* frame for ZvsE histo **************/
   fHzvseFrame = new TGGroupFrame(fMain, "PID vs. E histo");
   fHzvseNameFrame = new TGHorizontalFrame(fHzvseFrame);
   fHzvseNameLabel = new TGLabel(fHzvseNameFrame, "Name of histo (TH2F)");
   fHzvseNameEntry = new TGTextEntry(fNameZvsE, fHzvseNameFrame);
   fHzvseNameEntry->SetWidth(150);
   fHzvseNameFrame->AddFrame(fHzvseNameEntry,
                             new TGLayoutHints(kLHintsRight, 5, 5, 2, 2));
   fHzvseNameFrame->AddFrame(fHzvseNameLabel,
                             new TGLayoutHints(kLHintsRight, 5, 5, 2, 2));
   fHzvseXBinsFrame = new TGHorizontalFrame(fHzvseFrame);
   fHzvseXBinsLabel = new TGLabel(fHzvseXBinsFrame, "nbins");
   fHzvseXBinsEntry =
      new TGNumberEntry(fHzvseXBinsFrame, hzvsexbins, 4, 0,
                        TGNumberFormat::kNESInteger,
                        TGNumberFormat::kNEAPositive);
   fHzvseXminLabel = new TGLabel(fHzvseXBinsFrame, "E : min");
   fHzvseXminEntry =
      new TGNumberEntry(fHzvseXBinsFrame, hzvsexmin, 4, 0,
                        TGNumberFormat::kNESReal);
   fHzvseXmaxLabel = new TGLabel(fHzvseXBinsFrame, "max");
   fHzvseXmaxEntry =
      new TGNumberEntry(fHzvseXBinsFrame, hzvsexmax, 4, 0,
                        TGNumberFormat::kNESReal);
   fHzvseXBinsFrame->AddFrame(fHzvseXmaxEntry,
                              new TGLayoutHints(kLHintsRight, 5, 5, 2, 2));
   fHzvseXBinsFrame->AddFrame(fHzvseXmaxLabel,
                              new TGLayoutHints(kLHintsRight, 5, 5, 2, 2));
   fHzvseXBinsFrame->AddFrame(fHzvseXminEntry,
                              new TGLayoutHints(kLHintsRight, 5, 5, 2, 2));
   fHzvseXBinsFrame->AddFrame(fHzvseXminLabel,
                              new TGLayoutHints(kLHintsRight, 5, 5, 2, 2));
   fHzvseXBinsFrame->AddFrame(fHzvseXBinsEntry,
                              new TGLayoutHints(kLHintsRight, 5, 5, 2, 2));
   fHzvseXBinsFrame->AddFrame(fHzvseXBinsLabel,
                              new TGLayoutHints(kLHintsRight, 5, 5, 2, 2));
   fHzvseYBinsFrame = new TGHorizontalFrame(fHzvseFrame);
   fHzvseYBinsLabel = new TGLabel(fHzvseYBinsFrame, "nbins");
   fHzvseYBinsEntry =
      new TGNumberEntry(fHzvseYBinsFrame, hzvseybins, 4, 0,
                        TGNumberFormat::kNESInteger,
                        TGNumberFormat::kNEAPositive);
   fHzvseYminLabel = new TGLabel(fHzvseYBinsFrame, "PID : min");
   fHzvseYminEntry =
      new TGNumberEntry(fHzvseYBinsFrame, hzvseymin, 4, 0,
                        TGNumberFormat::kNESInteger,
                        TGNumberFormat::kNEAPositive);
   fHzvseYmaxLabel = new TGLabel(fHzvseYBinsFrame, "max");
   fHzvseYmaxEntry =
      new TGNumberEntry(fHzvseYBinsFrame, hzvseymax, 4, 0,
                        TGNumberFormat::kNESInteger,
                        TGNumberFormat::kNEAPositive);
   fHzvseYBinsFrame->AddFrame(fHzvseYmaxEntry,
                              new TGLayoutHints(kLHintsRight, 5, 5, 2, 2));
   fHzvseYBinsFrame->AddFrame(fHzvseYmaxLabel,
                              new TGLayoutHints(kLHintsRight, 5, 5, 2, 2));
   fHzvseYBinsFrame->AddFrame(fHzvseYminEntry,
                              new TGLayoutHints(kLHintsRight, 5, 5, 2, 2));
   fHzvseYBinsFrame->AddFrame(fHzvseYminLabel,
                              new TGLayoutHints(kLHintsRight, 5, 5, 2, 2));
   fHzvseYBinsFrame->AddFrame(fHzvseYBinsEntry,
                              new TGLayoutHints(kLHintsRight, 5, 5, 2, 2));
   fHzvseYBinsFrame->AddFrame(fHzvseYBinsLabel,
                              new TGLayoutHints(kLHintsRight, 5, 5, 2, 2));
   fHzvseFrame->AddFrame(fHzvseNameFrame,
                         new TGLayoutHints(kLHintsNormal, 5, 5, 2, 2));
   fHzvseFrame->AddFrame(fHzvseXBinsFrame,
                         new TGLayoutHints(kLHintsNormal, 5, 5, 2, 2));
   fHzvseFrame->AddFrame(fHzvseYBinsFrame,
                         new TGLayoutHints(kLHintsNormal, 5, 5, 2, 2));

   /********** progress bar **********/
   fProgressBar = new TGHProgressBar(fMain, TGProgressBar::kFancy, 300);
   fProgressBar->SetFillType(TGProgressBar::kBlockFill);
   fProgressBar->ShowPosition();
   fProgressBar->SetBarColor("red");

   /************buttons***************/
   fButtonsFrame = new TGHorizontalFrame(fMain, 100, 50);
   fTestButton = new TGTextButton(fButtonsFrame, "&Test");
   fTestButton->Connect("Clicked()", "KVTestIDGridDialog", this,
                        "TestGrid()");
   fCloseButton = new TGTextButton(fButtonsFrame, "&Close");
   fCloseButton->Connect("Clicked()", "KVTestIDGridDialog", this,
                         "DoClose()");

   fButtonsFrame->AddFrame(fTestButton,
                           new TGLayoutHints(kLHintsExpandX |
                                 kLHintsCenterX |
                                 kLHintsCenterY, 2, 2, 5, 5));
   fButtonsFrame->AddFrame(fCloseButton,
                           new TGLayoutHints(kLHintsExpandX |
                                 kLHintsCenterX |
                                 kLHintsCenterY, 2, 2, 5, 5));

   fMain->AddFrame(fHdataFrame,
                   new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 15,
                                     5));
   fMain->AddFrame(fHzrealFrame,
                   new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 5,
                                     5));
   fMain->AddFrame(fHzvseFrame,
                   new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 5,
                                     10));
   fMain->AddFrame(fProgressBar,
                   new TGLayoutHints(kLHintsCenterX | kLHintsTop, 2, 2, 5,
                                     5));
   fMain->AddFrame(fButtonsFrame,
                   new TGLayoutHints(kLHintsCenterX | kLHintsTop, 2, 2, 5,
                                     5));

   //layout and display window
   fMain->MapSubwindows();
   fMain->Resize(fMain->GetDefaultSize());

   // position relative to the parent's window
   fMain->CenterOnParent();

   fMain->SetWindowName("Test ID grid");
   fMain->MapWindow();

   // clean up results histograms
   histos.SetOwner();
}

void KVTestIDGridDialog::DoClose()
{
   TTimer::SingleShot(150, "KVTestIDGridDialog", this, "CloseWindow()");
}

void KVTestIDGridDialog::CloseWindow()
{
   delete this;
}

KVTestIDGridDialog::~KVTestIDGridDialog()
{
   //Delete all widgets

   delete fMain;
}

void KVTestIDGridDialog::TestGrid()
{
   //read current values of all fields, find/create the histograms,
   //and test the identification

   fNameData = fHdataNameEntry->GetText();
   fNameZreal = fHzrealNameEntry->GetText();
   fNameZvsE = fHzvseNameEntry->GetText();

   if (!CheckNameOK(fNameData) || !CheckNameOK(fNameZreal)
         || !CheckNameOK(fNameZvsE)) {

      new TGMsgBox(gClient->GetRoot(), fMain, "Test ID Grid",
                   "Give a valid name for each histo", kMBIconExclamation);
      return;
   }

   hzrealbins = fHzrealBinsEntry->GetIntNumber();
   hzrealxmin = fHzrealXminEntry->GetIntNumber();
   hzrealxmax = fHzrealXmaxEntry->GetIntNumber();

   hzvsexbins = fHzvseXBinsEntry->GetIntNumber();
   hzvsexmin = fHzvseXminEntry->GetNumber();
   hzvsexmax = fHzvseXmaxEntry->GetNumber();

   hzvseybins = fHzvseYBinsEntry->GetIntNumber();
   hzvseymin = fHzvseYminEntry->GetIntNumber();
   hzvseymax = fHzvseYmaxEntry->GetIntNumber();

   Int_t hnmin = ((KVIDentifier*)fSelectedGrid->GetIdentifiers()->First())->GetA() - ((KVIDentifier*)fSelectedGrid->GetIdentifiers()->First())->GetZ() - 1.0;
   Int_t hnmax = ((KVIDentifier*)fSelectedGrid->GetIdentifiers()->Last())->GetA() - ((KVIDentifier*)fSelectedGrid->GetIdentifiers()->Last())->GetZ() + 1.0;
   Int_t hamin = ((KVIDentifier*)fSelectedGrid->GetIdentifiers()->First())->GetA();
   Int_t hamax = ((KVIDentifier*)fSelectedGrid->GetIdentifiers()->Last())->GetA();
   Int_t hzmin = ((KVIDentifier*)fSelectedGrid->GetIdentifiers()->First())->GetZ();
   Int_t hzmax = ((KVIDentifier*)fSelectedGrid->GetIdentifiers()->Last())->GetZ();
   if (fSelectedGrid->InheritsFrom("KVIDZAFromZGrid")) {
      KVList* is_list = ((KVIDZAFromZGrid*)fSelectedGrid)->GetIntervalSets();
      if (is_list && is_list->GetEntries()) {
         KVList* iv_list = ((interval_set*)is_list->Last())->GetIntervals();
         if (iv_list && iv_list->GetEntries()) {
            interval* itv = (interval*)iv_list->Last();
            hnmax = itv->GetA() - itv->GetZ();
            hamax = itv->GetA();
            hzmax = itv->GetZ();
            itv = (interval*)((interval_set*)is_list->First())->GetIntervals()->First();
            if (itv) {
               hamin = itv->GetA();
               hzmin = itv->GetZ();
            }
         }
      }
   }

   TH2F* hdata = (TH2F*) gROOT->FindObject(fNameData.Data());
   if (!hdata) {

      new TGMsgBox(gClient->GetRoot(), fMain, "Test ID Grid",
                   "Check name of data histogram", kMBIconExclamation);
      return;
   }

   // clear any previous results
   histos.Clear();

   // make results histograms
   auto hzreal =
      new TH1F(fNameZreal.Data(), "PID distribution", hzrealbins,
               hzrealxmin, hzrealxmax);
   add_histo(hzreal);

   KVNameValueList histo_names;

   histo_names.SetValue("ID_REAL", fNameZreal.Data());

   auto hzvse =
      new TH2F(fNameZvsE.Data(), "PID vs. E_{res}", hzvsexbins, hzvsexmin,
               hzvsexmax, hzvseybins, hzvseymin, hzvseymax);

   histo_names.SetValue("ID_REAL_VS_ERES", fNameZvsE.Data());
   add_histo(hzvse);

   // A vs Z map in case of mass identification
   // A dist for isotopically identified particles
   TH2F* hazreal = 0;
   TH2F* adist_aident = 0;
   if (fSelectedGrid->HasMassIDCapability()) {
      hazreal = new TH2F("AZMap", "Z vs. A", 30 * (hnmax - hnmin + 0.5), hnmin - 0.5, hnmax + 1, 30 * (hzrealxmax - hzrealxmin + 1), hzrealxmin - 1, hzrealxmax + 1);

      histo_names.SetValue("Z_A_REAL", "AZMap");
      add_histo(hazreal);

      adist_aident = new TH2F("ZADIST_AIDENT", "Z-A distribution [isotopic ID OK]", hzmax - hzmin + 1, hzmin - .5, hzmax + .5, hamax - hamin + 1, hamin - .5, hamax + .5);
      histo_names.SetValue("ZADIST_AIDENT", "ZADIST_AIDENT");
      add_histo(adist_aident);
   }

   //progress bar set up
   fProgressBar->SetRange(0, hdata->GetSum());
   fProgressBar->Reset();
   fSelectedGrid->Connect("Increment(Float_t)", "TGHProgressBar",
                          fProgressBar, "SetPosition(Float_t)");
#ifdef __WITHOUT_TGBUTTON_SETENABLED
#define SetEnabled(flag) \
   SetState( flag ? kButtonUp : kButtonDisabled )
#endif
   fTestButton->SetEnabled(kFALSE);
   fCloseButton->SetEnabled(kFALSE);
   fSelectedGrid->TestIdentification(hdata, histos, histo_names);
   fTestButton->SetEnabled(kTRUE);
   fCloseButton->SetEnabled(kTRUE);
#ifdef __WITHOUT_TGBUTTON_SETENABLED
#undef SetEnabled
#endif
   fSelectedGrid->Disconnect("Increment(Float_t)", fProgressBar,
                             "SetPosition(Float_t)");
   fProgressBar->Reset();

   KVCanvas* cc = new KVCanvas;
   resultsCanvas.reset(cc);

   gStyle->SetOptTitle(); // display histo title in pads
   cc->Divide(2, 2);
   int ipad = 1;
   if (hazreal) {
      //cc->SetTitle(Form("Z vs N map for %s", fSelectedGrid->GetName()));
      auto pad = cc->cd(ipad++);
      pad->SetLogz();
      hazreal->Draw("col");

      if (!strcmp(hazreal->GetTitle(), "Z vs. A")) {
         TAxis* ax = 0;
         ax = hazreal->GetXaxis();
         ax->SetNdivisions(000);
         ax->SetLabelOffset(-0.04);
         ax->SetTickLength(0);

         ax = hazreal->GetYaxis();
         ax->SetNdivisions(000);
         ax->SetLabelOffset(-0.03);
         ax->SetTickLength(0);
         hazreal->SetMinimum(1);
         DrawChart(pad, (Int_t)hzrealxmin, (Int_t)hzrealxmax, (Int_t)hnmin, (Int_t)hnmax);
      }

      pad = cc->cd(ipad++);
      //cc->SetTitle(Form("ZA identification for %s", fSelectedGrid->GetName()));
      //cc->cd();
      pad->SetLogz();
      adist_aident->Draw("col");
   }


   // show results in canvas
   auto pad = cc->cd(ipad++);
   pad->SetGridx();
   pad->SetGridy();
   hzreal->SetStats(kFALSE);
   hzreal->Draw("hist");

   pad = cc->cd(ipad++);
   pad->SetLogz(kTRUE);
   pad->SetGridx();
   pad->SetGridy();
   hzvse->SetStats(kFALSE);
   hzvse->Draw("zcol");

   gStyle->SetOptTitle(0);
}


void KVTestIDGridDialog::DrawChart(TVirtualPad* pp, Int_t zmin, Int_t zmax, Int_t nmin, Int_t nmax)
{
   Double_t marging = 0.001;
   pp->SetTopMargin(marging);
   pp->SetRightMargin(marging);
   pp->SetBottomMargin(marging);
   pp->SetLeftMargin(marging);

   KVNuclearChart* nucChar = new KVNuclearChart(nmin + 1, nmax, zmin, zmax);
   nucChar->Draw("same");

   return;
}







