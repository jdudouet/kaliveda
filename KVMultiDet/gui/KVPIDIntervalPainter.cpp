//Created by KVClassFactory on Mon Jan 23 14:29:32 2017
//Author: Diego Gruyer

#include "KVPIDIntervalPainter.h"
#include "TCanvas.h"
#include "TFrame.h"
#include "TVirtualPad.h"
#include "TVirtualX.h"
#include "TPoint.h"
#include "TROOT.h"

ClassImp(KVPIDIntervalPainter)

void KVPIDIntervalPainter::Draw(Option_t*)
{
   fMarker.Draw();
   fLine1.Draw();
   fLine2.Draw();
}

void KVPIDIntervalPainter::HighLight(bool hi)
{
   fMarker.HighLight(hi);
   fLine1.HighLight(hi);
   fLine2.HighLight(hi);
}

void KVPIDIntervalPainter::Update()
{
   // updates the name & title from the interval's current Z & A
   //
   // updates the positions of the marker & lines from current values
   SetName(Form("%d_%d", fInterval->GetZ(), fInterval->GetA()));
   SetTitle(Form("%d_%d", fInterval->GetZ(), fInterval->GetA()));
   fZ = fInterval->GetZ();
   fA = fInterval->GetA();
   fMarker.Update(fZ, fA, fInterval->GetPID());
   fLine1.Update(fInterval->GetPIDmin());
   fLine2.Update(fInterval->GetPIDmax());
}

KVPIDIntervalPainter::pid_marker::pid_marker(KVPIDIntervalPainter* P, Color_t itv_col)
   : TMarker(P->GetInterval()->GetPID(), P->GetHisto()->GetBinContent(P->GetHisto()->FindBin(P->GetInterval()->GetPID())), 23),
     parent{P},
     fLabel{GetX(), GetY(), Form(" (%d,%d)", P->GetInterval()->GetZ(), P->GetInterval()->GetA())},
     def_color{itv_col},
     pid{P->GetInterval()->GetPID()}
{
   // Create marker for interval set.
   //
   // Marker coordinates are initialised from PID and contents of linearised histogram.
   //
   // Label is initialised with the (Z,A) of the interval
   fLabel.SetTextAlign(12);
   fLabel.SetTextFont(42);
   fLabel.SetTextSize(0.0421941);

   SetMarkerColor(def_color);
   fLabel.SetTextColor(def_color);
}

void KVPIDIntervalPainter::pid_marker::ExecuteEvent(Int_t event, Int_t px, Int_t py)
{
   if (pid < parent->GetHistoXAxisLowerLimit() || pid > parent->GetHistoXAxisUpperLimit()) {
      // we only handle markers which are currently displayed in the visible part of the histogram
      return;
   }

   if (!gPad) return;

   TPoint p;
   static Int_t pxold, pyold;
   static Bool_t ndcsav;
   Double_t dpx, dpy, xp1, yp1;
   Bool_t opaque  = gPad->OpaqueMoving();

   if (!gPad->IsEditable()) return;

   // force marker to stay on top of histogram bin
   double fixed_Y = parent->GetHisto()->GetBinContent(parent->GetHisto()->FindBin(GetX()));

   switch (event) {

      case kButton1Down:
         ndcsav = TestBit(kMarkerNDC);
         if (!opaque) {
            gVirtualX->SetTextColor(-1);  // invalidate current text color (use xor mode)
            TAttMarker::Modify();  //Change marker attributes only if necessary
         }
      // No break !!!

      case kMouseMotion:
         pxold = px;
         pyold = py;
         gPad->SetCursor(kMove);
         break;

      case kButton1Motion:
         p.fX = pxold;
         p.fY = pyold;
         if (!opaque) gVirtualX->DrawPolyMarker(1, &p);
         p.fX = px;
         p.fY = py;
         if (!opaque) gVirtualX->DrawPolyMarker(1, &p);
         pxold = px;
         pyold = py;
         if (opaque) {
            if (ndcsav) SetNDC(kFALSE);
            auto x = gPad->PadtoX(gPad->AbsPixeltoX(px));
            if (CheckPosition(x)) SetX(x);
            SetY(fixed_Y);
            gPad->ShowGuidelines(this, event, 'i', true);
            gPad->Modified(kTRUE);
            gPad->Update();
         }
         break;

      case kButton1Up:
         if (opaque) {
            if (ndcsav && !TestBit(kMarkerNDC)) {
               auto x = (fX - gPad->GetX1()) / (gPad->GetX2() - gPad->GetX1());
               if (CheckPosition(x)) SetX(x);
               SetY(fixed_Y);
               SetNDC();
            }
            gPad->ShowGuidelines(this, event);
         }
         else {
            if (TestBit(kMarkerNDC)) {
               dpx  = gPad->GetX2() - gPad->GetX1();
               dpy  = gPad->GetY2() - gPad->GetY1();
               xp1  = gPad->GetX1();
               yp1  = gPad->GetY1();
               auto x = (gPad->AbsPixeltoX(pxold) - xp1) / dpx;
               if (CheckPosition(x)) SetX(x);
               SetY(fixed_Y);
            }
            else {
               auto x = gPad->PadtoX(gPad->AbsPixeltoX(px));
               if (CheckPosition(x)) SetX(x);
               SetY(fixed_Y);
            }
            gPad->Modified(kTRUE);
            gPad->Update();
            gVirtualX->SetTextColor(-1);
         }
         break;
   }

   if (GetX() != pid) {
      // marker has been moved
      pid = GetX();
      // update PID value in mass interval
      parent->GetInterval()->SetPID(pid);
      // signal that something has changed
      parent->IntMod();
   }
}

void KVPIDIntervalPainter::pid_marker::Paint(Option_t* option)
{
   // overridden Paint() method, to display label next to marker
   //
   // when fDisplayLabel=true, we draw the label defined for the PID range just next to the marker

   TMarker::Paint(option);
   if (fDrawLabel) {
      fLabel.SetX(GetX());
      fLabel.SetY(GetY());
      fLabel.Paint(option);
   }
}

void KVPIDIntervalPainter::pid_marker::HighLight(bool hi)
{
   fHighlight = hi;
   if (hi) {
      SetMarkerSize(1);
      SetMarkerColor(kBlack);
      fLabel.SetTextColor(kBlack);
   }
   else {
      SetMarkerSize(1);
      SetMarkerColor(def_color);
      fLabel.SetTextColor(def_color);
   }
}

void KVPIDIntervalPainter::pid_marker::Update(int z, int a, double pid)
{
   // update Z, A, and position of PID marker
   double fixed_Y = parent->GetHisto()->GetBinContent(parent->GetHisto()->FindBin(pid));
   SetX(pid);
   SetY(fixed_Y);
   fLabel.SetText(GetX(), GetY(), Form(" (%d,%d)", z, a));
}

void KVPIDIntervalPainter::pid_line::ExecuteEvent(Int_t event, Int_t px, Int_t py)
{
   if (pid < parent->GetHistoXAxisLowerLimit() || pid > parent->GetHistoXAxisUpperLimit())
      return;

   if (!gPad) return;

   double fixed_Y1 = parent->GetCanvas()->GetFrame()->GetY1();
   double fixed_Y2 = parent->GetCanvas()->GetFrame()->GetY2();
   if (parent->GetCanvas()->GetLogy()) {
      fixed_Y1 = TMath::Exp(fixed_Y1 * TMath::Log(10));
      fixed_Y2 = TMath::Exp(fixed_Y2 * TMath::Log(10));
   }

   Int_t kMaxDiff = 20;
   static Int_t d1, d2, px1, px2, py1, py2;
   static Int_t pxold, pyold, px1old, py1old, px2old, py2old;
   static Double_t oldX1, oldY1, oldX2, oldY2;
   static Bool_t p1, p2, pL, ndcsav;
   Double_t dpx, dpy, xp1, yp1;
   Int_t dx, dy;

   Bool_t opaque  = gPad->OpaqueMoving();

   if (!gPad->IsEditable()) return;

   switch (event) {

      case kArrowKeyPress:
      case kButton1Down:
         oldX1 = GetX1();
         oldY1 = GetY1();
         oldX2 = GetX2();
         oldY2 = GetY2();
         ndcsav = TestBit(kLineNDC);
         if (!opaque) {
            gVirtualX->SetLineColor(-1);
            TAttLine::Modify();  //Change line attributes only if necessary
         }

      // No break !!!

      case kMouseMotion:

         if (TestBit(kLineNDC)) {
            px1 = gPad->UtoPixel(GetX1());
            py1 = gPad->VtoPixel(GetY1());
            px2 = gPad->UtoPixel(GetX2());
            py2 = gPad->VtoPixel(GetY2());
         }
         else {
            px1 = gPad->XtoAbsPixel(gPad->XtoPad(GetX1()));
            py1 = gPad->YtoAbsPixel(gPad->YtoPad(GetY1()));
            px2 = gPad->XtoAbsPixel(gPad->XtoPad(GetX2()));
            py2 = gPad->YtoAbsPixel(gPad->YtoPad(GetY2()));
         }
         p1 = p2 = pL = kFALSE;

         d1  = abs(px1 - px) + abs(py1 - py); //simply take sum of pixels differences
         if (d1 < kMaxDiff) { //*-*================>OK take point number 1
            px1old = px1;
            py1old = py1;
            p1 = kTRUE;
            gPad->SetCursor(kPointer);
            return;
         }
         d2  = abs(px2 - px) + abs(py2 - py); //simply take sum of pixels differences
         if (d2 < kMaxDiff) { //*-*================>OK take point number 2
            px2old = px2;
            py2old = py2;
            p2 = kTRUE;
            gPad->SetCursor(kPointer);
            return;
         }

         pL = kTRUE;
         pxold = px;
         pyold = py;
         gPad->SetCursor(kMove);

         break;

      case kArrowKeyRelease:
      case kButton1Motion:

         if (p1) {
            if (!opaque) {
               gVirtualX->DrawLine(px1old, py1old, px2, py2);
               gVirtualX->DrawLine(px, py, px2, py2);
            }
            else {
               if (ndcsav) {
                  SetNDC(kFALSE);
                  auto x = gPad->GetX1() + oldX2 * (gPad->GetX2() - gPad->GetX1());
                  if (CheckPosition(x)) SetX(x);
                  SetY2(fixed_Y2);
               }
               //SetX1(GetX2());
               SetY1(fixed_Y1);
            }
            px1old = px;
            py1old = py;
         }
         if (p2) {
            if (!opaque) {
               gVirtualX->DrawLine(px1, py1, px2old, py2old);
               gVirtualX->DrawLine(px1, py1, px, py);
            }
            else {
               if (ndcsav) {
                  SetNDC(kFALSE);
                  auto x = gPad->GetX1() + oldX1 * (gPad->GetX2() - gPad->GetX1());
                  if (CheckPosition(x)) SetX(x);
                  SetY1(fixed_Y1);
               }
               //SetX2(GetX1());
               SetY2(fixed_Y2);
            }
            px2old = px;
            py2old = py;
         }
         if (pL) {
            if (!opaque) gVirtualX->DrawLine(px1, py1, px2, py2);
            dx = px - pxold;
            dy = py - pyold;
            px1 += dx;
            py1 += dy;
            px2 += dx;
            py2 += dy;
            if (!opaque) gVirtualX->DrawLine(px1, py1, px2, py2);
            pxold = px;
            pyold = py;
            if (opaque) {
               if (ndcsav) SetNDC(kFALSE);
               auto x = gPad->AbsPixeltoX(px1);
               if (CheckPosition(x)) SetX(x);
               SetY1(fixed_Y1);
               //SetX2(GetX1());
               SetY2(fixed_Y2);
            }
         }
         if (opaque) {
            if (p1) {
               //check in which corner the BBox is edited
               if (GetX1() > GetX2()) {
                  if (GetY1() > GetY2())
                     gPad->ShowGuidelines(this, event, '2', true);
                  else
                     gPad->ShowGuidelines(this, event, '3', true);
               }
               else {
                  if (GetY1() > GetY2())
                     gPad->ShowGuidelines(this, event, '1', true);
                  else
                     gPad->ShowGuidelines(this, event, '4', true);
               }
            }
            if (p2) {
               //check in which corner the BBox is edited
               if (GetX1() > GetX2()) {
                  if (GetY1() > GetY2())
                     gPad->ShowGuidelines(this, event, '4', true);
                  else
                     gPad->ShowGuidelines(this, event, '1', true);
               }
               else {
                  if (GetY1() > GetY2())
                     gPad->ShowGuidelines(this, event, '3', true);
                  else
                     gPad->ShowGuidelines(this, event, '2', true);
               }
            }
            if (pL) {
               gPad->ShowGuidelines(this, event, 'i', true);
            }
            gPad->Modified(kTRUE);
            gPad->Update();
         }
         break;

      case kButton1Up:

         if (gROOT->IsEscaped()) {
            gROOT->SetEscape(kFALSE);
            if (opaque) {
               SetX(oldX1);
               SetY1(fixed_Y1);
               //SetX2(GetX1());
               SetY2(fixed_Y2);
               gPad->Modified(kTRUE);
               gPad->Update();
            }
            break;
         }
         if (opaque) {
            if (ndcsav && !TestBit(kLineNDC)) {
               auto x = ((GetX1() - gPad->GetX1()) / (gPad->GetX2() - gPad->GetX1()));
               if (CheckPosition(x)) SetX(x);
               //SetX2(GetX1());
               SetY1(fixed_Y1);
               SetY2(fixed_Y2);
               SetNDC();
            }
            gPad->ShowGuidelines(this, event);
         }
         else {
            if (TestBit(kLineNDC)) {
               dpx  = gPad->GetX2() - gPad->GetX1();
               dpy  = gPad->GetY2() - gPad->GetY1();
               xp1  = gPad->GetX1();
               yp1  = gPad->GetY1();
               if (p1) {
                  auto x = (gPad->AbsPixeltoX(px) - xp1) / dpx;
                  if (CheckPosition(x)) SetX(x);
                  //SetX2(GetX1());
                  SetY1(fixed_Y1);
               }
               if (p2) {
                  auto x = ((gPad->AbsPixeltoX(px) - xp1) / dpx);
                  if (CheckPosition(x)) SetX(x);
                  //SetX1(GetX2());
                  SetY2(fixed_Y2);
               }
               if (pL) {
                  auto x = ((gPad->AbsPixeltoX(px1) - xp1) / dpx);
                  if (CheckPosition(x)) SetX(x);
                  SetY1(fixed_Y1);
                  //SetX2(GetX1());
                  SetY2(fixed_Y2);
               }
            }
            else {
               if (p1) {
                  auto x = (gPad->PadtoX(gPad->AbsPixeltoX(px)));
                  if (CheckPosition(x)) SetX(x);
                  //SetX2(GetX1());
                  SetY1(fixed_Y1);
               }
               if (p2) {
                  auto x = (gPad->PadtoX(gPad->AbsPixeltoX(px)));
                  if (CheckPosition(x)) SetX(x);
                  //SetX1(GetX2());
                  SetY2(fixed_Y2);
               }
               if (pL) {
                  auto x = (gPad->PadtoX(gPad->AbsPixeltoX(px1)));
                  if (CheckPosition(x)) SetX(x);
                  SetY1(fixed_Y1);
                  //SetX2(GetX1());
                  SetY2(fixed_Y2);
               }
            }
            if (TestBit(kVertical)) {
               if (p1) SetX2(GetX1());
               if (p2) SetX1(GetX2());
            }
            if (TestBit(kHorizontal)) {
               if (p1) SetY2(fixed_Y1);
               if (p2) SetY1(fixed_Y2);
            }
            gPad->Modified(kTRUE);
            gPad->Update();
            if (!opaque) gVirtualX->SetLineColor(-1);
         }
         break;

      case kButton1Locate:

         ExecuteEvent(kButton1Down, px, py);
         while (1) {
            px = py = 0;
            event = gVirtualX->RequestLocator(1, 1, px, py);

            ExecuteEvent(kButton1Motion, px, py);

            if (event != -1) {                     // button is released
               ExecuteEvent(kButton1Up, px, py);
               return;
            }
         }
   }
}

void KVPIDIntervalPainter::pid_line::HighLight(bool hi)
{
   fHighlight = hi;
   if (hi) {
      SetLineWidth(3);
      SetLineColor(kBlack);
   }
   else {
      SetLineWidth(2);
      SetLineColor(def_color);
   }
}

void KVPIDIntervalPainter::pid_line::Paint(Option_t* option)
{
   if (pid < parent->GetHistoXAxisLowerLimit() || pid > parent->GetHistoXAxisUpperLimit())
      return;

   double fixed_Y1 = parent->GetCanvas()->GetFrame()->GetY1();
   double fixed_Y2 = parent->GetCanvas()->GetFrame()->GetY2();
   if (parent->GetCanvas()->GetLogy()) {
      fixed_Y1 = TMath::Exp(fixed_Y1 * TMath::Log(10));
      fixed_Y2 = TMath::Exp(fixed_Y2 * TMath::Log(10));
   }
   SetY1(fixed_Y1);
   SetY2(fixed_Y2);
   TLine::Paint(option);
}

bool KVPIDIntervalPainter::pid_line::CheckPosition(double x)
{
   // make sure that lower limit < PID marker < upper limit
   //
   // also, both limits must be > than the upper limit of the PID interval to our left (if it exists),
   // and also < the lower limit of the PID interval to our right (if it exists)

   bool ok;
   if (type == limit_t::lower) {
      ok = (x < parent->GetPIDPosition()) && (x < parent->GetPIDIntervalUpperLimit());
   }
   else {
      ok = (x > parent->GetPIDPosition()) && (x > parent->GetPIDIntervalLowerLimit());
   }
   if (!ok) return false;
   bool ok_left = parent->get_left_interval() ?
                  x > parent->get_left_interval()->GetPIDIntervalUpperLimit() : true;
   bool ok_right = parent->get_right_interval() ?
                   x < parent->get_right_interval()->GetPIDIntervalLowerLimit() : true;
   return (ok && ok_left && ok_right);
}
