#include <KVDataRepositoryManager.h>
#include <KVDataSetManager.h>
#include <KVEventReconstructor.h>
#include <KVFAZIADetector.h>
#include <KVMFMDataFileReader.h>
#include <iostream>
#include "TApplication.h"
#include "TROOT.h"
#include "KVINDRA.h"
#include "KVFAZIA.h"
#include "TH2.h"
using namespace std;

int main(int argc, char* argv[])
{
   KVBase::InitEnvironment();

   //TApplication myapp("myapp", &argc, argv);

   KVDataRepositoryManager drm;
   drm.Init();
   gDataSetManager->GetDataSet("INDRAFAZIA")->cd();

   KVMultiDetArray::MakeMultiDetector(gDataSet->GetName());
   gMultiDetArray->SetIdentifications();
   gMultiDetArray->SetGridsInTelescopes(12);
   gMultiDetArray->InitializeIDTelescopes();
   gMultiDetArray->PrintStatusOfIDTelescopes();

   unique_ptr<KVMFMDataFileReader> raw_file(KVMFMDataFileReader::Open(argv[1]));

   KVEventReconstructor erec(gMultiDetArray, new KVReconstructedEvent);
   //KVGroupReconstructor::SetDoIdentification(false);
   KVGroupReconstructor::SetDoCalibration(false);

   int first_frame = TString(argv[2]).Atoi();
   TFile f(Form("INDRAFAZIArecon_%d.root", first_frame), "recreate");

   TTree* tree = new TTree("ReconEvents", "Reconstructed INDRA-FAZIA events");
   KVEvent::MakeEventBranch(tree, "ReconEvent", "KVReconstructedEvent", erec.GetEventReference());

   TTree* idtree = new TTree("Ident", "Status identifications");
   int z, a;
   TString detid, array;
   idtree->Branch("array", &array);
   idtree->Branch("detid", &detid);
   idtree->Branch("z", &z);
   idtree->Branch("a", &a);

   KVHashList my_csi_hists;
   TIter nxt_csi(gFazia->GetIDTelescopesWithType("CsI"));
   KVIDTelescope* idt;
   while ((idt = (KVIDTelescope*)nxt_csi())) {
      if (((KVFAZIADetector*)idt->GetDetector(1))->GetBlockNumber() == 1)
         my_csi_hists.Add(new TH2F(idt->GetName(), idt->GetName(), 250, 0, 2500, 400, 0, 4000));
   }

   int i = 0;
   while (raw_file->GetNextEvent()) {
      ++i;
      if (i != first_frame) continue;
      first_frame += 20;
      //cout << "=================================================================" << endl;
      if (gMultiDetArray->HandleRawDataEvent(raw_file.get())) {
         erec.ReconstructEvent(gMultiDetArray->GetFiredDataParameters());
         erec.GetEvent()->SetNumber(i + 1);
         if (gFazia->HandledRawData()) {
            nxt_csi.Reset();
            while ((idt = (KVIDTelescope*)nxt_csi())) {
               if (idt->GetDetector(1)->Fired()) {
                  ((TH2*)my_csi_hists.FindObject(idt->GetName()))->Fill(idt->GetIDMapX(), idt->GetIDMapY());
               }
            }
         }
         tree->Fill();
         if (erec.GetEvent()->GetMult()) {
            for (KVEvent::Iterator it = erec.GetEvent()->begin(); it != erec.GetEvent()->end(); ++it) {
               KVReconstructedNucleus& n = it.reference<KVReconstructedNucleus>();
               if (n.IsIdentified()) {
                  if (n.IsZMeasured()) {
                     z = n.GetZ();
                     if (n.IsAMeasured()) a = n.GetA();
                     else a = -1;
                     array = n.GetParameters()->GetTStringValue("ARRAY");
                     detid = n.GetIdentifyingTelescope()->GetName();
                     idtree->Fill();
                  }
               }
            }
         }
      }
      else {
         cout << "Frame " << i << " not handled by anybody:" << endl;
         raw_file->PrintFrameRead();
      }
      if (!(i % 10000)) cout << "Treated " << i << " frames..." << endl;
   }

   f.Write();
}