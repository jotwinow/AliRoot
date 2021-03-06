/// 
/// Example of configuration for particle (photon) 
/// simulation in EMCal/DCal acceptance.
/// Include only EMCAL in this example, 
/// for fast testing of the simulation and reconstruction of the EMCal.
///
/// Example for the configuration needed for different years provided.
///

// One can use the configuration macro in compiled mode by
// root [0] gSystem->Load("libgeant321");
// root [0] gSystem->SetIncludePath("-I$ROOTSYS/include -I$ALICE_ROOT/include\
//                   -I$ALICE_ROOT -I$ALICE/geant3/TGeant3");
// root [0] .x grun.C(1,"Config.C++")

#if !defined(__CINT__) || defined(__MAKECINT__)
#include <Riostream.h>
#include <TPDGCode.h>
#include <TRandom.h>
#include <TSystem.h>
#include <TVirtualMC.h>
#include <TGeant3TGeo.h>
#include "STEER/AliRunLoader.h"
#include "STEER/AliRun.h"
#include "STEER/AliConfig.h"
#include "PYTHIA6/AliDecayerPythia.h"
#include "EVGEN/AliGenCocktail.h"
#include "EVGEN/AliGenHIJINGpara.h"
#include "STEER/AliMagF.h"
#include "STRUCT/AliBODY.h"
#include "STRUCT/AliMAG.h"
#include "STRUCT/AliABSOv3.h"
#include "STRUCT/AliDIPOv3.h"
#include "STRUCT/AliHALLv3.h"
#include "STRUCT/AliFRAMEv2.h"
#include "STRUCT/AliSHILv3.h"
#include "STRUCT/AliPIPEv3.h"
#include "ITS/AliITSv11.h"
#include "TPC/AliTPCv2.h"
#include "TOF/AliTOFv6T0.h"
#include "HMPID/AliHMPIDv2.h"
#include "ZDC/AliZDCv3.h"
#include "TRD/AliTRDv1.h"
#include "FMD/AliFMDv1.h"
#include "MUON/AliMUONv1.h"
#include "PHOS/AliPHOSv1.h"
#include "PMD/AliPMDv1.h"
#include "T0/AliT0v1.h"
#include "EMCAL/AliEMCALv2.h"
#include "ACORDE/AliACORDEv1.h"
#include "VZERO/AliVZEROv7.h"
#endif

Float_t EtaToTheta(Float_t arg);
void    LoadPythia();
AliGenerator *GenParamCalo(Int_t nPart, Int_t type, TString calo);

Int_t  year = 2015;
Bool_t checkGeoAndRun = kFALSE;

void Config()
{
  //AliLog::SetGlobalDebugLevel(2);
  
  // ThetaRange is (0., 180.). It was (0.28,179.72) 7/12/00 09:00
  // Theta range given through pseudorapidity limits 22/6/2001
  
  // Set Random Number seed
  //gRandom->SetSeed(123456); // Set 0 to use the current time
  
  AliLog::Message(AliLog::kInfo, 
                  Form("Seed for random number generation = %d",gRandom->GetSeed()), 
                  "Config.C", "Config.C", "Config()","Config.C", __LINE__);
  
  // Load Pythia libraries                                        
  LoadPythia();
  
  // libraries required by geant321
#if defined(__CINT__)
  gSystem->Load("libgeant321");
#endif
  
  new     TGeant3TGeo("C++ Interface to Geant3");
  
  AliRunLoader* rl=0x0;
  
  AliLog::Message(AliLog::kInfo, "Creating Run Loader", "Config.C", "Config.C", "Config()"," Config.C", __LINE__);
  
  rl = AliRunLoader::Open("galice.root",
                          AliConfig::GetDefaultEventFolderName(),
                          "recreate");
  if (rl == 0x0)
  {
    gAlice->Fatal("Config.C","Can not instatiate the Run Loader");
    return;
  }
  
  rl->SetCompressionLevel(2);
  rl->SetNumberOfEventsPerFile(10000);
  gAlice->SetRunLoader(rl);
  
  // Set the trigger configuration
  AliSimulation::Instance()->SetTriggerConfig("Pb-Pb");
  cout<<"Trigger configuration is set to  Pb-Pb"<<endl;
  
  //
  // Set External decayer
  TVirtualMCDecayer *decayer = new AliDecayerPythia();
  
  decayer->SetForceDecay(kAll);
  decayer->Init();

  TVirtualMC * vmc = TVirtualMC::GetMC();

  gMC->SetExternalDecayer(decayer);
  //=======================================================================
  // ************* STEERING parameters FOR ALICE SIMULATION **************
  // --- Specify event type to be tracked through the ALICE setup
  // --- All positions are in cm, angles in degrees, and P and E in GeV 
  
  gMC->SetProcess("DCAY",1);
  gMC->SetProcess("PAIR",1);
  gMC->SetProcess("COMP",1);
  gMC->SetProcess("PHOT",1);
  gMC->SetProcess("PFIS",0);
  gMC->SetProcess("DRAY",0);
  gMC->SetProcess("ANNI",1);
  gMC->SetProcess("BREM",1);
  gMC->SetProcess("MUNU",1);
  gMC->SetProcess("CKOV",1);
  gMC->SetProcess("HADR",1);
  gMC->SetProcess("LOSS",2);
  gMC->SetProcess("MULS",1);
  gMC->SetProcess("RAYL",1);
  
  Float_t cut    = 1.e-3;  // 1 MeV cut by default
  Float_t tofmax = 1.e10;
  
  gMC->SetCut("CUTGAM", cut);
  gMC->SetCut("CUTELE", cut);
  gMC->SetCut("CUTNEU", cut);
  gMC->SetCut("CUTHAD", cut);
  gMC->SetCut("CUTMUO", cut);
  gMC->SetCut("BCUTE",  cut); 
  gMC->SetCut("BCUTM",  cut); 
  gMC->SetCut("DCUTE",  cut); 
  gMC->SetCut("DCUTM",  cut); 
  gMC->SetCut("PPCUTM", cut);
  gMC->SetCut("TOFMAX", tofmax); 
  
  //
  // Particle generator settings
  //

  int     nParticles = 1;
  if (gSystem->Getenv("CONFIG_NPARTICLES"))
  {
    nParticles = atoi(gSystem->Getenv("CONFIG_NPARTICLES"));
  }
  
  if (gSystem->Getenv("CONFIG_YEAR"))
  {
    year = atoi(gSystem->Getenv("CONFIG_YEAR"));
  }
  
  AliGenBox *gener = new AliGenBox(nParticles);
  gener->SetMomentumRange(10.,10.);
  
  gener->SetPart(22); // Photons
  
  if     (year == 2010)
  gener->SetPhiRange(80.0,120.0);
  else if(year == 2011)
  gener->SetPhiRange(80.0,180.0);
  else if(year == 2012 || year == 2013)
  gener->SetPhiRange(80.0,190.0);
  else
  gener->SetPhiRange(80.0,330.0); // Include DCal
  
  gener->SetThetaRange(EtaToTheta(0.7), EtaToTheta(-0.7));
  
  //  AliGenLib* lib   = new AliGenPHOSlib();
  //  Int_t      type  = AliGenPHOSlib::kEtaFlat;
  //  AliGenParam *gener = new AliGenParam(1,lib,type,"");
  //	gener->SetMomentumRange(0,999);
  //	gener->SetPtRange(1,30);
  //	gener->SetPhiRange(80, 200.);
  //	gener->SetYRange(-2,2);
  //	gener->SetThetaRange(EtaToTheta(0.7),EtaToTheta(-0.7));
  //	gener->SetCutOnChild(1);
  //  gener->SetChildPtRange(0.1,30);
  //	gener->SetChildThetaRange(EtaToTheta(0.7),EtaToTheta(-0.7));
  //  gener->SetChildPhiRange(80, 180.);
  //	gener->SetOrigin(0,0,0);        //vertex position
  //	gener->SetSigma(0,0,5.3);       //Sigma in (X,Y,Z) (cm) on IP position
  //	gener->SetForceDecay(kGammaEM);
  //
  //  //gener->SetTrackingFlag(0);
  
  //  AliGenCocktail *gener = new AliGenCocktail();
  //  gener->SetProjectile("A", 208, 82);
  //  gener->SetTarget    ("A", 208, 82);
  //  
  //  // 1 Pi0 in EMCAL, 2010 configuration, 4 SM
  //  AliGenParam *gEMCPi0 = GenParamCalo(1, AliGenPHOSlib::kPi0Flat, "EMCAL");
  //  gener->AddGenerator(gEMCPi0,"pi0EMC", 1);
  //  
  //  // 1 Eta in EMCAL, 2010 configuration, 4 SM
  //  AliGenParam *gEMCEta = GenParamCalo(1, AliGenPHOSlib::kEtaFlat, "EMCAL");
  //  gener->AddGenerator(gEMCEta,"etaEMC", 1);
  //  
  
  gener->Init();
  
  // 
  // Activate this line if you want the vertex smearing to happen
  // track by track
  //
  //gener->SetVertexSmear(perTrack); 
  
  // Field (L3 0.5 T)
  TGeoGlobalMagField::Instance()->SetField(new AliMagF("Maps","Maps", -1., -1., AliMagF::k5kG));
  
  Int_t   iABSO  =  0;
  Int_t   iDIPO  =  0;
  Int_t   iFMD   =  0;
  Int_t   iFRAME =  0;
  Int_t   iHALL  =  0;
  Int_t   iITS   =  0;
  Int_t   iMAG   =  0;
  Int_t   iMUON  =  0;
  Int_t   iPHOS  =  0;
  Int_t   iPIPE  =  0;
  Int_t   iPMD   =  0;
  Int_t   iHMPID =  0;
  Int_t   iSHIL  =  0;
  Int_t   iT0    =  0;
  Int_t   iTOF   =  0;
  Int_t   iTPC   =  0;
  Int_t   iTRD   =  0;
  Int_t   iZDC   =  0;
  Int_t   iEMCAL =  1;
  Int_t   iACORDE=  0;
  Int_t   iVZERO =  0;
  
  rl->CdGAFile();
  
  //=================== Alice BODY parameters =============================
  AliBODY *BODY = new AliBODY("BODY", "Alice envelop");
  
  if (iMAG)
  {
    //=================== MAG parameters ============================
    // --- Start with Magnet since detector layouts may be depending ---
    // --- on the selected Magnet dimensions ---
    AliMAG *MAG = new AliMAG("MAG", "Magnet");
  }
  
  
  if (iABSO)
  {
    //=================== ABSO parameters ============================
    AliABSO *ABSO = new AliABSOv3("ABSO", "Muon Absorber");
  }
  
  if (iDIPO)
  {
    //=================== DIPO parameters ============================
    
    AliDIPO *DIPO = new AliDIPOv3("DIPO", "Dipole version 3");
  }
  
  if (iHALL)
  {
    //=================== HALL parameters ============================
    
    AliHALL *HALL = new AliHALLv3("HALL", "Alice Hall");
  }
  
  
  if (iFRAME)
  {
    //=================== FRAME parameters ============================
    
    AliFRAMEv2 *FRAME = new AliFRAMEv2("FRAME", "Space Frame");
  }
  
  if (iSHIL)
  {
    //=================== SHIL parameters ============================
    
    AliSHIL *SHIL = new AliSHILv3("SHIL", "Shielding Version 3");
  }
  
  
  if (iPIPE)
  {
    //=================== PIPE parameters ============================
    
    AliPIPE *PIPE = new AliPIPEv3("PIPE", "Beam Pipe");
  }
  
  if(iITS) {
    
    //=================== ITS parameters ============================
    
    AliITS *ITS  = new AliITSv11("ITS","ITS v11");
  }
  
  if (iTPC)
  {
    //============================ TPC parameters ===================
    AliTPC *TPC = new AliTPCv2("TPC", "Default");
  }
  
  
  if (iTOF) {
    //=================== TOF parameters ============================
    AliTOF *TOF = new AliTOFv6T0("TOF", "normal TOF");
  }
  
  
  if (iHMPID)
  {
    //=================== HMPID parameters ===========================
    AliHMPID *HMPID = new AliHMPIDv3("HMPID", "normal HMPID");
    
  }
  
  
  if (iZDC)
  {
    //=================== ZDC parameters ============================
    
    AliZDC *ZDC = new AliZDCv3("ZDC", "normal ZDC");
  }
  
  if (iTRD)
  {
    //=================== TRD parameters ============================
    
    AliTRD *TRD = new AliTRDv1("TRD", "TRD slow simulator");
    AliTRDgeometry *geoTRD = TRD->GetGeometry();
    // starting at 3h in positive direction
    if(year==2011 || year == 2010)
    { // not sure if good for 2010
      // Partial geometry: modules at 0,1,7,8,9,10,11,15,16,17
      printf("*** TRD configuration for 2011\n");
      geoTRD->SetSMstatus(2,0);
      geoTRD->SetSMstatus(3,0);
      geoTRD->SetSMstatus(4,0);
      geoTRD->SetSMstatus(5,0);
      geoTRD->SetSMstatus(6,0);
      geoTRD->SetSMstatus(12,0);
      geoTRD->SetSMstatus(13,0);
      geoTRD->SetSMstatus(14,0);
    }
    else if(year==2012)
    {
      printf("*** TRD configuration for 2012\n");
      geoTRD->SetSMstatus(4,0);
      geoTRD->SetSMstatus(5,0);
      geoTRD->SetSMstatus(12,0);
      geoTRD->SetSMstatus(13,0);
      geoTRD->SetSMstatus(14,0); 
    }
  }
  
  if (iFMD)
  {
    //=================== FMD parameters ============================
    AliFMD *FMD = new AliFMDv1("FMD", "normal FMD");
  }
  
  if (iMUON)
  {
    //=================== MUON parameters ===========================
    // New MUONv1 version (geometry defined via builders)
    AliMUON *MUON = new AliMUONv1("MUON", "default");
  }
  //=================== PHOS parameters ===========================
  
  if (iPHOS)
  {
    AliPHOS *PHOS = new AliPHOSv1("PHOS", "noCPV_Modules123");
  }
  
  
  if (iPMD)
  {
    //=================== PMD parameters ============================
    AliPMD *PMD = new AliPMDv1("PMD", "normal PMD");
  }
  
  if (iT0)
  {
    //=================== T0 parameters ============================
    AliT0 *T0 = new AliT0v1("T0", "T0 Detector");
  }
  
  if (iEMCAL)
  {
    //=================== EMCAL parameters ============================
    AliEMCAL *EMCAL = 0;
    if      (year == 2010)  // d phi = 40 degrees
      EMCAL = new AliEMCALv2("EMCAL", "EMCAL_FIRSTYEARV1",    checkGeoAndRun);
    else if (year == 2011)  // d phi = 100 degrees
      EMCAL = new AliEMCALv2("EMCAL", "EMCAL_COMPLETEV1",     checkGeoAndRun);
    else if (year == 2012 || year == 2013)   // d phi = 107 degrees
      EMCAL = new AliEMCALv2("EMCAL", "EMCAL_COMPLETE12SMV1", checkGeoAndRun);
    else
      EMCAL = new AliEMCALv2("EMCAL", "EMCAL_COMPLETE12SMV1_DCAL_8SM", checkGeoAndRun); // EMCAL+DCAL dphi = 107 (EMCAL) + 33 (gap) + 67 (DCAL)
  }
  
  if (iACORDE)
  {
    //=================== ACORDE parameters ============================
    AliACORDE *ACORDE = new AliACORDEv1("ACORDE", "normal ACORDE");
  }
  
  if (iVZERO)
  {
    //=================== ACORDE parameters ============================
    AliVZERO *VZERO = new AliVZEROv7("VZERO", "normal VZERO");
  }
  
  AliLog::Message(AliLog::kInfo, "End of Config", "Config.C", "Config.C", "Config()"," Config.C", __LINE__);
  
}

Float_t EtaToTheta(Float_t arg){
  return (180./TMath::Pi())*2.*atan(exp(-arg));
}

void LoadPythia()
{
  // Load Pythia related libraries                                                                
  gSystem->Load("liblhapdf");      // Parton density functions                                 
  gSystem->Load("libEGPythia6");   // TGenerator interface                                     
  gSystem->Load("libpythia6");     // Pythia                                                   
  gSystem->Load("libAliPythia6");  // ALICE specific
  // implementations                           
}


AliGenerator * GenParamCalo(Int_t nPart, Int_t type, TString calo)
{
  // nPart of type (Pi0, Eta, Pi0Flat, EtaFlat, ...) in EMCAL or PHOS
  // CAREFUL EMCAL year 2010 configuration
  AliGenParam *gener = new AliGenParam(nPart,new AliGenPHOSlib(),type,"");
  
  // meson cuts
  gener->SetMomentumRange(0,999);
  gener->SetYRange(-2,2);
  gener->SetPtRange(1,30);
  // photon cuts
  gener->SetForceDecay(kGammaEM); // Ensure the decays are photons
  gener->SetCutOnChild(1);
  gener->SetChildPtRange(0.,30);
  
  if(calo=="EMCAL")
  {
    //meson acceptance
    gener->SetPhiRange(80., 100.); // year 2010
    //gener->SetPhiRange(80., 180.); // year 2011
    gener->SetThetaRange(EtaToTheta(0.7),EtaToTheta(-0.7));
    //decay acceptance
    gener->SetChildThetaRange(EtaToTheta(0.7),EtaToTheta(-0.7));
    gener->SetChildPhiRange(80., 100.); // year 2010
    //gener->SetChildPhiRange(80., 180.); // year 2011
  }
  else if(calo=="PHOS")
  {
    //meson acceptance
    gener->SetPhiRange(260., 320.);
    gener->SetThetaRange(EtaToTheta(0.13),EtaToTheta(-0.13));
    //decay acceptance
    gener->SetChildThetaRange(EtaToTheta(0.13),EtaToTheta(-0.13));
    gener->SetChildPhiRange(260., 320.);
  }
  
  return gener;
}

