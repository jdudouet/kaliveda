//Created by KVClassFactory on Thu Dec 15 14:55:48 2016
//Author: John Frankland,,,

#ifndef __KVPROOFLITEBATCH_H
#define __KVPROOFLITEBATCH_H

#include "KVBatchSystem.h"
#include "KVDataAnalyser.h"

/**
\class KVPROOFLiteBatch
\brief Batch system interface to PROOFLite
\ingroup Infrastructure
*/

class KVPROOFLiteBatch : public KVBatchSystem {

   int max_num_cpus; // max number of CPUs to use: by default, all available

public:
   KVPROOFLiteBatch(const Char_t* name);
   virtual ~KVPROOFLiteBatch();

   void SubmitTask(KVDataAnalyser* da);

   void GetBatchSystemParameterList(KVNameValueList&);
   void SetBatchSystemParameters(const KVNameValueList&);

   ClassDef(KVPROOFLiteBatch, 1) //Batch system interface to PROOFLite
};

#endif
