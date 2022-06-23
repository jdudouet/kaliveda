#ifndef __KVNUCLEUSEVENT_H
#define __KVNUCLEUSEVENT_H

#include "KVTemplateEvent.h"
#include "KVNucleus.h"

using KVNucleusEvent = KVTemplateEvent<KVNucleus>;

/**
  \class KVNucleusEvent
  \brief An event container for KVNucleus objects
    \ingroup NucEvents

  This is a typedef for KVTemplateEvent<KVNucleus>.

  An event made up of KVNucleus nuclei.

  \sa KVTemplateEvent, KVEvent, NucEvents
 */

using EventIterator = KVTemplateEvent<KVNucleus>::EventIterator;
using EventGroupIterator = KVTemplateEvent<KVNucleus>::EventGroupIterator;
using EventOKIterator = KVTemplateEvent<KVNucleus>::EventOKIterator;

/**
 \class EventIterator
 \brief Class for iterating over nuclei in events accessed through base pointer/reference
 \ingroup NucEvents

 Iterators are not defined for the abstract base class KVEvent. This class is a wrapper for
 the KVTemplateEvent<KVNucleus>::Iterator class which allows to use iterators with events passed as base references or pointers:

 ~~~~{.cpp}
 KVEvent* event; // pointer to valid event object

 for(auto& nuc : EventIterator(event)) { // loop over nuclei in event }
 for(auto& nuc : EventIterator(event, {"selection",[](const KVNucleus* n){ return n->GetZ()>2; }})) { // loop over nuclei with Z>2 in event }
 ~~~~
 */
/**
 \class EventOKIterator
 \brief Class for iterating over "OK" nuclei in events accessed through base pointer/reference
 \ingroup NucEvents

 Iterators are not defined for the abstract base class KVEvent. This class is a wrapper for
 the KVTemplateEvent<KVNucleus>::Iterator class which allows to use iterators with events passed as base references or pointers:

 ~~~~{.cpp}
 KVEvent* event; // pointer to valid event object

 for(auto& nuc : EventOKIterator(event)) { // loop over OK nuclei in event }
 ~~~~
 */
/**
 \class EventGroupIterator
 \brief Class for iterating over nuclei of given group in events accessed through base pointer/reference
 \ingroup NucEvents

 Iterators are not defined for the abstract base class KVEvent. This class is a wrapper for
 the KVTemplateEvent<KVNucleus>::Iterator class which allows to use iterators with events passed as base references or pointers:

 ~~~~{.cpp}
 KVEvent* event; // pointer to valid event object

 for(auto& nuc : EventGroupIterator(event,"groupname")) { // loop over nuclei of group "groupname" in event }
 ~~~~
 */
#endif
