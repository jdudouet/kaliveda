#include "MFMFileReader.h"
#include "TString.h"

int main(int argc, char* argv[])
{
   // mfmfile_demerger [input_file] [output_directory]

   if (argc < 3) {
      std::cout << "Usage: mfmfile_demerger [input_file] [output_directory]" << std::endl;
      exit(0);
   }

   std::string input_file = argv[1];
   std::string output_directory = argv[2];
   auto output_file_path = output_directory + "/" + input_file;

   MFMFileReader file_reader(input_file, 1000 * 1024 * 1024);

   ULong64_t nframes{0};

   auto treat_frame = [&](const MFMCommonFrame & mfmframe) {
      std::cout << "FRAME TYPE : " << mfmframe.GetFrameType() << std::endl;
      ++nframes;
   };

   while (file_reader.ReadNextFrame()) {
      if (file_reader.IsFrameReadMerge()) {
         auto& mergeframe = file_reader.GetMergeManager();
         while (mergeframe.ReadNextFrame()) {
            treat_frame(mergeframe.GetFrameRead());
         }
      }
      else
         treat_frame(file_reader.GetFrameRead());
   }

   std::cout << "Read " << nframes << " frames" << std::endl;
}
