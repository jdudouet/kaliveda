#include "KVDataQualityAuditReportMaker.h"
#include "KVMultiDetArray.h"
#include <TColor.h>

TH1F* KVDataQualityAuditReportMaker::telescope::get_element_distribution(Color_t color) const
{
   // \returns histogram P(Z) probability distribution for elements

   auto proba = this_telescope->get_element_distribution();
   // find largest Z
   auto last = proba.rbegin();
   auto Zmax = (*last).first;
   auto histo = ::new TH1F(Form("Z_%s", this_telescope->GetName()), Form("P(Z) %s", this_telescope->GetName()),
                           Zmax + 1, .5, Zmax + 1.5);
   for (auto& p : proba) {
      histo->SetBinContent(p.first, p.second);
   }
   return KVDataQualityAuditReportMaker::FormatHisto(histo, color);
}

TGraph* KVDataQualityAuditReportMaker::telescope::get_element_thresholds_by_Z(Color_t color) const
{
   // \returns graph of energy threshold for each element identified by telescope

   auto gr = ::new TGraph;
   gr->SetTitle(Form("%s Z thresholds [MeV]", this_telescope->GetName()));
   gr->SetName(this_telescope->GetName());
   auto Zlist = this_telescope->GetElementList();
   Zlist.Begin();
   while (!Zlist.End()) {
      auto Z = Zlist.Next();
      gr->SetPoint(gr->GetN(), Z, this_telescope->GetElement(Z).emin);
   }
   return KVDataQualityAuditReportMaker::FormatGraph(gr, color);
}

TGraph* KVDataQualityAuditReportMaker::telescope::get_element_thresholds_by_Z_mev_per_nuc(Color_t color) const
{
   // \returns graph of energy threshold in MeV/nucleon for each element identified by telescope
   // for uncalibrated particles, threshold is -1 for all

   auto gr = ::new TGraph;
   gr->SetTitle(Form("%s Z thresholds [MeV/u]", this_telescope->GetName()));
   gr->SetName(this_telescope->GetName());
   auto Zlist = this_telescope->GetElementList();
   Zlist.Begin();
   KVNumberList abnormal;
   while (!Zlist.End()) {
      auto Z = Zlist.Next();
      double thresh;
      if (this_telescope->GetElement(Z).HasIsotopes())
         thresh = this_telescope->GetElement(Z).emin / this_telescope->GetElement(Z).get_mean_isotopic_mass();
      else
         thresh = this_telescope->GetElement(Z).emin / this_telescope->GetElement(Z).get_default_mass();
      if (thresh < 0) thresh = -1;
      gr->SetPoint(gr->GetN(), Z, thresh);
      if (TString(this_telescope->GetName()).BeginsWith("ID_CSI") && thresh > 0 && thresh < 10) {
         abnormal.Add(Z);
      }
   }
   if (abnormal.GetEntries()) {
      std::cout << "abnormal Z threshold (<10MeV/u): " << this_telescope->GetName() << " Z=" << abnormal.AsString();
      TString p(this_telescope->GetName());
      p.Remove(0, 7);
      p.Prepend("CSI-");
      if (!gMultiDetArray->GetDetector(p)->IsCalibrated()) std::cout << " : Detector " << p << " NOT CALIBRATED";
      std::cout << std::endl;
   }
   return KVDataQualityAuditReportMaker::FormatGraph(gr, color);
}

TGraph* KVDataQualityAuditReportMaker::telescope::get_isotope_thresholds_by_Z_mev_per_nuc(Color_t color) const
{
   // \returns graph of energy threshold in MeV/nucleon for isotopic identification of each element identified by telescope.
   //
   // this is the lowest recorded threshold value (in MeV/u) for the element to be isotopically identified

   auto gr = ::new TGraph;
   gr->SetTitle(Form("%s A thresholds [MeV/u]", this_telescope->GetName()));
   gr->SetName(this_telescope->GetName());
   auto Zlist = this_telescope->GetElementList();
   Zlist.Begin();
   while (!Zlist.End()) {
      auto Z = Zlist.Next();
      // only include elements with isotopic identification
      if (this_telescope->GetElement(Z).HasIsotopes())
         gr->SetPoint(gr->GetN(), Z, this_telescope->GetElement(Z).get_minimum_isotopic_threshold_mev_per_nuc());
   }
   return KVDataQualityAuditReportMaker::FormatGraph(gr, color);
}

TGraph* KVDataQualityAuditReportMaker::telescope::get_mean_isotopic_mass_by_Z(Color_t color) const
{
   // \returns graph of mean isotopic mass for each isotopically-identified Z
   auto gr = ::new TGraph;
   gr->SetTitle(Form("%s <A> vs. Z", this_telescope->GetName()));
   gr->SetName(this_telescope->GetName());
   auto Zlist = this_telescope->GetElementList();
   Zlist.Begin();
   while (!Zlist.End()) {
      auto Z = Zlist.Next();
      auto elem = this_telescope->GetElement(Z);
      if (elem.HasIsotopes()) {
         gr->SetPoint(gr->GetN(), Z, elem.get_mean_isotopic_mass());
      }
   }
   return KVDataQualityAuditReportMaker::FormatGraph(gr, color);
}

TMultiGraph* KVDataQualityAuditReportMaker::telescope::get_isotope_distributions() const
{
   // \returns multigraph of isotope distributions for all isotopically identified elements

   auto mg = ::new TMultiGraph;
   auto Zlist = this_telescope->GetElementList();
   int color_step = TColor::GetPalette().GetSize() / (Zlist.GetNValues() + 1);
   Zlist.Begin();
   int i = 1;
   while (!Zlist.End()) {
      auto Z = Zlist.Next();
      auto elem = get_element(Z);
      if (this_telescope->GetElement(Z).HasIsotopes()) {
         auto gr = elem.get_isotope_distribution(TColor::GetPalette()[color_step * i]);
         mg->Add(gr);
      }
      ++i;
   }
   return mg;
}

TMultiGraph* KVDataQualityAuditReportMaker::telescope::get_isotope_thresholds_by_A() const
{
   // \returns multigraph of isotope thresholds in [MeV] for all isotopically identified elements

   auto mg = ::new TMultiGraph;
   auto Zlist = this_telescope->GetElementList();
   int color_step = TColor::GetPalette().GetSize() / (Zlist.GetNValues() + 1);
   Zlist.Begin();
   int i = 1;
   while (!Zlist.End()) {
      auto Z = Zlist.Next();
      auto elem = get_element(Z);
      if (this_telescope->GetElement(Z).HasIsotopes()) {
         mg->Add(elem.get_isotope_thresholds_by_A(TColor::GetPalette()[color_step * i]));
      }
      ++i;
   }
   return mg;
}

TMultiGraph* KVDataQualityAuditReportMaker::telescope::get_isotope_thresholds_by_A_mev_per_nuc() const
{
   // \returns multigraph of isotope thresholds in [MeV/u] for all isotopically identified elements

   auto mg = ::new TMultiGraph;
   auto Zlist = this_telescope->GetElementList();
   int color_step = TColor::GetPalette().GetSize() / (Zlist.GetNValues() + 1);
   Zlist.Begin();
   int i = 1;
   while (!Zlist.End()) {
      auto Z = Zlist.Next();
      auto elem = get_element(Z);
      if (this_telescope->GetElement(Z).HasIsotopes()) {
         mg->Add(elem.get_isotope_thresholds_by_A_mev_per_nuc(TColor::GetPalette()[color_step * i]));
      }
      ++i;
   }
   return mg;
}

TGraph* KVDataQualityAuditReportMaker::FormatGraph(TGraph* G, Color_t col, Marker_t mark)
{
   // provide uniform style for all graphs

   if (!col) col = kBlue - 2;
   if (!mark) mark = 20;
   G->SetMarkerStyle(mark);
   G->SetMarkerSize(1.7);
   G->SetMarkerColor(col);
   G->SetLineWidth(2);
   G->SetLineColor(col);
   return G;
}

TH1F* KVDataQualityAuditReportMaker::FormatHisto(TH1F* G, Color_t col)
{
   // provide uniform style for all histos

   if (!col) col = kBlue - 2;
   G->SetLineWidth(2);
   G->SetLineColor(col);
   return G;
}

TGraph* KVDataQualityAuditReportMaker::element::get_isotope_distribution(Color_t color) const
{
   // \returns graph of isotope distribution P(A) for element.
   // The distribution is weighted by the number of elements Z observed.

   auto proba = this_element->get_isotopic_distribution();
   auto gr = ::new TGraph;
   gr->SetName(Form("Z=%d", (int)this_element->Z));
   gr->SetTitle(Form("P(A) Z=%d %s", (int)this_element->Z, telescope_name.c_str()));
   for (auto& p : proba) {
      gr->SetPoint(gr->GetN(), p.first, this_element->counts * p.second);
   }
   return KVDataQualityAuditReportMaker::FormatGraph(gr, color);
}

TGraph* KVDataQualityAuditReportMaker::element::get_isotope_thresholds_by_A(Color_t color) const
{
   // \returns thresholds in [MeV] for each isotope as a function of A
   auto gr = ::new TGraph;
   gr->SetTitle(Form("%s Z=%d A thresholds [MeV]", telescope_name.c_str(), this_element->Z));
   gr->SetName(Form("Z=%d", this_element->Z));
   auto Alist = this_element->GetIsotopeList();
   Alist.Begin();
   while (!Alist.End()) {
      auto A = Alist.Next();
      gr->SetPoint(gr->GetN(), A, this_element->GetIsotope(A).emin);
   }
   return KVDataQualityAuditReportMaker::FormatGraph(gr, color);
}

TGraph* KVDataQualityAuditReportMaker::element::get_isotope_thresholds_by_A_mev_per_nuc(Color_t color) const
{
   // \returns thresholds in [MeV/nucleon] for each isotope as a function of A
   auto gr = ::new TGraph;
   gr->SetTitle(Form("%s Z=%d A thresholds [MeV/u]", telescope_name.c_str(), this_element->Z));
   gr->SetName(Form("Z=%d", this_element->Z));
   auto Alist = this_element->GetIsotopeList();
   Alist.Begin();
   while (!Alist.End()) {
      auto A = Alist.Next();
      gr->SetPoint(gr->GetN(), A, this_element->GetIsotope(A).emin / A);
   }
   return KVDataQualityAuditReportMaker::FormatGraph(gr, color);
}

ClassImp(KVDataQualityAuditReportMaker)

void KVDataQualityAuditReportMaker::init_colors(int ncols)
{
   Double_t Red[]    = {0., 1.0, 0.0, 0.0, 1.0, 1.0};
   Double_t Green[]  = {0., 0.0, 0.0, 1.0, 0.0, 1.0};
   Double_t Blue[]   = {0., 1.0, 1.0, 0.0, 0.0, 0.0};
   Double_t Length[] = {0., .2, .4, .6, .8, 1.0};
   TColor::CreateGradientColorTable(6, Length, Red, Green, Blue, ncols);
}
