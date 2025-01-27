#ifndef __KVMACROS_H__
#define __KVMACROS_H__

// General macros should be defined here to ensure consistency across the code
// base.
//
// Added by Peter Wigg (4/12/2015)

#define IGNORE_UNUSED(x) (void)(x)

#define Deprecate(x) KVBase::Deprecated(__PRETTY_FUNCTION__,x)
#endif
