// concatenates raw files for a given dataset and generates a runinfos.root file
// containing the informations corresponding to the new runs
//
#include "KVString.h"
#include <IRODS.h>
#include <KVDataRepositoryManager.h>
#include <KVDataSetManager.h>
#include <KVDataRepository.h>
#include <KVSystemDirectory.h>
#include <KVSystemFile.h>
#include <iostream>
#include <map>
#include <KVMFMDataFileReader.h>
#include "KVAvailableRunsFile.h"
#include <fcntl.h>

struct runfile_t {
   KVString name;
   KVString date;
   Long64_t size;
};

struct run_t {
   std::map<int, runfile_t> files;
};

class file_helper {
public:
   KVString scan_dir, dataset;
   KVString file_format;
   KVString output_directory;
   int index_multiplier;
   int files_to_concat;
   std::map<int, run_t> runs;
   int file_LUN;
   uint64_t output_buffer_size;
   std::vector<uint8_t> output_buffer;
   uint64_t bytes_in_buffer{0};

   void flush_output_buffer_to_file()
   {
      // write buffer to file...
      auto bytes_written = write(file_LUN, output_buffer.data(), bytes_in_buffer);
      assert(bytes_written == bytes_in_buffer);
      //then add frame to beginning of (next) buffer
      bytes_in_buffer = 0;
   }
   void add_frame_to_output_buffer(const MFMCommonFrame& mfmframe)
   {
      // add frame to buffer. write buffer to file if full.
      auto frame_size = mfmframe.GetFrameSize();
      if (bytes_in_buffer + frame_size > output_buffer_size) {
         flush_output_buffer_to_file();
      }
      // add frame to buffer
      memcpy(&output_buffer[bytes_in_buffer], mfmframe.GetPointHeader(), frame_size);
      bytes_in_buffer += frame_size;
   }

   file_helper(uint64_t bufsiz)
      : output_buffer_size{bufsiz}, output_buffer(bufsiz)
   {}

   bool file_has_index_number(const KVString& name, int& index)
   {
      // return true if filename has an index number
      index = 0;
      if (name.GetNValues(".") == 3) return false;
      name.Begin(".");
      for (int i = 0; i < 3; i++) name.Next();
      index = name.Next().Atoi();
      return true;
   }

   bool find_next_sequential_file(int& run0, int& index0, runfile_t& runfile0, int& run, int& index, runfile_t& runfile, bool first_call = false)
   {
      // look for next file after (run0,index0) in directory
      // return false if no file found
      // if run0!=0 when function first called, set first_call=true

      if (!runs.size()) {
         // on first call, scan files in directory
         KVSystemDirectory dir("data", scan_dir);
         TList* files = dir.GetListOfFiles();

         // sort files into ordered map run[index]=filename
         TIter nxt(files);
         KVSystemFile* f;
         while ((f = (KVSystemFile*)nxt())) {
            if (!f->IsDirectory()) {
               int ir;
               if ((ir = KVAvailableRunsFile::IsRunFileName(file_format, f->GetName(), index_multiplier))) {
                  int idx = ir % index_multiplier;
                  ir /= index_multiplier;
                  runfile_t rf;
                  rf.name = f->GetName();
                  rf.size = f->GetSize();
                  rf.date = f->GetDate();
                  runs[ir].files[idx] = rf;
               }
            }
         }
      }

      if (!run0) {
         // use first file found in directory as current run
         auto first_run = runs.begin();
         run0 = first_run->first;
         auto first_index = first_run->second.files.begin();
         index0 = first_index->first;
      }
      else if (first_call) {
         // starting from a run which is not the first one in the directory
         // find first file corresponding to run
         auto first_run = runs.find(run0);
         if (first_run == runs.end()) return false;
         // find first file corresponding to run
         auto first_index = first_run->second.files.begin();
         index0 = first_index->first;
      }
      // update infos on current file (whose size may have changed since it was first seen)
      // unless this method was called with run0=0
      // the current file was previously found with this method therefore it exists
      runfile0 = runs[run0].files[index0];

      run = run0;
      index = index0 + 1;

      // now look for (run,index)
      // if not found, look for (run+1,0)
      auto find_run = runs.find(run);
      if (find_run != runs.end()) {
         auto find_index = find_run->second.files.find(index);
         if (find_index != find_run->second.files.end()) {
            runfile = find_index->second;
            return true;
         }
      }
      // look for (run+1,0)
      ++run;
      index = 0;
      find_run = runs.find(run);
      if (find_run != runs.end()) {
         map<int, runfile_t>::iterator find_index = find_run->second.files.find(index);
         if (find_index != find_run->second.files.end()) {
            runfile = find_index->second;
            return true;
         }
      }
      return false;
   }
   KVNameValueList read_infos_on_file(const runfile_t& runfile, bool count_events = true)
   {
      KVString path_to_file;
      path_to_file.Form("%s/%s", scan_dir.Data(), runfile.name.Data());
      int run_num = KVAvailableRunsFile::IsRunFileName(file_format, runfile.name, index_multiplier);
      KVDatime when;
      KVAvailableRunsFile::ExtractDateFromFileName(file_format, runfile.name, when);
      KVNameValueList infos;
      infos.SetName(Form("run%010d", run_num));
      infos.SetValue("Run", run_num);
      infos.SetValue("Start", when.AsSQLString());
      infos.SetValue("End", runfile.date);
      infos.SetValue64bit("Size", runfile.size);

      KVMFMDataFileReader reader(path_to_file);
      ULong64_t events = 0;
      // correct start date/time from infos in file ?
      if (reader.GetRunInfos().HasStringParameter("FileCreationTime")) {
         when.SetGanacqNarvalDate(reader.GetRunInfos().GetStringValue("FileCreationTime"));
         infos.SetValue("Start", when.AsSQLString());
      }

      // add XML header frame to output buffer
      add_frame_to_output_buffer(reader.GetFrameRead());

      if (count_events) {
         while (reader.GetNextEvent()) {
            cout << "\xd" << "Reading events " << ++events << flush;
            add_frame_to_output_buffer(reader.GetFrameRead());
         }
         cout << endl;
         infos.SetValue64bit("Events", events);
      }
      return infos;
   }
   void write_run_infos(const KVNameValueList& infos)
   {
      TFile f(Form("%s/runinfos.root", output_directory.Data()), "update");
      infos.Write();
   }
   bool concatenate_files(int run_number, int corrected_index_multiplier)
   {
      int current_run(run_number), current_index(0);
      runfile_t current_file;
      int next_run(0), next_index(0);
      runfile_t next_file;
      bool first_call = true;
      KVString base_name_for_files;

      int files_treated(0), new_index(0);
      ULong64_t total_events(0), total_size(0);
      KVDatime date_start, date_end;
      KVString concat_filename;
      while (find_next_sequential_file(current_run, current_index, current_file, next_run, next_index, next_file, first_call)) {
         std::cout << current_file.name << " " << current_run << " " << current_index << std::endl;
         if (base_name_for_files == "") {
            base_name_for_files = current_file.name;
            concat_filename = base_name_for_files;
            if (new_index) concat_filename.Append(Form(".%d", new_index));
            // open first output file
            KVString output_path = output_directory + "/" + concat_filename;
            file_LUN = open(output_path.Data(), (O_RDWR | O_CREAT | O_TRUNC), 0644);
         }
         std::cout << "New filename for current run is " << concat_filename << std::endl << std::endl;

         auto infos = read_infos_on_file(current_file);

         // sum up infos
         total_events += infos.HasValue64bit("Events") ? infos.GetValue64bit("Events") : 0;
         total_size += infos.GetValue64bit("Size");
         if (!files_treated) date_start.SetSQLDate(infos.GetStringValue("Start"));
         date_end.SetSQLDate(infos.GetStringValue("End"));

         ++files_treated;
         if (next_run != current_run || files_treated == files_to_concat) {
            int run_num = current_run * corrected_index_multiplier + new_index;
            std::cout << "Close run " << run_num << " with " << files_treated << " files concatenated\n\n";

            std::cout << "Date start: " << date_start.AsSQLString() << std::endl;
            std::cout << "Date end  : " << date_end.AsSQLString() << std::endl;
            std::cout << "Total events : " << total_events << std::endl;
            std::cout << "Total size   : " << total_size << std::endl;

            infos.SetName(Form("run%010d", run_num));
            infos.SetValue("Run", run_num);
            infos.SetValue("Start", date_start.AsSQLString());
            infos.SetValue("End", date_end.AsSQLString());
            infos.SetValue64bit("Size", total_size);
            infos.SetValue64bit("Events", total_events);

            write_run_infos(infos);

            // flush output buffer & close output file
            flush_output_buffer_to_file();
            close(file_LUN);

            if (next_run != current_run) break;

            files_treated = 0;
            total_events = total_size = 0;
            ++new_index;
            concat_filename = base_name_for_files;
            concat_filename.Append(Form(".%d", new_index));
            // open file for next concatenation
            KVString output_path = output_directory + "/" + concat_filename;
            file_LUN = open(output_path.Data(), (O_RDWR | O_CREAT | O_TRUNC), 0644);

         }
         current_run = next_run;
         current_index = next_index;
         current_file = next_file;
         first_call = false;
      }
      return true;
   }
};

int main(int argc, char* argv[])
{
   if (argc < 11) {
      cout << "Usage: mfmfile_concatenator [-r run] [-i input_directory] [-o output_directory] [-N files_to_concatenate] [-D dataset]" << endl << endl;
      exit(1);
   }

   int run_number, files_to_concat;
   TString input_dir, output_dir, dataset;
   // get input parameters
   int iarg(1);
   while (iarg < argc) {
      TString arg(argv[iarg]);
      if (arg == "-r") {
         ++iarg;
         run_number = TString(argv[iarg]).Atoi();
      }
      else if (arg == "-i") {
         ++iarg;
         input_dir = argv[iarg];
      }
      else if (arg == "-o") {
         ++iarg;
         output_dir = argv[iarg];
      }
      else if (arg == "-N") {
         ++iarg;
         files_to_concat = TString(argv[iarg]).Atoi();
      }
      else if (arg == "-D") {
         ++iarg;
         dataset = argv[iarg];
      }
      ++iarg;
   }
   std::cout << "Concatenation of run " << run_number << " by " << files_to_concat << "\n";
   std::cout << "Files for dataset " << dataset << " read from " << input_dir << ", written to " << output_dir << "\n";

   // make sure output directory exists
   gSystem->mkdir(output_dir);

   // Set up file helper for scanning files
   uint64_t output_buffer_size = 512 * 1024 * 1024;
   file_helper FILE_H(output_buffer_size);
   FILE_H.scan_dir = input_dir;
   FILE_H.output_directory = output_dir;
   FILE_H.dataset = dataset;
   FILE_H.files_to_concat = files_to_concat;

   KVDataRepositoryManager drm;
   drm.Init();
   KVDataSet* ds = gDataSetManager->GetDataSet(dataset);
   FILE_H.file_format = ds->GetDataSetEnv("DataSet.RunFileName.raw", "");
   FILE_H.index_multiplier = 10 * ds->GetDataSetEnv("DataSet.RunFileIndexMultiplier.raw", -1.);

   // loop over all files of run
   FILE_H.concatenate_files(run_number, ds->GetDataSetEnv("DataSet.RunFileIndexMultiplier.raw", -1.));
}
