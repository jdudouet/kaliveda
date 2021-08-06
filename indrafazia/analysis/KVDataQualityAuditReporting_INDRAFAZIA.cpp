#include "KVDataQualityAuditReporting_INDRAFAZIA.h"
#include "KVINDRA.h"
#include "KVFAZIA.h"

#include <KVFAZIADetector.h>
#include <KVGeoDNTrajectory.h>
#include <TColor.h>
#include <TLatex.h>
#include <TLegend.h>
#include <TMarker.h>
#include "TLegend.h"

ClassImp(KVDataQualityAuditReporting_INDRAFAZIA)

void KVDataQualityAuditReporting_INDRAFAZIA::make_canvas(canvas_t style)
{
   // Make an A4-size canvas
   Double_t w = 297 * 6;
   Double_t h = 210 * 6;
   if (style == canvas_t::kPortrait) std::swap(w, h);
   myCanvas = ::new TCanvas("c", "c", w, h);
   myCanvas->SetWindowSize(w + (w - myCanvas->GetWw()), h + (h - myCanvas->GetWh()));
}

void KVDataQualityAuditReporting_INDRAFAZIA::make_fazia_map(double theta_bin)
{
   // sort fazia telescopes into bins of theta

   TIter itdet(gFazia->GetDetectors());
   KVDetector* det;

   while ((det = (KVDetector*)itdet())) {
      if (det->IsLabelled("SI1")) {
         int bin = TMath::Nint(det->GetTheta() * (1 / theta_bin));
         fazia_map[bin].push_back(det);
      }
   }
   // Now sort detectors in vectors in order of increasing theta
   for (auto& p : fazia_map) {
      std::sort(std::begin(p.second), std::end(p.second), [](KVDetector * a, KVDetector * b) {
         return a->GetTheta() < b->GetTheta();
      });
   }
}

void KVDataQualityAuditReporting_INDRAFAZIA::do_report()
{
   if (!gFazia && !gIndra) {
      Error("do_report", "No detector defined!\nFirst do:\n\n\tKVDataSetManager dsm\n\tdsm.Init()\n\tKVMultiDetArray::MakeMultiDetector(\"INDRAFAZIA.E789\",[run])\n\nwhere [run] is an appropriate run number (corresponding to reaction being audited)");
      return;
   }

   make_fazia_map(0.4);

   make_canvas();

   TString pdf_file = Form("E789_DataQualityAudit_%s.pdf", fAudit->GetName());
   auto first_page = pdf_file + "(";
   auto last_page = pdf_file + ")";
   current_page = first_page;
   // INDRA Z identification bilan
   int nx(2), ny(3);
   myCanvas->Divide(nx, ny);
   int pad = 1;
   std::vector<TString> indra_id_types = {"SI_CSI", "CSI_R_L"};
   for (auto id : indra_id_types) {
      for (int ring = 6; ring <= 17; ++ring) {
         if (id == "SI_CSI" && ring > 9) break;

         myCanvas->cd(pad)->SetFillStyle(4000);//transparent pad
         INDRA_ring_reporting_Z(ring, id);
         ++pad;
         if (pad > nx * ny) {
            draw_sidebar_legend();
            myCanvas->Print(current_page, Form("Title:INDRA Z %s ID Quality by Ring", id.Data()));
            if (current_page == first_page) current_page = pdf_file;
            myCanvas->Clear();
            myCanvas->Divide(nx, ny);
            pad = 1;
         }
      }
      if (pad != 1) {
         draw_sidebar_legend();
         myCanvas->Print(current_page, Form("Title:INDRA Z %s ID Quality by Ring", id.Data()));
         if (current_page == first_page) current_page = pdf_file;
         myCanvas->Clear();
         myCanvas->Divide(nx, ny);
         pad = 1;
      }
      nx = 4;
      ny = 3;
      myCanvas->Clear();
      myCanvas->Divide(nx, ny);
      pad = 1;
      for (int ring = 6; ring <= 17; ++ring) {
         if (id == "SI_CSI") {
            if (ring < 8) continue; // no isotopes for 6/7
            if (ring > 9) break; // no si-csi after ring 9
         }

         INDRA_ring_mean_A_vs_Z(ring, id, pad, nx, ny);
      }
      if (pad != 1) {
         myCanvas->Print(current_page, Form("Title:INDRA <A> vs Z %s by Ring", id.Data()));
         myCanvas->Clear();
         myCanvas->Divide(nx, ny);
         pad = 1;
      }
      for (int ring = 6; ring <= 9; ++ring) { // no calibrations for ring>9
         INDRA_ring_Z_threshold_vs_Z(ring, id, pad, nx, ny);
      }
      if (pad != 1) {
         myCanvas->Print(current_page, Form("Title:INDRA Z thresh. vs Z %s by Ring", id.Data()));
         myCanvas->Clear();
         myCanvas->Divide(nx, ny);
         pad = 1;
      }
   }

   // FAZIA Z identification bilan
   std::vector<TString> fazia_id_types = {"SI1", "SI1_SI2", "SI2_CSI", "CSI"};
   nx = 4;
   ny = 3;
   for (auto id : fazia_id_types) {
      myCanvas->Clear();
      myCanvas->Divide(nx, ny);
      pad = 1;
      int group_num = 1;
      for (auto& p : fazia_map) {
         myCanvas->cd(pad);

         FAZIA_group_reporting_Z(group_num, p.second, id);
         ++group_num;
         ++pad;
         if (pad > nx * ny) {
            draw_sidebar_legend_fazia();
            myCanvas->Print(pdf_file, Form("Title:FAZIA Z %s ID Quality by Group", id.Data()));
            myCanvas->Clear();
            myCanvas->Divide(nx, ny);
            pad = 1;
         }
      }
      if (pad != 1) {
         draw_sidebar_legend_fazia();
         myCanvas->Print(pdf_file, Form("Title:FAZIA Z %s ID Quality by Group", id.Data()));
      }

      myCanvas->Clear();
      myCanvas->Divide(nx, ny);
      pad = 1;
      group_num = 1;
      for (auto& p : fazia_map) {
         myCanvas->cd(pad);

         FAZIA_group_mean_A_vs_Z(group_num, p.second, id);
         ++group_num;
         ++pad;
         if (pad > nx * ny) {
            myCanvas->Print(pdf_file, Form("Title:FAZIA <A> vs Z %s by Group", id.Data()));
            myCanvas->Clear();
            myCanvas->Divide(nx, ny);
            pad = 1;
         }
      }
      if (pad != 1) myCanvas->Print(pdf_file, Form("Title:FAZIA <A> vs Z %s by Group", id.Data()));

      myCanvas->Clear();
      myCanvas->Divide(nx, ny);
      pad = 1;
      group_num = 1;
      for (auto& p : fazia_map) {
         myCanvas->cd(pad);

         FAZIA_group_Z_threshold_vs_Z(group_num, p.second, id);
         ++group_num;
         ++pad;
         if (pad > nx * ny) {
            myCanvas->Print(pdf_file, Form("Title:FAZIA Z thresh. vs Z %s by Group", id.Data()));
            myCanvas->Clear();
            myCanvas->Divide(nx, ny);
            pad = 1;
         }
      }
      if (pad != 1) myCanvas->Print(pdf_file, Form("Title:FAZIA Z thresh. vs Z %s by Group", id.Data()));

      myCanvas->Clear();
      myCanvas->Divide(nx, ny);
      pad = 1;
      group_num = 1;
      for (auto& p : fazia_map) {
         myCanvas->cd(pad);

         FAZIA_group_A_threshold_vs_Z(group_num, p.second, id);
         ++group_num;
         ++pad;
         if (pad > nx * ny) {
            myCanvas->Print(pdf_file, Form("Title:FAZIA A thresh. vs Z %s by Group", id.Data()));
            myCanvas->Clear();
            myCanvas->Divide(nx, ny);
            pad = 1;
         }
      }
      if (pad != 1) myCanvas->Print(pdf_file, Form("Title:FAZIA A thresh. vs Z %s by Group", id.Data()));
   }
   myCanvas->Clear();
   myCanvas->Print(last_page, "Title:Last page");
}

void KVDataQualityAuditReporting_INDRAFAZIA::INDRA_ring_reporting_Z(int ring, const TString& idtype)
{
   if (!gIndra) {
      Error("INDRA_ring_reporting", "You need to build INDRA first...");
      return;
   }
   TList tels;
   for (int mod = 1; mod <= 24; ++mod) {
      TString name = Form("%s_%02d%02d", idtype.Data(), ring, mod);
      auto tel = gIndra->GetIDTelescope(name);
      if (tel) tels.Add(tel);
   }

   auto get_module_number = [](const KVIDTelescope * idt) {
      return (int)dynamic_cast<KVINDRADetector*>(idt->GetDetector(1))->GetModuleNumber();
   };
   auto zmean = fReport.get_mean_Z_for_telescopes(&tels, get_module_number);
   auto zmax = fReport.get_max_Z_for_telescopes(&tels, get_module_number, 24);
   auto zmax_iso = fReport.get_max_Z_with_isotopes_for_telescopes(&tels, get_module_number, 30);
   zmax_iso->SetMarkerColor(kOrange - 2);
   auto zmin = fReport.get_min_Z_for_telescopes(&tels, get_module_number, 25);
   TMultiGraph* mg = ::new TMultiGraph;
   mg->SetTitle(Form("Ring %d %s Min/Mean/Max Z vs. Module", ring, idtype.Data()));
   mg->Add(zmin);
   mg->Add(zmean);
   mg->Add(zmax);
   mg->Add(zmax_iso);
   mg->Draw("ap");
}
void KVDataQualityAuditReporting_INDRAFAZIA::INDRA_ring_mean_A_vs_Z(int ring, const TString& idtype, int& pad, int nx, int ny)
{
   TList tels;
   for (int mod = 1; mod <= 24; ++mod) {
      TString name = Form("%s_%02d%02d", idtype.Data(), ring, mod);
      auto tel = gIndra->GetIDTelescope(name);
      if (!tel) break;
      tels.Add(tel);
   }
   int nmods = tels.GetEntries();
   // 24 => 4 sets of 6, 16 => 2 sets of 8, 8 => 1 set of 8
   int mod_set = (nmods > 16 ? 6 : 8);
   int color_step = TColor::GetPalette().GetSize() / (mod_set + 1);
   KVIDTelescope* idt;
   nmods = 0;
   while (tels.GetEntries()) {
      TMultiGraph* mg = ::new TMultiGraph;
      int i = 1;
      while (i <= mod_set) {
         idt = (KVIDTelescope*)tels.Remove(tels.First()); // "pop" the first one in the list
         if (fAudit->HasTelescope(idt->GetName())) {
            auto gr = fReport[idt->GetName()].get_mean_isotopic_mass_by_Z();
            gr->SetMarkerColor(TColor::GetPalette()[color_step * i]);
            gr->SetMarkerStyle(markers[i - 1]);
            gr->SetLineWidth(0);
            mg->Add(gr);
         }
         ++i;
         ++nmods;
      }
      myCanvas->cd(pad);
      ++pad;
      // change title of graph => title of pad
      mg->SetTitle(Form("INDRA <A> vs. Z %s Ring %d [%d-%d]", idtype.Data(), ring, nmods - mod_set + 1, nmods));
      mg->Draw("ap");
      TLegend* leg;
      if (i > 12) {
         leg = gPad->BuildLegend(.11, .89, .61, .69);
         leg->SetNColumns(3);
      }
      else if (i > 6) {
         leg = gPad->BuildLegend(.11, .89, .61, .69);
         leg->SetNColumns(2);
      }
      else {
         leg = gPad->BuildLegend(.11, .89, .61, .69);
      }
      leg->SetBorderSize(0);
      leg->SetFillColorAlpha(kWhite, 1.00);
      if (pad > nx * ny) {
         myCanvas->Print(current_page, Form("Title:INDRA <A> vs Z %s by Ring", idtype.Data()));
         myCanvas->Clear();
         myCanvas->Divide(nx, ny);
         pad = 1;
      }
   }
}

void KVDataQualityAuditReporting_INDRAFAZIA::INDRA_ring_Z_threshold_vs_Z(int ring, const TString& idtype, int& pad, int nx, int ny)
{
   TList tels;
   for (int mod = 1; mod <= 24; ++mod) {
      TString name = Form("%s_%02d%02d", idtype.Data(), ring, mod);
      auto tel = gIndra->GetIDTelescope(name);
      if (!tel) break;
      tels.Add(tel);
   }
   int nmods = tels.GetEntries();
   // 24 => 4 sets of 6, 16 => 2 sets of 8, 8 => 1 set of 8
   int mod_set = (nmods > 16 ? 6 : 8);
   int color_step = TColor::GetPalette().GetSize() / (mod_set + 1);
   KVIDTelescope* idt;
   nmods = 0;
   while (tels.GetEntries()) {
      TMultiGraph* mg = ::new TMultiGraph;
      int i = 1;
      while (i <= mod_set) {
         idt = (KVIDTelescope*)tels.Remove(tels.First()); // "pop" the first one in the list
         if (fAudit->HasTelescope(idt->GetName())) {
            auto gr = fReport[idt->GetName()].get_element_thresholds_by_Z_mev_per_nuc(TColor::GetPalette()[color_step * i]);
            gr->SetMarkerStyle(markers[i - 1]);
            gr->SetLineWidth(0);
            mg->Add(gr);
            if (gr->GetMean(2) < 0) {
               // threshold = -1 => uncalibrated particles
               std::cout << idt->GetName() << " : particles are UNCALIBRATED";
               // check detector calibrations
               KVGeoDNTrajectory* traj;
               if (idt->GetSize() > 1) traj = (KVGeoDNTrajectory*)idt->GetDetector(2)->GetNode()->GetForwardTrajectories()->First();
               else traj = (KVGeoDNTrajectory*)idt->GetDetector(1)->GetNode()->GetForwardTrajectories()->First();
               int nuncal = 0;
               traj->IterateFrom();
               KVGeoDetectorNode* dn;
               while ((dn = traj->GetNextNode())) nuncal += (!dn->GetDetector()->IsCalibrated());
               if (nuncal == traj->GetN()) std::cout << " ... just like ALL DETECTORS";
               else {
                  std::cout << " ... just like ";
                  traj->IterateFrom();
                  KVGeoDetectorNode* dn;
                  while ((dn = traj->GetNextNode())) {
                     if (!dn->GetDetector()->IsCalibrated()) {
                        std::cout << dn->GetName() << " ";
                     }
                  }
               }
               std::cout << std::endl;
            }
         }
         ++i;
         ++nmods;
      }
      myCanvas->cd(pad);
      ++pad;
      // change title of graph => title of pad
      mg->SetTitle(Form("INDRA Z thresh. [MeV/u] vs. Z %s Ring %d [%d-%d]", idtype.Data(), ring, nmods - mod_set + 1, nmods));
      mg->Draw("ap");
      double x1, x2;
//        if(idtype == "CSI")
//        {
//            return;// no legend - doesn't fit
//            //x1 = .39; x2 = .89;
//        }
//        else
//        {
      x1 = .11;
      x2 = .61;
//        }
      TLegend* leg;
      if (mod_set > 12) {
         leg = gPad->BuildLegend(x1, .89, x2, .69);
         leg->SetNColumns(3);
      }
      else if (mod_set > 6) {
         leg = gPad->BuildLegend(x1, .89, x2, .69);
         leg->SetNColumns(2);
      }
      else {
         leg = gPad->BuildLegend(x1, .89, x2, .69);
      }
      leg->SetBorderSize(0);
      leg->SetFillColorAlpha(kWhite, 1.00);
      if (pad > nx * ny) {
         myCanvas->Print(current_page, Form("Title:INDRA Z thresh. vs Z %s by Ring", idtype.Data()));
         myCanvas->Clear();
         myCanvas->Divide(nx, ny);
         pad = 1;
      }
   }
}

void KVDataQualityAuditReporting_INDRAFAZIA::fill_telescopes_of_group(TList& tels, std::vector<KVDetector*>& dets, const TString& idtype, double& theta_min, double& theta_max)
{
   for (auto d : dets) {
      int index = dynamic_cast<KVFAZIADetector*>(d)->GetIndex();
      TString name = Form("ID_%s_%d", idtype.Data(), index);
      auto tel = gFazia->GetIDTelescope(name);
      if (tel) {
         if (fAudit->HasTelescope(tel->GetName())) tels.Add(tel);
         else {
            if (tel->IsReadyForID()) std::cout << tel->GetName() << " is absent from audit - BUT IS READY TO IDENTIFY!" << std::endl;
         }
      }
      theta_min = std::min(theta_min, d->GetTheta());
      theta_max = std::max(theta_min, d->GetTheta());
   }
}

void KVDataQualityAuditReporting_INDRAFAZIA::FAZIA_group_reporting_Z(int group_num, std::vector<KVDetector*>& dets, const TString& idtype)
{
   TList tels;
   double theta_min{360}, theta_max{0};
   Info("FAZIA_group_reporting_Z", "Group %d", group_num);
   fill_telescopes_of_group(tels, dets, idtype, theta_min, theta_max);

   int index = 0;
   auto get_index = [&](const KVIDTelescope*) {
      return index++;
   };
   auto zmean = fReport.get_mean_Z_for_telescopes(&tels, get_index);
   index = 0;
   auto zmax = fReport.get_max_Z_for_telescopes(&tels, get_index, 24);
   index = 0;
   auto zmax_iso = fReport.get_max_Z_with_isotopes_for_telescopes(&tels, get_index, 30);
   zmax_iso->SetMarkerColor(kOrange - 2);
   index = 0;
   auto zmin = fReport.get_min_Z_for_telescopes(&tels, get_index, 25);
   TMultiGraph* mg = ::new TMultiGraph;
   mg->SetTitle(Form("Group %d [%.2f#leq#theta#leq%.2f] %s Min/Mean/Max Z", group_num, theta_min, theta_max, idtype.Data()));
   mg->Add(zmin);
   mg->Add(zmean);
   mg->Add(zmax);
   mg->Add(zmax_iso);
   relabel_FAZIA_telescope_axis(mg, &tels);
   mg->Draw("ap");
}

void KVDataQualityAuditReporting_INDRAFAZIA::FAZIA_group_mean_A_vs_Z(int group_num, std::vector<KVDetector*>& dets, const TString& idtype)
{
   TList tels;
   double theta_min{360}, theta_max{0};
   fill_telescopes_of_group(tels, dets, idtype, theta_min, theta_max);

   int color_step = TColor::GetPalette().GetSize() / (dets.size() + 1);
   int i = 1;
   TIter next(&tels);
   KVIDTelescope* idt;
   TMultiGraph* mg = ::new TMultiGraph;
   while ((idt = (KVIDTelescope*)next())) {
      auto gr = fReport[idt->GetName()].get_mean_isotopic_mass_by_Z();
      gr->SetMarkerColor(TColor::GetPalette()[color_step * i]);
      gr->SetMarkerStyle(markers[i - 1]);
      gr->SetLineWidth(0);
      mg->Add(gr);
      ++i;
   }
   // change title of graph => title of pad
   mg->SetTitle(Form("FAZIA <A> vs. Z %s Group %d [%.2f#leq#theta#leq%.2f]", idtype.Data(), group_num, theta_min, theta_max));
   mg->Draw("ap");
   TLegend* leg;
   if (i > 12) {
      leg = gPad->BuildLegend(.11, .89, .61, .69);
      leg->SetNColumns(3);
   }
   else if (i > 6) {
      leg = gPad->BuildLegend(.11, .89, .61, .69);
      leg->SetNColumns(2);
   }
   else {
      leg = gPad->BuildLegend(.11, .89, .61, .69);
   }
   leg->SetBorderSize(0);
   leg->SetFillColorAlpha(kWhite, 1.00);
}

void KVDataQualityAuditReporting_INDRAFAZIA::FAZIA_group_Z_threshold_vs_Z(int group_num, std::vector<KVDetector*>& dets, const TString& idtype)
{
   TList tels;
   double theta_min{360}, theta_max{0};
   fill_telescopes_of_group(tels, dets, idtype, theta_min, theta_max);

   int color_step = TColor::GetPalette().GetSize() / (dets.size() + 1);
   int i = 1;
   TIter next(&tels);
   KVIDTelescope* idt;
   TMultiGraph* mg = ::new TMultiGraph;
   while ((idt = (KVIDTelescope*)next())) {
      auto gr = fReport[idt->GetName()].get_element_thresholds_by_Z_mev_per_nuc(TColor::GetPalette()[color_step * i]);
      gr->SetMarkerStyle(markers[i - 1]);
      gr->SetLineWidth(0);
      mg->Add(gr);
      if (gr->GetMean(2) < 0) {
         // threshold = -1 => uncalibrated particles
         std::cout << idt->GetName() << " : particles are UNCALIBRATED";
         // check detector calibrations
         KVGeoDNTrajectory* traj = nullptr;
         if (idt->GetSize() > 1) traj = (KVGeoDNTrajectory*)idt->GetDetector(2)->GetNode()->GetForwardTrajectories()->First();
         else {
            if (idt->GetDetector(1)->GetNode()->GetForwardTrajectories())
               traj = (KVGeoDNTrajectory*)idt->GetDetector(1)->GetNode()->GetForwardTrajectories()->First();
            else {
               if (idt->GetDetector(1)->IsCalibrated()) std::cout << " ... but " << idt->GetDetector(1)->GetName() << " is calibrated...";
               else std::cout << " ... just like " << idt->GetDetector(1)->GetName();
            }
         }
         if (traj) {
            int nuncal = 0;
            traj->IterateFrom();
            KVGeoDetectorNode* dn;
            while ((dn = traj->GetNextNode())) nuncal += (!dn->GetDetector()->IsCalibrated());
            if (nuncal == traj->GetN()) std::cout << " ... just like ALL DETECTORS";
            else {
               std::cout << " ... just like ";
               traj->IterateFrom();
               KVGeoDetectorNode* dn;
               while ((dn = traj->GetNextNode())) {
                  if (!dn->GetDetector()->IsCalibrated()) {
                     std::cout << dn->GetName() << " ";
                  }
               }
            }
         }
         std::cout << std::endl;
      }
      ++i;
   }
   // change title of graph => title of pad
   mg->SetTitle(Form("FAZIA Z thresh. [MeV/u] vs. Z %s Group %d [%.2f#leq#theta#leq%.2f]", idtype.Data(), group_num, theta_min, theta_max));
   mg->Draw("ap");
   double x1, x2;
   if (idtype == "CSI") {
      return;// no legend - doesn't fit
      //x1 = .39; x2 = .89;
   }
   else {
      x1 = .11;
      x2 = .61;
   }
   TLegend* leg;
   if (i > 12) {
      leg = gPad->BuildLegend(x1, .89, x2, .69);
      leg->SetNColumns(3);
   }
   else if (i > 6) {
      leg = gPad->BuildLegend(x1, .89, x2, .69);
      leg->SetNColumns(2);
   }
   else {
      leg = gPad->BuildLegend(x1, .89, x2, .69);
   }
   leg->SetBorderSize(0);
   leg->SetFillColorAlpha(kWhite, 1.00);
}

void KVDataQualityAuditReporting_INDRAFAZIA::FAZIA_group_A_threshold_vs_Z(int group_num, std::vector<KVDetector*>& dets, const TString& idtype)
{
   TList tels;
   double theta_min{360}, theta_max{0};
   fill_telescopes_of_group(tels, dets, idtype, theta_min, theta_max);

   int color_step = TColor::GetPalette().GetSize() / (dets.size() + 1);
   int i = 1;
   TIter next(&tels);
   KVIDTelescope* idt;
   TMultiGraph* mg = ::new TMultiGraph;
   while ((idt = (KVIDTelescope*)next())) {
      auto gr = fReport[idt->GetName()].get_isotope_thresholds_by_Z_mev_per_nuc(TColor::GetPalette()[color_step * i]);
      gr->SetMarkerStyle(markers[i - 1]);
      gr->SetLineWidth(0);
      mg->Add(gr);
      ++i;
   }
   // change title of graph => title of pad
   mg->SetTitle(Form("FAZIA A thresh. [MeV/u] vs. Z %s Group %d [%.2f#leq#theta#leq%.2f]", idtype.Data(), group_num, theta_min, theta_max));
   mg->Draw("ap");
   double x1, x2;
   if (idtype == "CSI") {
      return;// no legend - doesn't fit
      //x1 = .39; x2 = .89;
   }
   else {
      x1 = .11;
      x2 = .61;
   }
   TLegend* leg;
   if (i > 12) {
      leg = gPad->BuildLegend(x1, .89, x2, .69);
      leg->SetNColumns(3);
   }
   else if (i > 6) {
      leg = gPad->BuildLegend(x1, .89, x2, .69);
      leg->SetNColumns(2);
   }
   else {
      leg = gPad->BuildLegend(x1, .89, x2, .69);
   }
   leg->SetBorderSize(0);
   leg->SetFillColorAlpha(kWhite, 1.00);
}

void KVDataQualityAuditReporting_INDRAFAZIA::relabel_FAZIA_telescope_axis(TMultiGraph* graf, const TList* tels) const
{
   auto N = dynamic_cast<TGraph*>(graf->GetListOfGraphs()->First())->GetN();
   for (int i = 0; i < N; ++i) {
      auto bin = graf->GetXaxis()->FindBin(i);
      graf->GetXaxis()->SetBinLabel(bin, tels->At(i)->GetName());
   }
   graf->GetXaxis()->LabelsOption("u");// automatic rotation of labels to fit
}

void KVDataQualityAuditReporting_INDRAFAZIA::draw_sidebar_legend()
{
//=========Macro generated from canvas: c1/c1
//=========  (Tue Jul 27 11:21:28 2021) by ROOT version 6.24/02

   myCanvas->cd();
   // make sure all pads are transparent
   TIter it(myCanvas->GetListOfPrimitives());
   TObject* obj;
   while ((obj = it())) {
      if (obj->InheritsFrom("TPad")) {
         dynamic_cast<TPad*>(obj)->SetFillStyle(4000);
      }
   }
   TLatex*    tex = ::new TLatex(0.02336825, 0.86604, "Max. Z");
   tex->SetTextSize(0.028);
   tex->SetTextAngle(90);
   tex->SetLineWidth(2);
   tex->Draw();
   TMarker* marker = ::new TMarker(0.01611604, 0.8519389, 4);

   Int_t ci;      // for color index setting
   TColor* color; // for color definition with alpha
   ci = TColor::GetColor("#333399");
   marker->SetMarkerColor(ci);
   marker->SetMarkerStyle(4);
   marker->SetMarkerSize(1.7);
   marker->Draw();
   marker = ::new TMarker(0.01772764, 0.4453584, 30);

   ci = TColor::GetColor("#ffcc33");
   marker->SetMarkerColor(ci);
   marker->SetMarkerStyle(30);
   marker->SetMarkerSize(1.7);
   marker->Draw();
   tex = ::new TLatex(0.02497985, 0.4629847, "Max. Z with identified A");
   tex->SetTextSize(0.028);
   tex->SetTextAngle(90);
   tex->SetLineWidth(2);
   tex->Draw();
   tex = ::new TLatex(0.02659146, 0.2632197, "Mean Z");
   tex->SetTextSize(0.028);
   tex->SetTextAngle(90);
   tex->SetLineWidth(2);
   tex->Draw();
   tex = ::new TLatex(0.02820306, 0.1045828, "Min. Z");
   tex->SetTextSize(0.028);
   tex->SetTextAngle(90);
   tex->SetLineWidth(2);
   tex->Draw();
   marker = ::new TMarker(0.01853344, 0.2479436, 20);

   ci = TColor::GetColor("#333399");
   marker->SetMarkerColor(ci);
   marker->SetMarkerStyle(20);
   marker->SetMarkerSize(1.7);
   marker->Draw();
   marker = ::new TMarker(0.02095085, 0.08578143, 25);

   ci = TColor::GetColor("#333399");
   marker->SetMarkerColor(ci);
   marker->SetMarkerStyle(25);
   marker->SetMarkerSize(1.7);
   marker->Draw();
}

void KVDataQualityAuditReporting_INDRAFAZIA::draw_sidebar_legend_fazia()
{
//=========Macro generated from canvas: c1/c1
//=========  (Tue Jul 27 12:21:27 2021) by ROOT version 6.24/02
   myCanvas->cd();
   // make sure all pads are transparent
   TIter it(myCanvas->GetListOfPrimitives());
   TObject* obj;
   while ((obj = it())) {
      if (obj->InheritsFrom("TPad")) {
         dynamic_cast<TPad*>(obj)->SetFillStyle(4000);
      }
   }
   TLatex*    tex = new TLatex(0.9813084, 0.9470954, "Max. Z");
   tex->SetTextSize(0.028);
   tex->SetTextAngle(270);
   tex->SetLineWidth(2);
   tex->Draw();
   TMarker* marker = new TMarker(0.9853972, 0.9688797, 4);

   Int_t ci;      // for color index setting
   TColor* color; // for color definition with alpha
   ci = TColor::GetColor("#333399");
   marker->SetMarkerColor(ci);
   marker->SetMarkerStyle(4);
   marker->SetMarkerSize(1.7);
   marker->Draw();
   marker = new TMarker(0.984229, 0.8309129, 30);

   ci = TColor::GetColor("#ffcc33");
   marker->SetMarkerColor(ci);
   marker->SetMarkerStyle(30);
   marker->SetMarkerSize(1.7);
   marker->Draw();
   tex = new TLatex(0.9807243, 0.8143154, "Max. Z with identified A");
   tex->SetTextSize(0.028);
   tex->SetTextAngle(270);
   tex->SetLineWidth(2);
   tex->Draw();
   tex = new TLatex(0.9813084, 0.4419087, "Mean Z");
   tex->SetTextSize(0.028);
   tex->SetTextAngle(270);
   tex->SetLineWidth(2);
   tex->Draw();
   tex = new TLatex(0.978972, 0.2479253, "Min. Z");
   tex->SetTextSize(0.028);
   tex->SetTextAngle(270);
   tex->SetLineWidth(2);
   tex->Draw();
   marker = new TMarker(0.9853972, 0.4564315, 20);

   ci = TColor::GetColor("#333399");
   marker->SetMarkerColor(ci);
   marker->SetMarkerStyle(20);
   marker->SetMarkerSize(1.7);
   marker->Draw();
   marker = new TMarker(0.9830607, 0.2645228, 25);

   ci = TColor::GetColor("#333399");
   marker->SetMarkerColor(ci);
   marker->SetMarkerStyle(25);
   marker->SetMarkerSize(1.7);
   marker->Draw();

}
