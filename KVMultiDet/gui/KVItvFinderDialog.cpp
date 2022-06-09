//Created by KVClassFactory on Mon Jan 23 10:03:13 2017
//Author: Diego Gruyer

#include "KVItvFinderDialog.h"
#include "TRootEmbeddedCanvas.h"
#include "TStyle.h"
#include "TSystem.h"
#include "TROOT.h"
#include "TGMsgBox.h"
#include "TGFileDialog.h"
#include "KVTestIDGridDialog.h"
#include "KVIdentificationResult.h"
#include <thread>

ClassImp(KVItvFinderDialog)
//ClassImp(interval_painter)


void KVItvFinderDialog::delete_painter_from_painter_list(KVPIDIntervalPainter* p)
{
   // remove painter from list and modify the 'left_painter' and 'right_painter' references
   // in any adjacent painters/intervals, then delete painter

   std::unique_ptr<KVPIDIntervalPainter> _p(p);
   auto pleft = p->get_left_interval();
   auto pright = p->get_right_interval();
   if (pleft) pleft->set_right_interval(pright);
   if (pright) pright->set_left_interval(pleft);
   fItvPaint.Remove(p);
}

KVItvFinderDialog::KVItvFinderDialog(KVIDZAFromZGrid* gg, TH2* hh)//:fSpectrum(7,1)
{
   fGrid  = gg;
   fHisto = hh;

   fPoints  = new TGraph;
   fPoints->SetMarkerStyle(23);
   fNPoints = 0;

   gStyle->SetOptStat(0);
   gStyle->SetOptTitle(0);

   fMain = new TGTransientFrame(gClient->GetDefaultRoot(), gClient->GetDefaultRoot(), 10, 10);
   // Here is the recipe for cleanly closing a window
   // (see https://root-forum.cern.ch/t/error-in-rootx11errorhandler-baddrawable-quot/8095/6)
   fMain->Connect("CloseWindow()", "KVItvFinderDialog", this, "DoClose()");
   fMain->DontCallClose();
   fMain->SetCleanup(kDeepCleanup);
   // afterwards, in the destructor do fMain->CloseWindo(), and in DoClose(), do 'delete this'

   // Default constructor
   TGHorizontalFrame* fCanvasFrame = new TGHorizontalFrame(fMain, 627, 7000, kHorizontalFrame);
   //    fCanvasFrame->SetBackgroundColor(fColor);


   TRootEmbeddedCanvas* fRootEmbeddedCanvas615 = new TRootEmbeddedCanvas(0, fCanvasFrame, 800, 440);
   // to replace the TCanvas in a TRootEmbeddedCanvas, just delete the original like so:
   // (see https://root-forum.cern.ch/t/error-in-rootx11errorhandler-baddrawable-quot/8095/6)
   auto WID = fRootEmbeddedCanvas615->GetCanvasWindowId();
   delete fRootEmbeddedCanvas615->GetCanvas();
   fCanvas = new KVCanvas("c123", 10, 10, WID);
   fRootEmbeddedCanvas615->AdoptCanvas(fCanvas);
   fPad = fCanvas->cd();
   fCanvas->SetRightMargin(0.02);
   fCanvas->SetTopMargin(0.02);
   fCanvas->SetLeftMargin(0.08);
   fCanvas->SetBottomMargin(0.07);

   fCanvasFrame->AddFrame(fRootEmbeddedCanvas615, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2));
   fMain->AddFrame(fCanvasFrame, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 0, 0, 0, 0));

   TGVerticalFrame* fControlOscillo  = new TGVerticalFrame(fCanvasFrame, 2000, 7000, kVerticalFrame);
   //    fControlOscillo->SetBackgroundColor(fColor);


   {
      const char* xpms[] = {
         "filesaveas.xpm",
         "bld_copy.png",
         "ed_new.png",
         "sm_delete.xpm",
         "profile_t.xpm",
         "refresh2.xpm",
         "bld_colorselect.png",
         "latex.xpm",
         "move_cursor.png",
         0
      };
      // toolbar tool tip text
      const char* tips[] = {
         "Save intervals in current grid",
         "Create a new interval set",
         "Create a new interval",
         "Remove selected intervals",
         "Find intervals",
         "Update list views",
         "Test the grid",
         "Set log scale on y axis",
         "Unzoom the histogram",
         0
      };
      int spacing[] = {
         5,
         0,
         0,
         0,
         0,
         0,
         0,
         280,
         0,
         0
      };
      const char* method[] = {
         "SaveGrid()",
         "NewIntervalSet()",
         "NewInterval()",
         "RemoveInterval()",
         "Identify()",
         "UpdateLists()",
         "TestIdent()",
         "SetLogy()",
         "UnzoomHisto()",
         0
      };
      fNbButtons = 0;
      ToolBarData_t t[50];
      fToolBar = new TGToolBar(fControlOscillo, 450, 80);
      int i = 0;
      while (xpms[i]) {
         t[i].fPixmap = xpms[i];
         t[i].fTipText = tips[i];
         t[i].fStayDown = kFALSE;
         t[i].fId = i + 1;
         t[i].fButton = NULL;
         TGButton* bb = fToolBar->AddButton(fControlOscillo, &t[i], spacing[i]);
         bb->Connect("Clicked()", "KVItvFinderDialog", this, method[i]);
         fNbButtons++;
         i++;
      }
      fControlOscillo->AddFrame(fToolBar, new TGLayoutHints(kLHintsTop | kLHintsExpandX));
   }

   fCustomView = new KVListView(interval_set::Class(), fControlOscillo, 450, 200);
   fCustomView->SetDataColumns(3);
   fCustomView->SetDataColumn(0, "Z", "GetZ", kTextLeft);
   fCustomView->SetDataColumn(1, "PIDs", "GetNPID", kTextCenterX);
   fCustomView->SetDataColumn(2, "Masses", "GetListOfMasses", kTextLeft);
   fCustomView->Connect("SelectionChanged()", "KVItvFinderDialog", this, "DisplayPIDint()");
   fCustomView->SetDoubleClickAction("KVItvFinderDialog", this, "ZoomOnCanvas()");
   fCustomView->AllowContextMenu(kFALSE);
   fControlOscillo->AddFrame(fCustomView, new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2));

   fCurrentView = new KVListView(interval::Class(), fControlOscillo, 450, 180);
   fCurrentView->SetDataColumns(5);
   fCurrentView->SetDataColumn(0, "Z", "GetZ", kTextLeft);
   fCurrentView->SetDataColumn(1, "A", "GetA", kTextCenterX);
   fCurrentView->SetDataColumn(2, "min", "GetPIDmin", kTextCenterX);
   fCurrentView->SetDataColumn(3, "pid", "GetPID", kTextCenterX);
   fCurrentView->SetDataColumn(4, "max", "GetPIDmax", kTextCenterX);



   {
      const char* xpms[] = {
         "arrow_down.xpm",
         "arrow_up.xpm",
         0
      };
      const char* tips[] = {
         "Decrease A by one",
         "Increase A by one",
         0
      };
      int spacing[] = {
         120,
         0,
         0
      };
      const char* method[] = {
         "MassesDown()",
         "MassesUp()",
         0
      };
      fNbButtons = 0;
      ToolBarData_t t[50];
      fToolBar2 = new TGToolBar(fControlOscillo, 450, 80);
      int i = 0;
      while (xpms[i]) {
         t[i].fPixmap = xpms[i];
         t[i].fTipText = tips[i];
         t[i].fStayDown = kFALSE;
         t[i].fId = i + 1;
         t[i].fButton = NULL;
         TGButton* bb = fToolBar2->AddButton(fControlOscillo, &t[i], spacing[i]);
         bb->Connect("Clicked()", "KVItvFinderDialog", this, method[i]);
         fNbButtons++;
         i++;
      }
      fControlOscillo->AddFrame(fToolBar2, new TGLayoutHints(kLHintsTop | kLHintsExpandX));
   }

   //    fCurrentView->ActivateSortButtons();
   fCurrentView->Connect("SelectionChanged()", "KVItvFinderDialog", this, "SelectionITVChanged()");
   //    fCurrentView->SetDoubleClickAction("FZCustomFrameManager",this,"ChangeParValue()");
   fCurrentView->AllowContextMenu(kFALSE);

   fControlOscillo->AddFrame(fCurrentView, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2, 2, 2, 2));

   fCanvasFrame->AddFrame(fControlOscillo, new TGLayoutHints(kLHintsExpandY, 0, 0, 0, 0));


   //layout and display window
   fMain->MapSubwindows();
   fMain->Resize(fMain->GetDefaultSize());

   // position relative to the parent's window
   fMain->CenterOnParent();

   fMain->SetWindowName("Masses Identification");
   fMain->RequestFocus();
   fMain->MapWindow();

   fCustomView->Display(((KVIDZAFromZGrid*)fGrid)->GetIntervalSets());
   fPad->cd();

   LinearizeHisto(100);
   fLinearHisto->SetLineColor(kBlack);
   fLinearHisto->SetFillColor(kGray + 1);
   fLinearHisto->Draw("hist");

   int tmp[30] = {3, 3, 3, 4, 4, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6};
   for (int ii = 0; ii < 30; ii++) fNpeaks[ii] = tmp[ii];

   fSig = 0.1;
   fRat = 0.0001;

   DrawIntervals();

}

//____________________________________________________________________________//

void KVItvFinderDialog::DisplayPIDint()
{
   //    DrawIntervals();
   std::unique_ptr<TList> list(fCustomView->GetSelectedObjects());
   Int_t nSelected = list->GetSize();
   if (nSelected == 1) {
      interval_set* itv = (interval_set*)list->At(0);
      fCurrentView->Display(itv->GetIntervals());
   }
   //    ZoomOnCanvas();
   SelectionITVChanged();
}

void KVItvFinderDialog::SelectionITVChanged()
{
   fPad->cd();
   fItvPaint.Execute("HighLight", "0");

   if (fCurrentView->GetLastSelectedObject()) {
      int zz = ((interval*)fCurrentView->GetLastSelectedObject())->GetZ();
      int aa = ((interval*)fCurrentView->GetLastSelectedObject())->GetA();
      KVPIDIntervalPainter* painter = (KVPIDIntervalPainter*)fItvPaint.FindObject(Form("%d_%d", zz, aa));
      if (!painter) Info("SelectionITVChanged", "%d %d not found...", zz, aa);
      painter->HighLight();
   }
   fCanvas->Modified();
   fCanvas->Update();
}

void KVItvFinderDialog::UpdatePIDList()
{
   std::unique_ptr<TList> list(fCustomView->GetSelectedObjects());
   Int_t nSelected = list->GetSize();
   if (nSelected == 1) {
      interval_set* itv = (interval_set*)list->At(0);
      fCurrentView->Display(itv->GetIntervals());
   }
}

void KVItvFinderDialog::ZoomOnCanvas()
{
   if (!fCustomView->GetLastSelectedObject()) return;
   int zz = dynamic_cast<interval_set*>(fCustomView->GetLastSelectedObject())->GetZ();

   fLinearHisto->GetXaxis()->SetRangeUser(zz - 0.5, zz + 0.5);

   fItvPaint.Execute("SetDisplayLabel", "0");

   std::unique_ptr<KVSeqCollection> tmp(fItvPaint.GetSubListWithMethod(Form("%d", zz), "GetZ"));

   tmp->Execute("SetDisplayLabel", "1");

   fCanvas->Modified();
   fCanvas->Update();
}

void KVItvFinderDialog::DrawIntervals()
{
   interval_set* itvs = 0;
   last_drawn_interval = nullptr;
   TIter it(fGrid->GetIntervalSets());
   while ((itvs = (interval_set*)it())) {
      DrawInterval(itvs);
   }
}

void KVItvFinderDialog::DrawInterval(interval_set* itvs, bool label)
{
   fPad->cd();
   interval* itv = nullptr;
   // give a different colour to each interval
   auto nitv = itvs->GetIntervals()->GetEntries();
   auto cstep = TColor::GetPalette().GetSize() / (nitv + 1);
   int i = 1;
   TIter itt(itvs->GetIntervals());
   while ((itv = (interval*)itt())) {
      KVPIDIntervalPainter* dummy = new KVPIDIntervalPainter(itv, fLinearHisto, TColor::GetPalette()[cstep * i], last_drawn_interval);
      ++i;
      if (label) dummy->SetDisplayLabel();
      dummy->Draw();
      dummy->SetCanvas(fCanvas);
      dummy->Connect("IntMod()", "KVItvFinderDialog", this, "UpdatePIDList()");
      fItvPaint.Add(dummy);
      last_drawn_interval = dummy;
   }
}

void KVItvFinderDialog::ClearInterval(interval_set* itvs)
{
   for (int ii = 0; ii < itvs->GetNPID(); ii++) {
      interval* itv = (interval*)itvs->GetIntervals()->At(ii);
      KVPIDIntervalPainter* pid = (KVPIDIntervalPainter*)fItvPaint.FindObject(Form("%d_%d", itv->GetZ(), itv->GetA()));
      delete_painter_from_painter_list(pid);
   }
   itvs->GetIntervals()->Clear("all");
}

void KVItvFinderDialog::LinearizeHisto(int nbins)
{
   Double_t zmin  = ((KVIDentifier*)fGrid->GetIdentifiers()->First())->GetPID() - 1.0;
   Double_t zmax = 0;

   for (int iz = 1; iz < fGrid->GetIdentifiers()->GetSize() + 1; iz++)  {
      KVIDentifier* tmp = (KVIDentifier*)fGrid->GetIdentifiers()->At(iz);
      if (tmp && tmp->GetPID() > zmax) zmax = tmp->GetPID();
   }

   Int_t zbins = (Int_t)(zmax - zmin) * nbins;

   fLinearHisto = new TH1F("fLinearHisto", "fLinearHisto", zbins, zmin, zmax);

   fGrid->SetOnlyZId();

   // use multi-threading capacities
   auto available_cpu = WITH_MULTICORE_CPU;
   Int_t xbins_per_cpu = fHisto->GetNbinsX() / available_cpu;

   std::vector<std::thread> jobs; // threads to do the work

   // to clean up copies of grid
   KVList grid_copies;

   // do not add copies of grid to ID grid manager
   auto save_auto_add = KVIDGraph::GetAutoAdd();
   KVIDGraph::SetAutoAdd(false);

   std::cout << "Will run " << available_cpu << " threads, each for " << xbins_per_cpu << " bins in X" << std::endl;

   int nthreads = available_cpu;
   // join threads
   std::cout << "Histo linearization using " << nthreads << " threads..." << std::endl;

   for (int job = 0; job < available_cpu; ++job) {
      auto imin = 1 + job * xbins_per_cpu;
      auto imax = (job + 1) * xbins_per_cpu;
      if (job == available_cpu - 1) imax = fHisto->GetNbinsX();

      // make new copy of grid
      auto grid_copy = new KVIDZAFromZGrid(*fGrid);
      grid_copy->Initialize();
      grid_copies.Add(grid_copy);

      // start new thread
      jobs.push_back(std::thread([ =, &nthreads]() {

         KVIdentificationResult idr;

         for (int i = imin; i <= imax; ++i) {
            for (int j = 1; j <= fHisto->GetNbinsY(); j++) {
               Stat_t poids = fHisto->GetBinContent(i, j);
               if (poids == 0) continue;

               Axis_t x0 = fHisto->GetXaxis()->GetBinCenter(i);
               Axis_t y0 = fHisto->GetYaxis()->GetBinCenter(j);
               Axis_t wx = fHisto->GetXaxis()->GetBinWidth(i);
               Axis_t wy = fHisto->GetYaxis()->GetBinWidth(j);

               if (x0 < 4) continue;

               Double_t x, y;
               Int_t kmax = (Int_t) TMath::Min(20., poids);
               Double_t weight = (kmax == 20 ? poids / 20. : 1.);
               for (int k = 0; k < kmax; k++) {
                  x = gRandom->Uniform(x0 - .5 * wx, x0 + .5 * wx);
                  y = gRandom->Uniform(y0 - .5 * wy, y0 + .5 * wy);
                  if (grid_copy->IsIdentifiable(x, y)) {
                     grid_copy->KVIDZAGrid::Identify(x, y, &idr);
                     if (idr.HasFlag(grid_copy->GetName(), "MassID")
                           || (grid_copy->GetInfos()->FindObject("MassID") == nullptr)) {
                        Float_t PID = idr.PID;
                        fLinearHisto->Fill(PID, weight);
                     }
                  }
               }
            }
         }
         --nthreads;
         std::cout << "...remaining threads: " << nthreads << std::endl;
      }));
   }
   for (auto& j : jobs) {
      if (j.joinable()) j.join();
   }

   // reset automatic grid adding to previous state
   KVIDGraph::SetAutoAdd(save_auto_add);
}

void KVItvFinderDialog::Identify()
{
   //    KVBase::OpenContextMenu("Identify(double,double)",this);
   Identify(0.1, 0.001);
}

void KVItvFinderDialog::Identify(double sigma, double ratio)
{
   fSig = sigma;
   fRat = ratio;

   fPad->cd();
   std::unique_ptr<TList> list(fCustomView->GetSelectedObjects());
   if (!list->GetSize()) {
      ProcessIdentification(1, TMath::Min(fGrid->GetIdentifiers()->GetSize(), 25));
      for (int ii = 0; ii < fGrid->GetIntervalSets()->GetSize(); ii++) DrawInterval((interval_set*)fGrid->GetIntervalSets()->At(ii), 0);
   }
   else {
      for (int ii = 0; ii < list->GetSize(); ii++) {
         interval_set* itvs = (interval_set*) list->At(ii);
         ProcessIdentification(itvs->GetZ(), itvs->GetZ());
         DrawInterval(itvs, 0);
      }
   }

   fCanvas->Modified();
   fCanvas->Update();

}

void KVItvFinderDialog::SaveGrid()
{
   ExportToGrid();

   static TString dir(".");
   const char* filetypes[] = {
      "ID Grid files", "*.dat",
      "All files", "*",
      0, 0
   };
   TGFileInfo fi;
   fi.fFileTypes = filetypes;
   fi.fIniDir = StrDup(dir);
   new TGFileDialog(gClient->GetDefaultRoot(), gClient->GetDefaultRoot(), kFDSave, &fi);
   if (fi.fFilename) {
      TString filenam(fi.fFilename);
      if (filenam.Contains("toto")) filenam.ReplaceAll("toto", fGrid->GetName());
      if (!filenam.Contains('.')) filenam += ".dat";
      fGrid->WriteAsciiFile(filenam.Data());
   }
   dir = fi.fIniDir;
   fGrid->ReloadPIDRanges();

   fCustomView->Display(((KVIDZAFromZGrid*)fGrid)->GetIntervalSets());
   fCurrentView->RemoveAll();

   fItvPaint.Clear("all");
   DrawIntervals();
}

void KVItvFinderDialog::ExportToGrid()
{
   fGrid->ClearPIDIntervals();
   KVNumberList pids;
   interval_set* itvs = 0;
   TIter npid(fGrid->GetIntervalSets());
   while ((itvs = (interval_set*)npid())) {
      if (!itvs->GetNPID()) continue;
      pids.Add(itvs->GetZ());
   }
   fGrid->GetParameters()->SetValue("PIDRANGE", pids.AsString());

   itvs = 0;
   TIter next(fGrid->GetIntervalSets());
   while ((itvs = (interval_set*)next())) {
      if (!itvs->GetNPID()) continue;
      KVString par = Form("PIDRANGE%d", itvs->GetZ());
      KVString val = "";
      interval* itv = 0;
      TIter ni(itvs->GetIntervals());
      while ((itv = (interval*)ni())) {
         val += Form("%d:%lf,%lf,%lf|", itv->GetA(), itv->GetPIDmin(), itv->GetPID(), itv->GetPIDmax());
      }
      val.Remove(val.Length() - 1);
      fGrid->GetParameters()->SetValue(par.Data(), val.Data());
   }
}

void KVItvFinderDialog::NewInterval()
{
   std::unique_ptr<TList> list(fCustomView->GetSelectedObjects());
   if (!list->GetSize()) {
      return;
   }

   interval_set* itvs = (interval_set*)list->At(0);

   fPad->WaitPrimitive("TMarker");
   TMarker* mm = dynamic_cast<TMarker*>(fPad->GetListOfPrimitives()->Last());
   assert(mm);

   double pid = mm->GetX();
   int aa = 0;
   int iint = 0;

   if (!itvs->GetNPID()) {
      aa = itvs->GetZ() * 2;
      iint = 0;
   }
   else if (pid < ((interval*)itvs->GetIntervals()->First())->GetPID()) {
      aa = ((interval*)itvs->GetIntervals()->First())->GetA() - 1;
      iint = 0;
   }
   else if (pid > ((interval*)itvs->GetIntervals()->Last())->GetPID()) {
      aa = ((interval*)itvs->GetIntervals()->Last())->GetA() + 1;
      iint = itvs->GetNPID();
   }
   else {
      for (int ii = 1; ii < itvs->GetNPID(); ii++) {
         bool massok = false;
         if (pid > ((interval*)itvs->GetIntervals()->At(ii - 1))->GetPID() && pid < ((interval*)itvs->GetIntervals()->At(ii))->GetPID()) {
            aa = ((interval*)itvs->GetIntervals()->At(ii - 1))->GetA() + 1;
            iint = ii;
            if (aa <= ((interval*)itvs->GetIntervals()->At(ii))->GetA() - 1) massok = true;
         }
         if (aa && !massok)((interval*)itvs->GetIntervals()->At(ii))->SetA(((interval*)itvs->GetIntervals()->At(ii))->GetA() + 1);
      }
   }

   interval* itv = new interval(itvs->GetZ(), aa, mm->GetX(), mm->GetX() - 0.05, mm->GetX() + 0.05);
   itvs->GetIntervals()->AddAt(itv, iint);

   // find intervals which are now left (smaller mass) and right (higher mass) than this one
   interval* left_interval = iint > 0 ? (interval*)itvs->GetIntervals()->At(iint - 1) : nullptr;
   interval* right_interval = iint < itvs->GetIntervals()->GetEntries() - 1 ? (interval*)itvs->GetIntervals()->At(iint + 1) : nullptr;
   // find the corresponding painters
   KVPIDIntervalPainter* left_painter{nullptr}, *right_painter{nullptr};
   TIter next_painter(&fItvPaint);
   KVPIDIntervalPainter* pidpnt;
   while ((pidpnt = (KVPIDIntervalPainter*)next_painter())) {
      if (pidpnt->GetInterval() == left_interval) left_painter = pidpnt;
      else if (pidpnt->GetInterval() == right_interval) right_painter = pidpnt;
   }

   // give a different colour to each interval
   auto nitv = itvs->GetIntervals()->GetEntries();
   auto cstep = TColor::GetPalette().GetSize() / (nitv + 1);

   KVPIDIntervalPainter* dummy = new KVPIDIntervalPainter(itv, fLinearHisto, TColor::GetPalette()[cstep * (iint + 1)],
         left_painter);
   // set up links between painters
   if (right_painter) {
      right_painter->set_left_interval(dummy);
      dummy->set_right_interval(right_painter);
   }
   fPad->cd();
   dummy->Draw();
   dummy->Connect("IntMod()", "KVItvFinderDialog", this, "UpdatePIDList()");
   dummy->SetDisplayLabel(1);
   dummy->SetCanvas(fCanvas);
   fItvPaint.Add(dummy);

   delete mm;

   fCurrentView->Display(itvs->GetIntervals());
   fItvPaint.Execute("Update", "");

   fCanvas->Modified();
   fCanvas->Update();
}

void KVItvFinderDialog::NewIntervalSet()
{
   if (fGrid->GetIntervalSets()->GetSize() == 0) fNextIntervalZ = 1;
   else fNextIntervalZ = ((interval_set*)fGrid->GetIntervalSets()->Last())->GetZ() + 1;
   //   KVBase::OpenContextMenu("SetNextIntervalZ()",this);
   fGrid->GetIntervalSets()->Add(new interval_set(fNextIntervalZ, KVIDZAFromZGrid::kIntType));
   UpdateLists();
}

void KVItvFinderDialog::RemoveInterval()
{
   std::unique_ptr<TList> list(fCustomView->GetSelectedObjects());
   Int_t nSelected = list->GetSize();
   interval_set* itvs = 0;
   if (nSelected == 1) {
      itvs = (interval_set*)list->At(0);
      list.reset(fCurrentView->GetSelectedObjects());
      nSelected = list->GetSize();
      if (nSelected >= 1) {
         for (int ii = 0; ii < nSelected; ii++) {
            interval* itv = (interval*) list->At(ii);
            KVPIDIntervalPainter* pid = (KVPIDIntervalPainter*)fItvPaint.FindObject(Form("%d_%d", itv->GetZ(), itv->GetA()));
            itvs->GetIntervals()->Remove(itv);
            delete_painter_from_painter_list(pid);
         }
         fCurrentView->Display(itvs->GetIntervals());
         fCanvas->Modified();
         fCanvas->Update();
      }
      else ClearInterval(itvs);
   }
   else if (nSelected > 1) {
      for (int ii = 0; ii < nSelected; ii++) {
         itvs = (interval_set*)list->At(ii);
         ClearInterval(itvs);
      }
   }
}

void KVItvFinderDialog::MassesUp()
{
   std::unique_ptr<TList> list(fCustomView->GetSelectedObjects());
   Int_t nSelected = list->GetSize();
   interval_set* itvs = 0;
   if (nSelected == 1) {
      itvs = (interval_set*)list->At(0);

      list.reset(fCurrentView->GetSelectedObjects());
      nSelected = list->GetSize();

      if (nSelected == 1) {
         interval* itv = (interval*) list->At(0);
         itv->SetA(itv->GetA() + 1);
         fItvPaint.Execute("Update", "");
         //            fCurrentView->Display(itvs->GetIntervals());
         fCanvas->Modified();
         fCanvas->Update();
      }
      else {
         KVList* ll = itvs->GetIntervals();
         nSelected = ll->GetSize();
         if (nSelected >= 1) {
            for (int ii = 0; ii < nSelected; ii++) {
               interval* itv = (interval*) ll->At(ii);
               itv->SetA(itv->GetA() + 1);
            }
            fItvPaint.Execute("Update", "");
            //                fCurrentView->Display(itvs->GetIntervals());
            fCanvas->Modified();
            fCanvas->Update();
         }
      }
   }
}

void KVItvFinderDialog::MassesDown()
{
   std::unique_ptr<TList> list(fCustomView->GetSelectedObjects());
   Int_t nSelected = list->GetSize();
   interval_set* itvs = 0;
   if (nSelected == 1) {
      itvs = (interval_set*)list->At(0);
      list.reset(fCurrentView->GetSelectedObjects());
      nSelected = list->GetSize();

      if (nSelected == 1) {
         interval* itv = (interval*) list->At(0);
         itv->SetA(itv->GetA() - 1);
         fItvPaint.Execute("Update", "");
         fCanvas->Modified();
         fCanvas->Update();
      }
      else {

         KVList* ll = itvs->GetIntervals();
         nSelected = ll->GetSize();
         if (nSelected >= 1) {
            for (int ii = 0; ii < nSelected; ii++) {
               interval* itv = (interval*) ll->At(ii);
               itv->SetA(itv->GetA() - 1);
            }
            fItvPaint.Execute("Update", "");
            fCanvas->Modified();
            fCanvas->Update();
         }
      }
   }
}

void KVItvFinderDialog::UpdateLists()
{
   fCustomView->Display(((KVIDZAFromZGrid*)fGrid)->GetIntervalSets());
   std::unique_ptr<TList> list(fCustomView->GetSelectedObjects());
   Int_t nSelected = list->GetSize();
   interval_set* itvs = 0;
   if (nSelected == 1) {
      itvs = (interval_set*)list->At(0);
      fCurrentView->Display(itvs->GetIntervals());
   }
}

void KVItvFinderDialog::TestIdent()
{
   //fGrid->SetOnlyZId(0);
   fGrid->Initialize();
   ExportToGrid();

   fGrid->ReloadPIDRanges();

   fCustomView->Display(((KVIDZAFromZGrid*)fGrid)->GetIntervalSets());
   fCurrentView->RemoveAll();

   fItvPaint.Clear("all");
   DrawIntervals();

   new KVTestIDGridDialog(gClient->GetDefaultRoot(), gClient->GetDefaultRoot(), 10, 10, fGrid, fHisto);
}

void KVItvFinderDialog::SetLogy()
{
   fCanvas->SetLogy(!fCanvas->GetLogy());
   fCanvas->Modified();
   fCanvas->Update();
}

void KVItvFinderDialog::UnzoomHisto()
{
   fItvPaint.Execute("SetDisplayLabel", "0");
   fLinearHisto->GetXaxis()->UnZoom();
   fCanvas->Modified();
   fCanvas->Update();
}


void KVItvFinderDialog::FindPIDIntervals(Int_t zz)
{
   interval_set* itvs = fGrid->GetIntervalSet(zz);
   if (!itvs) {
      itvs = new interval_set(zz, KVIDZAFromZGrid::kIntType);
      fGrid->GetIntervalSets()->Add(itvs);
   }
   else ClearInterval(itvs);


   if (zz == 1) fLinearHisto->SetAxisRange(0.9, zz + 0.5, "X");
   else      fLinearHisto->SetAxisRange(zz - 0.5, zz + 0.5, "X");

   int nfound = fSpectrum.Search(fLinearHisto, fSig, "goff", ((zz == 2) ? 0.1 * fRat : fRat));

#if ROOT_VERSION_CODE > ROOT_VERSION(5,99,01)
   Double_t* xpeaks = fSpectrum.GetPositionX();
//    Double_t* ypeaks = fSpectrum.GetPositionY();
#else
   Float_t* xpeaks = fSpectrum.GetPositionX();
//    Float_t* ypeaks = fSpectrum.GetPositionY();
#endif

   nfound = TMath::Min(fNpeaks[zz - 1], nfound);

   int idx[nfound];
   TMath::Sort(nfound, xpeaks, idx, 0);

   int zrefs[] = {1, 4, 7, 9, 11, 12, 15, 16, 19, 21, 23, 25, 27, 29, 31, 34, 35, 38, 40, 42, 44, 47, 49, 51, 53};
   int zref = zrefs[zz - 1];

   int idref = -1;
   for (int p = 0; p < nfound; p++) {
      if (abs(xpeaks[idx[p]] - xpeaks[0]) < .0001) idref = p;
   }
   Info("FindPIDIntervals", "Z=%d : idref = %d ", zz, idref);

   for (int p = 0; p < nfound; p++) {
//        Info("FindPIDIntervals","Z=%d : (%.2lf %.2lf) (%d %d) ",zz,xpeaks[p],ypeaks[p],idx[p],p);
      double pid = xpeaks[idx[p]];//ff->GetParameter(3 * ii + 1);
      itvs->add(zref + (p - idref), pid, pid - 0.05, pid + 0.05);
   }


}

Double_t KVItvFinderDialog::fpeaks(Double_t* x, Double_t* par)
{
   Double_t result = 0;
   int np = par[0];
   for (Int_t p = 0; p < np; p++) {
      Double_t norm  = par[3 * p + 3];
      Double_t mean  = par[3 * p + 1];
      Double_t sigma = par[3 * p + 2];
      result += norm * TMath::Gaus(x[0], mean, sigma);
   }
   return result;
}
void KVItvFinderDialog::ProcessIdentification(Int_t zmin, Int_t zmax)
{

   if (zmin < 0) zmin = ((KVIDentifier*)fGrid->GetIdentifiers()->First())->GetZ();
   if (zmax < 0) zmax = ((KVIDentifier*)fGrid->GetIdentifiers()->Last())->GetZ();

   for (int z = zmin; z <= zmax; z++) FindPIDIntervals(z);

}








