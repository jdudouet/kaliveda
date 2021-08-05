#ifndef __KVDATAQUALITYAUDITSELECTOR_H
#define __KVDATAQUALITYAUDITSELECTOR_H

#include "KVReconEventSelector.h"
#include <KVDataQualityAudit.h>

/**
 \class KVDataQualityAuditSelector
 \brief Dedicated analysis class for preparing data quality audits
 \ingroup AnalysisInfra

 This analysis class is used by the data analysis task "Prepare data quality audits".

 It will fill three KVDataQualityAudit objects for data read in each run:
   - one for the specific reaction X+Y@E;
   - one for the collision system X+Y (all energies E included);
   - one for the dataset.

 It can be used for many different runs of different reactions of the same dataset separately,
 and the resulting ROOT files can be merged together using `hadd`.

 \sa KVDataQualityAuditReportMaker

 \author John Frankland
 \date Thu Aug  5 10:29:08 2021
*/

class KVDataQualityAuditSelector : public KVReconEventSelector {
   KVDataQualityAudit* audit_reac{nullptr};
   KVDataQualityAudit* audit_sys{nullptr};
   KVDataQualityAudit* audit_ds{nullptr};

public:
   KVDataQualityAuditSelector() {}
   virtual ~KVDataQualityAuditSelector() {}

   virtual void InitRun();
   virtual void EndRun() {}
   virtual void InitAnalysis() {};
   virtual Bool_t Analysis();
   virtual void EndAnalysis();

   ClassDef(KVDataQualityAuditSelector, 1) //Dedicated analysis class for preparing data quality audits
};

#endif
