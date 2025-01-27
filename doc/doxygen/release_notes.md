\page release_notes Release Notes for KaliVeda

Last update: 28th March 2022

## Version 1.12/06 (Released: 28/03/2022)

__Changes in__ \ref FAZIAnal : __Bugfix for raw FAZIA data analysis__

When analysing raw FAZIA data, detectors were not reset before reading a new event, leading to a steadily increasing number of
fired detectors with each new event. A small change to KVMultiDetArray::prepare_to_handle_new_raw_data() fixes this.

__Changes in Build System__

Starting from v1.12/05, we require a minimum ROOT version of 6.18, minimum cmake version 3.5,
and a compiler (the same as that used to compile ROOT) with at least support for C++14.

__Changes in__ \ref INDRAnalysis / \ref FAZIAnal / \ref INDRAFAZIAnalysis : __New methods and symbols to handle identification/calibration codes__

New symbolic names (enumerations) have been added to clarify the meanings of the different identification and calibration
codes for reconstructed particles detected in different arrays. See KVINDRA::IDCodes and KVFAZIA::IDCodes.

There are also some new methods to facilitate selection of appropriate codes for analysis of data in user's
KVEventSelector::InitRun() method:

 + Calling one or both of
   ~~~~{.cpp}
   gMultiDetArray->AcceptAllIDCodes();
   gMultiDetArray->AcceptAllECodes();
   ~~~~
   will remove any selection of particles according to one or the other codes. Note that these methods are overridden in KVExpSetUp
   in order to apply to *ALL* multidetectors in the setup, thus for E789 data this will apply to both INDRA *AND* FAZIA.

 + Previously, to select the codes the user had to pass a string containing the (explicit) numerical values of the codes,
   like so:
   ~~~~{.cpp}
   gMultiDetArray->GetArray("INDRA")->AcceptIDCodes("2,3,5");
   ~~~~
   It is now possible, and highly preferable, to use the newly-defined numeric constants, like so:
   ~~~~{.cpp}
   gMultiDetArray->GetArray("INDRA")->AcceptIDCodes( {KVINDRA::IDCodes::ID_CSI_PSA,
                                                      KVINDRA::IDCodes::ID_SI_CSI,
                                                      KVINDRA::IDCodes::ID_STOPPED_IN_FIRST_STAGE} );
   ~~~~
   which makes the selection criteria far more human-compatible. Note the use of `{}` to enclose the list of values.
   
__Change in__ \ref INDRAFAZIAnalysis : __Rejecting events based on DAQ trigger conditions (E789)__

FAZIA trigger conditions for each run of E789 have now been implemented. Calling SetTriggerConditionsForRun() in the
InitRun() method of an analysis class used on E789 data will now reject any event which does not have a FAZIA trigger
bit pattern compatible with that which is expected for the data.

In concrete terms, this means that for analysis of physics runs (for which FAZIA trigger conditions were "M>=2" and
"M>=1" downscaled by 100), only events for which the "M>=2" trigger pattern fired are accepted. Note that for the FAZIA
trigger pattern to be acceptable, FAZIA must be part of the event, i.e. we also reject the (very rare) spurious cases
where only INDRA is present.

See KVFAZIATrigger, KVINDRAFAZIAE789TriggerConditions, KVFAZIA::SetTriggerPatternsForDataSet(), KVFAZIA::GetTriggerForCurrentRun(), KVFAZIA::ReadTriggerPatterns.

__Changes in__ \ref GlobalVariables : __Dummy global variables__

A KVDummyGV can be added to a KVGVList of global variables, not to calculate anything,
but just to perform a selection of events with the KVVarGlob::TestEventSelection() mechanism.

To use, simply add a KVDummyGV to the list of global variables in your analysis,
and define the required event selection by calling method KVVarGlob::SetEventSelection()
with a lambda function having the required 'bool (const KVVarGlob*)' signature.

## Version 1.12/03 (Released: 04/5/2021)

__Bugfixes__

  * correct bug in KVParticleCondition when using (old-style) pseudo-code (not lambdas);
  
  * fix bug in KVEvent::MakeEventBranch which could either explode or (silently) not write any events in a TTree;

  * fix (potential) bug with gamma multiplicities in reconstructed events.

## Versions 1.12/01 (Released: 19/2/2021) &  1.12/02 (Released: 01/4/2021)

__Changes 19/2/2021 in__ \ref Analysis : __Reusable analysis classes__

As part of ongoing efforts to make analysis classes more flexible and efficient, it is now possible to use the same analysis class to analyse several different types of data.
Any analysis derived from KVReconEventSelector (generic reconstructed event analysis class) can now be used:
 
  * analyse generic reconstructed data [this was already the case];
  
  * analyse reconstructed INDRA data [previously only possible with a specific class derived from KVINDRAEventSelector];
  
  * analyse filtered simulated data
  
__Changes 28/1/2021 in__ \ref Analysis : __Rejecting events based on DAQ trigger conditions__

Rejection of reconstructed events which are not consistent with the online DAQ trigger of each run
is now handled by a new class KVTriggerConditions. This is in order to be able to handle situations which
are more complicated than a simple minimum global multiplicity.

In analysis classes, the rejection is handled by calling KVEventSelector::SetTriggerConditionsForRun() in the InitRun() method of the analysis class.
This replaces the condition

~~~{.cpp}
if( !GetEvent()->IsOK() ) return kFALSE;
~~~

which was previously used at the beginning of the Analysis() method. The new mechanism is implemented by
default in the new examples and templates for automatically-generated user analysis classes. For the moment, trigger conditions for INDRA data
are handled; the implementation for INDRA-FAZIA data will follow shortly.

__Changes 22/1/2021 in__ \ref GlobalVariables

Modification required to plugin declaration for any user-defined global variable classes,
constructor with `const char*` argument (variable name) must be used, like so:

~~~~{.cpp}
+Plugin.KVVarGlob:    MyNewVarGlob    MyNewVarGlob     MyNewVarGlob.cpp+   "MyNewVarGlob(const char*)"
~~~~

__Changes 11/12/2020 in__ \ref GlobalVariables : __Definition of new frames using global variables__

Global variables in a KVGVList can be used to define new kinematical reference frames which are available for
all variables which come later in the list.

As an example of use, imagine that KVZmax is used to find the heaviest (largest Z) fragment in the
forward CM hemisphere, then the velocity of this fragment is used to define a "QP_FRAME"
in order to calculate the KVFlowTensor in this frame:

~~~~{.cpp}
    KVGVList vglist;
    auto vg = vglist.AddGV("KVZmax", "zmax");
    vg->SetFrame("CM");
    vg->SetSelection( {"V>0", [](const KVNucleus* n){ return n->GetVpar()>0; }} );
    vg->SetNewFrameDefinition(
                [](KVEvent* e, const KVVarGlob* v){
        e->SetFrame("QP_FRAME", static_cast<const KVZmax*>(v)->GetZmax(0)->GetVelocity());
    });
    vg = AddGV("KVFlowTensor", "qp_tensor");
    vg->SetFrame("QP_FRAME"); // frame will have been defined before tensor is filled
~~~~

__Changes 21/9/2020 in__ \ref GlobalVariables : __Event selection using global variables__

Event selection can be performed automatically based on the values of the global variables in a KVGVList.
This is implemented for example in KVEventSelector, the base class for all analysis classes. This can
improve the speed of analyses, as the conditions are tested for each global variable as soon as they
are calculated, and processing of the event aborted if it fails. Variables used for event selection should
added to the list of gobal variables before any others in order to optimise the speed of analysis.

For example, to retain for analysis only events with a total measured charge in the forward c.m.
hemisphere which is at least equal to 80% of the charge of the projectile:

~~~~{.cpp}
int UserAnalysis::fZproj;// member variable (in .h file)

void UserAnalysis::InitAnalysis()
{
   auto vg = AddGV("KVZtot", "ztot");
   vg->SetFrame("cm");
   vg->SetSelection("Vcm>0", [](const KVNucleus* n){ return n->GetVpar()>0; });
   // note capture by reference in order to use value of fZproj (not defined yet)
   vg->SetEventSelection([&](const KVVarGlob* v){ return v->GetValue()>0.8*fZproj; });
}

void UserAnalysis::InitRun()
{
   // initialize fZproj for current run
   fZproj = GetCurrentRun()->GetSystem()->GetZproj();
}
~~~~

__Changes 11/8/2020 in__ \ref Core

Added STL-style iterator to KVNameValueList. It is now possible to do the following (with C++11 support enabled):
~~~~{.cpp}
KVNameValueList p;
p.SetValue("X",3.6);
p.SetValue("Y",false);
p.SetValue("Z","hello");

for(auto& d:p) { d.Print(); }

Info in <KVNamedParameter::Print>: Name = X type = Double_t value = 3.600000
Info in <KVNamedParameter::Print>: Name = Y type = Bool_t value = false
Info in <KVNamedParameter::Print>: Name = Z type = string value = hello
~~~~

__Changes 9/8/2020 in__ \ref GlobalVariables

Major rewrite of global variable classes.

Previously, there was much source of confusion as
different variables could have specific ways of defining which nuclei they would include,
in which frame, etc., while the base methods of KVVarGlob for defining particle selection
and kinematical frames were not always respected by all classes.

Now, the same logic is applied to all global variable classes:

  * individual variable classes define *only* how they are filled from nuclei
    by overriding KVVarGlob::fill(),
    KVVarGlob::fill2() or KVVarGlob::fillN() method, depending on whether they are 1-, 2- or
    \f$N\f$-body observables, respectively;

  * particle selection is handled *only* by use of
    KVVarGlob::SetSelection() and (new) KVVarGlob::AddSelection() methods; the latter
    adds a selection which will operate in addition to any existing selection (logical AND);

  * kinematic frame selection is handled *only* by use of KVVarGlob::SetFrame(); note that
  the same frame will also be used for any kinematic quantities used to select particles.

As an example, consider the KVEtrans variable, which calculates the sum of transverse kinetic
energies for each event. Without writing a new class, the same variable can be used
in very different ways:

~~~~~~{.cpp}
KVEtrans e1("et"); // total transverse energy of all particles

KVEtrans e2("et_imf");
e2.SetSelection("_NUC_->GetZ()>2"); // total transverse energy of fragments

KVEtrans e3("et_imf_fwcm");
e3.SetSelection("_NUC_->GetZ()>2");
e3.AddSelection("_NUC_->GetVpar()>0");
e3.SetFrame("CM"); // total transverse energy of fragments in forward CM hemisphere
~~~~~~

In addition, as part of this rationalization, all existing global variables calculating
multiplicities, or sums or mean values of scalar quantities have been reimplemented
using KVVGSum, which vastly reduces code replication.

All 'Av' variants (calculating various multiplicities or sums in the "forward" hemisphere)
have been removed, as they can all be implemented using existing classes just by applying
particle selection criterion
~~~~~{.cpp}
vg.AddSelection([](const KVNucleus*n){ n->GetVpar()>0; }
~~~~~
and optionally defining the correct frame in which to apply it:
~~~~~{.cpp}
vg.SetFrame("CM");
~~~~~

__Removed classes__ :   KVZboundMean, KVTenseur3, KVTensP, KVTensE, KVTensPCM, KVMultAv, KVMultLegAv,
KVMultIMFAv, KVZtotAv, KVRisoAv

__ALL EXISTING USER GLOBAL VARIABLES NEED TO BE REWRITTEN TO RESPECT THE NEW FRAMEWORK__
__________________________

__Changes 7/8/2020 in__ \ref NucEvents

Changes to KVEvent::Iterator
 + there was a bug with the implementation of the iterator, methods pointer() and reference() clashed with implicitly-declared
member types from std::iterator. Names of methods changed to KVEvent::Iterator::get_pointer() and KVEvent::Iterator::get_reference().

__________________________

__Changes 27/7/2020 in__ \ref Analysis : __Particle selection using lambda captures (C++11..)__

KVParticleCondition has been extended to use lambda expressions (if KaliVeda is compiled with ROOT
version 6 or later)

The lambda must take a `const KVNucleus*` pointer as argument and return a boolean:
~~~~~~{.cpp}
KVParticleCondition l1("Z>2", [](const KVNucleus* nuc){ return nuc->GetZ()>2; });
KVParticleCondition l2("Vpar>0", [](const KVNucleus* nuc){ return nuc->GetVpar()>0; });
~~~~~~
Note the first argument to the constructor is a name which the user is free to define
in order to remember what the condition does.

Like any lambda expressions, variables can be 'captured' from the surrounding scope, which
can be useful in some situations. For example, given the following definitions:
~~~~~~{.cpp}
int zmin = 3;
KVParticleCondition l3("Z>zmin", [&](const KVNucleus* nuc){ return nuc->GetZ()>=zmin; });
~~~~~~
then the limit for the selection can be changed dynamically like so:
~~~~~~{.cpp}
KVNucleus N("7Li");
l3.Test(&N);      ==> returns true
zmin=5;
l3.Test(&N);      ==> returns false
~~~~~~

___________________

## Version 1.11/00

Released: 9th March 2020 
