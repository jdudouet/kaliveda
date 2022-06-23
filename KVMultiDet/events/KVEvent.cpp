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
   } else {
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

void KVEvent::Copy(TObject& obj) const
{
   // Copy this event into the object referenced by obj,
   // assumed to be at least derived from KVEvent.
   KVBase::Copy(obj);
   fParameters.Copy(((KVEvent&)obj).fParameters);
   Int_t MTOT = fParticles->GetEntriesFast();
   for (Int_t nn = 0; nn < MTOT; nn += 1) {
      GetParticle(nn + 1)->Copy(*((KVEvent&) obj).AddParticle());
   }
}

void KVEvent::MergeEventFragments(TCollection* events, Option_t* opt)
{
   // Merge all events in the list into one event (this one)
   //
   // We also merge/sum the parameter lists of the events
   //
   // First we clear this event, then we fill it with copies of each particle in each event
   // in the list.
   //
   // If option "opt" is given, it is given as argument to each call to
   // KVEvent::Clear() - this option is then passed on to the KVParticle::Clear()
   // method of each particle in each event.
   //
   // \param[in] events A list of events to merge
   // \param[in] opt Optional argument transmitted to KVEvent::Clear()
   //
   // \note the events in the list will be empty and useless after this!

   Clear(opt);
   TIter it(events);
   KVEvent* e;
   while ((e = (KVEvent*)it())) {
      e->ResetGetNextParticle();
      while (auto n = e->GetNextParticle()) {
         n->Copy(*AddParticle());
      }
      GetParameters()->Merge(*(e->GetParameters()));
      e->Clear(opt);
   }
}

