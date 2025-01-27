#include "KVReconNucTrajectory.h"
#include "KVIDTelescope.h"

ClassImp(KVReconNucTrajectory)

KVReconNucTrajectory::KVReconNucTrajectory(const KVReconNucTrajectory& o) :
   KVGeoDNTrajectory(), fIndependentIdentifications(0)
{
   // Copy constructor
   o.Copy(*this);
}

KVReconNucTrajectory::KVReconNucTrajectory(const KVGeoDNTrajectory* tr, const KVGeoDetectorNode* n) :
   KVGeoDNTrajectory(), fIndependentIdentifications(0)
{
   // Build a reconstructed trajectory on tr starting from node n

   fAddToNodes = kFALSE;
   KVUniqueNameList* idtlist = dynamic_cast<KVUniqueNameList*>(AccessIDTelescopeList());

   tr->SaveIterationState();// in case an iteration was already underway
   // add all nodes starting at n
   tr->IterateFrom(n);
   KVGeoDetectorNode* _n;
   while ((_n = tr->GetNextNode())) {
      AddLast(_n);
      fDetLabels[_n->GetDetector()->GetLabel()] = _n->GetDetector();
   }
   // add all ID telescopes from parent trajectory which contain only
   // detectors on this trajectory
   TIter next(tr->GetIDTelescopes());
   KVIDTelescope* idt;
   while ((idt = (KVIDTelescope*)next())) {
      if (ContainsAll(idt->GetDetectors())) {
         idtlist->Add(idt);
         if (idtlist->ObjectAdded()) fIndependentIdentifications += (Int_t)idt->IsIndependent();
      }
   }

   // unique name for fast look-up in hash table
   SetName(Form("%s_%s", tr->GetTrajectoryName(), n->GetName()));
   tr->RestoreIterationState();
}

KVReconNucTrajectory& KVReconNucTrajectory::operator=(const KVReconNucTrajectory& r)
{
   r.Copy(*this);
   return (*this);
}

void KVReconNucTrajectory::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVReconNucTrajectory::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVGeoDNTrajectory::Copy(obj);
   KVReconNucTrajectory& CastedObj = (KVReconNucTrajectory&)obj;
   CastedObj.fIndependentIdentifications = fIndependentIdentifications;
}

void KVReconNucTrajectory::ls(Option_t*) const
{
   KVGeoDNTrajectory::ls();
   std::cout << "Identifications [" << GetIDTelescopes()->GetEntries() << "/"
             << fIndependentIdentifications << "] : " << std::endl;
   TIter next(GetIDTelescopes());
   KVIDTelescope* idt;
   while ((idt = (KVIDTelescope*)next())) {
      std::cout << "\t" << idt->GetName() << " (" << idt->IsIndependent() << ")" << std::endl;
   }
}

KVDetector* KVReconNucTrajectory::GetDetector(const TString& label) const
{
   // Returns detector with given *label* on this trajectory.
   //
   // **N.B.** *label*, not *type*: several detectors of same *type* may occur on
   // trajectory. The geometry should be defined so that labels are unique on all trajectories.
   //
   // For example, given a reconstructed nucleus whose trajectory includes silicon detectors (same type)
   // with labels "SI1" and "SI2", here is how to access the "SI2" detector through which it passed:
   //
   //~~~~~~~{.cpp}
   //KVReconstructedNucleus* rnuc_p; /* pointer to reconstructed nucleus */
   //KVDetector* si2_det = rnuc_p->GetReconstructionTrajectory()->GetDetector("SI2");
   //~~~~~~~
   //
   // If no such detector exists, returns nullptr.

#ifdef WITH_CPP11
   auto it
#else
   std::map<std::string, KVDetector*>::const_iterator it
#endif
      = fDetLabels.find(label.Data());
   return it != fDetLabels.end() ? it->second : nullptr;
}

