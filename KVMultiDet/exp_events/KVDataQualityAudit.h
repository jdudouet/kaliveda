#ifndef __KVDATAQUALITYAUDIT_H
#define __KVDATAQUALITYAUDIT_H

#include "KVBase.h"

#include <KVNumberList.h>
#include <KVReconstructedNucleus.h>
#include <KVUniqueNameList.h>

/**
 \class KVDataQualityAudit
 \brief Audit of experimental data identification and calibrations
 \ingroup AnalysisInfra

 A KVDataQualityAudit object contains many essential informations about experimental data identification and calibrations,
 which can be used:
   - to assess the quality of newly-reconstructed & calibrated data;
   - to provide an accurate replica of experimental identification capabilites, when filtering simulated data.

 The audit is performed by reading a set of experimental data (see KVDataQualityAuditSelector).
 At the end, the audit contains a list of all identification telescopes (see KVIDTelescope) which contributed
 to the data, with for each:
   - the list of elements identified (with or without mass identification), with, for each element:
     + the number of times it was identified;
     + the minimum energy with which it was identified, \f$E_{min}^{Z}\f$, used as the threshold for \f$Z\f$
       identification of this element (for calibrated particles);
     + when identified only in \f$Z\f$, the mass \f$A\f$ that was attributed by default to the isotope;
     + the list of isotopes, if any, identified for the element, with, for each isotope:
       - the number of times it was identified;
       - the minimum energy with which it was identified, \f$E_{min}^{A}\f$, used as the threshold for
         identification of the isotope (for calibrated particles).

 \sa KVDataQualityAuditReportMaker

 \author John Frankland
 \date Fri Jul  9 22:37:24 2021
*/

class KVDataQualityAudit : public KVBase {
private:
   KVUniqueNameList telescopes;

   Long64_t merge(KVDataQualityAudit*);

public:
   KVDataQualityAudit() : KVDataQualityAudit("") {}
   KVDataQualityAudit(const Char_t* name, const Char_t* title = "")
      : KVBase(name, title)
   {
      telescopes.SetOwner();
   }
   virtual ~KVDataQualityAudit() {}
   void Add(const KVReconstructedNucleus& N);
   void Print(Option_t* opt = "") const;
   const KVSeqCollection* GetTelescopeList() const
   {
      return &telescopes;
   }
   Bool_t HasTelescope(const TString& tel_name) const
   {
      return telescopes.FindObject(tel_name) != nullptr;
   }
   Long64_t Merge(TCollection*);

   struct isotope {
      // watch the alignment !
      Double_t counts{0};
      Float_t emin{-1.0};
      UShort_t A;
      void add(const KVReconstructedNucleus& N);
      void print() const;
      void merge(const isotope& isoto)
      {
         // Merge two isotopes. This means keeping the lower of the two threshold values,
         // and summing the occurences.
         if (emin > 0) {
            if (isoto.emin > 0) emin = std::min(emin, isoto.emin);
         }
         else if (isoto.emin > 0) emin = isoto.emin;
         counts += isoto.counts;
      }

      ClassDef(isotope, 1)
   };
   struct element {
      // watch the alignment !
      Double_t counts{0};
      Float_t emin{-1.0};
      UShort_t A{0};// default (calculated) mass given when no isotopic identification available
      UChar_t Z;
      std::map<int, isotope> isotopes;
      void add(const KVReconstructedNucleus& N);
      void print() const;
      void merge(const element&);
      Bool_t HasIsotopes() const
      {
         return !isotopes.empty();
      }
      Bool_t HasIsotope(int A) const
      {
         return isotopes.find(A) != std::end(isotopes);
      }
      const isotope& GetIsotope(int A) const
      {
         return isotopes.at(A);
      }
      KVNumberList GetIsotopeList() const
      {
         // fill and return list of isotope mass numbers A
         KVNumberList l;
         for (auto& p : isotopes) l.Add(p.first);
         return l;
      }
      double get_mean_isotopic_mass() const;
      int get_max_isotopic_mass() const;
      std::map<int, double> get_isotopic_distribution() const;
      int get_min_isotopic_mass() const;
      double get_minimum_isotopic_threshold_mev_per_nuc() const;
      int get_default_mass() const
      {
         return A;
      }

      ClassDef(element, 1)
   };
   class idtelescope : public TNamed {
      std::map<int, element> elements;
   public:
      idtelescope(const TString& name = "") : TNamed(name, name) {}
      virtual ~idtelescope() {}
      ROOT_COPY_CTOR(idtelescope, TNamed)
      ROOT_COPY_ASSIGN_OP(idtelescope)
      void Copy(TObject& o) const
      {
         // Make o into a copy of this object
         TNamed::Copy(o);
         idtelescope& other = (idtelescope&)o;
         other.elements = elements;
      }
      void add(const KVReconstructedNucleus& N);
      void Print(Option_t* opt = "") const;
      void merge(const idtelescope*);
      Bool_t HasElement(int Z) const
      {
         return elements.find(Z) != std::end(elements);
      }
      const element& GetElement(int Z) const
      {
         return elements.at(Z);
      }
      KVNumberList GetElementList() const
      {
         // fills and returns a list with all Z numbers of elements

         KVNumberList l;
         for (auto& p : elements) l.Add(p.first);
         return l;
      }
      std::map<int, double> get_element_distribution() const;
      double get_mean_Z() const;
      int get_max_Z() const;
      double get_mean_A() const;
      int get_max_A() const;
      int get_min_A() const;
      int get_min_Z() const;
      int get_max_Z_with_isotopes() const;

      ClassDef(idtelescope, 1)
   };

   Bool_t HasTelescope(const idtelescope* idt) const
   {
      return HasTelescope(idt->GetName());
   }
   idtelescope* GetTelescope(const TString& tel_name) const
   {
      return (idtelescope*)telescopes.FindObject(tel_name);
   }
   idtelescope* GetTelescope(const idtelescope* idt) const
   {
      return GetTelescope(idt->GetName());
   }

private:
   void add(const idtelescope*);

   ClassDef(KVDataQualityAudit, 1) //Audit of experimental data identification and calibrations
};

#endif
