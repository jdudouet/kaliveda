#ifndef __KVDATATRANSFERDMS_H
#define __KVDATATRANSFERDMS_H

#include "KVDataTransfer.h"

/**
 \class KVDataTransferDMS
 \brief Transfer data from/to a DMS data repository

 If one or other of two repositories uses a DMS (i.e. IRODS or SRB), data can be transferred between them
 using this class. To initiate a transfer from repository "A" to repository "B":

~~~~{.cpp}
KVDataTransfer::NewTransfer("A","B")->Run();
~~~~

 \author eindra
 \date Tue May  3 09:16:55 2022
*/

class KVDataTransferDMS : public KVDataTransfer {
public:
   virtual void ExecuteCommand();
   virtual void WriteTransferScript() {}

   ClassDef(KVDataTransferDMS, 1) //Transfer data from/to a DMS data repository
};

#endif
