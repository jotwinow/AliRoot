#ifndef TRD_H
#define TRD_H
////////////////////////////////////////////////
//  Manager and hits classes for set: TRD     //
////////////////////////////////////////////////
 
#include "AliDetector.h"
#include "AliHit.h" 
#include "AliTRDconst.h"

//_____________________________________________________________________________
class AliTRD : public AliDetector {
 
protected:
  Int_t         fGasMix;            // Gas mixture. 0: Xe/Isobutane 1: Xe/CO2

  Float_t       fClengthI[kNplan];  // Length of the inner chambers
  Float_t       fClengthM[kNplan];  // Length of the middle chambers
  Float_t       fClengthO[kNplan];  // Length of the outer chambers
  Float_t       fCwidth[kNplan];    // Width of the chambers

public:
  AliTRD();
  AliTRD(const char *name, const char *title);
  virtual      ~AliTRD() {}
  virtual void  AddHit(Int_t, Int_t*, Float_t*);
  virtual void  BuildGeometry();
  virtual void  CreateGeometry();
  virtual void  CreateMaterials();
  virtual void  DrawModule();
  Int_t         DistancetoPrimitive(Int_t px, Int_t py);
  virtual void  Init();
  virtual Int_t IsVersion() const = 0;
  virtual void  StepManager() = 0; 
  virtual void  SetGasMix(Int_t imix = 0);
  virtual void  SetHits(Int_t ihit) {};
  virtual void  SetSensPlane(Int_t iplane) {};
  virtual void  SetSensChamber(Int_t ichamber) {};
  virtual void  SetSensSector(Int_t isector) {};

  ClassDef(AliTRD,1)                // Transition Radiation Detector base class

};

//_____________________________________________________________________________
class AliTRDhit : public AliHit {

public:
  Int_t        fSector;     // TRD sector number
  Int_t        fChamber;    // TRD chamber number
  Int_t        fPlane;      // TRD plane number 
  Float_t      fQ;          // Charge created by a hit (geometry 2)
 
public:
  AliTRDhit() {}
  AliTRDhit(Int_t shunt, Int_t track, Int_t *vol, Float_t *hits);
  virtual ~AliTRDhit() {}
 
  ClassDef(AliTRDhit,1)     // Hits for Transition Radiation Detector

};

#endif
