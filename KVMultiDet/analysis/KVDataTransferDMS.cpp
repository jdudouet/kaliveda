#include "KVDataTransferDMS.h"
#include "KVDataSetManager.h"

ClassImp(KVDataTransferDMS)

void KVDataTransferDMS::ExecuteCommand()
{
   // loop over runs
   GetRunList().Begin();
   while (!GetRunList().End()) {

      Int_t irun = GetRunList().Next();

      // name of file to transfer
      TString file_name = GetDataSet()->GetRunfileName(GetDataType(), irun);

      // full path to destination in target repository
      KVDataSet* targ_ds = fTargetRep->GetDataSetManager()->GetDataSet(GetDataSet()->GetName());
      TString dest_full = fTargetRep->GetFullPathToTransferFile(targ_ds, GetDataType(), file_name);

      // copy file
      fSourceRep->CopyFileFromRepository(GetDataSet(), GetDataType(), file_name, dest_full);
   }
}



