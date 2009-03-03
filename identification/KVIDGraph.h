/*
$Id: KVIDGraph.h,v 1.3 2009/03/03 14:27:15 franklan Exp $
$Revision: 1.3 $
$Date: 2009/03/03 14:27:15 $
*/

//Created by KVClassFactory on Mon Apr 14 13:42:47 2008
//Author: franklan

#ifndef __KVIDGRAPH_H
#define __KVIDGRAPH_H

#include "TCutG.h"
#include "KVGenParList.h"
#include "KVIDentifier.h"
#include <RQ_OBJECT.h>
#include <TH2.h>
#include "KVNumberList.h"
#include "KVIDTelescope.h"

class KVReconstructedNucleus;
class TVirtualPad;

class KVIDGraph : public TCutG
{
	friend class KVIDGridManager;
	
   RQ_OBJECT("KVIDGraph")

	protected:

   Bool_t 			fOnlyZId;			//set to kTRUE when only to be used to give Z identification of nuclei, no mass info
   KVList 			*fIdentifiers;		//-> list of identification objects
   KVList 			*fCuts;       		//-> cuts used to define area in which identification is possible
   Axis_t 			fXmin, fXmax; 		//!min/max X coordinates of graph
   Axis_t 			fYmin, fYmax; 		//!min/max Y coordinates of graph
   KVGenParList 	*fPar;				//-> parameters associated to grid
   Double_t 		fLastScaleX;		//last applied scaling factor on X
   Double_t 		fLastScaleY;		//last applied scaling factor on Y
	TVirtualPad 	*fPad; 				//!pad in which graph is drawn 
	KVNumberList	fRunList;			//runs for which grid is valid
	TList 			fTelescopes;		//ID telescopes for which grid is valid
	TString			fDyName; 			//!dynamically generated name
   TString  		fPattern;			//pattern of filenames used to write or read grid
	
   void Scale(Double_t sx = -1, Double_t sy = -1);
   virtual void ReadFromAsciiFile(ifstream & gridfile);
   virtual void WriteToAsciiFile(ofstream & gridfile);
   void init();
	virtual KVIDentifier* New(const Char_t*);
	virtual void ReadIdentifierFromAsciiFile(TString &name, TString &type, TString &cl, ifstream& gridfile);
	void FillListOfIDTelescopes();
	void WriteParameterListOfIDTelescopes();
	virtual void BackwardsCompatibilityFix();
	virtual Bool_t AcceptIDForTest()
	{
   	// Used by TestIdentification.
   	// The result of the identification may be excluded from the histograms of PID
   	// and PID vs. Eres, depending on e.g. some status code of the identification algorithm.
   	// By default, this returns kTRUE (accept all), but may be overridden in child classes.
   	return kTRUE;
	};
	
	
   public:
			
   KVIDGraph();
   KVIDGraph(const KVIDGraph &);
   virtual ~KVIDGraph();
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   virtual void Copy(TObject &) const;
#else
   virtual void Copy(TObject &);
#endif
   virtual void Identify(Double_t /*x*/, Double_t /*y*/, KVReconstructedNucleus * /*nuc*/) const = 0;   
   virtual void Initialize()=0; 
   virtual Bool_t IsIdentifiable(Double_t /*x*/, Double_t /*y*/) const;

	static KVIDGraph* MakeIDGraph(const Char_t *);
	
   void FindAxisLimits();
   void Scale(TF1 *sx,TF1 *sy);
	
   KVIDentifier *GetIdentifier(Int_t Z, Int_t A) const;
   void RemoveIdentifier(KVIDentifier*);
   void RemoveCut(KVIDentifier*);
	
   void ReadAsciiFile(const Char_t * filename);
	void AddIDTelescopes(const TList*);
   virtual void     SetName(const char *name){TNamed::SetName(name); Modified();}; // *MENU*
	virtual void Clear(Option_t* opt=""); //  *MENU*	
	void SetIDTelescopes();// *MENU={Hierarchy="Set.../ID Telescopes"}*
   void SetXVariable(const char* v){ SetVarX(v);  Modified();}; //  *MENU={Hierarchy="Set.../X Variable"}* *ARGS={v=>fVarX}
   void SetYVariable(const char* v){ SetVarY(v);  Modified();};//  *MENU={Hierarchy="Set.../Y Variable"}* *ARGS={v=>fVarY}
	void SetRunList(const char* runlist){ SetRuns( KVNumberList(runlist) ); };  // *MENU={Hierarchy="Set.../List of Runs"}*
   // Use this method if the graph is only to be used for Z identification
   // (no isotopic information). Default is to identify both Z & A
   // (fOnlyZid = kFALSE). Note that setting fOnlyZid=kTRUE changes the way line
   // widths are calculated (see KVIDGrid::CalculateLineWidths)
   void SetOnlyZId(Bool_t yes=kTRUE) { fOnlyZId = yes; Modified(); };//  *TOGGLE={Hierarchy="Set.../Z identification only"}*
	void SetMassFormula(Int_t);// *MENU={Hierarchy="Set.../Mass Formula"}*
   void WriteAsciiFile(const Char_t * filename);// *MENU*
   void SetXScaleFactor(Double_t = 0); //  *MENU={Hierarchy="Scale.../X Scale Factor"}*
   void SetYScaleFactor(Double_t = 0);//  *MENU={Hierarchy="Scale.../Y Scale Factor"}*
	virtual void NewCut();// *MENU={Hierarchy="AddLine.../NewCut"}*
	virtual void NewIdentifier();// *MENU={Hierarchy="AddLine.../NewIdentifier"}*
	
   void Draw(Option_t * opt = ""); //  *MENU={Hierarchy="View.../Draw"}*
   void UnDraw(); //  *MENU={Hierarchy="View.../UnDraw"}*
	void ResetDraw();// *MENU={Hierarchy="View.../ResetDraw"}*
	void SetLineColor(Color_t lcolor)
	{
   	//Set line colour of all objects in grid
   	fIdentifiers->Execute("SetLineColor", Form("%d", (Int_t) lcolor));
   	fCuts->Execute("SetLineColor", Form("%d", (Int_t) lcolor));Modified();
	};// *MENU={Hierarchy="View.../SetLinecolor"}*
	void SetLineStyle(Style_t lstyle)
	{
   	//Set line style of all objects in grid
   	fIdentifiers->Execute("SetLineStyle", Form("%d", (Int_t) lstyle));
   	fCuts->Execute("SetLineStyle", Form("%d", (Int_t) lstyle));Modified();
	};// *MENU={Hierarchy="View.../SetLineStyle"}*
	void SetLineWidth(Width_t lwidth)
	{
   	//Set line width of all objects in grid
   	fIdentifiers->Execute("SetLineWidth", Form("%d", (Int_t) lwidth));
   	fCuts->Execute("SetLineWidth", Form("%d", (Int_t) lwidth));Modified();
	};// *MENU={Hierarchy="View.../SetLineWidth"}*
	
	void TestGrid(); // *MENU={Hierarchy="Test.../TestGrid"}*
   void TestIdentificationWithTree(const Char_t* hname); //  *MENU={Hierarchy="Test.../TestIdentificationWithTree"}*
	
   Double_t GetXScaleFactor();
   Double_t GetYScaleFactor();
   void Add(TString, KVIDentifier *);
   KVIDentifier* Add(TString, TString);
   virtual void DrawAndAdd(const Char_t* type="ID", const Char_t* classname="KVIDentifier");
   void Print(Option_t * opt = "") const;
   void TestIdentification(TH2F * data, TH1F * id_real,
                           TH2F * id_real_vs_e_res);
	
   static KVIDGraph *AddGraphs(KVIDGraph * g1, Int_t id1_min, Int_t id1_max,
                             KVIDGraph * g2, Int_t id2_min, Int_t id2_max);
	void SetRuns(const KVNumberList& nl);
	
	const Char_t* GetName() const;
	virtual void WriteAsciiFile_WP(Int_t version=-1);//WP means with pattern
   virtual void ReadAsciiFile_WP(Int_t version=-1); //WP means with pattern
	virtual Int_t CheckVersion(Int_t version);
	virtual Bool_t ExistVersion(Int_t version){
		if (version!=-1) return (version==CheckVersion(version));
		else return (version!=-1);
	};
   void SetPattern(TString pattern) { pattern.ReplaceAll(".","_"); fPattern = pattern;}	
   TString GetPattern(void) {return fPattern;}
	
	const TList* GetIDTelescopes() const { return &fTelescopes; };	
   void Increment(Float_t x)  // *SIGNAL*
	{
   	// Used by TestIdentification and KVTestIDGridDialog to send
   	// signals to TGHProgressBar about the progress of the identification test
   	Emit("Increment(Float_t)", x);
	};
	void Modified() // *SIGNAL*
	{
		// Signal sent out when properties of graph change
		Emit("Modified()");
	};
	
   // Returns kTRUE if the graph is only to be used for Z identification
   // (no isotopic information). Default is to identify both Z & A
   // (fOnlyZid = kFALSE)
   Bool_t OnlyZId() const { return fOnlyZId; };
   // Returns kTRUE if the graph is only to be used for Z identification
   // (no isotopic information). Default is to identify both Z & A
   // (fOnlyZid = kFALSE)
   Bool_t IsOnlyZId() const { return fOnlyZId; };
   
	const KVNumberList& GetRuns() const
	{
		// Get list of runs for which grid is valid
		return fRunList;
	};
	const Char_t* GetRunList() const
	{
		// Get list of runs for which grid is valid
		return fRunList;
	};
   KVIDentifier *GetIdentifierAt(Int_t index) const
	{
		// Return identifier at position 'index' (=0,1,...) in list of identifiers
		return (KVIDentifier*)fIdentifiers->At(index);
	};
   KVIDentifier *GetIdentifier(const Char_t * name) const
	{
		return (KVIDentifier*)fIdentifiers->FindObject(name);
	};
   KVIDentifier *GetCut(const Char_t * name) const
	{
		return (KVIDentifier*)fCuts->FindObject(name);
	};
   // Return pointer to list of parameters associated to grid
   KVGenParList *GetParameters() const {
      return fPar;
   };
   KVList *GetIdentifiers() const {
		// Returns list of identifier objects (derived from KVIDentifier)
      return fIdentifiers;
   };
   KVList *GetCuts() const {
		// Returns list of cuts (derived from KVIDentifier)
      return fCuts;
   };
   Int_t GetNumberOfIdentifiers() const {
      return fIdentifiers->GetSize();
   };
   Int_t GetNumberOfCuts() const {
      return fCuts->GetSize();
   };
   void AddIdentifier(KVIDentifier *id) {
		// Add identifier to the graph. It will be deleted by the graph.
      fIdentifiers->Add(id);
		id->SetParent(this);
		id->SetVarX(GetVarX());
		id->SetVarY(GetVarY());
		id->SetBit(kMustCleanup);
 		Modified();    };
   void AddCut(KVIDentifier *cut) {
		// Add cut to the graph. It will be deleted by the graph.
      cut->SetLineColor(kRed);
		cut->SetParent(this);
      fCuts->Add(cut);
		cut->SetVarX(GetVarX());
		cut->SetVarY(GetVarY());
		cut->SetBit(kMustCleanup);
 		Modified();    };
   void SortIdentifiers() {
      fIdentifiers->Sort();
 		//Modified();
	};
   Bool_t IsSorted() const
	{
		// Return kTRUE if list of identifiers has been sorted
		return fIdentifiers->IsSorted();
	};
   Axis_t GetXmin() const {
      return fXmin;
   };
   Axis_t GetYmin() const {
      return fYmin;
   };
   Axis_t GetXmax() const {
      return fXmax;
   };
   Axis_t GetYmax() const {
      return fYmax;
   };
   // Return quality code related to previously-attempted identification using grid.
   // Redefine in child classes.
   virtual Int_t GetQualityCode() const
   {
      return 0;
   };
	void AddIDTelescope(KVIDTelescope*t)
	{
		// Associate this graph with an identification telescope, i.e. add the
		// telescope to the list of telescopes which can use this
		// graph for identification purposes.
	
		fTelescopes.Add(t);Modified();
	};
	void RemoveIDTelescope(KVIDTelescope*t)
	{
		// Remove telescope from the list of telescopes
		// which can use this graph for identification purposes.
	
		fTelescopes.Remove(t);Modified();
	};
	Bool_t HandlesIDTelescope(KVIDTelescope*t) const
	{
		// Returns kTRUE if telescope can use this graph for identification purposes.
	
		return fTelescopes.Contains(t);
	};
	const Char_t* GetIDTelescopeLabel() const
	{
		// Returns type of ID telescope to which this grid is associated
		// (actually returns value of KVIDTelescope::GetLabel())
		
		KVIDTelescope* id = (KVIDTelescope*)fTelescopes.First();
		return (id ? id->GetLabel() : "");
	};
	Int_t GetMassFormula() const;
	void ResetPad();
	void ClearPad(TVirtualPad*);
	
   //---- The following redeclarations are here just to remove the *MENU* tag which
   //---- is present in TGraph.h, to stop these methods appearing in the ID line context menus
   virtual void      SetMaximum(Double_t maximum=-1111){TGraph::SetMaximum(maximum);}; 
   virtual void      SetMinimum(Double_t minimum=-1111){TGraph::SetMinimum(minimum);};
   //virtual void      SetTitle(const char *title="") {TGraph::SetTitle(title);};
   virtual void      FitPanel() { TGraph::FitPanel(); };
   virtual void      DrawPanel() { TGraph::DrawPanel(); };
   virtual Int_t     Fit(const char *formula ,Option_t *option="" ,Option_t *goption="", Axis_t xmin=0, Axis_t xmax=0)
   { return TGraph::Fit(formula,option,goption,xmin,xmax);};
   virtual Int_t     Fit(TF1 *f1 ,Option_t *option="" ,Option_t *goption="", Axis_t xmin=0, Axis_t xmax=0)
   { return TGraph::Fit(f1,option,goption,xmin,xmax);};
   //---- The following redeclarations are here just to remove the *MENU* tag which
   //---- is present in TObject.h, to stop these methods appearing in the ID line context menus
   //virtual void        Delete(Option_t *option=""){TGraph::Delete(option);}; 
   virtual void        DrawClass() const {TGraph::DrawClass();};
   virtual TObject    *DrawClone(Option_t *option="") const {return TGraph::DrawClone(option);};
   virtual void        Dump() const {TGraph::Dump();}; 
   virtual void        Inspect() const {TGraph::Inspect();};
   virtual void        SaveAs(const char *filename="",Option_t *option="") const {TGraph::SaveAs(filename,option);};
   virtual void        SetDrawOption(Option_t *option="") {TGraph::SetDrawOption(option);};
   virtual void        SetLineAttributes() {TGraph::SetLineAttributes();};
   virtual void        SetFillAttributes() {TGraph::SetFillAttributes();};
   virtual void        SetMarkerAttributes(){TGraph::SetMarkerAttributes();};
   virtual void 			SetVarX(const char* v){TCutG::SetVarX(v); };
   virtual void 			SetVarY(const char* v){TCutG::SetVarY(v); };
   virtual Int_t         InsertPoint(){return TCutG::InsertPoint();};
   virtual Int_t         RemovePoint(){return TCutG::RemovePoint();};
	
   ClassDef(KVIDGraph,1)//Base class for particle identification in a 2D map
};

#endif
