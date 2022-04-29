//Created by KVClassFactory on Thu Dec 15 14:55:48 2016
//Author: John Frankland,,,

#include "KVPROOFLiteBatch.h"

#include <TProof.h>

ClassImp(KVPROOFLiteBatch)

KVPROOFLiteBatch::KVPROOFLiteBatch(const Char_t* name)
   : KVBatchSystem(name), max_num_cpus(WITH_MULTICORE_CPU)
{
}

//____________________________________________________________________________//

KVPROOFLiteBatch::~KVPROOFLiteBatch()
{
   // Destructor
}

void KVPROOFLiteBatch::SubmitTask(KVDataAnalyser* da)
{
   // Run analysis on PROOFLite facility

   // Open PROOFLite session with required number of workers and initialise KaliVeda package
   if (!gProof) {
      TProof* p = TProof::Open(Form("workers=%d", max_num_cpus));
      p->ClearCache();//to avoid problems with compilation of KVParticleCondition
      // enable KaliVeda on PROOF cluster
      if (p->EnablePackage("KaliVeda") != 0) {
         // first time, need to 'upload' package
         TString fullpath = KVBase::GetETCDIRFilePath("KaliVeda.par");
         p->UploadPackage(fullpath);
         p->EnablePackage("KaliVeda");
      }
   }
#ifdef WITH_CPP11
   da->SetProofMode(KVDataAnalyser::EProofMode::Lite);
#else
   da->SetProofMode(KVDataAnalyser::Lite);
#endif
   SetAnalyser(da);
   da->SubmitTask();
}

void KVPROOFLiteBatch::GetBatchSystemParameterList(KVNameValueList& nl)
{
   // Add to batch parameters the number of CPUs to use
   //
   // By default, it is the number of CPUs on the machine

   KVBatchSystem::GetBatchSystemParameterList(nl);
   nl.SetValue("MaxNumCPUs", max_num_cpus);
}

void KVPROOFLiteBatch::SetBatchSystemParameters(const KVNameValueList& nl)
{
   // Add to batch parameters the number of CPUs to use
   //
   // By default, it is the number of CPUs on the machine

   KVBatchSystem::SetBatchSystemParameters(nl);
   max_num_cpus = nl.GetIntValue("MaxNumCPUs");
}


