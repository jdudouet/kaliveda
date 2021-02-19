//Created by KVClassFactory on Thu Jul 19 15:38:10 2018
//Author: eindra

#include "KVReconEventSelector.h"

#include <KVClassFactory.h>
#include <KVReconDataAnalyser.h>

ClassImp(KVReconEventSelector)


void KVReconEventSelector::Init(TTree* tree)
{
   // When using PROOF, need to set tree pointer in KVDataAnalyser
   KVEventSelector::Init(tree);
#ifdef WITH_CPP11
   if (tree && gDataAnalyser->GetProofMode() != KVDataAnalyser::EProofMode::None) {
#else
   if (tree && gDataAnalyser->GetProofMode() != KVDataAnalyser::None) {
#endif
      dynamic_cast<KVDataSetAnalyser*>(gDataAnalyser)->SetTree(tree);
   }
}

void KVReconEventSelector::Make(const Char_t* kvsname)
{
   // Generate a new recon data analysis selector class

   KVClassFactory cf(kvsname, "Analysis of reconstructed events", "",
#ifdef USING_ROOT6
                     kTRUE, "ROOT6ReconDataSelectorTemplate");
#else
                     kTRUE, "ReconDataSelectorTemplate");
#endif
   cf.AddImplIncludeFile("KVReconstructedNucleus.h");
   cf.AddImplIncludeFile("KVBatchSystem.h");

   cf.GenerateCode();
}


/** \example ExampleReconAnalysis.cpp
# Example of an analysis class for reconstructed data (i.e. events with nuclei)

This is the analysis class generated by default by KaliVedaGUI for reconstructed data analysis.

\include ExampleReconAnalysis.h
*/
