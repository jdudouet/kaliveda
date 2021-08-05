#ifndef __KVDATAQUALITYAUDITSELECTOR_E789_H
#define __KVDATAQUALITYAUDITSELECTOR_E789_H

#include "KVDataQualityAuditSelector.h"

/**
 \class KVDataQualityAuditSelector_E789
 \brief Special audit preparation for E789

 Specific audit preparation class for E789, used during data reduction.

 All identified particles are included in the audit, whether or not they have calibrated energies
 (only calibrated particles are used to defined thresholds).

 \author John Frankland
 \date Thu Aug  5 12:28:44 2021
*/

class KVDataQualityAuditSelector_E789 : public KVDataQualityAuditSelector {
public:
   KVDataQualityAuditSelector_E789() {}
   virtual ~KVDataQualityAuditSelector_E789() {}

   void InitRun();

   ClassDef(KVDataQualityAuditSelector_E789, 1) //Special audit preparation for E789
};

#endif
