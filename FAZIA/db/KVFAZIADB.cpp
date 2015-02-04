//Created by KVClassFactory on Tue Jan 20 11:52:49 2015
//Author: ,,,

#include "KVFAZIADB.h"
#include "KVNumberList.h"
#include "KVDataSetManager.h"
#include "KVRunListLine.h"

KVFAZIADB *gFaziaDB;

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
   kFirstRun = 0;
   kLastRun = 0;

   fRuns = AddTable("Runs", "List of available runs");
   fSystems = AddTable("Systems", "List of available systems");
}

KVFAZIADB::KVFAZIADB(const Char_t * name):KVDataBase(name,
                                                     "FAZIA experiment parameter database")
{
   init();
}


KVFAZIADB::KVFAZIADB():KVDataBase("KVFAZIADB",
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
	KVDataBase::cd();
	gFaziaDB = this;

}
//__________________________________________________________________________________________________________________

Bool_t KVFAZIADB::OpenCalibFile(const Char_t * type, std::ifstream & fs) const
{
   //Find and open calibration parameter file of given type. Return kTRUE if all OK.
   //types are defined in $KVROOT/KVFiles/.kvrootrc by lines such as
   //
   //# Default name for file describing systems for each dataset.
   //INDRADB.Systems:     Systems.dat
   //
   //A file with the given name will be looked for in $KVROOT/KVFiles/name_of_dataset
   //where name_of_dataset is given by KVDataBase::fDataSet.Data()
   //i.e. the name of the associated dataset.
   //
   //Filenames specific to a given dataset may also be defined:
   //
   //INDRA_camp5.INDRADB.Pedestals:      Pedestals5.dat
   //
   //where 'INDRA_camp5' is the name of the dataset in question.
   //GetDBEnv() always returns the correct filename for the currently active dataset.

   return KVBase::SearchAndOpenKVFile(GetDBEnv(type), fs, fDataSet.Data());
}
//__________________________________________________________________________________________________________________

const Char_t *KVFAZIADB::GetDBEnv(const Char_t * type) const
{
   //Will look for gEnv->GetValue name "name_of_dataset.INDRADB.type"
   //then "INDRADB.type" if no dataset-specific value is found.

   if (!gDataSetManager)
      return "";
   KVDataSet *ds = gDataSetManager->GetDataSet(fDataSet.Data());
   if (!ds)
      return "";
   return ds->GetDataSetEnv(Form("INDRADB.%s", type));
}

//_____________________________________________________________________
void KVFAZIADB::LinkRecordToRunRange(KVDBRecord * rec, UInt_t first_run,
                                     UInt_t last_run)
{
   //If the KVDBRecord 'rec' (i.e. set of calibration parameters, reaction system, etc.) is
   //associated to, or valid for, a range of runs, we use this method in order to link the record
   //and the runs. The list of associated runs will be kept with the record, and each of the runs
   //will have a link to the record.

   for (UInt_t ii = first_run; ii <= last_run; ii++) {
      LinkRecordToRun(rec,ii);
   }
}
//_____________________________________________________________________
void KVFAZIADB::LinkRecordToRunRange(KVDBRecord * rec, KVNumberList nl)
{
   //If the KVDBRecord 'rec' (i.e. set of calibration parameters, reaction system, etc.) is
   //associated to, or valid for, a range of runs, we use this method in order to link the record
   //and the runs. The list of associated runs will be kept with the record, and each of the runs
   //will have a link to the record.
	nl.Begin(); 
	while (!nl.End()){
		Int_t rr = nl.Next();
		//Info("LinkRecordToRunRange","run number %d",rr);
   	LinkRecordToRun(rec,rr);
	}
}

//_____________________________________________________________________
void KVFAZIADB::LinkRecordToRun(KVDBRecord * rec, Int_t rnumber)
{

	KVFAZIADBRun *run = GetRun(rnumber);
	if (run)
		rec->AddLink("Runs", run);

}

//_____________________________________________________________________
void KVFAZIADB::LinkRecordToRunRanges(KVDBRecord * rec, UInt_t rr_number,
                                      UInt_t run_ranges[][2])
{
   //Call LinkRecordToRunRange for a set of run ranges stored in the two-dimensional array
   //in the following way:
   //      run_ranges[0][0] = first run of first run range
   //      run_ranges[0][1] = last run of first run range
   //      run_ranges[1][0] = first run of second run range etc. etc.
   //rr_number is the number of run ranges in the array

   for (UInt_t i = 0; i < rr_number; i++) {
      LinkRecordToRunRange(rec, run_ranges[i][0], run_ranges[i][1]);
   }
}

//______________________________________________________________________________
void KVFAZIADB::LinkListToRunRanges(TList * list, UInt_t rr_number,
                                    UInt_t run_ranges[][2])
{
   //Link the records contained in the list to the set of runs (see LinkRecordToRunRanges).

   if (!list) {
      Error("LinkListToRunRanges",
            "NULL pointer passed for parameter TList");
      return;
   }
   if (list->GetSize() == 0) {
      Error("LinkListToRunRanges(TList*,UInt_t,UInt_t*)",
            "The list is empty");
      return;
   }
   TIter next(list);
   KVDBRecord *rec;
   for (UInt_t ru_ra = 0; ru_ra < rr_number; ru_ra++) {
      UInt_t first_run = run_ranges[ru_ra][0];
      UInt_t last_run = run_ranges[ru_ra][1];
      for (UInt_t i = first_run; i <= last_run; i++) {
         KVFAZIADBRun *run = GetRun(i);
         while ((rec = (KVDBRecord *) next())) {
            if (run)
               rec->AddLink("Runs", run);
         }
         next.Reset();
      }
   }
}
//______________________________________________________________________________
void KVFAZIADB::LinkListToRunRange(TList * list, KVNumberList nl)
{
   //Link the records contained in the list to the set of runs (see LinkRecordToRunRanges).

   if (!list) {
      Error("LinkListToRunRange",
            "NULL pointer passed for parameter TList");
      return;
   }
   if (list->GetSize() == 0) {
      Error("LinkListToRunRange(TList*,KVNumberList)",
            "The list is empty");
      return;
   }
   TIter next(list);
   KVDBRecord *rec;
  	while ((rec = (KVDBRecord *) next())) {
   	LinkRecordToRunRange(rec, nl);
	}
}

//____________________________________________________________________________
void KVFAZIADB::ReadSystemList()
{
   //Reads list of systems with associated run ranges, creates KVDBSystem
   //records for these systems, and links them to the appropriate KVFAZIADBRun
   //records using LinkListToRunRanges.
   //
   //There are 2 formats for the description of systems:
   //
   //+129Xe + natSn 25 MeV/A           '+' indicates beginning of system description
   //129 54 119 50 0.1 25.0            A,Z of projectile and target, target thickness (mg/cm2), beam energy (MeV/A)
   //Run Range : 614 636               runs associated with system
   //Run Range : 638 647               runs associated with system
   //
   //This is sufficient in the simple case where the experiment has a single
   //layer target oriented perpendicular to the beam. However, for more
   //complicated targets we can specify as follows :
   //
   //+155Gd + 238U 36 MeV/A
   //155 64 238 92 0.1 36.0
   //Target : 3 0.0                    target with 3 layers, angle 0 degrees
   //C 0.02                            1st layer : carbon, 20 g/cm2
   //238U 0.1                          2nd layer : uranium-238, 100 g/cm2
   //C 0.023                           3rd layer : carbon, 23 g/cm2
   //Run Range : 770 804
   //
   //Lines beginning '#' are comments.


   std::ifstream fin;
   if (OpenCalibFile("Systems", fin)) {
      Info("ReadSystemList()", "Reading Systems parameters ...");

      TString line;

      char next_char = fin.peek();
      while( next_char!='+' && fin.good() ){
         line.ReadLine(fin, kFALSE);
         next_char = fin.peek();
      }

      while( fin.good() && !fin.eof() && next_char=='+' ){
         KVDBSystem* sys = new KVDBSystem("NEW SYSTEM");
         AddSystem(sys);
         sys->Load(fin);
       next_char = fin.peek();
      }
      fin.close();
   }
   else {
      Error("ReadSystemList()", "Could not open file %s",
            GetCalibFileName("Systems"));
   }
   // if any runs are not associated with any system
   // we create an 'unknown' system and associate it to all runs
   KVDBSystem* sys = 0;
   TIter nextRun(GetRuns());
   KVFAZIADBRun* run;
   while ( (run = (KVFAZIADBRun*)nextRun()) ) {
      if(!run->GetSystem()){
         if(!sys) {
            sys = new KVDBSystem("[unknown]");
            AddSystem(sys);
         }
         sys->AddRun(run);
      }
   }
}

void KVFAZIADB::WriteSystemsFile() const
{
   //Write the 'Systems.dat' file for this database.
   //The actual name of the file is given by the value of the environment variable
   //[dataset_name].INDRADB.Systems (if it exists), otherwise the value of
   //INDRADB.Systems is used. The file is written in the
   //$KVROOT/[dataset_directory] directory.

   std::ofstream sysfile;
   KVBase::SearchAndOpenKVFile(GetDBEnv("Systems"), sysfile, fDataSet.Data());
   TIter next(GetSystems()); KVDBSystem* sys;
   TDatime now;
   sysfile << "# " << GetDBEnv("Systems") << " file written by "
         << ClassName() << "::WriteSystemsFile on " << now.AsString() << std::endl;
   std::cout << GetDBEnv("Systems") << " file written by "
         << ClassName() << "::WriteSystemsFile on " << now.AsString() << std::endl;
   while( (sys = (KVDBSystem*)next()) ){
      if(strcmp(sys->GetName(),"[unknown]")){//do not write dummy 'unknown' system
         sys->Save(sysfile);
         sysfile << std::endl;
      }
   }
   sysfile.close();
}

//__________________________________________________________________________________________________________________

void KVFAZIADB::Save(const Char_t* what)
{
   //Save (in the appropriate text file) the informations on:
   // what = "Systems" : write Systems.dat file
   // what = "Runlist" : write Runlist.csv
   if( !strcmp(what, "Systems") ) WriteSystemsFile();
   else if( !strcmp(what, "Runlist") ) WriteRunListFile();
}

void KVFAZIADB::WriteRunListFile() const
{
   //Write a file containing a line describing each run in the database.
   //The delimiter symbol used in each line is '|' by default.
   //The first line of the file will be a header description, given by calling
   //KVFAZIADBRun::WriteRunListHeader() for the first run in the database.
   //Then we call KVFAZIADBRun::WriteRunListLine() for each run.
   //These are virtual methods redefined by child classes of KVFAZIADBRun.

   std::ofstream rlistf;
   KVBase::SearchAndOpenKVFile(GetDBEnv("Runlist"), rlistf, fDataSet.Data());
   TDatime now;
   rlistf << "# " << GetDBEnv("Runlist") << " file written by "
         << ClassName() << "::WriteRunListFile on " << now.AsString() << std::endl;
   std::cout << GetDBEnv("Runlist") << " file written by "
         << ClassName() << "::WriteRunListFile on " << now.AsString() << std::endl;
   TIter next_run(GetRuns());
   //write header in file
   ((KVFAZIADBRun *) GetRuns()->At(0))->WriteRunListHeader(rlistf, GetDBEnv("Runlist.Separator")[0]);
   KVFAZIADBRun *run;
   while ((run = (KVFAZIADBRun *) next_run())) {

      run->WriteRunListLine(rlistf, GetDBEnv("Runlist.Separator")[0]);

   }
   rlistf.close();
}

//__________________________________________________________________________________________________________________

void KVFAZIADB::Build()
{

   //Use KVINDRARunListReader utility subclass to read complete runlist

   //get full path to runlist file, using environment variables for the current dataset
   TString runlist_fullpath;
   KVBase::SearchKVFile(GetDBEnv("Runlist"), runlist_fullpath, fDataSet.Data());

   //set comment character for current dataset runlist
   //SetRLCommentChar(GetDBEnv("Runlist.Comment")[0]);

   //set field separator character for current dataset runlist
   /*
   if (!strcmp(GetDBEnv("Runlist.Separator"), "<TAB>"))
      SetRLSeparatorChar('\t');
   else
      SetRLSeparatorChar(GetDBEnv("Runlist.Separator")[0]);
	*/
   //by default we set two keys for both recognising the 'header' lines and deciding
   //if we have a good run line: the "Run" and "Events" fields must be present
   /*
   GetLineReader()->SetFieldKeys(2, GetDBEnv("Runlist.Run"),
                                 GetDBEnv("Runlist.Events"));
   GetLineReader()->SetRunKeys(2, GetDBEnv("Runlist.Run"),
                               GetDBEnv("Runlist.Events"));
	*/
   
   kFirstRun = 999999;
   kLastRun = 0;
   /*
   ReadRunList(runlist_fullpath.Data());
   //new style runlist
   if( IsNewRunList() ){ ReadNewRunList(); };
	*/
   ReadNewRunList();
   ReadSystemList();
   
}


//____________________________________________________________________________
void KVFAZIADB::ReadNewRunList()
{
   //Read new-style runlist (written using KVFAZIADBRun v.10 or later)

   std::ifstream fin;
   if (!OpenCalibFile("Runlist", fin)) {
      Error("ReadNewRunList()", "Could not open file %s",
            GetCalibFileName("Runlist"));
      return;
   }

   Info("ReadNewRunList()", "Reading run parameters ...");

   KVString line;
   KVFAZIADBRun* run;
	TObjArray* toks = 0;
   
   while( fin.good() && !fin.eof() ){
      line.ReadLine( fin );
		
      if( line.Length()>1 && !line.BeginsWith("#") && !line.BeginsWith("Version") ){
         run = new KVFAZIADBRun;
         //run->ReadRunListLine( line );
         toks = line.Tokenize(" | ");
         for (Int_t ii=0;ii<toks->GetEntries();ii+=1){
         	KVString couple = ((TObjString* )toks->At(ii))->GetString();
            couple.Begin("=");
            KVString name = couple.Next();
            KVString value="";
            if (!couple.End())
            	value = couple.Next();
            if (name=="Run"){
            	run->SetNumber(value.Atoi());	
            }
            if (name=="Events"){
            	run->SetEvents(value.Atoi());	
            }
            if (name=="Trigger"){
            	run->SetTrigger(value.Atoi());	
            }
         }
         delete toks;
         if( run->GetNumber()<1 ){
            delete run;
         } else {
            AddRun( run );
            kLastRun = TMath::Max(kLastRun, run->GetNumber());
            kFirstRun = TMath::Min(kFirstRun, run->GetNumber());
         }
      }
   }
 
   fin.close();
}

//__________________________________________________________________________________________________________________

void KVFAZIADB::PrintRuns(KVNumberList& nl) const
{
	// Print compact listing of runs in the number list like this:
	//
	// root [9] gFaziaDB->PrintRuns("8100-8120")
	// RUN     SYSTEM                          TRIGGER         EVENTS          COMMENTS
	// ------------------------------------------------------------------------------------------------------------------
	// 8100    129Xe + 58Ni 8 MeV/A            M>=2            968673
	// 8101    129Xe + 58Ni 8 MeV/A            M>=2            969166
	// 8102    129Xe + 58Ni 8 MeV/A            M>=2            960772
	// 8103    129Xe + 58Ni 8 MeV/A            M>=2            970029
	// 8104    129Xe + 58Ni 8 MeV/A            M>=2            502992          disjonction ht chassis 1
	// 8105    129Xe + 58Ni 8 MeV/A            M>=2            957015          intensite augmentee a 200 pA
	
	printf("RUN\tSYSTEM\t\t\t\tTRIGGER\t\tEVENTS\t\tCOMMENTS\n");
	printf("------------------------------------------------------------------------------------------------------------------\n");
	nl.Begin();
	while( !nl.End() ){
		KVFAZIADBRun* run = GetRun(nl.Next());
		if(!run) continue;
		printf("%4d\t%-30s\t%s\t\t%d\t\t%s\n",
				run->GetNumber(), (run->GetSystem()?run->GetSystem()->GetName():"            "), run->GetTriggerString(),
				run->GetEvents(), run->GetComments());
	}
}
