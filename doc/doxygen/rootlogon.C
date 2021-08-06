{
   gROOT->ProcessLine(".L DocConverter.cxx+");
   ConvertAllModules("kaliveda.git/KVMultiDet");
   ConvertAllModules("kaliveda.git/KVIndra");
   ConvertAllModules("kaliveda.git/FAZIA");
   ConvertAllModules("kaliveda.git/indrafazia");
   ConvertAllModules("kaliveda.git/MicroStat");
}
