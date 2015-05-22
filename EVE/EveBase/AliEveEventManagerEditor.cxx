// @(#)root/eve:$Id$
// Author: Matevz Tadel 2007

/**************************************************************************
 * Copyright(c) 1998-2008, ALICE Experiment at CERN, all rights reserved. *
 * See http://aliceinfo.cern.ch/Offline/AliRoot/License.html for          *
 * full copyright notice.                                                 *
 **************************************************************************/

#include "AliEveEventManagerEditor.h"
#include "AliEveEventManager.h"

#include <AliESDEvent.h>

#include <TVirtualPad.h>
#include "TColor.h"

#include <TEveGValuators.h>
#include <TGButton.h>
#include <TGTextView.h>
#include <TGLabel.h>
#include <TGFileDialog.h>

#include "Riostream.h"

#ifdef ZMQ
#include "AliStorageAdministratorPanelListEvents.h"
#include "AliStorageAdministratorPanelMarkEvent.h"
#endif

//______________________________________________________________________________
// GUI editor for AliEveEventManager.
//

using std::ifstream;
using std::ofstream;
using std::ios;
using std::cout;
using std::endl;
using std::string;

ClassImp(AliEveEventManagerEditor)

//______________________________________________________________________________
AliEveEventManagerEditor::AliEveEventManagerEditor(const TGWindow *p, Int_t width, Int_t height,
                                                   UInt_t options, Pixel_t back) :
TGedFrame(p, width, height, options | kVerticalFrame, back),
fM(0),
fDumpEventInfo(0),
fEventInfo(0)
{
    // Constructor.
    
    MakeTitle("AliEveEventManager");
    
    {
        TGHorizontalFrame* f = new TGHorizontalFrame(this);
        fDumpEventInfo = new TGTextButton(f, "Dump Event Info");
        fDumpEventInfo->SetToolTipText("Append information about current event to event_info.txt file.");
        fDumpEventInfo->SetWidth(120);
        fDumpEventInfo->ChangeOptions(fDumpEventInfo->GetOptions() | kFixedWidth);
        f->AddFrame(fDumpEventInfo, new TGLayoutHints(kLHintsNormal, 4,0,0,0));
        fDumpEventInfo->Connect("Clicked()",
                                "AliEveEventManagerEditor", this, "DumpEventInfo()");
        AddFrame(f, new TGLayoutHints(kLHintsExpandX, 8,8,8,0));
    }
    {
        TGVerticalFrame* f = new TGVerticalFrame(this);
        
        TGLabel *eventInfoLabel = new TGLabel(f, "Event Information:");
        f->AddFrame(eventInfoLabel, new TGLayoutHints(kLHintsNormal, 0,0,6,2));
        
        fEventInfo = new TGTextView(f, 200, 300);
        f->AddFrame(fEventInfo, new TGLayoutHints(kLHintsNormal | kLHintsExpandX));
        
        AddFrame(f, new TGLayoutHints(kLHintsNormal | kLHintsExpandX));
    }
}

/******************************************************************************/

//______________________________________________________________________________
void AliEveEventManagerEditor::SetModel(TObject* obj)
{
    // Set model object.
    
    fM = static_cast<AliEveEventManager*>(obj);
    
    fEventInfo->LoadBuffer(fM->GetEventInfoVertical());
}

/******************************************************************************/

//______________________________________________________________________________
void AliEveEventManagerEditor::DumpEventInfo()
{
    // Dump event-info into event_info.txt.
    // The info is appended into the file.
    
    ofstream f("event_info.txt", ios::out | ios::app);
    
    f << "================================================================================\n\n";
    f << fM->GetEventInfoHorizontal() << std::endl << std::endl;
    
    f.close();
}


//==============================================================================
// AliEveEventManagerWindow
//==============================================================================

//______________________________________________________________________________
//
// Horizontal GUI for AliEveEventManager, to be placed in the
// bottom part of ROOT browser.

ClassImp(AliEveEventManagerWindow)

AliEveEventManagerWindow::AliEveEventManagerWindow(AliEveEventManager* mgr,bool storageManager) :
TGMainFrame(gClient->GetRoot(), 400, 100, kVerticalFrame),
fM            (mgr),
fFirstEvent   (0),
fPrevEvent    (0),
fNextEvent    (0),
fLastEvent    (0),
fRefresh      (0),
fEventId      (0),
fInfoLabel    (0),
fAutoLoad     (0),
fAutoLoadTime (0),
fTrigSel      (0),
fEventInfo    (0)
{
    // Constructor.
    
    const TString cls("AliEveEventManagerWindow");
    TGTextButton *b = 0;
    {
        Int_t width = 50;
        
        TGHorizontalFrame* f = new TGHorizontalFrame(this);
        AddFrame(f, new TGLayoutHints(kLHintsExpandX, 0,0,2,2));
        
        fFirstEvent = b = MkTxtButton(f, "First", width);
        b->Connect("Clicked()", cls, this, "DoFirstEvent()");
        fPrevEvent = b = MkTxtButton(f, "Prev", width);
        b->Connect("Clicked()", cls, this, "DoPrevEvent()");
        
        fEventId = new TGNumberEntry(f, 0, 5, -1,TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative,
                                     TGNumberFormat::kNELLimitMinMax, 0, 10000);
        f->AddFrame(fEventId, new TGLayoutHints(kLHintsNormal, 10, 5, 0, 0));
        fEventId->Connect("ValueSet(Long_t)", cls, this, "DoSetEvent()");
        fInfoLabel = new TGLabel(f);
        f->AddFrame(fInfoLabel, new TGLayoutHints(kLHintsNormal, 5, 10, 4, 0));
        
        fNextEvent = b = MkTxtButton(f, "Next", width);
        b->Connect("Clicked()", cls, this, "DoNextEvent()");
        fLastEvent = b = MkTxtButton(f, "Last", width);
        b->Connect("Clicked()", cls, this, "DoLastEvent()");
        fMarkEvent = b = MkTxtButton(f, "Mark", width);
        b->Connect("Clicked()", cls, this, "DoMarkEvent()");

        fScreenshot = b = MkTxtButton(f, "Screenshot", 2*width);
        b->Connect("Clicked()", cls, this, "DoScreenshot()");
        
        
        MkLabel(f, "||", 0, 8, 8);
        
        fRefresh = b = MkTxtButton(f, "Refresh", width + 8);
        b->Connect("Clicked()",cls, this, "DoRefresh()");
        
        MkLabel(f, "||", 0, 8, 8);
        
        fAutoLoad = new TGCheckButton(f, "Autoload");
        f->AddFrame(fAutoLoad, new TGLayoutHints(kLHintsLeft, 0, 4, 3, 0));
        fAutoLoad->SetToolTipText("Automatic event loading.");
        fAutoLoad->Connect("Toggled(Bool_t)", cls, this, "DoSetAutoLoad()");
        
        fAutoLoadTime = new TEveGValuator(f, "Time: ", 110, 0);
        f->AddFrame(fAutoLoadTime);
        fAutoLoadTime->SetShowSlider(kFALSE);
        fAutoLoadTime->SetNELength(4);
        fAutoLoadTime->Build();
        fAutoLoadTime->SetLimits(0, 1000);
        fAutoLoadTime->SetToolTip("Automatic event loading time in seconds.");
        fAutoLoadTime->Connect("ValueSet(Double_t)", cls, this, "DoSetAutoLoadTime()");
        
        fTrigSel = new TGComboBox(f);
        fTrigSel->Resize(4*width,b->GetDefaultHeight());
        fTrigSel->AddEntry("No trigger selection",-1);
        fTrigSel->Select(-1,kFALSE);
        f->AddFrame(fTrigSel, new TGLayoutHints(kLHintsNormal, 10, 5, 0, 0));
        fTrigSel->Connect("Selected(char*)", cls, this, "DoSetTrigSel()");
        
        fStorageStatus = MkLabel(f,"Storage: Waiting",0,8,8);
        fEventServerStatus = MkLabel(f,"Event Server: Waiting",0,10,10);
        
    }
    
    fEventInfo = new TGTextView(this, 400, 600);
    AddFrame(fEventInfo, new TGLayoutHints(kLHintsNormal | kLHintsExpandX | kLHintsExpandY));
    
    fM->Connect("NewEventLoaded()", cls, this, "Update(=1)");
    fM->Connect("NoEventLoaded()", cls, this, "Update(=0)");

    if(storageManager) // if SM is enabled in general
    {
        fM->Connect("StorageManagerOk()",cls,this,"StorageManagerChangedState(=1)");
        fM->Connect("StorageManagerDown()",cls,this,"StorageManagerChangedState(=0)");
    }
    else
    {
        StorageManagerChangedState(0);
    }
        
    SetCleanup(kDeepCleanup);
    Layout();
    MapSubwindows();
    MapWindow();
}

//______________________________________________________________________________
AliEveEventManagerWindow::~AliEveEventManagerWindow()
{
    // Destructor.
    
    fM->Disconnect("NewEventLoaded()", this);
}

//______________________________________________________________________________
void AliEveEventManagerWindow::DoFirstEvent()
{
    // Load previous event
    fM->GotoEvent(0);
}

//______________________________________________________________________________
void AliEveEventManagerWindow::DoPrevEvent()
{
    // Load previous event
    // fM->PrevEvent();
    if (fM->IsOnlineMode()) {
        fM->GotoEvent(1);
    }
    else {
        fM->GotoEvent((Int_t) fEventId->GetNumber()-1);
        
    }
    
}

//______________________________________________________________________________
void AliEveEventManagerWindow::DoNextEvent()
{
    // Load next event
    // fM->NextEvent();
    if (fM->IsOnlineMode()) {
        cout<<"next event, online node"<<endl;
        fM->GotoEvent(2);
    }
    else {
        cout<<"next event, offline mode"<<endl;
        fM->GotoEvent((Int_t) fEventId->GetNumber()+1);
    }
}

//______________________________________________________________________________
void AliEveEventManagerWindow::DoLastEvent()
{
    // Load previous event
    fM->GotoEvent(-1);
}

//______________________________________________________________________________
void AliEveEventManagerWindow::DoMarkEvent()
{
    // Mark current event
    fM->MarkCurrentEvent();
}

//______________________________________________________________________________
void AliEveEventManagerWindow::DoScreenshot()
{
    // Save screenshot to file

    TGFileInfo fileinfo;
    const char *filetypes[] = {"PNG images", "*.png", 0, 0};
    fileinfo.fFileTypes = filetypes;
    fileinfo.fIniDir = StrDup(".");
    new TGFileDialog(gClient->GetDefaultRoot(), gClient->GetDefaultRoot(),kFDOpen, &fileinfo);
    if(!fileinfo.fFilename)
    {
        return;
    }

    AliEveSaveViews *viewsSaver = new AliEveSaveViews();
    viewsSaver->Save(fileinfo.fFilename);
    
    
}

//______________________________________________________________________________
void AliEveEventManagerWindow::DoSetEvent()
{
    // Set current event
    fM->GotoEvent((Int_t) fEventId->GetNumber());
}

//______________________________________________________________________________
void AliEveEventManagerWindow::DoRefresh()
{
    // Refresh event status.
    
    Int_t ev = fM->GetEventId();
    fM->Close();
    fM->Open();
    fM->GotoEvent(ev);
}

//______________________________________________________________________________
void AliEveEventManagerWindow::DoSetAutoLoad()
{
    // Set the auto-load flag
    
    fM->SetAutoLoad(fAutoLoad->IsOn());
    Update(fM->NewEventAvailable());
}

//______________________________________________________________________________
void AliEveEventManagerWindow::DoSetAutoLoadTime()
{
    // Set the auto-load time in seconds
    
    fM->SetAutoLoadTime(fAutoLoadTime->GetValue());
}

//______________________________________________________________________________
void AliEveEventManagerWindow::DoSetTrigSel()
{
    // Set the trigger selection
    
    fM->SetTrigSel(fTrigSel->GetSelectedEntry()->EntryId());
}

//______________________________________________________________________________
void AliEveEventManagerWindow::Update(int state)
{
    Bool_t autoLoad = fM->GetAutoLoad();
    
    if (state==1)
    {
        fRefresh->SetEnabled(!autoLoad);
        
        fEventId->SetNumber(fM->GetEventId());
        fEventId->SetState(kTRUE);
        
        fAutoLoad->SetState(fM->GetAutoLoad() ? kButtonDown : kButtonUp);
        fAutoLoadTime->SetValue(fM->GetAutoLoadTime());
        
        // Loop over active trigger classes
        if (fM->GetESD() && !fM->IsOnlineMode())
        {
            for(Int_t iTrig = 0; iTrig < AliESDRun::kNTriggerClasses; iTrig++)
            {
                TString trigName = fM->GetESD()->GetESDRun()->GetTriggerClass(iTrig);
                if (trigName.IsNull())
                {
                    if (fTrigSel->GetListBox()->GetEntry(iTrig)) {
                        if (fTrigSel->GetSelected() == iTrig) fTrigSel->Select(-1);
                        fTrigSel->RemoveEntry(iTrig);
                    }
                    continue;
                }
                if (!fTrigSel->FindEntry(trigName.Data())){
                    fTrigSel->AddEntry(trigName.Data(),iTrig);
                }
            }
        }
        fTrigSel->SetEnabled(autoLoad);
        Layout();
    }
}

void AliEveEventManagerWindow::StorageManagerChangedState(int state)
{
#ifdef ZMQ
    if (!fM->IsOnlineMode())return;
    
    Bool_t autoLoad = fM->GetAutoLoad();
    AliStorageAdministratorPanelListEvents* listEventsTab = AliStorageAdministratorPanelListEvents::GetInstance();
    
    if (state == 0)// SM off
    {
        fMarkEvent->SetEnabled(kFALSE);
        fNextEvent->SetEnabled(kFALSE);
        fLastEvent->SetEnabled(kFALSE);
        fPrevEvent->SetEnabled(kFALSE);
        fFirstEvent->SetEnabled(kFALSE);
        listEventsTab->SetOfflineMode(kTRUE);
        fEventId->SetState(kTRUE);
    }
    else if(state == 1)// SM on
    {
        fMarkEvent->SetEnabled(kTRUE);
        fNextEvent->SetEnabled(kTRUE);
        fLastEvent->SetEnabled(kTRUE);
        fPrevEvent->SetEnabled(kTRUE);
        fFirstEvent->SetEnabled(kTRUE);
        listEventsTab->SetOfflineMode(kFALSE);
        fEventId->SetState(kTRUE);
    }
#endif
}

//------------------------------------------------------------------------------
// Protected methods
//------------------------------------------------------------------------------

//______________________________________________________________________________
TGTextButton* AliEveEventManagerWindow::MkTxtButton(TGCompositeFrame* p,
                                                    const char* txt, Int_t width,
                                                    Int_t lo, Int_t ro, Int_t to, Int_t bo)
{
    // Create a standard button.
    // If width is not zero, the fixed-width flag is set.
    
    TGTextButton* b = new TGTextButton(p, txt);
    if (width > 0) {
        b->SetWidth(width);
        b->ChangeOptions(b->GetOptions() | kFixedWidth);
    }
    p->AddFrame(b, new TGLayoutHints(kLHintsNormal, lo,ro,to,bo));
    return b;
}

//______________________________________________________________________________
TGLabel* AliEveEventManagerWindow::MkLabel(TGCompositeFrame* p,
                                           const char* txt, Int_t width,
                                           Int_t lo, Int_t ro, Int_t to, Int_t bo)
{
    // Create a standard button.
    // If width is not zero, the fixed-width flag is set.
    
    TGLabel* l = new TGLabel(p, txt);
    if (width > 0) {
        l->SetWidth(width);
        l->ChangeOptions(l->GetOptions() | kFixedWidth);
    }
    p->AddFrame(l, new TGLayoutHints(kLHintsNormal, lo,ro,to,bo));
    return l;
}

