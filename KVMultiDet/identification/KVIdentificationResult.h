#ifndef KVIDENTIFICATIONRESULT_H
#define KVIDENTIFICATIONRESULT_H

#include "KVBase.h"
#include "TString.h"
#include <unordered_map>

/**
\class KVIdentificationResult
\brief Full result of one attempted particle identification
\ingroup Identification

When we attempt to identify a reconstructed charged particle (KVReconstructedNucleus) using an identification
telescope (KVIDTelescope), we call the
KVIDTelescope::Identify() method of the telescope in question, and the results of the
identification attempt are stored in the KVIdentificationResult object.
<br><br>
The informations stored are:
<ul>
<li>Bool_t IDattempted : (=kTRUE if an identification attempt was made)</li>
<li>Bool_t IDOK : general status of identification (=kTRUE if identification attempt successful)</li>
<li>GetIDType() : returns string containing type of identification (corresponds to type of identification telescope)</li>
<li>GetGridName() : returns the name of the grid used for identification (VARY_VARX)
<li>Int_t IDcode : general identification quality code associated with this type of identification</li>
<li>Int_t IDquality : specific quality code returned by identification procedure</li>
<li>GetComment() : explanatory message regarding quality code returned by identification procedure</li>
<li>Double_t PID : the particle identifier (interpolated mass or charge)</li>
<li>Bool_t Zident : (=kTRUE if particle's Z was determined by the identification)</li>
<li>Bool_t Aident : (=kTRUE if particle's A was determined by the identification)</li>
<li>Int_t Z : the Z given by the identification attempt if Zident==kTRUE</li>
<li>Int_t A : the A given by the identification attempt if Aident==kTRUE</li>
<li>Int_t deltaEpedestal : tells if particle is in pedestal region of delta-E</li>
</ul>
Note that, apart from GetIDType() and GetComment() methods, all other informations are public member variables
which can be accessed directly.
*/

class KVIdentificationResult : public KVBase {
public:
   Bool_t IDattempted;  // =kTRUE if identification was attempted
   Bool_t IDOK;  // general quality of identification, =kTRUE if acceptable identification made
   Int_t IDcode; // a general identification code for this type of identification
   Bool_t Zident;  // =kTRUE if Z of particle established
   Bool_t Aident; // = kTRUE if A of particle established
   Int_t IDquality; // specific quality code returned by identification procedure
   Int_t Z; // Z of particle found (if Zident==kTRUE)
   Int_t A; // A of particle found (if Aident==kTRUE)
   Double_t PID; // = "real" Z if Zident==kTRUE and Aident==kFALSE, "real" A if Zident==Aident==kTRUE
   Int_t deltaEpedestal; // special code for handling particles which give no signal in deltaE
   TString Rejecting_Cut; // name of cut in grid which rejected particle for identification

   using grid_infos = std::unordered_map<std::string, std::vector<TString>>; //infos from each grid used to identify particle
   grid_infos flags;

   enum {
      deltaEpedestal_UNKNOWN, // status unknown, case not treated
      deltaEpedestal_YES,     // the particle to identify has a delta-E consistent with pedestal
      deltaEpedestal_NO       // the particle to identify has a delta-E > pedestal
   };

   KVIdentificationResult() :
      IDattempted(0), IDOK(0), IDcode(-1),
      Zident(0), Aident(0), IDquality(-1), Z(-1), A(-1), PID(-1.0), deltaEpedestal(deltaEpedestal_UNKNOWN)
   {};

   KVIdentificationResult(const KVIdentificationResult& id) : KVBase()
   {
      // copy constructor
      id.Copy(*this);
   }
   void Clear(Option_t* opt = "");
   void Copy(TObject&) const;
   void Print(Option_t* opt = "") const;
   KVIdentificationResult& operator=(const KVIdentificationResult& i)
   {
      i.Copy(*this);
      return *this;
   }

   void SetIDType(const Char_t* t)
   {
      // Set type of identification (= type of KVIDTelescope)
      SetType(t);
   }
   const Char_t* GetIDType() const
   {
      // Gives type of identification (= type of KVIDTelescope)
      return GetType();
   }
   void SetComment(const Char_t* c)
   {
      // Set an explanatory comment for the identification procedure quality code
      SetLabel(c);
   }
   const Char_t* GetComment() const
   {
      // Give an explanatory comment for the identification procedure quality code
      return GetLabel();
   }
   void SetGridName(const Char_t* n)
   {
      // Set name of grid (VARY_VARX) used for identification
      SetName(n);
   }
   const Char_t* GetGridName() const
   {
      // Give name of grid (VARY_VARX) used for identification
      return GetName();
   }

   void AddFlag(std::string grid_name, TString flag)
   {
      // add informational flag for the given grid
      flags[grid_name].push_back(flag);
   }

   Bool_t HasFlag(std::string grid_name, TString flag)
   {
      // return true if informational flag with exact name set for the given grid
      auto fv = flags.find(grid_name);
      if (fv != flags.end()) return (std::find(fv->second.begin(), fv->second.end(), flag) != fv->second.end());
      return kFALSE;
   }

   Bool_t HasFlagWhichBegins(std::string grid_name, TString flag_beginning)
   {
      // return true if informational flag with name beginning with flag_beginning set for the given grid
      auto fv = flags.find(grid_name);
      if (fv != flags.end()) {
         for (auto& f : fv->second) {
            if (f.BeginsWith(flag_beginning)) return kTRUE;
         }
      }
      return kFALSE;
   }

   TString GetFlagWhichBegins(std::string grid_name, TString flag_beginning)
   {
      // return the informational flag with name beginning with flag_beginning set for the given grid
      auto fv = flags.find(grid_name);
      if (fv != flags.end()) {
         for (auto& f : fv->second) {
            if (f.BeginsWith(flag_beginning)) return f;
         }
      }
      return "";
   }

   Bool_t IdentifyingGridHasFlag(TString flag)
   {
      // return true if informational flag with exact name set for last grid used for identification
      // (the one whose name is returned by GetGridName())
      return HasFlag(GetGridName(), flag);
   }

   Bool_t IdentifyingGridHasFlagWhichBegins(TString flag_beginning)
   {
      // return true if informational flag with name beginning with flag_beginning set for last grid used for identification
      // (the one whose name is returned by GetGridName())
      return HasFlagWhichBegins(GetGridName(), flag_beginning);
   }

   TString IdentifyingGridGetFlagWhichBegins(TString flag_beginning)
   {
      // return the informational flag with name beginning with flag_beginning set for last grid used for identification
      // (the one whose name is returned by GetGridName())
      return GetFlagWhichBegins(GetGridName(), flag_beginning);
   }

   Bool_t HasFlag_AnyGrid(TString flag, std::string& grid_name)
   {
      // return true if informational flag with exact name set for any grid
      // if true, grid_name contains name of grid info was set for

      for (auto& fv : flags) {
         grid_name = fv.first;
         if (HasFlag(grid_name, flag)) return kTRUE;
      }
      return kFALSE;
   }

   Bool_t HasFlagWhichBegins_AnyGrid(TString flag_beginning, std::string& grid_name)
   {
      // return true if informational flag with name beginning with flag_beginning set for any grid
      // if true, grid_name contains name of grid info was set for
      for (auto& fv : flags) {
         grid_name = fv.first;
         if (HasFlagWhichBegins(grid_name, flag_beginning)) return kTRUE;
      }
      return kFALSE;
   }

   TString GetFlagWhichBegins_AnyGrid(TString flag_beginning, std::string& grid_name)
   {
      // return the informational flag with name beginning with flag_beginning set for the any grid
      // if found, grid_name contains name of grid info was set for
      for (auto& fv : flags) {
         grid_name = fv.first;
         auto f = GetFlagWhichBegins(grid_name, flag_beginning);
         if (f.Length()) return f;
      }
      return "";
   }

   ClassDef(KVIdentificationResult, 4) //Full result of one attempted particle identification
};

#endif
