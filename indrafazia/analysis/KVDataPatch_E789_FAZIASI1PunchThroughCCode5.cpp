#include "KVDataPatch_E789_FAZIASI1PunchThroughCCode5.h"



void KVDataPatch_E789_FAZIASI1PunchThroughCCode5::PrintPatchInfo() const
{
   std::cout << "In 1.12/05 unidentified particles apparently stopped in SI2, with good Si1-PSA identification\n";
   std::cout << "were added to the event, (with CCode=5) but they (mostly) correspond to\n";
   std::cout << "punch-through in SI1 and give bad 'accumulations' of data in e.g. Z-V plots.\n\n";

   std::cout << "So they are best left unidentified! (they still keep CCode value 5)\n";
   std::cout << "They receive idcode KVFAZIA::ID_SI1_PUNCH_THROUGH, are identified, but not in Z (or A).\n";
}

ClassImp(KVDataPatch_E789_FAZIASI1PunchThroughCCode5)


