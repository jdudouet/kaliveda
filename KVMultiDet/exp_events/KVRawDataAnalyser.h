//Created by KVClassFactory on Thu Sep 24 11:07:45 2009
//Author: John Frankland,,,

#ifndef __KVRAWDATAANALYSER_H
#define __KVRAWDATAANALYSER_H

#include "KVDataSetAnalyser.h"
#include "KVHashList.h"
#include "KVRawDataReader.h"

class KVDetectorEvent;

class KVRawDataAnalyser : public KVDataSetAnalyser {
protected:

   KVRawDataReader* fRunFile;    //currently analysed run file
   Int_t fRunNumber;             //run number of current file
   Long64_t fEventNumber;        //event number in current run
   KVHashList fHistoList;        //list of histograms of user analysis

   virtual void ProcessRun();
   void clearallhistos(TCollection*);

   Long64_t TotalEntriesToRead;

   void AbortDuringRunProcessing();

public:
   Long64_t GetTotalEntriesToRead() const
   {
      return TotalEntriesToRead;
   }

   KVRawDataAnalyser();
   virtual ~KVRawDataAnalyser();

   KVDetectorEvent* GetDetectorEvent() const
   {
      Obsolete("GetDetectorEvent", "1.11", "2.0");
      return nullptr;
   }

   virtual void InitAnalysis() = 0;
   virtual void InitRun() = 0;
   virtual Bool_t Analysis() = 0;
   virtual void EndRun() = 0;
   virtual void EndAnalysis() = 0;

   Int_t GetRunNumber() const
   {
      return fRunNumber;
   }
   Long64_t GetEventNumber() const
   {
      return fEventNumber;
   }

   virtual Bool_t FileHasUnknownParameters() const
   {
      return (fRunFile->GetUnknownParameters()->GetSize() > 0);
   }
   virtual void SubmitTask();
   static void Make(const Char_t* kvsname = "MyOwnRawDataAnalyser");
   virtual void AddHisto(TH1*, const Char_t* /* family */ = 0);
   virtual void SaveSpectra(const Char_t* filename);
   virtual void ClearAllHistos();
   TH1* FindHisto(const Char_t* path);

   void CalculateTotalEventsToRead();

   const KVRawDataReader& GetRunFileReader() const
   {
      return *fRunFile;
   }

   ClassDef(KVRawDataAnalyser, 1) //Abstract base class for user analysis of raw data
};

#endif
