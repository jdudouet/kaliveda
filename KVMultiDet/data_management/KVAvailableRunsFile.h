/*
$Id: KVAvailableRunsFile.h,v 1.5 2008/02/07 09:25:39 franklan Exp $
$Revision: 1.5 $
$Date: 2008/02/07 09:25:39 $
*/

//Created by KVClassFactory on Fri May  5 10:46:40 2006
//Author: franklan

#ifndef __KVAVAILABLERUNSFILE_H
#define __KVAVAILABLERUNSFILE_H

#include <KVBase.h>
#include "Riostream.h"

#include "TDatime.h"
#include "TString.h"
#include "KVNumberList.h"
#include "KVLockfile.h"
#include "KVDatime.h"
#include "KVDataSet.h"

class KVDBSystem;
class TList;
class KVList;
class KVHashList;
class KVNameValueList;

/**
  \class KVAvailableRunsFile
  \brief Handles lists of available runs for different datasets and types of data
  \ingroup DM

For each type of data associated with each dataset (type="raw", "recon", "ident" or "root")
we maintain runlist files which contain the run numbers of datafiles which are physically
present/available, along with the date/time of last modification of each file, and the filename.

These files are kept in the dataset's KVFiles subdirectory, i.e. in $KVROOT/KVFiles/[name of dataset]

The name of each file has the following format:
~~~~
      [repository].available_runs.[dataset subdir].[type of data]
~~~~
*/

class KVAvailableRunsFile: public KVBase {

   static KVString date_read_from_filename;//!

protected:
   std::ifstream fRunlist;     //for reading runlist file
   KVLockfile runlist_lock;   //for locking runlist file

   const KVDataSet* fDataSet;         //dataset to which this file belongs
   KVHashList* fAvailableRuns;//! temporary list used to store infos when updating
   void ReadFile();
   KVNameValueList* RunHasFileWithDateAndName(Int_t run, const Char_t* filename, TDatime modtime, Int_t& OccNum);

   const Char_t* GetFileName() const;
   const Char_t* GetFilePath() const;
   const Char_t* GetFullPathToAvailableRunsFile() const;
   Bool_t CheckDirectoryForAvailableRunsFile();
   virtual Bool_t OpenAvailableRunsFile();
   virtual void CloseAvailableRunsFile();

   Bool_t IsFileOpen()
   {
      return fRunlist.is_open();
   };

public:

   KVAvailableRunsFile();
   KVAvailableRunsFile(const Char_t* type);
   KVAvailableRunsFile(const Char_t* type, const KVDataSet* parent);
   virtual ~ KVAvailableRunsFile();

   Bool_t FileExists() const
   {
      return !gSystem->AccessPathName(GetFullPathToAvailableRunsFile());
   }

   const Char_t* GetDataType() const
   {
      return GetName();
   }
   Int_t IsRunFileName(const Char_t* filename);
   static Int_t IsRunFileName(const KVString& fmt, const Char_t* filename, Int_t index_multiplier = 10, const Char_t* separators = ".");
   Bool_t ExtractDateFromFileName(const Char_t* name, KVDatime& date);
   static Bool_t ExtractDateFromFileName(const TString& fmt, const Char_t* name, KVDatime& date);
   virtual void Update(Bool_t no_existing_file = kFALSE);
   virtual Bool_t CheckAvailable(Int_t run);
   virtual Int_t Count(Int_t run);
   virtual Bool_t GetRunInfo(Int_t run, TDatime& modtime,
                             TString& filename);
   virtual void GetRunInfos(Int_t run, KVList* dates, KVList* names);
   const Char_t* GetFileName(Int_t run);
   virtual TList* GetListOfAvailableSystems(const KVDBSystem* systol = 0);
   virtual KVNumberList GetRunList(const KVDBSystem* system = 0);

   virtual void Remove(Int_t run, const Char_t* filename = "");
   virtual void UpdateInfos(Int_t run, const Char_t* filename, const Char_t* kvversion, const Char_t* username);
   virtual Bool_t InfosNeedUpdate(Int_t run, const Char_t* filename);
   virtual void Add(Int_t run, const Char_t* filename);
   const KVDataSet* GetDataSet() const
   {
      //Dataset to which this file belongs
      return fDataSet;
   }

   void SetDataSet(const KVDataSet* d)
   {
      // Set dataset to which this file belongs
      fDataSet = d;
   }

   KVNumberList CheckMultiRunfiles();
   void RemoveDuplicateLines(KVNumberList lines_to_be_removed);

   ClassDef(KVAvailableRunsFile, 1)     //Handles text files containing list of available runs for different datasets and types of data
};

#endif
