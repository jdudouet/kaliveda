//Created by KVClassFactory on Tue Jan 20 11:52:49 2015
//Author: ,,,

#include "KVFAZIADB.h"
#include "TSystem.h"
#include "KVNumberList.h"
#include "KVDataSetManager.h"
#include "KVRunListLine.h"

#include "KVDBParameterList.h"
#include "KVDBParameterSet.h"
#include "IRODS.h"
#include "TRandom.h"
#include "KVMultiDetArray.h"

KVFAZIADB* gFaziaDB;

ClassImp(KVFAZIADB)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVFAZIADB</h2>
<h4>database for FAZIA detector</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

void KVFAZIADB::init()
{
   //default initialisations

   fDBType = "FAZIADB";

   fExceptions = AddTable("Exceptions", "List signals with different PSA parameters");
   fCalibrations = AddTable("Calibrations", "Available calibration for FAZIA detectors");
   fOoODets = nullptr;
   //filenames to manage the data transfer via IRODS
   fDONEfile = "FromKVFAZIADB_transferdone.list";
   fFAILEDfile = "FromKVFAZIADB_transferfailed.list";

   Info("init", "file names for tranfert:\n%s\n%s", fDONEfile.Data(), fFAILEDfile.Data());
}

KVFAZIADB::KVFAZIADB(const Char_t* name): KVExpDB(name,
         "FAZIA experiment parameter database")
{
   init();
}


KVFAZIADB::KVFAZIADB(): KVExpDB("KVFAZIADB",
                                   "FAZIA experiment parameter database")
{
   init();
}

//___________________________________________________________________________
KVFAZIADB::~KVFAZIADB()
{
   //reset global pointer gFaziaDB if it was pointing to this database

   if (gFaziaDB == this)
      gFaziaDB = 0;
}

//___________________________________________________________________________
void KVFAZIADB::cd()
{
   KVExpDB::cd();
   gFaziaDB = this;

}

//__________________________________________________________________________________________________________________

void KVFAZIADB::Build()
{

   //Use KVINDRARunListReader utility subclass to read complete runlist

   //get full path to runlist file, using environment variables for the current dataset

   ReadNewRunList();
   ReadSystemList();
   ReadExceptions();
   ReadRutherfordCounting();
   ReadRutherfordCrossSection();
   ReadComments();
   KVMultiDetArray::MakeMultiDetector(fDataSet);
   gMultiDetArray->MakeCalibrationTables(this);
   ReadOoODetectors();

}

//____________________________________________________________________________
void KVFAZIADB::ReadNewRunList()
{
   //Read first runlist.dat file if exists
   //and then read
   //runsheets
   KVString line;
   KVFAZIADBRun* run = 0;
   TObjArray* toks = 0;

   std::ifstream fin;
   if (!OpenCalibFile("Runlist", fin)) {
      Error("ReadNewRunList()", "Could not open file %s",
            GetCalibFileName("Runlist").Data());
      return;
   }
   Info("ReadNewRunList()", "Reading run list ...");

   while (fin.good() && !fin.eof()) {
      line.ReadLine(fin);
      if (line.Length() > 1 && !line.BeginsWith("#") && !line.BeginsWith("Version")) {
         run = new KVFAZIADBRun;

         toks = line.Tokenize("|");
         for (Int_t ii = 0; ii < toks->GetEntries(); ii += 1) {
            KVString couple = ((TObjString*)toks->At(ii))->GetString();
            couple.Begin("=");
            KVString name = couple.Next();
            name = name.Strip(TString::kBoth);
            KVString value = "";
            if (!couple.End()) {
               value = couple.Next();
               value = value.Strip(TString::kBoth);
            }
            if (name == "run") {
               run->SetNumber(value.Atoi());
            }
            else if (name == "read events") {
               run->SetEvents(value.Atoi());
            }
            else if (name == "good events") {
               run->SetGoodEvents(value.Atoi());
            }
            else if (name == "starting date") {
               run->SetStartDate(value);
            }
            else if (name == "stopping date") {
               run->SetEndDate(value);
            }
            else if (name == "aqcuisition status") {
               run->SetACQStatus(value);
            }
            else if (name == "wrong number of blocks") {   //events rejected due to the wrong number of blocks
               run->SetError_WrongNumberOfBlocks(value.Atoi());
            }
            else if (name == "block errors") {   //events rejected due to internal error in one block
               run->SetError_InternalBlockError(value.Atoi());
            }
            else if (name == "nfiles") {   //number of acquisition files
               run->SetNumberOfAcqFiles(value.Atoi());
            }
            else if (name == "duration") {   //duration in seconds of the run
               run->SetDuration(value.Atof());
            }
            else if (name == "frequency") {   //number of evts per seconds (aquisition rate)
               run->SetFrequency(value.Atof());
            }
            else if (name == "triggerrate") {   //trigger rate
               run->SetTriggerRate(value.Atof());
            }
            else if (name == "mtrigger") {   //trigger multiplicity
               run->SetTrigger(value.Atof());
            }
            else if (name == "deadtime") {   //deadtime of the acquisition between 0 and 1
               run->SetDeadTime(value.Atof());
            }
            else if (name == "trig info") {   //number of trigger block in the acquisition file
               run->SetNumberOfTriggerBlocks(value.Atoi());
            }
            else if (name == "size(GB)") {   //size in GB
               run->SetSize(value.Atof());
            }
            else {
               //Info("ReadNewRunList","Unknown field %s=%s",name.Data(),value.Data());
            }
         }
         delete toks;
         if (run->GetNumber() < 1) {
            delete run;
         }
         else {
            //run->ReadRunSheet();
            AddRun(run);
         }
      }
   }
   fin.close();

   std::ifstream ffin;
   if (!OpenCalibFile("Runsheets", ffin)) {
      Error("ReadNewRunList()", "Could not open file %s",
            GetCalibFileName("Runsheets").Data());
      return;
   }
   Info("ReadNewRunList()", "Reading run sheets ...");

   Bool_t newrun = kFALSE;
   while (ffin.good() && !ffin.eof()) {
      line.ReadLine(ffin);
      if (line.Length() > 1 && !line.BeginsWith("#") && !line.BeginsWith("Version")) {

         toks = line.Tokenize("|");
         for (Int_t ii = 0; ii < toks->GetEntries(); ii += 1) {
            KVString couple = ((TObjString*)toks->At(ii))->GetString();
            couple.Begin("=");
            KVString name = couple.Next();
            name = name.Strip(TString::kBoth);
            KVString value = "";
            if (!couple.End()) {
               value = couple.Next();
               value = value.Strip(TString::kBoth);
            }
            if (name == "run") {
               Int_t number = value.Atoi();
               newrun = kFALSE;
               if (!(run = GetRun(number))) {
                  run = new KVFAZIADBRun();
                  run->SetNumber(number);
                  newrun = kTRUE;
               }
            }
            else if (name == "read events") {
               run->SetEvents(value.Atoi());
            }
            else if (name == "good events") {
               run->SetGoodEvents(value.Atoi());
            }
            else if (name == "starting date") {
               run->SetStartDate(value);
            }
            else if (name == "stopping date") {
               run->SetEndDate(value);
            }
            else if (name == "aqcuisition status") {
               run->SetACQStatus(value);
            }
            else if (name == "wrong number of blocks") {   //events rejected due to the wrong number of blocks
               run->SetError_WrongNumberOfBlocks(value.Atoi());
            }
            else if (name == "block errors") {   //events rejected due to internal error in one block
               run->SetError_InternalBlockError(value.Atoi());
            }
            else if (name == "nfiles") {   //number of acquisition files
               run->SetNumberOfAcqFiles(value.Atoi());
            }
            else if (name == "duration") {   //duration in seconds of the run
               run->SetDuration(value.Atof());
            }
            else if (name == "frequency") {   //number of evts per seconds (aquisition rate)
               run->SetFrequency(value.Atof());
            }
            else if (name == "triggerrate") {   //trigger rate
               run->SetTriggerRate(value.Atof());
            }
            else if (name == "mtrigger") {   //trigger multiplicity
               run->SetTrigger(value.Atof());
            }
            else if (name == "deadtime") {   //deadtime of the acquisition between 0 and 1
               run->SetDeadTime(value.Atof());
            }
            else if (name == "trig info") {   //number of trigger block in the acquisition file
               run->SetNumberOfTriggerBlocks(value.Atoi());
            }
            else {
               //Info("ReadNewRunList","Unknown field %s=%s",name.Data(),value.Data());
            }
         }
         delete toks;
         if (newrun) {
            if (run->GetNumber() < 1) {
               delete run;
            }
            else {
               AddRun(run);
            }
         }
      }
   }
   ffin.close();
}

//____________________________________________________________________________
Bool_t KVFAZIADB::TransferAcquisitionFileToCcali(TString file, TString ccali_rep, TString option)
{
//Perform the tranfer via irods to ccali for the indicated directory [file]
//return status of the iput command
// OK if the transfer is done
// not OK if it has failed
//

   IRODS ir;
   Int_t retour = ir.put(
                     Form("-%s %s", option.Data(), file.Data()),
                     Form("%s/%s/%s", ccali_rep.Data(), gDataSet->GetDataPathSubdir(), gDataSet->GetDataTypeSubdir("bin"))
                  );
   return (retour == 0);

   /*
   //some lines to test all the process
   Double_t val = gRandom->Uniform(0,1);
   printf("%s %lf\n",file.Data(),val);
   return (val<0.5);
   */

}

//____________________________________________________________________________
Bool_t KVFAZIADB::TransferRunToCcali(Int_t run, TString path, TString ccali_rep, TString option)
{
// test the status of the transfer
// write to the corresponding file : done or failed
//

   Bool_t retour = TransferAcquisitionFileToCcali(Form("%s/run%06d", path.Data(), run), ccali_rep, option);
   if (retour) {
      FILE* ff = fopen(fDONEfile.Data(), "a");
      fprintf(ff, "%d\n", run);
      fclose(ff);
   }
   else {
      FILE* ff = fopen(fFAILEDfile.Data(), "a");
      fprintf(ff, "%d\n", run);
      fclose(ff);
   }

   return retour;

}

//____________________________________________________________________________
void KVFAZIADB::TransferRunListToCcali(KVNumberList lrun, TString path, TString ccali_rep, TString option)
{

//Remove file where failed tranfer runs are listed
//
   KVNumberList lFAILED;
   gSystem->Exec(Form("rm %s", fFAILEDfile.Data()));

//Loop on run list
//If transfer fails, the run number is kept in a kvnumberlist
//for status at the end
//
   lrun.Begin();
   while (!lrun.End()) {
      Int_t run = lrun.Next();
      if (!TransferRunToCcali(run, path, ccali_rep, option)) {
         lFAILED.Add(run);
      }
   }

//print list of failed tranfers
   if (!lFAILED.IsEmpty())
      Warning("TransferRunListToCcali", "Transfer failed for %d runs :\n%s", lFAILED.GetNValues(), lFAILED.AsString());

}

//____________________________________________________________________________
void KVFAZIADB::StartTransfer(TString filename, TString ccali_rep, TString option)
{

   Int_t run;
   TString datadir = "";

//-----------
// list of runs for which transfer failed during the last tentative
//-----------
   KVNumberList lFAILED;
   KVFileReader fr;
   fr.OpenFileToRead(fFAILEDfile.Data());
   while (fr.IsOK()) {
      fr.ReadLine(0);
      if (fr.GetCurrentLine() != "") {
         run = fr.GetCurrentLine().Atoi();
         lFAILED.Add(run);
      }
   }
   fr.CloseFile();

//-----------
// list of runs for which transfer succeeded since the beginning
//-----------
   KVNumberList lDONE;
   fr.Clear();
   fr.OpenFileToRead(fDONEfile.Data());
   while (fr.IsOK()) {
      fr.ReadLine(0);
      if (fr.GetCurrentLine() != "") {
         run = fr.GetCurrentLine().Atoi();
         lDONE.Add(run);
      }
   }
   if (lDONE.GetNValues() > 0)
      Info("StartTransfer", "%d runs already tranfered:\n%s", lDONE.GetNValues(), lDONE.AsString());
   fr.CloseFile();

//-----------
// all runs produced
//-----------

   KVNumberList lALL;
   fr.Clear();
   fr.OpenFileToRead(filename.Data());
   while (fr.IsOK()) {

      fr.ReadLine("|");
      if (fr.GetCurrentLine().BeginsWith("#")) {
         KVString line = fr.GetCurrentLine();
         line.ReplaceAll("#", "");
         line = line.StripAllExtraWhiteSpace();
         line.Begin("=");
         KVString name = line.Next();
         if (!line.End() && name == "--dir") {
            datadir = line.Next();
            Info("ReadDBFile", "acquisition file directory: %s", datadir.Data());
         }
      }
      else {
         if (datadir == "") {
            fr.CloseFile();
            Warning("ReadDBFile", "no data directory has been found ...");
            return;
         }
         if (fr.GetNparRead() > 0) {
            for (Int_t ii = 0; ii < fr.GetNparRead(); ii += 1) {

               KVString couple = fr.GetReadPar(ii);
               couple.Begin("=");
               KVString name = couple.Next();
               name = name.Strip(TString::kBoth);
               KVString value = "";
               if (!couple.End()) {
                  value = couple.Next();
                  value = value.Strip(TString::kBoth);
               }
               if (name == "run") {
                  run = value.Atoi();
                  if (!lDONE.Contains(run))
                     lALL.Add(run);
               }
            }
         }
      }
   }
   fr.CloseFile();

   if (lALL.GetNValues() > 0) {
      Info("StartTransfer", "Start transfer for %d runs:\n%s", lALL.GetNValues(), lALL.AsString());
      TransferRunListToCcali(lALL, datadir.Data(), ccali_rep, option);
   }
   else
      Info("StartTransfer", "no runs to tranfer ...");

}


//__________________________________________________________________________________________________________________
const Char_t* KVFAZIADB::GetFileName(const Char_t* meth, const Char_t* keyw)
{

   TString basic_name = GetCalibFileName(keyw);
   if (basic_name == "") {
      Info(meth, "No name found for \"%s\" file", keyw);
      return "";
   }
   Info(meth, "Search for %s for dataset %s ...", basic_name.Data(), gDataSet->GetName());
   static TString fp;
   fp = gDataSet->GetFullPathToDataSetFile(basic_name.Data());
   if (fp == "") {
      Info(meth, "\tNo file found ...");
      return "";
   }
   return fp.Data();

}

//__________________________________________________________________________________________________________________
KVFileReader* KVFAZIADB::GetKVFileReader(const Char_t* meth, const Char_t* keyw)
{

   TString fp = GetFileName(meth, keyw);
   if (fp == "")
      return 0;

   KVFileReader* fr = new KVFileReader();
   if (!fr->OpenFileToRead(fp.Data())) {
      Error(meth, "Error in opening file %s", fp.Data());
      delete fr;
      return 0;
   }

   Info(meth, "Reading %s file", fp.Data());
   return fr;

}

//__________________________________________________________________________________________________________________
TEnv* KVFAZIADB::GetFileTEnv(const Char_t* meth, const Char_t* keyw)
{

   TString fp = GetFileName(meth, keyw);
   if (fp == "")
      return 0;

   Info(meth, "Reading %s file", fp.Data());
   TEnv* env = new TEnv();
   env->ReadFile(fp.Data(), kEnvAll);
   return env;

}

//__________________________________________________________________________________________________________________
void KVFAZIADB::ReadExceptions()
{
   KVFileReader* fr = GetKVFileReader("ReadExceptions()", "Exceptions");
   if (!fr)
      return;

   TList* ll = new TList();
   KVNumberList lruns;
   KVDBParameterList* dbp = 0;

   ll->SetOwner(kFALSE);
   while (fr->IsOK()) {
      fr->ReadLine(":");
      if (fr->GetNparRead() == 2) {
         if (fr->GetReadPar(0) == "RunRange") {
            if (ll->GetEntries() > 0) {
               LinkListToRunRange(ll, lruns);
               ll->Clear();
            }
            lruns.SetList(fr->GetReadPar(1));
            //printf("nouvelle plage : %s\n", lruns.AsString());
         }
         else {
            KVString name(fr->GetReadPar(0));
            name.Begin(".");
            KVString sdet = name.Next();

            KVString sig = name.Next();
            if (sig == "QH1" || sig == "QL1" || sig == "I1")   sdet.Prepend("SI1-");
            else if (sig == "Q2" || sig == "I2")        sdet.Prepend("SI2-");
            else if (sig == "Q3")                   sdet.Prepend("CSI-");

            KVString par = name.Next();
            if (!(dbp = (KVDBParameterList*)ll->FindObject(Form("%s.%s", sdet.Data(), sig.Data())))) {
               dbp = new KVDBParameterList(Form("%s.%s", sdet.Data(), sig.Data()), sdet.Data());
               dbp->AddKey("Runs", "List of Runs");
               fExceptions->AddRecord(dbp);
               ll->Add(dbp);
               dbp->GetParameters()->SetValue("RunRange", lruns.AsString());
               dbp->GetParameters()->SetValue("Detector", sdet.Data());
               dbp->GetParameters()->SetValue("Signal", sig.Data());
            }
            dbp->GetParameters()->SetValue(par.Data(), fr->GetDoubleReadPar(1));
         }
      }
   }

   if (ll->GetEntries() > 0) {
      LinkListToRunRange(ll, lruns);
      ll->Clear();
   }

   delete ll;
   delete fr;

}
//__________________________________________________________________________________________________________________
void KVFAZIADB::ReadComments()
{
   KVFileReader* fr = GetKVFileReader("ReadComments()", "Comments");
   if (!fr)
      return;

   KVFAZIADBRun* dbrun = 0;
   while (fr->IsOK()) {
      fr->ReadLine("|");
      if (fr->GetCurrentLine().BeginsWith("#")) {

      }
      else if (fr->GetCurrentLine() == "") {

      }
      else {
         if (fr->GetNparRead() == 2) {
            KVString srun(fr->GetReadPar(0));
            srun.Begin("=");
            srun.Next();
            KVNumberList lruns(srun.Next());
            KVString comments(fr->GetReadPar(1));
            lruns.Begin();
            while (!lruns.End()) {
               Int_t run = lruns.Next();
               dbrun = (KVFAZIADBRun*)GetRun(run);
               if (dbrun)
                  dbrun->SetComments(comments.Data());
            }
         }
      }
   }
   delete fr;

}

//__________________________________________________________________________________________________________________
void KVFAZIADB::ReadRutherfordCounting()
{

   TEnv* env = GetFileTEnv("ReadRutherfordCounting()", "RutherfordCounting");
   if (!env)
      return;

   TIter next(env->GetTable());
   TEnvRec* rec = 0;
   KVFAZIADBRun* dbrun = 0;
   while ((rec = (TEnvRec*)next())) {
      TString sname(rec->GetName());
      dbrun = GetRun(sname.Atoi());
      dbrun->SetRutherfordCount(TString(rec->GetValue()).Atoi());
   }
   delete env;
}

//__________________________________________________________________________________________________________________
void KVFAZIADB::ReadRutherfordCrossSection()
{
   KVFileReader* fr = GetKVFileReader("ReadRutherfordCrossSection()", "RutherfordCrossSection");
   if (!fr)
      return;

   KVFAZIADBRun* dbrun = 0;
   KVDBSystem* dbsys = 0;
   while (fr->IsOK()) {
      fr->ReadLine(":");
      if (fr->GetCurrentLine().BeginsWith("#")) {

      }
      else if (fr->GetCurrentLine() == "") {

      }
      else {
         if (fr->GetNparRead() == 2) {
            dbsys = GetSystem(fr->GetReadPar(0));
            if (dbsys) {
               Double_t val = fr->GetDoubleReadPar(1);
               TIter next(dbsys->GetRuns());
               while ((dbrun = (KVFAZIADBRun*)next())) {
                  dbrun->SetRutherfordCrossSection(val);
               }
            }
         }
      }
   }
   fr->CloseFile();
   delete fr;

}

//__________________________________________________________________________________________________________________
void KVFAZIADB::ReadCalibrationFiles()
{

   KVFileReader* fr = GetKVFileReader("ReadCalibrationFiles()", "CalibrationFiles");
   if (!fr)
      return;

   while (fr->IsOK()) {
      fr->ReadLine(0);
      if (fr->GetCurrentLine().BeginsWith("#") || fr->GetCurrentLine() == "") {}
      else {
         ReadCalibFile(fr->GetCurrentLine().Data());
      }
   }
   fr->CloseFile();
   delete fr;

}
//__________________________________________________________________________________________________________________
void KVFAZIADB::ReadCalibFile(const Char_t* filename)
{

   Bool_t find = kFALSE;
   TString fullpath = "";
   if (gSystem->Exec(Form("test -e %s/%s", GetDataSetDir(), filename)) == 0) {
      //Info("ReadCalibFile", "%s/%s do not exist", GetDataSetDir(), filename);
      //return;
      find = kTRUE;
      fullpath.Form("%s/%s", GetDataSetDir(), filename);
   }
   else if (gSystem->Exec(Form("test -e %s", filename)) == 0) {
      //Info("ReadCalibFile", "%s do not exist", filename);
      //return;
      find = kTRUE;
      fullpath.Form("%s", filename);
   }
   if (!find) {
      Info("ReadCalibFile", "%s does not exist or not found", filename);
      return;
   }

   Info("ReadCalibFile", "file : %s found", fullpath.Data());
   TEnv env;
   env.ReadFile(Form("%s", fullpath.Data()), kEnvAll);
   TIter next(env.GetTable());
   TEnvRec* rec = 0;
   KVDBParameterSet* par = 0;
   KVNumberList default_run_list = GetRunList();

   TString ssignal = "";
   TString stype = "";

   while ((rec = (TEnvRec*)next())) {
      TString sname(rec->GetName());
      if (sname == "Signal") {
         ssignal = rec->GetValue();
      }
      else if (sname == "CalibType") {
         stype = rec->GetValue();
      }
      else if (sname == "RunList") {
         default_run_list.Set(rec->GetValue());
      }
      else {
         KVString lval(rec->GetValue());
         Int_t np = 0;
         lval.Begin(",");
         while (!lval.End()) {
            KVString sval = lval.Next();
            np += 1;
         }
         par = new KVDBParameterSet(sname.Data(), stype.Data(), np);

         np = 0;
         lval.Begin(",");
         while (!lval.End()) {
            par->SetParameter(np++, lval.Next().Atof());
         }
         fCalibrations->AddRecord(par);
         LinkRecordToRunRange(par, default_run_list);
      }
   }

   if (ssignal == "") Error("ReadCalibFile", "No signal defined");
   if (stype == "") Error("ReadCalibFile", "No calibration type defined");
}

//__________________________________________________________________________________________________________________
void KVFAZIADB::ReadOoODetectors()
{

   //Lit le fichier ou sont listes les dedtecteurs ne marchant plus au cours
   //de la manip
   TEnv* env = GetFileTEnv("ReadOoODetectors()", "OoODet");
   if (!env)
      return;

   fOoODets = AddTable("OoO Detectors", "Name of out of order detectors");

   KVDBRecord* dbrec = 0;

   TEnvRec* rec = 0;

   TIter it(env->GetTable());
   while ((rec = (TEnvRec*)it.Next())) {
      KVString srec(rec->GetName());
      KVNumberList nl(rec->GetValue());

      if (srec.Contains(",")) {
         srec.Begin(",");
         while (!srec.End()) {
            dbrec = new KVDBRecord(srec.Next(), "OoO Detector");
            dbrec->AddKey("Runs", "List of Runs");
            fOoODets->AddRecord(dbrec);
            LinkRecordToRunRange(dbrec, nl);
         }
      }
      else {
         dbrec = new KVDBRecord(rec->GetName(), "OoO Detector");
         dbrec->AddKey("Runs", "List of Runs");
         fOoODets->AddRecord(dbrec);
         LinkRecordToRunRange(dbrec, nl);
      }
   }
   delete env;
}

//---------------------------------------
void KVFAZIADB::BuildQuickAndDirtyDataBase(TString acqfiles_dir)
//---------------------------------------
{
   /*
   Use this method in order to read runs written on disk
   during the experiment
   File is produced with main information for each run :
      number of files
      size
      starting date
   Information on runs has to completed after reading acquisition file
   specially information on trigger and deatdime

   usage : acqfiles_dir is the repository where acquisition files are written

   2 output files are written
   - runlist.dat containing all runs
   - database.log giving a summarize of stored data
   the two file names are set by default in .kvrootrc

   */
   if (gSystem->Exec(Form("test -d %s", acqfiles_dir.Data())) != 0) {
      Error("BuildQuickAndDirtyDataBase", "%s is not an existing directory", acqfiles_dir.Data());
      return;
   }

   TString sfout = GetCalibFileName("Runlist");
   FILE* fout = fopen(sfout.Data(), "w");
   fprintf(fout, "# DataSet %s\n", gDataSet->GetName());
   fprintf(fout, "# Runlist generated by KVFAZIADB::BuildQuickAndDirtyDataBase from files located at:\n");
   fprintf(fout, "# --dir=%s\n", acqfiles_dir.Data());

   TString sflog = GetCalibFileName("DBLog");
   FILE* flog = fopen(sflog.Data(), "w");
   fprintf(flog, "# DataSet %s\n", gDataSet->GetName());
   fprintf(flog, "# Log generated by KVFAZIADB::BuildQuickAndDirtyDataBase from files located at:\n");
   fprintf(flog, "# --dir=%s\n", acqfiles_dir.Data());

   Int_t run, nfiles;
   Long64_t size = 0;
   Long64_t totalsize = 0;
   Int_t date, idx;
   KVDatime kvdate;

   KVString ldir = gSystem->GetFromPipe(Form("du %s", acqfiles_dir.Data()));
   ldir.Begin("\n");
   Int_t numberofruns = 0;
   KVNumberList lruns;

   while (!ldir.End()) {
      KVString sdir = ldir.Next();
      sdir.Begin("\t");
      size = sdir.Next().Atoll(); // /TMath::Power(2,10)+1; //read values are in KB
      sdir = gSystem->BaseName(sdir.Next());
      if (sscanf(sdir.Data(), "run%d", &run) == 1) {
         totalsize += size;
         Int_t dmin = 0;
         Int_t dmax = 0;
         KVString lfile = gSystem->GetFromPipe(Form("ls %s/%s", acqfiles_dir.Data(), sdir.Data()));
         lfile.Begin("\n");
         nfiles = 0;
         while (!lfile.End()) {
            KVString sfile = lfile.Next();
            //FzEventSet-1434434675-41784.pb
            if (sscanf(sfile.Data(), "FzEventSet-%d-%d.pb", &date, &idx) == 2) {
               if (dmin == 0) {
                  dmin = dmax = date;
               }
               else {
                  if (date < dmin) dmin = date;
                  if (date > dmax) dmax = date;
               }
               nfiles += 1;
            }
         }
         if (nfiles == 0 || size == 0) {
            Warning("BuildQuickAndDirtyDataBase", "%d -> empty run", run);
         }
         kvdate.Set(dmin);
         fprintf(fout, "run=%d | starting date=%s | nfiles=%d | size(GB)=%1.2lf\n", run, kvdate.AsString(), nfiles, size * TMath::Power(2., -20));
         numberofruns += 1;
         lruns.Add(run);
      }
      else {
         Double_t conv = TMath::Power(2., -30);
         fprintf(flog, "total size %lld - %lld\n", size, totalsize);
         fprintf(flog, "total size (TB) %1.2lf - %1.2lf\n", size * conv, totalsize * conv);
         fprintf(flog, "number of runs %d\n", numberofruns);
         fprintf(flog, "last run %d done at %s\n", lruns.Last(), kvdate.AsString());
      }
   }

   fclose(fout);
   fclose(flog);
}
