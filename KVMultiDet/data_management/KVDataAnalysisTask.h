/*
$Id: KVDataAnalysisTask.h,v 1.7 2009/01/14 16:01:38 franklan Exp $
$Revision: 1.7 $
$Date: 2009/01/14 16:01:38 $
$Author: franklan $
*/

#ifndef __KVDATAANALYSISTASK_H
#define __KVDATAANALYSISTASK_H

#include "KVBase.h"
#include "KVList.h"
#include "KVString.h"

/**
\class KVDataAnalysisTask
\brief Define and manage data analysis tasks
\ingroup AnalysisInfra

A KVDataAnalysisTask contains all information required by the system in order to be able to set up and run
a data analysis task for data of a given dataset.
Data analysis tasks are defined by variables in the environment files `*.kvrootrc` as in the following examples:

~~~~
+DataAnalysisTask:     Reconstruction
Reconstruction.DataAnalysisTask.Title:     Event reconstruction from raw data (raw->recon)
Reconstruction.DataAnalysisTask.Prereq:     raw
Reconstruction.DataAnalysisTask.Analyser:     RawDataReconstructor
Reconstruction.DataAnalysisTask.UserClass:     no
Reconstruction.DataAnalysisTask.StatusUpdateInterval:     5000

+Plugin.KVDataAnalyser:   RawDataReconstructor         KVRawDataReconstructor        KVMultiDetexp_events   "KVRawDataReconstructor()"
~~~~

Each task has a name, a descriptive title, a prerequisite type of data on which the analysis can be performed ("raw"),
and the name of a KVDataAnalyser plugin which will be used to pilot the analysis.
For a given dataset, only analysis tasks whose prerequisite data type is available can be performed.

Each value can be 'tweaked' for specific datasets by prefixing with the name of the dataset, e.g.
~~~~
SomeDataSet.Reconstruction.DataAnalysisTask.Analyser:  MySpecialReconstructor

+Plugin.KVDataAnalyser:   MySpecialReconstructor   ReconstructSomeDataSet   ReconstructSomeDataSet.cpp+   "ReconstructSomeDataSet()"
~~~~

The following example defines an analysis task which requires the user to supply an analysis class,
and defines that this class must
be derived from the TSelector plugin class, KVReconEventSelector.
The analysis will be piloted by the KVReconDataAnalyser class:

~~~~
+DataAnalysisTask:     ReconAnalysis
ReconAnalysis.DataAnalysisTask.Title:     Analysis of reconstructed events (recon)
ReconAnalysis.DataAnalysisTask.Prereq:     recon
ReconAnalysis.DataAnalysisTask.UserClass:     yes
ReconAnalysis.DataAnalysisTask.StatusUpdateInterval:     5000
ReconAnalysis.DataAnalysisTask.Analyser:     ReconDataAnalyser
ReconAnalysis.DataAnalysisTask.UserClass.Base:     ReconEventSelector/TSelector

+Plugin.KVDataAnalyser:   ReconDataAnalyser         KVReconDataAnalyser     KVMultiDetexp_events "KVReconDataAnalyser()"

+Plugin.TSelector:    ReconEventSelector    KVReconEventSelector    KVMultiDetanalysis   "KVReconEventSelector()"
~~~~

The following analysis task also requires the user to supply an analysis class (`"...UserClass: yes"`),
but as the base class for the analysis class is itself derived from KVDataAnalyser
it is the user's class which will be used to pilot the analysis (`"...Analyser: UserClass"`):

~~~~
+DataAnalysisTask:     RawAnalysis
RawAnalysis.DataAnalysisTask.Title:     Analysis of raw data
RawAnalysis.DataAnalysisTask.Prereq:     raw
RawAnalysis.DataAnalysisTask.Analyser:     UserClass
RawAnalysis.DataAnalysisTask.UserClass:     yes
RawAnalysis.DataAnalysisTask.UserClass.Base:     RawDataAnalyser
RawAnalysis.DataAnalysisTask.StatusUpdateInterval:     20000

+Plugin.KVDataAnalyser:   RawDataAnalyser    KVRawDataAnalyser    KVMultiDetexp_events "KVRawDataAnalyser()"
~~~~
*/

class KVDataAnalysisTask: public KVBase {

   KVString fPrereq;           //prerequisite data directory i.e. data on which analysis task is performed
   KVString fAnalyser;         //name of KVDataAnalyser class used to perform analysis
   KVString fBaseClass;        //base class for user analysis
   Bool_t   fUserClass;        //true if analysis task requires user-supplied class (derived from fBaseClass)

   Bool_t   fBaseIsPlugin;     //true if base class for user analysis is in a plugin library
   KVString fPluginURI;        //uri of the plugin library containing user base class
   KVString fPluginBase;       //known base class extended by plugin library
   KVString fExtraAClicIncludes;  //to be added to AClic include paths before compilation
   Long64_t    fStatusUpdateInterval;//interval (number of events) after which batch job progress and status are updated

public:

   KVDataAnalysisTask();
   KVDataAnalysisTask(const KVDataAnalysisTask&);
   ROOT_COPY_ASSIGN_OP(KVDataAnalysisTask)
   virtual ~ KVDataAnalysisTask();

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   virtual void Copy(TObject&) const;
#else
   virtual void Copy(TObject&);
#endif
   virtual void SetPrereq(const Char_t* p)
   {
      fPrereq = p;
   };
   virtual const Char_t* GetPrereq() const
   {
      return fPrereq.Data();
   };

   virtual void ls(Option_t* opt = "") const;
   virtual void Print(Option_t* opt = "") const
   {
      ls(opt);
   };

   virtual void SetDataAnalyser(const Char_t* d)
   {
      fAnalyser = d;
   };
   virtual const Char_t* GetDataAnalyser() const
   {
      return fAnalyser;
   };
   virtual void SetUserBaseClass(const Char_t* d);
   virtual const Char_t* GetUserBaseClass() const
   {
      return fBaseClass;
   };
   virtual void SetWithUserClass(Bool_t w = kTRUE)
   {
      fUserClass = w;
   };
   virtual Bool_t WithUserClass() const
   {
      return fUserClass;
   };
   virtual void SetStatusUpdateInterval(Long64_t n)
   {
      fStatusUpdateInterval = n;
   }
   virtual Long64_t GetStatusUpdateInterval() const
   {
      return fStatusUpdateInterval;
   }

   virtual Bool_t CheckUserBaseClassIsLoaded();
   void SetExtraAClicIncludes(const KVString& list)
   {
      fExtraAClicIncludes = list;
   }

   ClassDef(KVDataAnalysisTask, 0)      //A data analysis task
};

#endif
