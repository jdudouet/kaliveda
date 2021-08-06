#ifndef E789_DATA_QUALITY_AUDIT_REPORTING_H
#define E789_DATA_QUALITY_AUDIT_REPORTING_H

#include "KVBase.h"
#include <KVDataQualityAuditReportMaker.h>
#include <TCanvas.h>

/**
  \class KVDataQualityAuditReporting_INDRAFAZIA
  \brief Prepare PDF report on data quality audits for INDRA-FAZIA experiments
  \ingroup Analysis

  This class will produce a multi-page PDF report from the data quality audits for INDRA-FAZIA experiments.

  To use it, open a file containing data quality audits, then generate the PDF as follows:

~~~~
kaliveda [1] .ls
TFile**     bilan_58Ni58Ni52.root
 TFile*     bilan_58Ni58Ni52.root
  KEY: KVDataQualityAudit  58Ni58Ni52;1   58Ni + 58Ni[300] 52 MeV/A
  KEY: KVDataQualityAudit  58Ni58Ni;1  58Ni + 58Ni[300]
  KEY: KVDataQualityAudit  INDRAFAZIA.E789;1 E789 (April/May 2019)

kaliveda [2] KVDataQualityAuditReporting_INDRAFAZIA DQA("58Ni58Ni52")
kaliveda [3] DQA.do_report()
~~~~

  Many useful informations will also be printed in the terminal as the PDF is generated,
  don't forget to read them.

  Each page in the PDF file is given a title which appears in the 'Outline' which can be used in a PDF
  viewer in order to navigate within the file.
 */
class KVDataQualityAuditReporting_INDRAFAZIA : public KVBase {
   KVDataQualityAuditReportMaker fReport;
   const KVDataQualityAudit* fAudit;

   TCanvas* myCanvas;
   enum class canvas_t {
      kLandscape,
      kPortrait
   };

   void make_canvas(canvas_t style = canvas_t::kLandscape);

   TString current_page;

   std::map<double, std::vector<KVDetector*>> fazia_map;
   void make_fazia_map(double theta_bin);

   std::vector<int> markers {20, 24, 21, 25, 34, 28, 47, 46, 45, 44, 29, 30, 43, 42, 22, 26, 23, 32};

   std::pair<int, int> get_layout(size_t ntels)
   {
      assert(ntels <= 16);
      if (ntels <= 8) return {4, 2};
      else if (ntels <= 12) return {4, 3};
      return {4, 4};
   }
   void fill_telescopes_of_group(TList& tels, std::vector<KVDetector*>& dets, const TString& idtype, double& theta_min, double& theta_max);
   void draw_sidebar_legend();
   void draw_sidebar_legend_fazia();

public:
   KVDataQualityAuditReporting_INDRAFAZIA(const TString& audit_name)
      : fReport{audit_name}, fAudit{fReport.GetAudit()}
   {}
   virtual ~KVDataQualityAuditReporting_INDRAFAZIA() {}

   void do_report();
   void INDRA_ring_reporting_Z(int ring, const TString& idtype);
   void INDRA_ring_mean_A_vs_Z(int ring, const TString& idtype, int& pad, int nx, int ny);
   void INDRA_ring_Z_threshold_vs_Z(int ring, const TString& idtype, int& pad, int nx, int ny);
   void FAZIA_group_reporting_Z(int group_num, std::vector<KVDetector*>&, const TString& idtype);
   void FAZIA_group_mean_A_vs_Z(int group_num, std::vector<KVDetector*>&, const TString& idtype);
   void FAZIA_group_Z_threshold_vs_Z(int group_num, std::vector<KVDetector*>& dets, const TString& idtype);
   void FAZIA_group_A_threshold_vs_Z(int group_num, std::vector<KVDetector*>& dets, const TString& idtype);
   template<typename TelescopeFunction>
   void FAZIA_group_reporting_detail(const TString& pdf_file, const TString& title, int group_num, std::vector<KVDetector*>& dets, const TString& idtype, TelescopeFunction TF, Bool_t logY = kFALSE)
   {
      // Detailed reports on given idtype for all telescopes in group:
      //  TelescopeFunction signature: void (const KVDataQualityAuditReportMaker::telescope&)

      myCanvas->Clear();
      auto lay = get_layout(dets.size());
      myCanvas->Divide(lay.first, lay.second);
      int max_pad = lay.first * lay.second;

      TList tels;
      double theta_min{360}, theta_max{0};
      fill_telescopes_of_group(tels, dets, idtype, theta_min, theta_max);

      TIter next(&tels);
      KVIDTelescope* tel;
      int pad = 1;
      while ((tel = (KVIDTelescope*)next())) {
         myCanvas->cd(pad);
         gPad->SetLogy(logY);
         TF(fReport[tel->GetName()]);
         ++pad;
         if (pad > max_pad) {
            myCanvas->Print(pdf_file, Form("Title:FAZIA %s Group %d : %s", idtype.Data(), group_num, title.Data()));
            myCanvas->Clear();
            myCanvas->Divide(lay.first, lay.second);
            pad = 1;
         }
      }
      if (pad != 1) myCanvas->Print(pdf_file, Form("Title:FAZIA %s Group %d : %s", idtype.Data(), group_num, title.Data()));
   }

   void relabel_FAZIA_telescope_axis(TMultiGraph* graf, const TList* tels) const;

   ClassDef(KVDataQualityAuditReporting_INDRAFAZIA, 0)
};

#endif // E789_DATA_QUALITY_AUDIT_REPORTING_H
