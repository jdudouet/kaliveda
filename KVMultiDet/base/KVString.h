//$Id: KVString.h,v 1.15 2009/01/16 14:55:20 franklan Exp $

#ifndef __KVSTRING_H
#define __KVSTRING_H

#include "TString.h"
#include "KVConfig.h"
#include "RVersion.h"

#include <TObjArray.h>

/*
The following macro can be used in cases where Tokenize is used
to split a string into different integer elements and we want to store
the integers in Int_t variables. This macro is backwards compatible.
*/
#define KV__TOBJSTRING_TO_INT(arr,index,var) \
   Int_t var; \
   { \
      KVString _temp_string_ = ((TObjString *) arr->At(index))->String(); \
      if(_temp_string_.IsDigit()) var = _temp_string_.Atoi(); else var = 0; \
   }

/**
  \class KVString
\brief Extension of ROOT TString class which allows backwards compatibility with ROOT v3.10/02 onwards.
\ingroup Core

 Tokenize() and IsDigit() methods are added here if they don't exist
 in the version of ROOT being used.

 IsFloat() method is added: returns true if string is a floating-point number, i.e. if it
              has any of following formats (only first one returns kTRUE with TString::IsDigit()):
 ~~~~~
      64320
      6.4320  6,4320
      6.43e20   6.43E20 6,43e20
      6.43e-20  6.43E-20 6,43e-20
 ~~~~~

 Also Atoi() and Atof() are extended to include the following possibilities:

 - Atoi(): with string="123 456", TString::Atoi() gives value=123. KVString::Atoi() gives value=123456
 - Atof(): with string="12,34", TString::Atof() gives value=12. KVString::Atof() gives value=12.34

 Begin(), Next() and End() can be used to loop over items in
 a string separated by the delimiter character given as argument
 to Begin().

 Example:
 ~~~~{.cpp}
   KVString str("First | Second | Third");
   str.Begin("|");
   while( !str.End() ){
     cout << str.Next().Data() << endl;
   }
 ~~~~
 This will give the following output:
~~~~
 First
  Second
  Third
~~~~
 or, if Next() is called with argument strip_whitespace=kTRUE,
~~~~
 First
 Second
 Third
~~~~
*/

class KVString : public TString {

protected:

   mutable unique_ptr<TObjArray>  kObjArr;//!used by Next() to iterate over list
   mutable Int_t       fIterIndex;//! used by Next() to iterate over list
   mutable Bool_t      fEndList;//! used by Next() & End() to iterate over list

public:

   KVString() : TString(), kObjArr(nullptr), fIterIndex(-1), fEndList(kTRUE)
   {
   }
   KVString(const Char_t* s): TString(s), kObjArr(nullptr), fIterIndex(-1), fEndList(kTRUE)
   {
   }
   KVString(const TString& s): TString(s), kObjArr(nullptr), fIterIndex(-1), fEndList(kTRUE)
   {
   }
   KVString(const KVString& s): TString((const TString&)s), kObjArr(nullptr), fIterIndex(-1), fEndList(kTRUE)
   {
   }
   KVString(Double_t value, Double_t error);

   virtual ~ KVString() {}

#ifdef __WITHOUT_TSTRING_TOKENIZE
   TObjArray* Tokenize(const TString& delim) const;
#endif
#ifdef __WITH_KVSTRING_ISDIGIT
   Bool_t IsDigit() const;
#endif
#ifdef __WITH_KVSTRING_ISFLOAT
   Bool_t IsFloat() const;
#endif
#ifdef __WITH_KVSTRING_ATOI
   Int_t Atoi() const;
#endif
#ifdef __WITH_KVSTRING_ATOF
   Double_t Atof() const;
#endif
#ifdef __WITH_KVSTRING_REMOVE
   KVString& Remove(TString::EStripType s, char c);     // Like Strip() but changing string directly
   KVString& Remove(Ssiz_t pos)
   {
      return (KVString&)(TString::Remove(pos));
   };                           // Remove pos to end of string
   KVString& Remove(Ssiz_t pos, Ssiz_t n)
   {
      return (KVString&)(TString::Remove(pos, n));
   };                           // Remove n chars starting at pos
#endif
   inline KVString& operator=(const KVString& s);
   inline KVString& operator=(const char* s);
   inline KVString& operator=(const TString& s);

   virtual KVString& Substitute(const Char_t c1, const Char_t c2);
   virtual Int_t Sscanf(const Char_t* fmt, ...);

   virtual Bool_t Match(TString pattern);
   void Begin(TString delim) const;
   Bool_t End() const;
   KVString Next(Bool_t strip_whitespace = kFALSE) const;
   void RBegin(TString delim) const;
   KVString RNext(Bool_t strip_whitespace = kFALSE) const;
   Int_t GetNValues(TString delim) const;
   std::vector<KVString> Vectorize(TString delim, Bool_t strip_whitespace = kFALSE);

#ifdef __WITH_KVSTRING_ISWHITESPACE
   Bool_t IsWhitespace() const;
#endif
#ifdef __WITH_KVSTRING_ITOA
   Bool_t       IsBin() const;
   Bool_t       IsOct() const;
   Bool_t       IsDec() const;
   Bool_t       IsInBaseN(Int_t base) const;
#endif
   static KVString Itoa(Int_t value, Int_t base);         // Converts int to string with respect to the base specified (2-36)
   static KVString UItoa(UInt_t value, Int_t base);
   static KVString LLtoa(Long64_t value, Int_t base);
   static KVString ULLtoa(ULong64_t value, Int_t base);
   static KVString BaseConvert(const KVString& s_in, Int_t base_in, Int_t base_out);  // Converts string from base base_in to base base_out (supported bases 2-36)

   void RemoveAllExtraWhiteSpace();
   KVString StripAllExtraWhiteSpace() const;

   KVString& FindCommonCharacters(const TCollection*, const char bug = '*');
   KVString& FindCommonTitleCharacters(const TCollection*, const char bug = '*');

   void RandomLetterSequence(Int_t length);

   void Capitalize();

   ClassDef(KVString, 1)//TString wrapper compatible with ROOT versions 3.10/02 onwards
};

inline KVString& KVString::operator=(const KVString& s)
{
   if (this != &s) {
      Form("%s", s.Data());
   }
   return *this;
}

inline KVString& KVString::operator=(const TString& s)
{
   Form("%s", s.Data());
   return *this;
}

inline KVString& KVString::operator=(const char* s)
{
   Form("%s", s);
   return *this;
}
#endif
