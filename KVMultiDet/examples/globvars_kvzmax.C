#include "TTree.h"
#include "TCanvas.h"
#include "KVZmax.h"
#include "KVGVList.h"
#include "KVSimEvent.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>

void zmax_example1()
{
   // Example to demonstrate basic use of KVZmax to access sorted list
   // of nuclei in descending order of Z

   /* STEP 1: make random event */
   int mult = gRandom->Integer(10) + 5; // random multiplicity 5<=M<15
   KVSimEvent event;
   for (int i = 0; i < mult; ++i) {
      // fill event with random nuclei 1<=Z<=50
      event.AddParticle()->SetZ(gRandom->Integer(50) + 1);
   }
   std::cout << "Random event :\n\n";
   event.Print();

   /* STEP 2: use KVZmax global variable to rank nuclei in Z */
   KVZmax zmax("ZMAX");
   zmax.Init();
   KVNucleus* nunuc;
   while ((nunuc = event.GetNextParticle())) zmax.Fill(nunuc);

   std::cout << "\nSorted event :\n\n";
   for (int i = 0; i < mult; ++i) zmax.GetZmax(i)->Print();
}

void zmax_example2()
{
   // Example showing how to:
   //   - use KVZmax with a KVGVList in order to automatically create
   //     and fill branches in a TTree containing the Z of the N largest
   //     nuclei in each event
   //   - apply a selection condition to a KVZmax to limit the nuclei
   //     included in the sorted list

   // We use a global variable list (KVGVList) in order to use the automatic
   // branch creation facility KVGVList::MakeBranches(TTree*)
   KVGVList varlist;

   // 1st KVZmax: rank all nuclei according to Z
   KVZmax zmax("ZMAX");
   zmax.SetMaxNumBranches(5);//define number of branches to add to TTree
   varlist.Add(&zmax);

   // 2nd KVZmax: only include nuclei with Z<6
   KVZmax zmax2("ZMAX_UPTO6");
   zmax2.SetMaxNumBranches(1);//only add branch for largest Z
   zmax2.SetSelection(KVParticleCondition("_NUC_->GetZ()<6"));
   varlist.Add(&zmax2);

   // initialisation of global variables
   varlist.Init();

   // In order to check the Z-sorting, we generate random events which all
   // contain the same nuclei in a different order
   std::vector<int> zlist(10);
   int z = 1;
   std::generate_n(zlist.begin(), 10, [&z]() {
      return z++;
   }); // see http://www.cplusplus.com/reference/algorithm/generate_n

   TTree tree;
   varlist.MakeBranches(&tree);  // automatically generate branches for global variables
   tree.Print();

   // randomly generate 1000 events & fill tree
   KVSimEvent event;
   // to use std::shuffle
   std::random_device rd;
   std::mt19937 g(rd());

   for (int i = 0; i < 1000; ++i) {

      // shuffle list of Z
      std::shuffle(zlist.begin(), zlist.end(), g); // see http://www.cplusplus.com/reference/algorithm/random_shuffle
      // fill event
      event.Clear();
      for (auto z : zlist) {
         event.AddParticle()->SetZ(z);
      }

      // print 1st event, check it's random
      if (i == 0) {
         event.Print();
      }

      // calculate global variable(s) for event
      varlist.Reset();
      varlist.CalculateGlobalVariables(&event);

      // fill tree with Z of 5 biggest nuclei
      varlist.FillBranches();
      tree.Fill();
   }

   // Draw histograms of the 5 largest Z
   // These should be: Zmax1=10, Zmax2=9, ..., Zmax5=6
   TCanvas* can = new TCanvas("c", "5 Largest Z", 1644, 267);
   can->Divide(5, 1);
   for (int i = 1; i <= 5; ++i) {
      can->cd(i);
      tree.Draw(Form("ZMAX.Zmax%d", i));
   }

   can = new TCanvas;
   tree.Draw("ZMAX_UPTO6.Zmax1"); // should be equal to 5
}
