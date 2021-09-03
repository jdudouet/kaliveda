//Created by KVClassFactory on Fri Jul 20 15:49:04 2018
//Author: eindra

#include "KVExpSetUpDB.h"
#include "KVDataSet.h"
#include "KVMultiDetArray.h"

#include <TKey.h>

ClassImp(KVExpSetUpDB)

void KVExpSetUpDB::ReadOoODetectors(const TString&)
{
   // Overrides base class method.
   //
   // Calls KVExpDB::ReadOoODetectors for each multidetector in the KVExpSetUp in turn with the
   // name of the array as value of the argument.
   //
   // This will create a table for each multidetector with names: NAME1.OoO Detectors, NAME2.OoODetectors, etc.

}

KVExpSetUpDB::KVExpSetUpDB()
   : KVExpDB()
{
   // Default constructor
}

//____________________________________________________________________________//

KVExpSetUpDB::KVExpSetUpDB(const Char_t* name)
   : KVExpDB(name)
{
   // Constructor inherited from KVExpDB
}

//____________________________________________________________________________//

KVExpSetUpDB::KVExpSetUpDB(const Char_t* name, const Char_t* title)
   : KVExpDB(name, title)
{
   // Constructor inherited from KVExpDB
}

//____________________________________________________________________________//

KVExpSetUpDB::~KVExpSetUpDB()
{
   // Destructor
}

void KVExpSetUpDB::Build()
{
   // Build the database.
   // Runs & Systems tables are handled by us, calibrations are handled by each multidetector

   FillRunsTable();
   ReadComments();
   ReadSystemList();
   ReadScalerInfos();

   KVMultiDetArray::MakeMultiDetector(fDataSet);
   gMultiDetArray->MakeCalibrationTables(this);
}

void KVExpSetUpDB::ReadScalerInfos()
{
   // Look for file scalers.root and read scalers from it
   // scalers are assumed to be stored as 64-bit parameters in the list

   TString runinfos = KVDataSet::GetFullPathToDataSetFile(fDataSet, "scalers.root");
   if (runinfos == "") return;

   Info("ReadScalerInfos", "Reading scaler infos from %s", runinfos.Data());
   TFile runinfos_file(runinfos);
   TIter it_run(GetRuns());
   KVDBRun* run;
   while ((run = (KVDBRun*)it_run())) {
      KVNameValueList* scalist = (KVNameValueList*)runinfos_file.Get(Form("run_%06d", run->GetNumber()));
      if (scalist) {
         int npar = scalist->GetNpar();
         for (int i = 0; i < npar; i += 2) {
            TString parname = scalist->GetParameter(i)->GetName();
            parname.Remove(parname.Index("_hi"), 3);
            run->SetScaler64(parname, scalist->GetValue64bit(parname));
         }
      }
   }
}

void KVExpSetUpDB::FillRunsTable()
{
   // Fill the Runs table using the informations in file runinfos.root
   // (which can be generated using KVRunListCreator).

   TString runinfos = KVDataSet::GetFullPathToDataSetFile(fDataSet, "runinfos.root");
   Info("FillRunsTable", "Reading run infos from %s", runinfos.Data());
   TFile runinfos_file(runinfos);
   TIter it(runinfos_file.GetListOfKeys());
   TKey* run_key;
   KVList garbage;
   while ((run_key = (TKey*)it())) {
      if (TString(run_key->GetClassName()) == "KVNameValueList") {
         // make sure we only use the highest cycle number of each key
         if (run_key->GetCycle() == runinfos_file.GetKey(run_key->GetName())->GetCycle()) {
            KVNameValueList* run = (KVNameValueList*)run_key->ReadObj();
            garbage.Add(run);
            KVDBRun* dbrun = new KVDBRun;
            dbrun->SetNumber(run->GetIntValue("Run"));
            dbrun->SetStartDate(run->GetStringValue("Start"));
            dbrun->SetEndDate(run->GetStringValue("End"));
            if (run->HasValue64bit("Size"))
               dbrun->SetSize(run->GetValue64bit("Size") / 1024. / 1024.);
            else
               dbrun->SetSize(run->GetIntValue("Size") / 1024. / 1024.);

            if (run->HasValue64bit("Events")) {
               dbrun->SetEvents(run->GetValue64bit("Events"));
            }
            else
               dbrun->SetEvents(run->GetIntValue("Events"));
            AddRun(dbrun);
         }
      }
   }
}

//____________________________________________________________________________//

