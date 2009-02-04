#ifndef ALIITSTRACKERSA_H
#define ALIITSTRACKERSA_H 



#include "AliITSgeomTGeo.h"
#include "AliITStrackerMI.h"

/* Copyright(c) 1998-2003, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

/* $Id$ */


////////////////////////////////////////////////////
//  Stand alone tracker class                     //
//  Origin:  Elisabetta Crescio                   //
//  e-mail:  crescio@to.infn.it                   //
//  adapted for cosmics by A.Dainese              //
////////////////////////////////////////////////////

class AliITSclusterTable;
class AliITStrackSA;
class AliESDVertex;
class AliESDEvent;
class AliITSVertexer;
class TTree;
class TArrayD;

class AliITStrackerSA : public AliITStrackerMI {


 public:

  AliITStrackerSA();
  AliITStrackerSA(const Char_t *geom);
  AliITStrackerSA(const Char_t *geom,AliESDVertex *vert);
  AliITStrackerSA(const Char_t *geom,AliITSVertexer *vertexer);
  AliITStrackerSA(const AliITStrackerSA& tracker);
  AliITStrackerSA& operator=(const AliITStrackerSA& source);
  virtual ~AliITStrackerSA();  
  virtual Int_t Clusters2Tracks(AliESDEvent *event);
  Int_t FindTracks(AliESDEvent* event);

  AliITStrackV2* FitTrack(AliITStrackSA* tr,Double_t* primaryVertex,Bool_t onePoint=kFALSE);
  void StoreTrack(AliITStrackV2 *t,AliESDEvent *event) const; 
  Int_t FindTrackLowChiSquare() const;
  Int_t LoadClusters(TTree *cf) {Int_t rc=AliITStrackerMI::LoadClusters(cf); SetClusterTree(cf); SetSixPoints(kTRUE); return rc;}
  void SetVertex(AliESDVertex *vtx){fVert = vtx;}
  void SetClusterTree(TTree * itscl){fITSclusters = itscl;}
  void SetSixPoints(Bool_t sp = kFALSE){fSixPoints = sp;}
  Bool_t GetSixPoints() const {return fSixPoints;}
  void SetOuterStartLayer(Int_t osl = 0) {if(osl>(AliITSgeomTGeo::GetNLayers()-2)) osl=AliITSgeomTGeo::GetNLayers()-2; fOuterStartLayer = osl;}
  Int_t GetOuterStartLayer() const {return fOuterStartLayer;}
  void SetSAFlag(Bool_t fl){fITSStandAlone=fl;}  // StandAlone flag setter
  Bool_t GetSAFlag() const {return fITSStandAlone;} // StandAlone flag getter
  void SetFixedWindowSizes(Int_t n=46, Double_t *phi=0, Double_t *lam=0);
  void SetCalculatedWindowSizes(Int_t n=10, Float_t phimin=0.002, Float_t phimax=0.0145, Float_t lambdamin=0.003, Float_t lambdamax=0.008);

  void SetMinimumChargeSDDSSD(Float_t minq=0.){fMinQ=minq;}
  enum {kSAflag=0x8000}; //flag to mark clusters used in the SA tracker

 protected:

  //Initialization
  void Init();
  void ResetForFinding();
  void UpdatePoints();

  static Double_t Curvature(Double_t x1,Double_t y1,Double_t x2,Double_t y2,
                     Double_t x3,Double_t y3);

  Double_t ChoosePoint(Double_t p1, Double_t p2, Double_t pp); 

  static Int_t   FindIntersection(Float_t a1, Float_t b1, Float_t c1, Float_t c2, 
                           Float_t& x1,Float_t& y1, Float_t& x2, Float_t& y2);
  static Int_t   FindEquation(Float_t x1, Float_t y1, Float_t x2, Float_t y2, 
                       Float_t x3, Float_t y3,Float_t& a, Float_t& b, 
                       Float_t& c);
 
  static Int_t FindLabel(Int_t l1, Int_t l2, Int_t l3, Int_t l4, Int_t l5, Int_t l6);
  static Int_t Label(Int_t gl1, Int_t gl2, Int_t gl3, Int_t gl4, Int_t gl5, 
              Int_t gl6,Int_t gl7, Int_t gl8, Int_t gl9, Int_t gl10,Int_t gl11,
              Int_t gl12, Int_t gl13, Int_t gl14,Int_t gl15, Int_t gl16, 
              Int_t gl17, Int_t gl18, Int_t numberofpoints=6);
 
  Int_t SearchClusters(Int_t layer,Double_t phiwindow,Double_t lambdawindow, 
                       AliITStrackSA* trs,Double_t zvertex,Int_t flagp); 

  void GetCoorAngles(AliITSRecPoint* cl,Double_t &phi,Double_t &lambda,Float_t &x,Float_t &y,Float_t &z,Double_t* vertex);
  void GetCoorErrors(AliITSRecPoint* cl,Float_t &sx,Float_t &sy, Float_t &sz);

  AliITSclusterTable* GetClusterCoord(Int_t layer,Int_t n) const {return (AliITSclusterTable*)fCluCoord[layer]->UncheckedAt(n);}
  void RemoveClusterCoord(Int_t layer, Int_t n) {fCluCoord[layer]->RemoveAt(n);fCluCoord[layer]->Compress();}


  Double_t fPhiEstimate; //Estimation of phi angle on next layer
  Bool_t fITSStandAlone; //Tracking is performed in the ITS alone if kTRUE
  Float_t fPoint1[2];   //! coord. of 1-st point to evaluate the curvature
  Float_t fPoint2[2];   //! coord. of 2-nd point to evaluate the curvature
  Float_t fPoint3[2];   //! coord. of 3-rd point to evaluate the curvature
  Float_t fPointc[2];   //! current point coord (for curvature eval.)
  Double_t fLambdac;    //! current value of the Lambda angle in the window
  Double_t fPhic;       //! current value of the Phi angle in the window
  Float_t fCoef1;       //! param. of the equation of the circ. approx a layer
  Float_t fCoef2;       //! param. of the equation of the circ. approx a layer
  Float_t fCoef3;       //! param. of the equation of the circ. approx a layer
  Int_t fNloop;         //  Number of iterqations on phi and lambda windows
  Double_t *fPhiWin;    // phi window sizes
  Double_t *fLambdaWin; // lambda window sizes
  AliESDVertex *fVert;        //! primary vertex
  AliITSVertexer *fVertexer;  //! vertexer 
  TObjArray *fListOfTracks;   //! container for found tracks 
  TObjArray *fListOfSATracks; //! container for found SA tracks 
  TTree *fITSclusters;        //! pointer to ITS tree of clusters
  Bool_t fSixPoints;          // If true 6/6 points are required (default). 5/6 otherwise
  Int_t fOuterStartLayer;     // Search for tracks with <6 points: outer layer to start from
  Float_t fMinQ;              // lower cut on cluster charge (SDD and SSD)

  TClonesArray** fCluLayer; //! array with clusters 
  TClonesArray** fCluCoord; //! array with cluster info

  ClassDef(AliITStrackerSA,8)
};

#endif
