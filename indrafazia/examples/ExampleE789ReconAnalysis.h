#ifndef __EXAMPLEE789RECONANALYSIS_H
#define __EXAMPLEE789RECONANALYSIS_H

/**
 \class ExampleE789ReconAnalysis
 \brief Example analysis of reconstructed events for INDRAFAZIA experiment E789

 See the comments in the code.

 \author John Frankland
 \date Wed Mar 23 10:14:34 2022
*/

#include "KVReconEventSelector.h"

class ExampleE789ReconAnalysis : public KVReconEventSelector {

   void add_idcode_histos(const TString&);
   void fill_idcode_histos(const TString&, const KVReconstructedNucleus&);
   void fill_ecode_histos(const TString&, const KVReconstructedNucleus&);

public:
   ExampleE789ReconAnalysis() {}
   virtual ~ExampleE789ReconAnalysis() {}

   virtual void InitRun();
   virtual void EndRun() {}
   virtual void InitAnalysis();
   virtual Bool_t Analysis();
   virtual void EndAnalysis() {}

   ClassDef(ExampleE789ReconAnalysis, 1) //Analysis of reconstructed events
};

#endif
