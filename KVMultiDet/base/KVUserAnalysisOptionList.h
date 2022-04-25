#ifndef __KVUSERANALYSISOPTIONLIST_H
#define __KVUSERANALYSISOPTIONLIST_H

#include "KVBase.h"
#include "KVNameValueList.h"

/**
 \class KVUserAnalysisOptionList
 \brief Handle list of options and input parameters for user analyis

 Write a detailed documentation for your class here, see doxygen manual for help.

 \author eindra
 \date Mon Apr 25 10:44:27 2022
*/

class KVUserAnalysisOptionList : public KVBase {
   KVNameValueList fOptionList;

public:
   void SetOpt(const Char_t* option, const Char_t* value)
   {
      //Set a value for an option
      KVString tmp(value);
      fOptionList.SetValue(option, tmp);
   }
   Bool_t IsOptGiven(const Char_t* opt)
   {
      // Returns kTRUE if the option 'opt' has been set

      return fOptionList.HasParameter(opt);
   }
   TString GetOpt(const Char_t* opt) const
   {
      // Returns the value of the option
      //
      // Only use after checking existence of option with IsOptGiven(const Char_t* opt)

      return fOptionList.GetTStringValue(opt);
   }
   void UnsetOpt(const Char_t* opt)
   {
      // Removes the option 'opt' from the internal lists, as if it had never been set

      fOptionList.RemoveParameter(opt);
   }
   void ParseOptions(const KVString& optlist)
   {
      // Analyse comma-separated list of options
      // and store all `"option=value"` pairs in the list.
      // Options can then be accessed using IsOptGiven(), GetOptString(), etc.

      fOptionList.Clear(); // clear list
      optlist.Begin(",");
      while (!optlist.End()) {

         KVString opt = optlist.Next();
         opt.Begin("=");
         KVString param = opt.Next();
         KVString val = opt.Next();
         while (!opt.End()) {
            val += "=";
            val += opt.Next();
         }

         SetOpt(param.Data(), val.Data());
      }
      fOptionList.Print();
   }

   ClassDef(KVUserAnalysisOptionList, 1) //Handle list of options and input parameters for user analyis
};

#endif
