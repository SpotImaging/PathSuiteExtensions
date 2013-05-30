#pragma once

#include "HostVariables.h"

class Case
{
public:
};


class MGR
{

public:
    // Readable variables
    static bool CameraCanRotate()                       { return HostInterop::GetBoolVariable("MGR_bCameraCanRotate");}
    static bool CameraCanZoom()                         { return HostInterop::GetBoolVariable("MGR_bCameraCanZoom");}
    static bool CameraHasMultObjectives()               { return HostInterop::GetBoolVariable("MGR_bCameraHasMultObjectives");}
    static bool CameraInited()                          { return HostInterop::GetBoolVariable("MGR_bCameraInited");}
    static bool CaseLoggedIn()                          { return HostInterop::GetBoolVariable("MGR_bCaseLoggedIn");}
    static bool ImageOpen()                             { return HostInterop::GetBoolVariable("MGR_bImageOpen");}
    static bool PreselectPrefix()                       { return HostInterop::GetBoolVariable("MGR_bPreselectPrefix");}
    static bool SetupMain_OKpushed()                    { return HostInterop::GetBoolVariable("MGR_bSetupMain_OKpushed");}
    static bool CaseLogin_OKpushed()                    { return HostInterop::GetBoolVariable("MGR_bCaseLogin_OKpushed");}
    static bool ShowingLiveVideoUpper()                 { return HostInterop::GetBoolVariable("MGR_bShowingLiveVideoUpper");}
    static bool ShowingObjectivesPane()                 { return HostInterop::GetBoolVariable("MGR_bShowingObjectivesPane");}
    static bool ShowingDocModeUpper()                   { return HostInterop::GetBoolVariable("MGR_bShowingDocModeUpper");}
    static bool ShowingThumbstrip()                     { return HostInterop::GetBoolVariable("MGR_bShowingThumbstrip");}
    static bool TryForImageOpen()                       { return HostInterop::GetBoolVariable("MGR_bTryForImageOpen");}
    static bool UseDefaultSettings()                    { return HostInterop::GetBoolVariable("MGR_bUseDefaultSettings");}
    static int  OpenImage()                             { return (int)HostInterop::GetNumericVariable("MGR_idOpenImage");}
    static int  SpcmnDropListLock()                     { return (int)HostInterop::GetNumericVariable("MGR_iSpcmnDropListLock");}
    static std::string BlockLabel()                     { return HostInterop::GetTextVariable("MGR_strBlockLabel");}
    static std::string LastPrefixUsed()                 { return HostInterop::GetTextVariable("MGR_strLastPrefixUsed");}
    static std::string LiveVideoDialog()                { return HostInterop::GetTextVariable("MGR_strLiveVideoDialog");}
    static std::string SectionLabel()                   { return HostInterop::GetTextVariable("MGR_strSectionLabel");}
    static std::string CalibUnits()                     { return HostInterop::GetTextVariable("MGR_strCalibUnits");}
    static std::string SpcmnDropListBinding()           { return HostInterop::GetTextVariable("MGR_strSpcmnDropListBinding");}
    static std::string MasterCatalogFolder()            { return HostInterop::GetTextVariable("MasterCatalogFolder");}
    static std::string PrefsFilePath()                  { return HostInterop::GetTextVariable("PrefsFilePath");}

    // Writable variables
    static void CameraCanRotate (bool value)                    { return HostInterop::SetBoolVariable ("MGR_bCameraCanRotate", value);}
    static void CameraCanZoom (bool value)                      { return HostInterop::SetBoolVariable ("MGR_bCameraCanZoom", value);}
    static void CameraHasMultObjectives (bool value)            { return HostInterop::SetBoolVariable ("MGR_bCameraHasMultObjectives", value);}
    static void CameraInited (bool value)                       { return HostInterop::SetBoolVariable ("MGR_bCameraInited", value);}
    static void CaseLoggedIn (bool value)                       { return HostInterop::SetBoolVariable ("MGR_bCaseLoggedIn", value);}
    static void ImageOpen (bool value)                          { return HostInterop::SetBoolVariable ("MGR_bImageOpen", value);}
    static void PreselectPrefix (bool value)                    { return HostInterop::SetBoolVariable ("MGR_bPreselectPrefix", value);}
    static void SetupMain_OKpushed (bool value)                 { return HostInterop::SetBoolVariable ("MGR_bSetupMain_OKpushed", value);}
    static void CaseLogin_OKpushed (bool value)                 { return HostInterop::SetBoolVariable ("MGR_bCaseLogin_OKpushed", value);}
    static void ShowingLiveVideoUpper (bool value)              { return HostInterop::SetBoolVariable ("MGR_bShowingLiveVideoUpper", value);}
    static void ShowingObjectivesPane (bool value)              { return HostInterop::SetBoolVariable ("MGR_bShowingObjectivesPane", value);}
    static void ShowingDocModeUpper (bool value)                { return HostInterop::SetBoolVariable ("MGR_bShowingDocModeUpper", value);}
    static void ShowingThumbstrip (bool value)                  { return HostInterop::SetBoolVariable ("MGR_bShowingThumbstrip", value);}
    static void TryForImageOpen (bool value)                    { return HostInterop::SetBoolVariable ("MGR_bTryForImageOpen", value);}
    static void UseDefaultSettings (bool value)                 { return HostInterop::SetBoolVariable ("MGR_bUseDefaultSettings", value);}
    static void OpenImage (int value)                           { return HostInterop::SetNumericVariable ("MGR_idOpenImage", value);}
    static void SpcmnDropListLock (int value)                   { return HostInterop::SetNumericVariable ("MGR_iSpcmnDropListLock", value);}
    static void BlockLabel (const std::string& value)           { return HostInterop::SetTextVariable ("MGR_strBlockLabel", value);}
    static void LastPrefixUsed (const std::string& value)       { return HostInterop::SetTextVariable ("MGR_strLastPrefixUsed", value);}
    static void LiveVideoDialog (const std::string& value)      { return HostInterop::SetTextVariable ("MGR_strLiveVideoDialog", value);}
    static void SectionLabel (const std::string& value)         { return HostInterop::SetTextVariable ("MGR_strSectionLabel", value);}
    static void CalibUnits (const std::string& value)           { return HostInterop::SetTextVariable ("MGR_strCalibUnits", value);}
    static void SpcmnDropListBinding (const std::string& value) { return HostInterop::SetTextVariable ("MGR_strSpcmnDropListBinding", value);}
    static void MasterCatalogFolder(const std::string& value)   { return HostInterop::SetTextVariable ("MasterCatalogFolder", value);}
};
