#include "KVEvent.h"
#include "KVNucleus.h"

ClassImp(KVEvent)

void KVEvent::Streamer(TBuffer& R__b)
{
   // Customised Streamer for KVEvent.
   // This is just the automatic Streamer with the addition of a call to the Clear()
   // method before reading a new object (avoid memory leaks with lists of parameters).

   if (R__b.IsReading()) {
      Clear();
      R__b.ReadClassBuffer(KVEvent::Class(), this);
   }
   else {
      R__b.WriteClassBuffer(KVEvent::Class(), this);
   }
}

KVNucleus* KVEvent::GetNextNucleus(Option_t* opt) const
{
   // \warning Only use with events containing objects derived from KVNucleus
   //
   // Use this method to iterate over the list of nuclei in the event
   // After the last particle GetNextNucleus() returns a null pointer and
   // resets itself ready for a new iteration over the particle list.
   //
   // If opt="" all particles are included in the iteration.
   // If opt="ok" or "OK" only nuclei whose KVNucleus::IsOK() method returns kTRUE are included.
   //
   // Any other value of opt is interpreted as a (case-insensitive) particle group name: only
   // particles with BelongsToGroup(opt) returning kTRUE are included.
   //
   // If you want to start from the beginning again before getting to the end
   // of the list, especially if you want to change the selection criteria,
   // call method ResetGetNextNucleus() before continuing.
   //
   // If you interrupt an iteration before the end, then start another iteration
   // without calling ResetGetNextNucleus(), even if you change the argument of
   // the call to GetNextNucleus(), you will repeat exactly the same iteration
   // as the previous one.
   //
   // \warning Only one iteration at a time over the event can be performed
   //          using this method. If you want/need to perform several i.e. nested
   //          iterations, see KVTemplateEvent::EventIterator

   return dynamic_cast<KVNucleus*>(GetNextParticle(opt));
}

KVNucleus* KVEvent::GetNucleus(Int_t npart) const
{
   // \warning Only use with events containing objects derived from KVNucleus
   //
   // \param[in] npart index of particle in event, which is a non-zero value from 1 to the value returned by GetMult()
   // \returns pointer to the particle with index npart

   return dynamic_cast<KVNucleus*>(GetParticle(npart));
}

KVNucleus* KVEvent::AddNucleus()
{
   // Add a particle to the event
   //
   // \returns pointer to new particle if it inherits from KVNucleus, nullptr if not
   return dynamic_cast<KVNucleus*>(AddParticle());
}


