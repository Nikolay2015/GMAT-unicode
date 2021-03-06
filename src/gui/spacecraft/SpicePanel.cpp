//$Id: SpicePanel.cpp 9514 2011-04-30 21:44:00Z djcinsb $
//------------------------------------------------------------------------------
//                            SpicePanel
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under
// FDSS Task order 28.
//
// Author: Wendy C. Shoan
// Created: 2010.05.03
//
//
/**
 * This is the panel for the Spice tab on the notebook on the Spacecraft
 * Panel.
 *
 */
//------------------------------------------------------------------------------

#include "SpicePanel.hpp"
#include "SolarSystem.hpp"
#include "GmatBaseException.hpp"
#include "GmatAppData.hpp"
#include "GmatStaticBoxSizer.hpp"
#include "MessageInterface.hpp"
#include "StringUtil.hpp"
//#include "bitmaps/OpenFolder.xpm"
#include <wx/config.h>
#include <fstream>

//#define DEBUG_SPICE_PANEL

// event tables for wxMac/Widgets
BEGIN_EVENT_TABLE(SpicePanel, wxPanel)
   EVT_BUTTON(ID_BROWSE_BUTTON_SPK_FILE, SpicePanel::OnSpkFileBrowseButton)
   EVT_BUTTON(ID_REMOVE_BUTTON_SPK_FILE, SpicePanel::OnSpkFileRemoveButton)
   EVT_LISTBOX(ID_LIST_BOX_SPK_FILE, SpicePanel::OnSpkFileListBoxChange)
   EVT_BUTTON(ID_BROWSE_BUTTON_CK_FILE, SpicePanel::OnCkFileBrowseButton)
   EVT_BUTTON(ID_REMOVE_BUTTON_CK_FILE, SpicePanel::OnCkFileRemoveButton)
   EVT_LISTBOX(ID_LIST_BOX_CK_FILE, SpicePanel::OnCkFileListBoxChange)
   EVT_BUTTON(ID_BROWSE_BUTTON_SCLK_FILE, SpicePanel::OnSclkFileBrowseButton)
   EVT_BUTTON(ID_REMOVE_BUTTON_SCLK_FILE, SpicePanel::OnSclkFileRemoveButton)
   EVT_LISTBOX(ID_LIST_BOX_SCLK_FILE, SpicePanel::OnSclkFileListBoxChange)
   EVT_BUTTON(ID_BROWSE_BUTTON_FK_FILE, SpicePanel::OnFkFileBrowseButton)
   EVT_BUTTON(ID_REMOVE_BUTTON_FK_FILE, SpicePanel::OnFkFileRemoveButton)
   EVT_LISTBOX(ID_LIST_BOX_FK_FILE, SpicePanel::OnFkFileListBoxChange)
//   EVT_BUTTON(ID_BROWSE_BUTTON_PCK_FILE, SpicePanel::OnPckFileBrowseButton)
//   EVT_BUTTON(ID_REMOVE_BUTTON_PCK_FILE, SpicePanel::OnPckFileRemoveButton)
//   EVT_LISTBOX(ID_LIST_BOX_PCK_FILE, SpicePanel::OnPckFileListBoxChange)
   EVT_TEXT(ID_TEXT_CTRL_NAIF_ID, SpicePanel::OnNaifIdTextCtrlChange)
   EVT_TEXT(ID_TEXT_CTRL_NAIF_ID_REF_FRAME, SpicePanel::OnNaifIdRefTextCtrlChange)
END_EVENT_TABLE()

//------------------------------------------------------------------------------
// public methods
//------------------------------------------------------------------------------

SpicePanel::SpicePanel(GmatPanel *scPanel,
                       wxWindow *parent, Spacecraft *theSC) :
   wxPanel          (parent),
   naifId           (-1234567890),
   dataChanged      (false),
   canClose         (true),
   theSpacecraft    (theSC),
   spkNameChanged   (false),
   ckNameChanged    (false),
   sclkNameChanged  (false),
   fkNameChanged    (false),
//   pckNameChanged   (false),
   naifIdChanged    (false),
   naifIdRefFrameChanged (false),
   spkFilesDeleted  (false),
   ckFilesDeleted   (false),
   sclkFilesDeleted (false),
   fkFilesDeleted   (false),
//   pckFilesDeleted  (false),
   theScPanel       (scPanel)
{
   #ifdef DEBUG_SPICE_PANEL
      MessageInterface::ShowMessage(wxT("Entering SpicePanel Constructor, with theSC = %s (%s)\n"),
            (theSC->GetName()).c_str(), (theSC->GetTypeName()).c_str());
   #endif
   guiManager     = GuiItemManager::GetInstance();
   guiInterpreter = GmatAppData::Instance()->GetGuiInterpreter();

#ifdef __USE_SPICE__
   Create();
#else
   ; // throw an exception here?
#endif

}

SpicePanel::~SpicePanel()
{

}

void SpicePanel::SaveData()
{
   wxString strval;
//   Real        tmpval;
   Integer     tmpint;
   bool        retval;

   // don't do anything if no data has been changed.
   // note that dataChanged will be true if the user modified any combo box or
   // text ctrl, whether or not he/she actually changed the value; we want to only
   // send the values to the object if something was really changed, to avoid
   // the hasBeenModified flag being set to true erroneously
   canClose    = true;

   try
   {
      unsigned int numKernels = 0;
      if (spkNameChanged)
      {
         #ifdef DEBUG_SPICE_PANEL
            MessageInterface::ShowMessage(wxT("spkNameChanged is true : %s\n"),
                  strval.c_str());
         #endif

         numKernels = spkFileListBox->GetCount();
         for (unsigned int ii = 0; ii < numKernels; ii++)
         {
            strval = spkFileListBox->GetString(ii);
            std::ifstream filename(strval.char_str());

            if (!filename)
            {
               wxString errmsg = wxT("File \"") + strval;
               errmsg += wxT("\" does not exist.\n");
               MessageInterface::PopupMessage(Gmat::ERROR_, errmsg);
               canClose = false;
            }
            else
            {
               filename.close();
               theSpacecraft->SetStringParameter(theSpacecraft->GetParameterID(wxT("OrbitSpiceKernelName")),
                     strval);
            }
         }
      }
      if (ckNameChanged)
      {
         #ifdef DEBUG_SPICE_PANEL
            MessageInterface::ShowMessage(wxT("ckNameChanged is true : %s\n"),
                  strval.c_str());
         #endif

         numKernels = ckFileListBox->GetCount();
         for (unsigned int ii = 0; ii < numKernels; ii++)
         {
            strval = ckFileListBox->GetString(ii);
            std::ifstream filename(strval.char_str());

            if (!filename)
            {
               wxString errmsg = wxT("File \"") + strval;
               errmsg += wxT("\" does not exist.\n");
               MessageInterface::PopupMessage(Gmat::ERROR_, errmsg);
               canClose = false;
            }
            else
            {
               filename.close();
               theSpacecraft->SetStringParameter(theSpacecraft->GetParameterID(wxT("AttitudeSpiceKernelName")),
                     strval);
            }
         }
      }
      if (sclkNameChanged)
      {
         #ifdef DEBUG_SPICE_PANEL
            MessageInterface::ShowMessage(wxT("sclkNameChanged is true : %s\n"),
                  strval.c_str());
         #endif

         numKernels = sclkFileListBox->GetCount();
         for (unsigned int ii = 0; ii < numKernels; ii++)
         {
            strval = sclkFileListBox->GetString(ii);
            std::ifstream filename(strval.char_str());

            if (!filename)
            {
               wxString errmsg = wxT("File \"") + strval;
               errmsg += wxT("\" does not exist.\n");
               MessageInterface::PopupMessage(Gmat::ERROR_, errmsg);
               canClose = false;
            }
            else
            {
               filename.close();
               theSpacecraft->SetStringParameter(theSpacecraft->GetParameterID(wxT("SCClockSpiceKernelName")),
                     strval);
            }
         }
      }
      if (fkNameChanged)
      {
         #ifdef DEBUG_SPICE_PANEL
            MessageInterface::ShowMessage(wxT("fkNameChanged is true : %s\n"),
                  strval.c_str());
         #endif

         numKernels = fkFileListBox->GetCount();
         for (unsigned int ii = 0; ii < numKernels; ii++)
         {
            strval = fkFileListBox->GetString(ii);
            std::ifstream filename(strval.char_str());

            if (!filename)
            {
               wxString errmsg = wxT("File \"") + strval;
               errmsg += wxT("\" does not exist.\n");
               MessageInterface::PopupMessage(Gmat::ERROR_, errmsg);
               canClose = false;
            }
            else
            {
               filename.close();
               theSpacecraft->SetStringParameter(theSpacecraft->GetParameterID(wxT("FrameSpiceKernelName")),
                     strval);
            }
         }
      }
      if (naifIdChanged)
      {
         strval = naifIdTextCtrl->GetValue();
         retval = theScPanel->CheckInteger(tmpint, strval, wxT("NAIF ID"), wxT("Integer Number"));
         canClose = retval;
         if (retval)
         {
            theSpacecraft->SetIntegerParameter(theSpacecraft->GetParameterID(wxT("NAIFId")), tmpint);
         }
      }
      if (naifIdRefFrameChanged)
      {
         strval = naifIdRefFrameTextCtrl->GetValue();
         retval = theScPanel->CheckInteger(tmpint, strval, wxT(" Reference Frame NAIF ID"), wxT("Integer Number"));
         canClose = retval;
         if (retval)
         {
            theSpacecraft->SetIntegerParameter(theSpacecraft->GetParameterID(wxT("NAIFIdReferenceFrame")), tmpint);
         }
      }
      if (spkFilesDeleted)
      {
         for (unsigned int ii = 0; ii < spkFilesToDelete.size(); ii++)
         {
            theSpacecraft->RemoveSpiceKernelName(wxT("Orbit"), spkFilesToDelete.at(ii));
         }
      }
      if (ckFilesDeleted)
      {
         for (unsigned int ii = 0; ii < ckFilesToDelete.size(); ii++)
         {
            theSpacecraft->RemoveSpiceKernelName(wxT("Attitude"), ckFilesToDelete.at(ii));
         }
      }
      if (sclkFilesDeleted)
      {
         for (unsigned int ii = 0; ii < sclkFilesToDelete.size(); ii++)
         {
            theSpacecraft->RemoveSpiceKernelName(wxT("SCClock"), sclkFilesToDelete.at(ii));
         }
      }
      if (fkFilesDeleted)
      {
         for (unsigned int ii = 0; ii < fkFilesToDelete.size(); ii++)
         {
            theSpacecraft->RemoveSpiceKernelName(wxT("Frame"), fkFilesToDelete.at(ii));
         }
      }
   }
   catch (BaseException &ex)
   {
      canClose = false;
      dataChanged = true;
      MessageInterface::PopupMessage(Gmat::ERROR_, ex.GetFullMessage());
   }

   #ifdef DEBUG_SPICE_ORBIT_PANEL
      MessageInterface::ShowMessage(wxT("at end of SaveData, canClose = %s\n"),
            (canClose? wxT("true") : wxT("false")));
   #endif
   if (canClose)
   {
      dataChanged = false;
      ResetChangeFlags(true);
   }
}

void SpicePanel::LoadData()
{
   try
   {
      naifId         = theSpacecraft->GetIntegerParameter(theSpacecraft->GetParameterID(wxT("NAIFId")));
      naifIdStringWX = guiManager->ToWxString(naifId);
      naifIdTextCtrl->SetValue(naifIdStringWX);

      naifIdRefFrame         = theSpacecraft->GetIntegerParameter(theSpacecraft->GetParameterID(wxT("NAIFIdReferenceFrame")));
      naifIdRefFrameStringWX = guiManager->ToWxString(naifIdRefFrame);
      naifIdRefFrameTextCtrl->SetValue(naifIdRefFrameStringWX);

      spkKernelNames           = theSpacecraft->GetStringArrayParameter(
                                 theSpacecraft->GetParameterID(wxT("OrbitSpiceKernelName")));
      unsigned int spkListSz   = spkKernelNames.size();
      spkFileArrayWX           = new wxString[spkListSz];
      for (unsigned int jj = 0; jj < spkListSz; jj++)
      {
         spkFileArrayWX[jj] = spkKernelNames[jj].c_str();
      }
      spkFileListBox->InsertItems(spkListSz, spkFileArrayWX, 0);
      spkFileListBox->SetSelection(spkListSz-1); // Select the last item

      fkKernelNames           = theSpacecraft->GetStringArrayParameter(
                                theSpacecraft->GetParameterID(wxT("FrameSpiceKernelName")));
      unsigned int fkListSz   = fkKernelNames.size();
      fkFileArrayWX           = new wxString[fkListSz];
      for (unsigned int jj = 0; jj < fkListSz; jj++)
      {
         fkFileArrayWX[jj] = fkKernelNames[jj].c_str();
      }
      fkFileListBox->InsertItems(fkListSz, fkFileArrayWX, 0);
      fkFileListBox->SetSelection(fkListSz-1); // Select the last item

      ckKernelNames           = theSpacecraft->GetStringArrayParameter(
                                theSpacecraft->GetParameterID(wxT("AttitudeSpiceKernelName")));
      unsigned int ckListSz   = ckKernelNames.size();
      ckFileArrayWX           = new wxString[ckListSz];
      for (unsigned int jj = 0; jj < ckListSz; jj++)
      {
         ckFileArrayWX[jj] = ckKernelNames[jj].c_str();
      }
      ckFileListBox->InsertItems(ckListSz, ckFileArrayWX, 0);
      ckFileListBox->SetSelection(ckListSz-1); // Select the last item

      sclkKernelNames           = theSpacecraft->GetStringArrayParameter(
                                theSpacecraft->GetParameterID(wxT("SCClockSpiceKernelName")));
      unsigned int sclkListSz   = sclkKernelNames.size();
      sclkFileArrayWX           = new wxString[sclkListSz];
      for (unsigned int jj = 0; jj < sclkListSz; jj++)
      {
         sclkFileArrayWX[jj] = sclkKernelNames[jj].c_str();
      }
      sclkFileListBox->InsertItems(sclkListSz, sclkFileArrayWX, 0);
      sclkFileListBox->SetSelection(sclkListSz-1); // Select the last item

      ResetChangeFlags();

   }
   catch (BaseException &e)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
   }
}


//------------------------------------------------------------------------------
// private methods
//------------------------------------------------------------------------------
void SpicePanel::Create()
{
   int bSize     = 2;
//   #if __WXMAC__
//   int buttonWidth = 40;
//   #else
//   int buttonWidth = 25;
//   #endif

//   wxBitmap openBitmap = wxBitmap(OpenFolder_xpm);

   // get the config object
   wxConfigBase *pConfig = wxConfigBase::Get();
   // SetPath() understands ".."
   pConfig->SetPath(wxT("/Spacecraft Spice"));

   // empty the temporary value strings
   naifIdStringWX             = wxT("");
   naifIdRefFrameStringWX     = wxT("");
   spkFileStringWX            = wxT("");
   ckFileStringWX             = wxT("");
   sclkFileStringWX           = wxT("");
   fkFileStringWX             = wxT("");

   wxBoxSizer *spkButtonSizer  = NULL;
   wxBoxSizer *ckButtonSizer   = NULL;
   wxBoxSizer *sclkButtonSizer = NULL;
   wxBoxSizer *fkButtonSizer   = NULL;

   // naif ID for spacecraft
   naifIdStaticText   = new wxStaticText(this, ID_TEXT,wxT(GUI_ACCEL_KEY) wxT("NAIF ID"),
                        wxDefaultPosition, wxSize(-1,-1), 0);
   naifIdTextCtrl     = new wxTextCtrl(this, ID_TEXT_CTRL_NAIF_ID, wxT(""),
                        wxDefaultPosition, wxSize(80, -1), 0);
   naifIdTextCtrl->SetToolTip(pConfig->Read(wxT("NAIFIDHint")));
   naifIdBlankText    = new wxStaticText(this, ID_TEXT,wxT(""),
                        wxDefaultPosition, wxSize(-1,-1), 0);
   // naif ID of the reference frame for the spacecraft
   naifIdRefFrameStaticText   = new wxStaticText(this, ID_TEXT,wxT("F") wxT(GUI_ACCEL_KEY) wxT("rame\nNAIF ID"),
                        wxDefaultPosition, wxSize(-1,-1), 0);
   naifIdRefFrameTextCtrl     = new wxTextCtrl(this, ID_TEXT_CTRL_NAIF_ID_REF_FRAME, wxT(""),
                        wxDefaultPosition, wxSize(80, -1), 0);
   naifIdRefFrameTextCtrl->SetToolTip(pConfig->Read(wxT("NAIFIDRefFrameHint")));
   naifIdRefFrameBlankText    = new wxStaticText(this, ID_TEXT,wxT(""),
                        wxDefaultPosition, wxSize(-1,-1), 0);
   // SPK file(s)
   wxArrayString emptyList;
   spkFileStaticText   = new wxStaticText(this, ID_TEXT, wxT(GUI_ACCEL_KEY) wxT("SPK Files"),
                         wxDefaultPosition, wxSize(-1,-1), 0);
   spkFileListBox      = new wxListBox(this, ID_LIST_BOX_SPK_FILE, wxDefaultPosition, wxSize(150, 100),
                         emptyList, wxLB_EXTENDED|wxLB_NEEDED_SB|wxLB_HSCROLL);
   spkFileListBox->SetToolTip(pConfig->Read(wxT("SPKFileListHint")));
   spkFileBrowseButton = new wxButton(this, ID_BROWSE_BUTTON_SPK_FILE, wxT(GUI_ACCEL_KEY) wxT("Add"),
                         wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
   spkFileBrowseButton->SetToolTip(pConfig->Read(wxT("AddSPKFileHint")));
   spkFileRemoveButton = new wxButton(this, ID_REMOVE_BUTTON_SPK_FILE, wxT(GUI_ACCEL_KEY) wxT("Remove"),
                         wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
   spkFileRemoveButton->SetToolTip(pConfig->Read(wxT("RemoveSPKFileHint")));
   spkButtonSizer = new wxBoxSizer(wxHORIZONTAL);
   spkButtonSizer->Add(spkFileBrowseButton,0, wxGROW|wxALIGN_CENTRE|wxALL, bSize);
   spkButtonSizer->Add(spkFileRemoveButton,0, wxGROW|wxALIGN_CENTRE|wxALL, bSize);

   // FK file(s)
   fkFileStaticText   = new wxStaticText(this, ID_TEXT, wxT(GUI_ACCEL_KEY) wxT("FK Files"),
                         wxDefaultPosition, wxSize(-1,-1), 0);
   fkFileListBox      = new wxListBox(this, ID_LIST_BOX_FK_FILE, wxDefaultPosition, wxSize(150, 100),
                         emptyList, wxLB_EXTENDED|wxLB_NEEDED_SB|wxLB_HSCROLL);
   fkFileListBox->SetToolTip(pConfig->Read(wxT("FKFileListHint")));
   fkFileBrowseButton = new wxButton(this, ID_BROWSE_BUTTON_FK_FILE, wxT(GUI_ACCEL_KEY) wxT("Add"),
                         wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
   fkFileBrowseButton->SetToolTip(pConfig->Read(wxT("AddFKFileHint")));
   fkFileRemoveButton = new wxButton(this, ID_REMOVE_BUTTON_FK_FILE, wxT(GUI_ACCEL_KEY) wxT("Remove"),
                         wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
   fkFileRemoveButton->SetToolTip(pConfig->Read(wxT("RemoveFKFileHint")));
   fkButtonSizer = new wxBoxSizer(wxHORIZONTAL);
   fkButtonSizer->Add(fkFileBrowseButton,0, wxGROW|wxALIGN_CENTRE|wxALL, bSize);
   fkButtonSizer->Add(fkFileRemoveButton,0, wxGROW|wxALIGN_CENTRE|wxALL, bSize);

   // CK file(s)
   ckFileStaticText   = new wxStaticText(this, ID_TEXT, wxT(GUI_ACCEL_KEY) wxT("CK Files"),
                         wxDefaultPosition, wxSize(-1,-1), 0);
   ckFileListBox      = new wxListBox(this, ID_LIST_BOX_CK_FILE, wxDefaultPosition, wxSize(150, 100),
                         emptyList, wxLB_EXTENDED|wxLB_NEEDED_SB|wxLB_HSCROLL);
   ckFileListBox->SetToolTip(pConfig->Read(wxT("CKFileListHint")));
   ckFileBrowseButton = new wxButton(this, ID_BROWSE_BUTTON_CK_FILE, wxT(GUI_ACCEL_KEY) wxT("Add"),
                         wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
   ckFileBrowseButton->SetToolTip(pConfig->Read(wxT("AddCKFileHint")));
   ckFileRemoveButton = new wxButton(this, ID_REMOVE_BUTTON_CK_FILE, wxT(GUI_ACCEL_KEY) wxT("Remove"),
                         wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
   ckFileRemoveButton->SetToolTip(pConfig->Read(wxT("RemoveCKFileHint")));
   ckButtonSizer = new wxBoxSizer(wxHORIZONTAL);
   ckButtonSizer->Add(ckFileBrowseButton,0, wxGROW|wxALIGN_CENTRE|wxALL, bSize);
   ckButtonSizer->Add(ckFileRemoveButton,0, wxGROW|wxALIGN_CENTRE|wxALL, bSize);

   // SCLK file(s)
   sclkFileStaticText   = new wxStaticText(this, ID_TEXT, wxT("SC") wxT(GUI_ACCEL_KEY) wxT("LK Files"),
                         wxDefaultPosition, wxSize(-1,-1), 0);
   sclkFileListBox      = new wxListBox(this, ID_LIST_BOX_SCLK_FILE, wxDefaultPosition, wxSize(150, 100),
                         emptyList, wxLB_EXTENDED|wxLB_NEEDED_SB|wxLB_HSCROLL);
   sclkFileListBox->SetToolTip(pConfig->Read(wxT("SCLKFileListHint")));
   sclkFileBrowseButton = new wxButton(this, ID_BROWSE_BUTTON_SCLK_FILE, wxT(GUI_ACCEL_KEY) wxT("Add"),
                         wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
   sclkFileBrowseButton->SetToolTip(pConfig->Read(wxT("AddSCLKFileHint")));
   sclkFileRemoveButton = new wxButton(this, ID_REMOVE_BUTTON_SCLK_FILE, wxT(GUI_ACCEL_KEY) wxT("Remove"),
                         wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
   sclkFileRemoveButton->SetToolTip(pConfig->Read(wxT("RemoveSCLKFileHint")));
   sclkButtonSizer = new wxBoxSizer(wxHORIZONTAL);
   sclkButtonSizer->Add(sclkFileBrowseButton,0, wxGROW|wxALIGN_CENTRE|wxALL, bSize);
   sclkButtonSizer->Add(sclkFileRemoveButton,0, wxGROW|wxALIGN_CENTRE|wxALL, bSize);

   // sizers

   leftFlexGridSizer = new wxFlexGridSizer(2,0,0);
   leftFlexGridSizer->Add(naifIdStaticText,0, wxGROW|wxALIGN_LEFT|wxALL, bSize);
   leftFlexGridSizer->Add(naifIdTextCtrl,0, wxGROW|wxALIGN_LEFT|wxALL, bSize);
//   leftFlexGridSizer->Add(0, 0);

   leftFlexGridSizer->Add(spkFileStaticText,0, wxGROW|wxALIGN_LEFT|wxALL, bSize);
   leftFlexGridSizer->Add(spkFileListBox,0, wxGROW|wxALIGN_LEFT|wxALL, bSize);
//   leftFlexGridSizer->Add(0, 0);

   leftFlexGridSizer->Add(0, 0);
   leftFlexGridSizer->Add(spkButtonSizer, 0, wxALIGN_CENTRE|wxALL, bSize);
//   leftFlexGridSizer->Add(0, 0);

   leftFlexGridSizer->Add(fkFileStaticText,0, wxGROW|wxALIGN_LEFT|wxALL, bSize);
   leftFlexGridSizer->Add(fkFileListBox,0, wxGROW|wxALIGN_LEFT|wxALL, bSize);
//   leftFlexGridSizer->Add(0, 0);

   leftFlexGridSizer->Add(0, 0);
   leftFlexGridSizer->Add(fkButtonSizer, 0, wxALIGN_CENTRE|wxALL, bSize);
//   leftFlexGridSizer->Add(0, 0);


   rightFlexGridSizer = new wxFlexGridSizer(2,0,0);
   rightFlexGridSizer->Add(naifIdRefFrameStaticText,0, wxGROW|wxALIGN_LEFT|wxALL, bSize);
   rightFlexGridSizer->Add(naifIdRefFrameTextCtrl,0, wxGROW|wxALIGN_LEFT|wxALL, bSize);
//   rightFlexGridSizer->Add(0, 0);

   rightFlexGridSizer->Add(ckFileStaticText,0, wxGROW|wxALIGN_LEFT|wxALL, bSize);
   rightFlexGridSizer->Add(ckFileListBox,0, wxGROW|wxALIGN_LEFT|wxALL, bSize);
//   rightFlexGridSizer->Add(0, 0);

   rightFlexGridSizer->Add(0, 0);
   rightFlexGridSizer->Add(ckButtonSizer, 0, wxALIGN_CENTRE|wxALL, bSize);
//   rightFlexGridSizer->Add(0, 0);

   rightFlexGridSizer->Add(sclkFileStaticText,0, wxGROW|wxALIGN_LEFT|wxALL, bSize);
   rightFlexGridSizer->Add(sclkFileListBox,0, wxGROW|wxALIGN_LEFT|wxALL, bSize);
//   rightFlexGridSizer->Add(0, 0);

   rightFlexGridSizer->Add(0, 0);
   rightFlexGridSizer->Add(sclkButtonSizer, 0, wxALIGN_CENTRE|wxALL, bSize);
//   rightFlexGridSizer->Add(0, 0);
   
   mainBoxSizer = new wxBoxSizer(wxHORIZONTAL);
   
   GmatStaticBoxSizer  *boxSizer1 = new GmatStaticBoxSizer(wxVERTICAL, this, wxT("") );
   boxSizer1->Add(leftFlexGridSizer, 0, wxALIGN_CENTRE|wxALL, bSize);
   
   GmatStaticBoxSizer  *boxSizer2 = new GmatStaticBoxSizer(wxVERTICAL, this, wxT(""));
   boxSizer2->Add(rightFlexGridSizer, 0, wxALIGN_CENTRE|wxALL, bSize);
   
   mainBoxSizer->Add(boxSizer1, 1, wxGROW|wxALIGN_CENTRE|wxALL, bSize);
   mainBoxSizer->Add(boxSizer2, 1, wxGROW|wxALIGN_CENTRE|wxALL, bSize);
   
   // Added #if in case if we want to add another layer of lined box for consistency.
   // I like without another layer, too many lines, so set to 0 (LOJ: 2009.11.18) - from CelestialBody
   #if 0
   
   GmatStaticBoxSizer *mainStaticBoxSizer = new GmatStaticBoxSizer(wxVERTICAL, this, "");
   mainStaticBoxSizer->Add(mainBoxSizer, 1, wxGROW|wxALIGN_CENTRE|wxALL, bSize);
   
   this->SetAutoLayout(true);
   this->SetSizer(mainStaticBoxSizer);
   mainStaticBoxSizer->Fit(this);
   mainStaticBoxSizer->SetSizeHints(this);
   
   #else
   
   wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
   mainSizer->Add(mainBoxSizer, 1, wxGROW|wxALIGN_CENTRE|wxALL, bSize);
   
   this->SetAutoLayout(true);
   this->SetSizer(mainSizer);
   mainSizer->Fit(this);
   mainSizer->SetSizeHints(this);
   
   #endif

}

void SpicePanel::ResetChangeFlags(bool discardMods)
{
   spkNameChanged        = false;
   ckNameChanged         = false;
   sclkNameChanged       = false;
   fkNameChanged         = false;
   naifIdChanged         = false;
   naifIdRefFrameChanged = false;
   spkFilesDeleted       = false;
   ckFilesDeleted        = false;
   sclkFilesDeleted      = false;
   fkFilesDeleted       = false;

   if (discardMods)
   {
//      spkFileListBox->DiscardEdits();  // why doesn't this work??
//      ckFileListBox->DiscardEdits();  // why doesn't this work??
//      sclkFileListBox->DiscardEdits();  // why doesn't this work??
//      fkFileListBox->DiscardEdits();  // why doesn't this work??
      naifIdTextCtrl->DiscardEdits();
      naifIdRefFrameTextCtrl->DiscardEdits();
   }
}

//Event Handling
//------------------------------------------------------------------------------
// void OnSpkFileBrowseButton(wxCommandEvent &event)
//------------------------------------------------------------------------------
void SpicePanel::OnSpkFileBrowseButton(wxCommandEvent &event)
{
   wxArrayString oldFiles = spkFileListBox->GetStrings();
   wxFileDialog dialog(this, wxT("Choose a file to add"), wxT(""), wxT(""), wxT("*.*"));
   Integer foundAt = -99;
   if (dialog.ShowModal() == wxID_OK)
   {
      wxString fileName = (dialog.GetPath()).c_str();
      for (Integer ii = 0; ii < (Integer) oldFiles.GetCount(); ii++)
      {
         if (fileName.IsSameAs(oldFiles[ii]))
         {
            foundAt = ii;
            break;
         }
      }
      if (foundAt == -99) // not found, so it's new
      {
         // Deselect current selections first
         wxArrayInt selections;
         int numSelect = spkFileListBox->GetSelections(selections);
         for (int i = 0; i < numSelect; i++)
            spkFileListBox->Deselect(selections[i]);

         spkNameChanged   = true;
         dataChanged      = true;
         spkFileListBox->Append(fileName);
         spkFileListBox->SetStringSelection(fileName);
         theScPanel->EnableUpdate(true);
      }
   }
}

//------------------------------------------------------------------------------
// void OnSpkFileRemoveButton(wxCommandEvent &event)
//------------------------------------------------------------------------------
void SpicePanel::OnSpkFileRemoveButton(wxCommandEvent &event)
{
   wxArrayInt selections;
   int numSelect = spkFileListBox->GetSelections(selections);
   // get the string values and delete the selected names from the list
   wxString    fileSelected;
   for (int i = numSelect-1; i >= 0; i--)
   {
      fileSelected = spkFileListBox->GetString(selections[i]);
      spkFilesToDelete.push_back(fileSelected.c_str());
      spkFileListBox->Delete(selections[i]);
   }
   spkFilesDeleted = true;
   dataChanged     = true;
   theScPanel->EnableUpdate(true);

   // Select the last item
   unsigned int count = spkFileListBox->GetCount();
   if (count > 0)
      spkFileListBox->SetSelection(count -1);
}

//------------------------------------------------------------------------------
// void OnSpkFileListBoxChange(wxCommandEvent &event)
//------------------------------------------------------------------------------
void SpicePanel::OnSpkFileListBoxChange(wxCommandEvent &event)
{
   spkNameChanged = true;
   dataChanged    = true;
   theScPanel->EnableUpdate(true);
}
//------------------------------------------------------------------------------
// void OnCkFileBrowseButton(wxCommandEvent &event)
//------------------------------------------------------------------------------
void SpicePanel::OnCkFileBrowseButton(wxCommandEvent &event)
{
   wxArrayString oldFiles = ckFileListBox->GetStrings();
   wxFileDialog dialog(this, wxT("Choose a file to add"), wxT(""), wxT(""), wxT("*.*"));
   Integer foundAt = -99;
   if (dialog.ShowModal() == wxID_OK)
   {
      wxString fileName = (dialog.GetPath()).c_str();
      for (Integer ii = 0; ii < (Integer) oldFiles.GetCount(); ii++)
      {
         if (fileName.IsSameAs(oldFiles[ii]))
         {
            foundAt = ii;
            break;
         }
      }
      if (foundAt == -99) // not found, so it's new
      {
         // Deselect current selections first
         wxArrayInt selections;
         int numSelect = ckFileListBox->GetSelections(selections);
         for (int i = 0; i < numSelect; i++)
            ckFileListBox->Deselect(selections[i]);

         ckNameChanged    = true;
         dataChanged      = true;
         ckFileListBox->Append(fileName);
         ckFileListBox->SetStringSelection(fileName);
         theScPanel->EnableUpdate(true);
      }
   }
}

//------------------------------------------------------------------------------
// void OnCkFileRemoveButton(wxCommandEvent &event)
//------------------------------------------------------------------------------
void SpicePanel::OnCkFileRemoveButton(wxCommandEvent &event)
{
   wxArrayInt selections;
   int numSelect = ckFileListBox->GetSelections(selections);
   // get the string values and delete the selected names from the list
   wxString    fileSelected;
   for (int i = numSelect-1; i >= 0; i--)
   {
      fileSelected = ckFileListBox->GetString(selections[i]);
      ckFilesToDelete.push_back(fileSelected.c_str());
      ckFileListBox->Delete(selections[i]);
   }
   ckFilesDeleted  = true;
   dataChanged     = true;
   theScPanel->EnableUpdate(true);

   // Select the last item
   unsigned int count = ckFileListBox->GetCount();
   if (count > 0)
      ckFileListBox->SetSelection(count -1);
}

//------------------------------------------------------------------------------
// void OnCkFileListBoxChange(wxCommandEvent &event)
//------------------------------------------------------------------------------
void SpicePanel::OnCkFileListBoxChange(wxCommandEvent &event)
{
   ckNameChanged  = true;
   dataChanged    = true;
   theScPanel->EnableUpdate(true);
}

//------------------------------------------------------------------------------
// void OnSclkFileBrowseButton(wxCommandEvent &event)
//------------------------------------------------------------------------------
void SpicePanel::OnSclkFileBrowseButton(wxCommandEvent &event)
{
   wxArrayString oldFiles = sclkFileListBox->GetStrings();
   wxFileDialog dialog(this, wxT("Choose a file to add"), wxT(""), wxT(""), wxT("*.*"));
   Integer foundAt = -99;
   if (dialog.ShowModal() == wxID_OK)
   {
      wxString fileName = (dialog.GetPath()).c_str();
      for (Integer ii = 0; ii < (Integer) oldFiles.GetCount(); ii++)
      {
         if (fileName.IsSameAs(oldFiles[ii]))
         {
            foundAt = ii;
            break;
         }
      }
      if (foundAt == -99) // not found, so it's new
      {
         // Deselect current selections first
         wxArrayInt selections;
         int numSelect = sclkFileListBox->GetSelections(selections);
         for (int i = 0; i < numSelect; i++)
            sclkFileListBox->Deselect(selections[i]);

         sclkNameChanged  = true;
         dataChanged      = true;
         sclkFileListBox->Append(fileName);
         sclkFileListBox->SetStringSelection(fileName);
         theScPanel->EnableUpdate(true);
      }
   }
}

//------------------------------------------------------------------------------
// void OnSclkFileRemoveButton(wxCommandEvent &event)
//------------------------------------------------------------------------------
void SpicePanel::OnSclkFileRemoveButton(wxCommandEvent &event)
{
   wxArrayInt selections;
   int numSelect = sclkFileListBox->GetSelections(selections);
   // get the string values and delete the selected names from the list
   wxString    fileSelected;
   for (int i = numSelect-1; i >= 0; i--)
   {
      fileSelected = sclkFileListBox->GetString(selections[i]);
      sclkFilesToDelete.push_back(fileSelected.c_str());
      sclkFileListBox->Delete(selections[i]);
   }
   sclkFilesDeleted = true;
   dataChanged      = true;
   theScPanel->EnableUpdate(true);

   // Select the last item
   unsigned int count = sclkFileListBox->GetCount();
   if (count > 0)
      sclkFileListBox->SetSelection(count -1);
}

//------------------------------------------------------------------------------
// void OnSclkFileListBoxChange(wxCommandEvent &event)
//------------------------------------------------------------------------------
void SpicePanel::OnSclkFileListBoxChange(wxCommandEvent &event)
{
   sclkNameChanged = true;
   dataChanged     = true;
   theScPanel->EnableUpdate(true);
}

//------------------------------------------------------------------------------
// void OnFkFileBrowseButton(wxCommandEvent &event)
//------------------------------------------------------------------------------
void SpicePanel::OnFkFileBrowseButton(wxCommandEvent &event)
{
   wxArrayString oldFiles = fkFileListBox->GetStrings();
   wxFileDialog dialog(this, wxT("Choose a file to add"), wxT(""), wxT(""), wxT("*.*"));
   Integer foundAt = -99;
   if (dialog.ShowModal() == wxID_OK)
   {
      wxString fileName = (dialog.GetPath()).c_str();
      for (Integer ii = 0; ii < (Integer) oldFiles.GetCount(); ii++)
      {
         if (fileName.IsSameAs(oldFiles[ii]))
         {
            foundAt = ii;
            break;
         }
      }
      if (foundAt == -99) // not found, so it's new
      {
         // Deselect current selections first
         wxArrayInt selections;
         int numSelect = fkFileListBox->GetSelections(selections);
         for (int i = 0; i < numSelect; i++)
            fkFileListBox->Deselect(selections[i]);

         fkNameChanged    = true;
         dataChanged      = true;
         fkFileListBox->Append(fileName);
         fkFileListBox->SetStringSelection(fileName);
         theScPanel->EnableUpdate(true);
      }
   }
}

//------------------------------------------------------------------------------
// void OnFkFileRemoveButton(wxCommandEvent &event)
//------------------------------------------------------------------------------
void SpicePanel::OnFkFileRemoveButton(wxCommandEvent &event)
{
   wxArrayInt selections;
   int numSelect = fkFileListBox->GetSelections(selections);
   // get the string values and delete the selected names from the list
   wxString    fileSelected;
   for (int i = numSelect-1; i >= 0; i--)
   {
      fileSelected = fkFileListBox->GetString(selections[i]);
      fkFilesToDelete.push_back(fileSelected.c_str());
      fkFileListBox->Delete(selections[i]);
   }
   fkFilesDeleted  = true;
   dataChanged     = true;
   theScPanel->EnableUpdate(true);

   // Select the last item
   unsigned int count = fkFileListBox->GetCount();
   if (count > 0)
      fkFileListBox->SetSelection(count -1);
}

//------------------------------------------------------------------------------
// void OnFkFileListBoxChange(wxCommandEvent &event)
//------------------------------------------------------------------------------
void SpicePanel::OnFkFileListBoxChange(wxCommandEvent &event)
{
   fkNameChanged  = true;
   dataChanged    = true;
   theScPanel->EnableUpdate(true);
}

//------------------------------------------------------------------------------
// void OnNaifIdTextCtrlChange(wxCommandEvent &event)
//------------------------------------------------------------------------------
void SpicePanel::OnNaifIdTextCtrlChange(wxCommandEvent &event)
{
   if (naifIdTextCtrl->IsModified())
   {
      naifIdChanged   = true;
      dataChanged     = true;
      theScPanel->EnableUpdate(true);
   }
}

//------------------------------------------------------------------------------
// void OnNaifIdRefTextCtrlChange(wxCommandEvent &event)
//------------------------------------------------------------------------------
void SpicePanel::OnNaifIdRefTextCtrlChange(wxCommandEvent &event)
{
   if (naifIdRefFrameTextCtrl->IsModified())
   {
      naifIdRefFrameChanged   = true;
      dataChanged     = true;
      theScPanel->EnableUpdate(true);
   }
}
