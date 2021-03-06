// $Id$

/**************************************************************************
 * This file is property of and copyright by the ALICE HLT Project        * 
 * ALICE Experiment at CERN, All rights reserved.                         *
 *                                                                        *
 * Primary Authors: Matthias Richter <Matthias.Richter@ift.uib.no>        *
 *                  for The ALICE HLT Project.                            *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

/// @file   AliHLTHOMERLibManager.cxx
/// @author Matthias Richter
/// @date   
/// @brief  dynamic HLT HOMER reader/writer generation and destruction.

#include <cerrno>
#include <cassert>
#include "AliHLTHOMERLibManager.h"
#include "AliHLTHOMERReader.h"
#include "AliHLTHOMERWriter.h"
#include "AliHLTLogging.h"
#include "TString.h"
#include "TSystem.h"

/** ROOT macro for the implementation of ROOT specific class methods */
ClassImp(AliHLTHOMERLibManager)

// global flag of the library status
int AliHLTHOMERLibManager::fgLibraryStatus=0;
// This list must be NULL terminated, since we use it as a marker to identify
// the end of the list.
const char* AliHLTHOMERLibManager::fgkLibraries[] = {"libAliHLTHOMER.so", "libHOMER.so", NULL};
// The size of the list of reference counts must be one less than fgkLibraries.
int AliHLTHOMERLibManager::fgkLibRefCount[] = {0, 0};

AliHLTHOMERLibManager::AliHLTHOMERLibManager()
  :
  fFctCreateReaderFromTCPPort(NULL),
  fFctCreateReaderFromTCPPorts(NULL),
  fFctCreateReaderFromBuffer(NULL),
  fFctDeleteReader(NULL),
  fFctCreateWriter(NULL),
  fFctDeleteWriter(NULL),
  fLoadedLib(NULL)
{
  // constructor
  // 
  // Interface to the HLT Online Monitoring Including Root (HOMER) library.
  // It allows to decouple the HLT base library from this additional library
  // while providing the basic functionality to the component libraries
}

AliHLTHOMERLibManager::~AliHLTHOMERLibManager()
{
  // destructor
  //
  // the library load strategy has been changed in March 2013 in order to
  // stabilize the runtime memory layout of AliRoot in an attemp to get control
  // over memory corruptions
  //  UnloadHOMERLibrary();
}

AliHLTHOMERReader* AliHLTHOMERLibManager::OpenReader(const char* hostname, unsigned short port )
{
  // Open Reader instance for host
  if (fgLibraryStatus<0) return NULL;

  fgLibraryStatus=LoadHOMERLibrary();
  if (fgLibraryStatus <= 0) {
	return NULL;
  }
  
  AliHLTHOMERReader* pReader=NULL;
  if (fFctCreateReaderFromTCPPort!=NULL && (pReader=(((AliHLTHOMERReaderCreateFromTCPPort_t)fFctCreateReaderFromTCPPort)(hostname, port)))==NULL) {
    cout <<"can not create instance of HOMER reader from ports" << endl;
  }
  
  return pReader;
}

AliHLTHOMERReader* AliHLTHOMERLibManager::OpenReader(unsigned int tcpCnt, const char** hostnames, unsigned short* ports)
{
  // Open Reader instance for a list of hosts
  if (fgLibraryStatus<0) return NULL;

  fgLibraryStatus=LoadHOMERLibrary();
  if (fgLibraryStatus <= 0) {
  	return NULL;
  }
  
  AliHLTHOMERReader* pReader=NULL;
  if (fFctCreateReaderFromTCPPorts!=NULL && (pReader=(((AliHLTHOMERReaderCreateFromTCPPorts_t)fFctCreateReaderFromTCPPorts)(tcpCnt, hostnames, ports)))==NULL) {
    //HLTError("can not create instance of HOMER reader (function %p)", fFctCreateReaderFromTCPPorts);
    cout << "can not create instance of HOMER reader from port"<<endl;
  }
  
  return pReader;
}

AliHLTHOMERReader* AliHLTHOMERLibManager::OpenReaderBuffer(const AliHLTUInt8_t* pBuffer, int size)
{
  // Open Reader instance for a data buffer
  if (fgLibraryStatus<0) return NULL;

  fgLibraryStatus=LoadHOMERLibrary();
  if (fgLibraryStatus <= 0) {
	return NULL;
  }
  
  AliHLTHOMERReader* pReader=NULL;
  if (fFctCreateReaderFromBuffer!=NULL && (pReader=(((AliHLTHOMERReaderCreateFromBuffer_t)fFctCreateReaderFromBuffer)(pBuffer, size)))==NULL) {
    //HLTError("can not create instance of HOMER reader (function %p)", fFctCreateReaderFromBuffer);
  }
  
  return pReader;
}

int AliHLTHOMERLibManager::DeleteReader(AliHLTHOMERReader* pReader)
{
  // delete a reader

  // the actual deletion function is inside the HOMER library
  if (fgLibraryStatus<0) return fgLibraryStatus;

  fgLibraryStatus=LoadHOMERLibrary();
  if (fgLibraryStatus <= 0) {
  	return fgLibraryStatus;
  }
  
  if (fFctDeleteReader!=NULL) {
    ((AliHLTHOMERReaderDelete_t)fFctDeleteReader)(pReader);
  }
  
  return 0;
}

AliHLTHOMERWriter* AliHLTHOMERLibManager::OpenWriter()
{
  // open a Writer instance
  if (fgLibraryStatus<0) return NULL;

  fgLibraryStatus=LoadHOMERLibrary();
  if (fgLibraryStatus <= 0) {
	return NULL;
  }
  
  AliHLTHOMERWriter* pWriter=NULL;
  if (fFctCreateWriter!=NULL && (pWriter=(((AliHLTHOMERWriterCreate_t)fFctCreateWriter)()))==NULL) {
//     HLTError("can not create instance of HOMER writer (function %p)", fFctCreateWriter);
  }
  
  return pWriter;
}

int AliHLTHOMERLibManager::DeleteWriter(AliHLTHOMERWriter* pWriter)
{
  // see header file for class documentation
  if (fgLibraryStatus<0) return fgLibraryStatus;

  fgLibraryStatus=LoadHOMERLibrary();
  if (fgLibraryStatus <= 0) {
	return fgLibraryStatus;
  }
  
  if (fFctDeleteWriter!=NULL) {
    ((AliHLTHOMERWriterDelete_t)fFctDeleteWriter)(pWriter);
  }
  
  return 0;
}

int AliHLTHOMERLibManager::LoadHOMERLibrary()
{
  // delete a writer

  // the actual deletion function is inside the HOMER library
  int iResult=-EBADF;
  const char** library=&fgkLibraries[0];
  int* refcount = &fgkLibRefCount[0];
  do {
    TString libs = gSystem->GetLibraries();
    if (libs.Contains(*library) ||
	(gSystem->Load(*library)) >= 0) {
      ++(*refcount);
      fLoadedLib = *library;
      iResult=1;
      break;
    }
    ++library;
    ++refcount;
  } while ((*library)!=NULL);

  if (iResult>0 && *library!=NULL) {
    // print compile info
    typedef void (*CompileInfo)( char*& date, char*& time);

    fFctCreateReaderFromTCPPort=(void (*)())gSystem->DynFindSymbol(*library, ALIHLTHOMERREADER_CREATE_FROM_TCPPORT);
    fFctCreateReaderFromTCPPorts=(void (*)())gSystem->DynFindSymbol(*library, ALIHLTHOMERREADER_CREATE_FROM_TCPPORTS);
    fFctCreateReaderFromBuffer=(void (*)())gSystem->DynFindSymbol(*library, ALIHLTHOMERREADER_CREATE_FROM_BUFFER);
    fFctDeleteReader=(void (*)())gSystem->DynFindSymbol(*library, ALIHLTHOMERREADER_DELETE);
    fFctCreateWriter=(void (*)())gSystem->DynFindSymbol(*library, ALIHLTHOMERWRITER_CREATE);
    fFctDeleteWriter=(void (*)())gSystem->DynFindSymbol(*library, ALIHLTHOMERWRITER_DELETE);
    if (fFctCreateReaderFromTCPPort==NULL ||
	fFctCreateReaderFromTCPPorts==NULL ||
	fFctCreateReaderFromBuffer==NULL || 
	fFctDeleteReader==NULL ||
	fFctCreateWriter==NULL ||
	fFctDeleteWriter==NULL) {
      iResult=-ENOSYS;
    } else {
    }
  }
  if (iResult<0 || *library==NULL) {
    fFctCreateReaderFromTCPPort=NULL;
    fFctCreateReaderFromTCPPorts=NULL;
    fFctCreateReaderFromBuffer=NULL;
    fFctDeleteReader=NULL;
    fFctCreateWriter=NULL;
    fFctDeleteWriter=NULL;
  }

  return iResult;
}

int AliHLTHOMERLibManager::UnloadHOMERLibrary()
{
  // unload HOMER library
  int iResult=0;
  
  if (fLoadedLib != NULL)
  {
    // Find the corresponding reference count.
    const char** library=&fgkLibraries[0];
    int* refcount = &fgkLibRefCount[0];
    while (*library != NULL)
    {
      if (strcmp(*library, fLoadedLib) == 0) break;
      ++library;
      ++refcount;
    }
    
    // Decrease the reference count and remove the library if it is zero.
    if (*refcount >= 0) --(*refcount);
    if (*refcount == 0)
    {
      // Check that the library we are trying to unload is actually the last library
      // in the gSystem->GetLibraries() list. If not then we must abort the removal.
      // This is because of a ROOT bug/feature/limitation. If we try unload the library
      // then ROOT will also wipe all libraries in the gSystem->GetLibraries() list
      // following the library we want to unload.
      TString libstring = gSystem->GetLibraries();
      TString token, lastlib;
      Ssiz_t from = 0;
      Int_t numOfLibs = 0, posOfLib = -1;
      while (libstring.Tokenize(token, from, " "))
      {
        ++numOfLibs;
        lastlib = token;
        if (token.Contains(fLoadedLib)) posOfLib = numOfLibs;
      }
      if (numOfLibs == posOfLib)
      {
        gSystem->Unload(fLoadedLib);

        // Check that the library is gone, since Unload() does not return a status code.
        libstring = gSystem->GetLibraries();
        if (libstring.Contains(fLoadedLib)) iResult = -EBADF;
      }
      else
      {
        AliHLTLogging log;
        log.LoggingVarargs(kHLTLogWarning, Class_Name(), FUNCTIONNAME(), __FILE__, __LINE__,
          Form("ROOT limitation! Cannot properly cleanup and unload the shared"
            " library '%s' since another library '%s' was loaded afterwards. Trying to"
            " unload this library will remove the others and lead to serious memory faults.",
            fLoadedLib, lastlib.Data()
        ));
      }
    }
  }

  // Clear the function pointers.
  fFctCreateReaderFromTCPPort = NULL;
  fFctCreateReaderFromTCPPorts = NULL;
  fFctCreateReaderFromBuffer = NULL;
  fFctDeleteReader = NULL;
  fFctCreateWriter = NULL;
  fFctDeleteWriter = NULL;

  return iResult;
}
