#include "KVDataQualityAuditSelector.h"
#include "KVDataSet.h"

ClassImp(KVDataQualityAuditSelector)

void KVDataQualityAuditSelector::InitRun(void)
{
   // Create audit objects if not already done
   auto sys = gDataAnalyser->GetAnalysedSystem();
   if (!audit_reac) audit_reac = ::new KVDataQualityAudit(sys->GetBatchName(), sys->GetName());
   if (!audit_sys) audit_sys = ::new KVDataQualityAudit(sys->GetBatchNameWithoutEnergy(), sys->GetReactionNameWithoutEnergy());
   if (!audit_ds) audit_ds = ::new KVDataQualityAudit(gDataSet->GetName(), gDataSet->GetTitle());
}

Bool_t KVDataQualityAuditSelector::Analysis(void)
{
   for (auto& particle : ReconEventOKIterator(GetEvent())) {
      audit_reac->Add(particle);
      audit_sys->Add(particle);
      audit_ds->Add(particle);
   }

   return kTRUE;
}

void KVDataQualityAuditSelector::EndAnalysis()
{
   TString filename;
   if (gDataAnalyser->GetBatchSystem())
      filename.Form("%s.root", gDataAnalyser->GetBatchSystem()->GetJobName());
   else
      filename = "DataQualityAudit.root";
   TFile output(filename, "recreate");
   audit_reac->Write();
   audit_sys->Write();
   audit_ds->Write();
   output.Write();
   output.Close();
}

