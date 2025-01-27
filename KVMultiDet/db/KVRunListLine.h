//$Id: KVRunListLine.h,v 1.4 2006/10/19 14:32:43 franklan Exp $

#ifndef __KVRUNLISTLINE_H
#define __KVRUNLISTLINE_H

#include "TObjString.h"
#include "TObjArray.h"
#include "Riostream.h"
#include "KVNameValueList.h"
#include "KVString.h"
#include "KVList.h"

/**
\class KVRunListLine
\brief Base class for reading runlists for experiments
\ingroup DM,Core

Base class for reading runlists for experiments, either 'real' runlists
with all info (KVLineCSV) or the minimalist files used for old data and
based on the DST runlists kept at CC-IN2P3 (KVLineDST).

For backwards compatibility, this uses KVString whenever possible.
 */

class KVRunListLine {

private:
   Int_t fMaxNFields;    //maximum allowed number of fields i.e. size of TObjArray fFields
   TString fLine;               //line to analyse
   Bool_t fFieldsSet;           //flag set once fields have been set
   TObjArray* fFields;          //array of all fields after break down of fLine
   KVList fFieldKeys;           //key words used to recognise field headings
   KVList fRunKeys;             //fields that must be present (have a value) for a line to be considered a "good run line"

   inline Int_t GetFieldIndex(const Char_t*) const;

   bool calling_field_keys;//!
   bool calling_run_keys;//!
   void SetFieldKeys()
   {
      calling_field_keys = false;   // just to terminate recursion of parameter pack
   }
   void SetRunKeys()
   {
      calling_run_keys = false;   // just to terminate recursion of parameter pack
   }

protected:
   KVNameValueList fIndexList;       //list of integer indexes corresponding to field names

public:
   KVRunListLine();
   virtual ~ KVRunListLine();

   virtual const Char_t* GetField(const Char_t*) const;
   TObjString* GetFieldString(const Char_t*) const;
   Int_t GetIntField(const Char_t*);
   Float_t GetFloatField(const Char_t*);

   virtual void SetFields();
   Bool_t FieldsSet() const
   {
      return fFieldsSet;
   };

   inline void SetLine(const Char_t*);
   inline void SetLine(TString&);
   inline void Clear();
   Int_t BreakLineIntoFields(const char);
   inline Bool_t HasFieldValue(const Char_t*) const;
   inline Bool_t HasField(const Char_t*) const;
   Bool_t FieldContains(const Char_t*, const Char_t*) const;
   Bool_t IsFloat(const Char_t* name);

   virtual void Print() const;
   inline void PrintFieldValue(const Char_t* name) const;
   virtual Int_t GetTrigger(const Char_t* field_name =
                               "Trigger", const Char_t* fmt = "M>=%d");

   template<typename T, typename... Args>
   void SetFieldKeys(T key0, Args... keys)
   {
      //Set keywords used to identify lines in the file which define the column headings, i.e. the field names.
      //Give the list of keywords (at least one).
      //
      //Each and every keyword given as argument here (at least one word must be given) must appear in a line for it to be considered
      //a column heading. Lines identified as such (by IsFieldHeader()) are used to set indices for each column (SetFields()).

      if (!calling_field_keys) {
         // at beginning of call, clear out list of keywords
         fFieldKeys.Clear();
         calling_field_keys = true; // to avoid clearing in every recursion
      }
      fFieldKeys.Add(new TObjString(key0));
      SetFieldKeys(keys...);
   }
   template<typename T, typename... Args>
   void SetRunKeys(T key0, Args... keys)
   {
      //Set list of fields which must have values for a line to be considered a "good" run line
      //Give the list of field names (at least one).

      if (!calling_run_keys) {
         // at beginning of call, clear out list of keywords
         fRunKeys.Clear();
         calling_run_keys = true; // to avoid clearing in every recursion
      }

      fRunKeys.Add(new TObjString(key0));
      SetRunKeys(keys...);
   }

   virtual Bool_t IsFieldHeader();
   virtual Bool_t GoodRunLine();

   ClassDef(KVRunListLine, 0)  //Base class for reading runlist files
};

//INLINES_______________________________________________________________________________

inline Bool_t KVRunListLine::HasField(const Char_t* fname) const
{
   //we check that the field in question is set, i.e. does the current definition
   //of possible fields include field "fname" ?

   return (FieldsSet() ? fIndexList.HasParameter(fname) : kFALSE);
}

//_____________________________________________________________________________________

inline Bool_t KVRunListLine::HasFieldValue(const Char_t* fname) const
{
   //we check that the field in question exists and has been filled in this line
   //if the field index is greater than the actual number of fields in the line, we assume the field was not filled (HasFieldValue=kFALSE)
   //fields which are present but empty or filled with whitespace are not considered to have a field value
   Int_t index = GetFieldIndex(fname);
   if (!(index > -1 && index < fFields->GetEntries()))
      return kFALSE;
   KVString tmp(((TObjString*)(*fFields)[index])->String());
   return !tmp.IsWhitespace();
}

//_____________________________________________________________________________________

inline Int_t KVRunListLine::GetFieldIndex(const Char_t* fname) const
{
   //PROTECTED method.
   //Returns index of field with name 'fname'
   //This index can be used to retrieve the field-value from fFields
   //Returns -1 if : field indices not set yet (no call to SetFields)
   //         or if : field is unknown

   return fIndexList.GetIntValue(fname);
}

//_____________________________________________________________________________________

inline void KVRunListLine::SetLine(const Char_t* line)
{
   //set new line to be read
   fLine = line;
}

//_____________________________________________________________________________________

inline void KVRunListLine::SetLine(TString& line)
{
   //set new line to be read
   fLine = line;
}

//_____________________________________________________________________________________

inline void KVRunListLine::Clear()
{

   //reset before reading new line
   fFields->Clear();
}

//_____________________________________________________________________________________

inline void KVRunListLine::PrintFieldValue(const Char_t* name) const
{
   //for a given field "name", print out current value of field as:
   // Field name : current value
   //if the field is absent from current line (HasFieldValue=kFALSE)
   //we print
   // Field name : [absent]

   std::cout << name << " : ";
   if (HasFieldValue(name))
      std::cout << GetField(name) << std::endl;
   else
      std::cout << "[absent]" << std::endl;
}

#endif
