#ifndef __KVDATAQUALITYAUDITREPORTMAKER_H
#define __KVDATAQUALITYAUDITREPORTMAKER_H

#include "KVBase.h"
#include "KVDataQualityAudit.h"
#include "TGraph.h"
#include <THStack.h>
#include <TMultiGraph.h>
/**
 \class KVDataQualityAuditReportMaker
 \brief Produce graphs and histograms from KVDataQualityAudit
 \ingroup Analysis

 This is a helper class for extracting & presenting useful informations from a KVDataQualityAudit
 in the form of graphs and histograms.

 To initialise, open a file containing data quality audit objects and call the constructor with the
 name of the audit you want to use, as shown here:

~~~~{.cpp}
kaliveda [1] .ls
TFile**     bilan_58Ni58Ni52.root
 TFile*     bilan_58Ni58Ni52.root
  KEY: KVDataQualityAudit  58Ni58Ni52;1   58Ni + 58Ni[300] 52 MeV/A
  KEY: KVDataQualityAudit  58Ni58Ni;1  58Ni + 58Ni[300]
  KEY: KVDataQualityAudit  INDRAFAZIA.E789;1 E789 (April/May 2019)

kaliveda [2] KVDataQualityAuditReportMaker rep("58Ni58Ni52")
(KVDataQualityAuditReportMaker &) Name: 58Ni58Ni52 Title: 58Ni + 58Ni[300] 52 MeV/A
~~~~

 \author John Frankland
 \date Tue Jul 20 15:25:56 2021
*/

class KVDataQualityAuditReportMaker : public KVBase {
public:
   struct element {
      std::string telescope_name;
      const KVDataQualityAudit::element* this_element;
      element(const std::string& n, const KVDataQualityAudit::element& e)
         : telescope_name{n}, this_element{&e} {}
      TGraph* get_isotope_distribution(Color_t = 0) const;
      TGraph* get_isotope_thresholds_by_A(Color_t = 0) const;
      TGraph* get_isotope_thresholds_by_A_mev_per_nuc(Color_t = 0) const;
   };
   struct telescope {
      KVDataQualityAudit::idtelescope* this_telescope;
      telescope(const TString& name, const KVDataQualityAudit* audit)
         : this_telescope{audit->GetTelescope(name)}
      {
         if (!this_telescope) std::cerr << "Requested telescope not found in audit: " << name << std::endl;
      }
      TH1F* get_element_distribution(Color_t = 0) const;
      TGraph* get_element_thresholds_by_Z(Color_t = 0) const;
      TGraph* get_element_thresholds_by_Z_mev_per_nuc(Color_t = 0) const;
      TGraph* get_isotope_thresholds_by_Z_mev_per_nuc(Color_t color = 0) const;
      element operator[](int Z) const
      {
         return get_element(Z);
      }
      element get_element(int Z) const
      {
         return element(this_telescope->GetName(), this_telescope->GetElement(Z));
      }
      TGraph* get_mean_isotopic_mass_by_Z(Color_t = 0) const;
      TMultiGraph* get_isotope_distributions() const;
      TMultiGraph* get_isotope_thresholds_by_A() const;
      TMultiGraph* get_isotope_thresholds_by_A_mev_per_nuc() const;
      ClassDef(telescope, 0)
   };

private:
   const KVDataQualityAudit* fAudit{nullptr};
   void init_colors(int ncols = 100);
public:
   KVDataQualityAuditReportMaker()
      : KVBase("KVDataQualityReportMaker", "Produce graphs and histograms from KVDataQualityAudit")
   {
      init_colors();
   }
   KVDataQualityAuditReportMaker(const TString& audit_name)
      : KVBase(audit_name)
   {
      // Open audit with given name in the current ROOT directory, gDirectory
      // \param[in] audit_name name of KVDataQualityAudit object to open
      init_colors();
      fAudit = dynamic_cast<KVDataQualityAudit*>(gDirectory->Get(audit_name));
      if (fAudit) SetTitle(fAudit->GetTitle());
      else MakeZombie();
   }
   KVDataQualityAuditReportMaker(const KVDataQualityAudit* audit)
      : KVBase(audit->GetName(), audit->GetTitle()), fAudit(audit)
   {
      init_colors();
   }
   virtual ~KVDataQualityAuditReportMaker() {}

   telescope operator[](const TString& name) const
   {
      return telescope(name, fAudit);
   }

   static TGraph* FormatGraph(TGraph*, Color_t = 0, Marker_t = 0);
   static TH1F* FormatHisto(TH1F*, Color_t = 0);

   const KVDataQualityAudit* GetAudit() const
   {
      return fAudit;
   }

   template<typename TelescopeIndexFunction>
   TGraph* get_mean_Z_for_telescopes(TSeqCollection* idtels, TelescopeIndexFunction F, Marker_t marker_style = 20) const
   {
      // \returns graph of mean Z detected in each KVIDTelescope in the list.
      //
      // F is a function with the signature: int F(const KVIDTelescope*)
      // It should return an 'index' value which will be used to identify each telescope on the X-axis

      auto gr = ::new TGraph;
      gr->SetTitle("Mean Z vs. telescope number");
      KVIDTelescope* idt;
      TIter next(idtels);
      while ((idt = (KVIDTelescope*)next())) {
         auto t = fAudit->GetTelescope(idt->GetName());
         if (t) gr->SetPoint(gr->GetN(), F(idt), t->get_mean_Z());
      }
      return FormatGraph(gr, 0, marker_style);
   }
   template<typename TelescopeIndexFunction>
   TGraph* get_max_Z_for_telescopes(TSeqCollection* idtels, TelescopeIndexFunction F, Marker_t marker_style = 20) const
   {
      // \returns graph of max Z detected in each KVIDTelescope in the list.
      //
      // F is a function with the signature: int F(const KVIDTelescope*)
      // It should return an 'index' value which will be used to identify each telescope on the X-axis

      auto gr = ::new TGraph;
      gr->SetTitle("Max Z vs. telescope number");
      KVIDTelescope* idt;
      TIter next(idtels);
      while ((idt = (KVIDTelescope*)next())) {
         auto t = fAudit->GetTelescope(idt->GetName());
         if (t) gr->SetPoint(gr->GetN(), F(idt), t->get_max_Z());
      }
      return FormatGraph(gr, 0, marker_style);
   }
   template<typename TelescopeIndexFunction>
   TGraph* get_max_Z_with_isotopes_for_telescopes(TSeqCollection* idtels, TelescopeIndexFunction F, Marker_t marker_style = 20) const
   {
      // \returns graph of max Z for which isotopic masses were measured in each KVIDTelescope in the list.
      //
      // F is a function with the signature: int F(const KVIDTelescope*)
      // It should return an 'index' value which will be used to identify each telescope on the X-axis

      auto gr = ::new TGraph;
      gr->SetTitle("Max Z with isotopes vs. telescope number");
      KVIDTelescope* idt;
      TIter next(idtels);
      while ((idt = (KVIDTelescope*)next())) {
         auto t = fAudit->GetTelescope(idt->GetName());
         if (t) gr->SetPoint(gr->GetN(), F(idt), t->get_max_Z_with_isotopes());
      }
      return FormatGraph(gr, 0, marker_style);
   }
   template<typename TelescopeIndexFunction>
   TGraph* get_min_Z_for_telescopes(TSeqCollection* idtels, TelescopeIndexFunction F, Marker_t marker_style = 20) const
   {
      // \returns graph of smallest Z detected in each KVIDTelescope in the list.
      //
      // F is a function with the signature: int F(const KVIDTelescope*)
      // It should return an 'index' value which will be used to identify each telescope on the X-axis

      auto gr = ::new TGraph;
      gr->SetTitle("Min Z vs. telescope number");
      KVIDTelescope* idt;
      TIter next(idtels);
      while ((idt = (KVIDTelescope*)next())) {
         auto t = fAudit->GetTelescope(idt->GetName());
         if (t) gr->SetPoint(gr->GetN(), F(idt), t->get_min_Z());
      }
      return FormatGraph(gr, 0, marker_style);
   }
   template<typename TelescopeIndexFunction>
   TGraph* get_mean_A_for_telescopes(TSeqCollection* idtels, TelescopeIndexFunction F, Marker_t marker_style = 20) const
   {
      // \returns graph of mean A detected in each KVIDTelescope in the list.
      //
      // F is a function with the signature: int F(const KVIDTelescope*)
      // It should return an 'index' value which will be used to identify each telescope on the X-axis

      auto gr = ::new TGraph;
      gr->SetTitle("Mean A vs. telescope number");
      KVIDTelescope* idt;
      TIter next(idtels);
      while ((idt = (KVIDTelescope*)next())) {
         auto t = fAudit->GetTelescope(idt->GetName());
         if (t) gr->SetPoint(gr->GetN(), F(idt), t->get_mean_A());
      }
      return FormatGraph(gr, 0, marker_style);
   }
   template<typename TelescopeIndexFunction>
   TGraph* get_max_A_for_telescopes(TSeqCollection* idtels, TelescopeIndexFunction F, Marker_t marker_style = 20) const
   {
      // \returns graph of max A detected in each KVIDTelescope in the list.
      //
      // F is a function with the signature: int F(const KVIDTelescope*)
      // It should return an 'index' value which will be used to identify each telescope on the X-axis

      auto gr = ::new TGraph;
      gr->SetTitle("Max A vs. telescope number");
      KVIDTelescope* idt;
      TIter next(idtels);
      while ((idt = (KVIDTelescope*)next())) {
         auto t = fAudit->GetTelescope(idt->GetName());
         if (t) gr->SetPoint(gr->GetN(), F(idt), t->get_max_A());
      }
      return FormatGraph(gr, 0, marker_style);
   }

   template<typename TelescopeIndexFunction>
   TGraph* get_min_A_for_telescopes(TSeqCollection* idtels, TelescopeIndexFunction F, Marker_t marker_style = 20) const
   {
      // \returns graph of smallest A detected in each KVIDTelescope in the list.
      //
      // F is a function with the signature: int F(const KVIDTelescope*)
      // It should return an 'index' value which will be used to identify each telescope on the X-axis

      auto gr = ::new TGraph;
      gr->SetTitle("Min A vs. telescope number");
      KVIDTelescope* idt;
      TIter next(idtels);
      while ((idt = (KVIDTelescope*)next())) {
         auto t = fAudit->GetTelescope(idt->GetName());
         if (t) gr->SetPoint(gr->GetN(), F(idt), t->get_min_A());
      }
      return FormatGraph(gr, 0, marker_style);
   }

   ClassDef(KVDataQualityAuditReportMaker, 1) //Produce graphs and histograms from KVDataQualityAudit
};

#endif
