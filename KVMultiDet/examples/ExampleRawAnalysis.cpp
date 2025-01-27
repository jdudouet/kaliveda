//Created by KVClassFactory on Fri Aug  2 15:34:58 2019
//Author: John Frankland,,,

#include "ExampleRawAnalysis.h"
#include "KVMultiDetArray.h"

ClassImp(ExampleRawAnalysis)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>exampleRawAnalysis</h2>
<h4>Analysis of raw data</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

// This class is derived from the KaliVeda class KVRawDataAnalyser.
// It is to be used for analysis of raw (i.e. unreconstructed) data.
//
// The following member functions are called in turn:
//
//    InitAnalysis():  called at the very beginning of the analysis
//    InitRun():       called everytime a run starts
//    Analysis():      called for each event
//    EndRun():        called everytime a run ends
//    EndAnalysis():   called after all requested data has been read
//
// Modify these methods as you wish in order to create your analysis class.
// Don't forget that for every class used in the analysis, you must put a
// line '#include' at the beginning of this file.

void ExampleRawAnalysis::InitAnalysis()
{
   // Declaration of histograms, trees, etc.
   // Called at the beginning of the analysis
   // The examples given are compatible with interactive and batch analyses.

   /*** DECLARING SOME HISTOGRAMS ***/
   AddHisto(new TH1F("Mult", "Number of fired detectors in each event", 1000, -.5, 999.5));

   /*** USING A TREE ***/
   CreateTreeFile();//<--- essential
   TTree* t = new TTree("myTree", "");
   AddTree(t);
   t->Branch("Name", &DetSigName);
   t->Branch("Value", &DetSigVal);

   /*** DEFINE WHERE TO SAVE THE RESULTS ***/
   // When running in batch mode, we use the job name for the output file
   if (gDataAnalyser->GetBatchSystem())
      SetCombinedOutputFile(Form("%s.root", gDataAnalyser->GetBatchSystem()->GetJobName()));
   else
      SetCombinedOutputFile(Form("ExampleRawAnalysis_results.root"));
}

//____________________________________________________________________________//

void ExampleRawAnalysis::InitRun()
{
   //Initialisation performed at beginning of each run
   //  GetRunNumber() returns current run number
   //  GetCurrentRun() returns KVDBRun pointer to current run in database
   //When this method is called, the detector geometry has been initialised and
   //can be accessed through global pointer gMultiDetArray

   Info("InitRun", "Beginning analysis of run %d containing %llu events", GetRunNumber(), GetCurrentRun()->GetEvents());

   // Set tree title to name of system being analysed
   GetTree("myTree")->SetTitle(GetSystem()->GetName());
}

//____________________________________________________________________________//

Bool_t ExampleRawAnalysis::Analysis()
{
   //Analysis method called for each event
   //  GetEventNumber() returns current event number
   //  GetRunFileReader() returns object used to read data (KVRawDataReader child class)
   //  gMultiDetArray->HandledRawData() returns kTRUE if interesting data was read
   //
   //  Processing will stop if this method returns kFALSE

   if (gMultiDetArray->HandledRawData()) {
      // loop over all detectors and count how many fired
      Mult = 0;
      TIter it(gMultiDetArray->GetDetectors());
      // Note - if experimental set up is a combination of two or more detector arrays,
      //        and you are interested in only one of them, you could use
      //           gMultiDetArray->GetArray("[name]")->GetDetectors()
      KVDetector* det;
      while ((det = (KVDetector*)it())) {
         if (det->Fired()) {
            ++Mult;
            // if detector has a signal "QH1FPGAEnergy", store it in tree
            if (det->HasDetectorSignalValue("QH1FPGAEnergy")) {
               DetSigName = det->GetName();
               DetSigVal = det->GetDetectorSignalValue("QH1FPGAEnergy");
               FillTree();
            }
         }
      }
      FillHisto("Mult", Mult);
   }

   return kTRUE;
}
