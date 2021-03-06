//$Id: CompareTextDialog.cpp 9514 2011-04-30 21:44:00Z djcinsb $
//------------------------------------------------------------------------------
//                              CompareTextDialog
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Linda Jun
// Created: 2006/06/27
//
/**
 * Shows dialog for comparing text line by line.
 */
//------------------------------------------------------------------------------

#include "CompareTextDialog.hpp"
#include "GmatStaticBoxSizer.hpp"
#include "FileManager.hpp"
#include "MessageInterface.hpp"

#include <wx/dir.h>
#include <wx/filename.h>

//#define DEBUG_COMPARE_TEXT_DIALOG 1

//------------------------------------------------------------------------------
// event tables and other macros for wxWindows
//------------------------------------------------------------------------------

BEGIN_EVENT_TABLE(CompareTextDialog, GmatDialog)
   EVT_BUTTON(ID_BUTTON_OK, GmatDialog::OnOK)
   EVT_BUTTON(ID_BUTTON_CANCEL, GmatDialog::OnCancel)
   EVT_BUTTON(ID_BUTTON, CompareTextDialog::OnButtonClick)
   EVT_CHECKBOX(ID_CHECKBOX, CompareTextDialog::OnCheckBoxChange)
   EVT_COMBOBOX(ID_COMBOBOX, CompareTextDialog::OnComboBoxChange)
   EVT_TEXT_ENTER(ID_TEXTCTRL, CompareTextDialog::OnTextEnterPress)
END_EVENT_TABLE()

   
//------------------------------------------------------------------------------
// CompareTextDialog(wxWindow *parent)
//------------------------------------------------------------------------------
CompareTextDialog::CompareTextDialog(wxWindow *parent)
   : GmatDialog(parent, -1, wxString(wxT("CompareTextDialog")))
{
   mCompareFiles = false;
   mSaveCompareResults = false;
   mHasDir1 = false;
   mHasDir2 = false;
   mHasDir3 = false;
   
   mNumFilesToCompare = 0;
   mNumDirsToCompare = 1;
   mBaseDirectory = wxT("");
   mBasePrefix = wxT("");
   
   mCompareDirs.Add(wxT(""));
   mCompareDirs.Add(wxT(""));
   mCompareDirs.Add(wxT(""));
   
   mComparePrefixes.Add(wxT(""));
   mComparePrefixes.Add(wxT(""));
   mComparePrefixes.Add(wxT(""));
   
   Create();
   ShowData();
}


//------------------------------------------------------------------------------
// ~CompareTextDialog()
//------------------------------------------------------------------------------
CompareTextDialog::~CompareTextDialog()
{
}


//------------------------------------------------------------------------------
// virtual void Create()
//------------------------------------------------------------------------------
void CompareTextDialog::Create()
{
   #if DEBUG_COMPARE_TEXT_DIALOG
   MessageInterface::ShowMessage(wxT("CompareTextDialog::Create() entered.\n"));
   #endif
   
   int bsize = 2;
   
   //------------------------------------------------------
   // compare base directory
   //------------------------------------------------------
   wxStaticText *baseDirLabel =
      new wxStaticText(this, ID_TEXT, wxT("Base Directory:"),
                       wxDefaultPosition, wxDefaultSize, 0);
   
   mBaseDirTextCtrl =
      new wxTextCtrl(this, ID_TEXTCTRL, wxT(""),
                     wxDefaultPosition, wxSize(320,20), 0);
   
   mBaseDirButton =
      new wxButton(this, ID_BUTTON, wxT("Browse"),
                    wxDefaultPosition, wxSize(60,20), 0);
   
   wxStaticText *basePrefixLabel =
      new wxStaticText(this, ID_TEXT, wxT("Prefix to use for file name:"),
                       wxDefaultPosition, wxDefaultSize, 0);
   
   mBasePrefixTextCtrl =
      new wxTextCtrl(this, ID_TEXTCTRL, wxT(""),
                     wxDefaultPosition, wxSize(80,20), 0);
   mBasePrefixButton =
      new wxButton(this, ID_BUTTON, wxT("Update"),
                    wxDefaultPosition, wxSize(60,20), 0);
   
   wxStaticText *numFilesBaseDirLabel =
      new wxStaticText(this, ID_TEXT, wxT("Number of Files (.txt, .report, .data, .script, .eph):"),
                       wxDefaultPosition, wxDefaultSize, 0);
   
   mNumFilesInBaseDirTextCtrl =
      new wxTextCtrl(this, ID_TEXTCTRL, wxT("0"),
                     wxDefaultPosition, wxSize(80,20), 0);
   
   
   wxFlexGridSizer *baseDirSizer = new wxFlexGridSizer(2, 0, 0);
   baseDirSizer->Add(baseDirLabel, 0, wxALIGN_RIGHT|wxALL|wxGROW, bsize);
   baseDirSizer->Add(20,20, 0, wxALIGN_RIGHT|wxALL|wxGROW, bsize);
   baseDirSizer->Add(mBaseDirTextCtrl, 0, wxALIGN_RIGHT|wxALL|wxGROW, bsize);
   baseDirSizer->Add(mBaseDirButton, 0, wxALIGN_CENTER|wxALL, bsize);
   
   wxFlexGridSizer *baseFileSizer = new wxFlexGridSizer(3, 0, 0);
   baseFileSizer->Add(basePrefixLabel, 0, wxALIGN_LEFT|wxALL, bsize);
   baseFileSizer->Add(mBasePrefixTextCtrl, 0, wxALIGN_RIGHT|wxALL|wxGROW, bsize);
   baseFileSizer->Add(mBasePrefixButton, 0, wxALIGN_RIGHT|wxALL|wxGROW, bsize);
   baseFileSizer->Add(numFilesBaseDirLabel, 0, wxALIGN_LEFT|wxALL, bsize);
   baseFileSizer->Add(mNumFilesInBaseDirTextCtrl, 0, wxALIGN_RIGHT|wxALL|wxGROW, bsize);
   baseFileSizer->Add(20,20);
   
   GmatStaticBoxSizer *baseSizer =
      new GmatStaticBoxSizer(wxVERTICAL, this, wxT("Base Directory"));
   
   baseSizer->Add(baseDirSizer, 0, wxALIGN_RIGHT|wxALL|wxGROW, bsize);
   baseSizer->Add(baseFileSizer, 0, wxALIGN_RIGHT|wxALL|wxGROW, bsize);
   
   //------------------------------------------------------
   // compare file names
   //------------------------------------------------------
   wxString dirArray[] = {wxT("Directory1"), wxT("Directory2"), wxT("Directory3")};
   
   mCompareDirsComboBox = 
      new wxComboBox(this, ID_COMBOBOX, wxT("Compare Directories"),
                     wxDefaultPosition, wxDefaultSize, 3, dirArray,
                     wxCB_READONLY);
   mCompareDirTextCtrl =
      new wxTextCtrl(this, ID_TEXTCTRL, wxT(""),
                     wxDefaultPosition, wxSize(320,20), 0);
   
   mCompareDirButton =
      new wxButton(this, ID_BUTTON, wxT("Browse"),
                    wxDefaultPosition, wxSize(60,20), 0);
   
   wxStaticText *comparePrefixLabel =
      new wxStaticText(this, ID_TEXT, wxT("Prefix to use for file name:"),
                       wxDefaultPosition, wxDefaultSize, 0);
   mComparePrefixTextCtrl =
      new wxTextCtrl(this, ID_TEXTCTRL, wxT(""),
                     wxDefaultPosition, wxSize(80,20), 0);
   mComparePrefixButton =
      new wxButton(this, ID_BUTTON, wxT("Update"),
                    wxDefaultPosition, wxSize(60,20), 0);
   
   wxStaticText *numFilesInCompareDirLabel =
      new wxStaticText(this, ID_TEXT, wxT("Number of Files (.txt, .report, .data, .script, .eph):"),
                       wxDefaultPosition, wxDefaultSize, 0);
   
   mNumFilesInCompareDirTextCtrl =
      new wxTextCtrl(this, ID_TEXTCTRL, wxT("0"),
                     wxDefaultPosition, wxSize(80,20), 0);
   
   //---------- sizer
   wxFlexGridSizer *compareDirSizer = new wxFlexGridSizer(2, 0, 0);   
   compareDirSizer->Add(mCompareDirsComboBox, 0, wxALIGN_LEFT|wxALL, bsize);
   compareDirSizer->Add(20, 20, 0, wxALIGN_LEFT|wxALL, bsize);
   compareDirSizer->Add(mCompareDirTextCtrl, 0, wxALIGN_LEFT|wxALL, bsize);
   compareDirSizer->Add(mCompareDirButton, 0, wxALIGN_LEFT|wxALL, bsize);
   
   wxFlexGridSizer *compareFileSizer = new wxFlexGridSizer(3, 0, 0);   
   compareFileSizer->Add(comparePrefixLabel, 0, wxALL, bsize);
   compareFileSizer->Add(mComparePrefixTextCtrl, 0, wxALL|wxGROW, bsize);
   compareFileSizer->Add(mComparePrefixButton, 0, wxALL|wxGROW, bsize);
   compareFileSizer->Add(numFilesInCompareDirLabel, 0, wxALL|wxGROW, bsize);
   compareFileSizer->Add(mNumFilesInCompareDirTextCtrl, 0, wxALL|wxGROW, bsize);
   compareFileSizer->Add(20, 20);
   
   GmatStaticBoxSizer *compareDirsSizer =
      new GmatStaticBoxSizer(wxVERTICAL, this, wxT("Compare Directories"));
   
   compareDirsSizer->Add(compareDirSizer, 0, wxALIGN_LEFT|wxALL|wxGROW, bsize);
   compareDirsSizer->Add(compareFileSizer, 0, wxALIGN_LEFT|wxALL|wxGROW, bsize);
   
   //------------------------------------------------------
   // compare results
   //------------------------------------------------------
   wxStaticText *numDirsToCompareLabel =
      new wxStaticText(this, ID_TEXT, wxT("Number of Directories to Compare:"),
                       wxDefaultPosition, wxDefaultSize, 0);
   
   mNumDirsToCompareTextCtrl =
      new wxTextCtrl(this, ID_TEXTCTRL, wxT("0"),
                     wxDefaultPosition, wxSize(80,20), 0);
   
   wxStaticText *numFilesToCompareLabel =
      new wxStaticText(this, ID_TEXT, wxT("Number of Files to Compare:"),
                       wxDefaultPosition, wxDefaultSize, 0);
   
   mNumFilesToCompareTextCtrl =
      new wxTextCtrl(this, ID_TEXTCTRL, wxT("0"),
                     wxDefaultPosition, wxSize(80,20), 0);
      
   //---------- sizer
   wxFlexGridSizer *numFilesGridSizer = new wxFlexGridSizer(2, 0, 0);
   numFilesGridSizer->Add(numDirsToCompareLabel, 0, wxALIGN_LEFT|wxALL, bsize);
   numFilesGridSizer->Add(mNumDirsToCompareTextCtrl, 0,
                          wxALIGN_RIGHT|wxALL|wxGROW, bsize);
   numFilesGridSizer->Add(numFilesToCompareLabel, 0, wxALIGN_LEFT|wxALL, bsize);
   numFilesGridSizer->Add(mNumFilesToCompareTextCtrl, 0,
                          wxALIGN_RIGHT|wxALL|wxGROW, bsize);   
   
   mSaveResultCheckBox =
      new wxCheckBox(this, ID_CHECKBOX, wxT("Save Compare Results to File"),
                     wxDefaultPosition, wxSize(-1, -1), 0);
   
   wxStaticText *saveFileLabel =
      new wxStaticText(this, ID_TEXT, wxT("File Name to Save:"),
                       wxDefaultPosition, wxDefaultSize, 0);

   mSaveFileTextCtrl =
      new wxTextCtrl(this, ID_TEXTCTRL, wxT(""),
                     wxDefaultPosition, wxSize(320,20), 0);
   
   mSaveBrowseButton =
      new wxButton(this, ID_BUTTON, wxT("Browse"),
                    wxDefaultPosition, wxSize(60,20), 0);

   //---------- sizer
   wxFlexGridSizer *saveGridSizer = new wxFlexGridSizer(2, 0, 0);
   saveGridSizer->Add(mSaveFileTextCtrl, 0, wxALIGN_LEFT|wxALL, bsize);
   saveGridSizer->Add(mSaveBrowseButton, 0, wxALIGN_CENTRE|wxALL, bsize);
   
   GmatStaticBoxSizer *compareSizer =
      new GmatStaticBoxSizer(wxVERTICAL, this, wxT("Compare"));
   
   compareSizer->Add(numFilesGridSizer, 0, wxALIGN_LEFT|wxALL, bsize);
   compareSizer->Add(mSaveResultCheckBox, 0, wxALIGN_LEFT|wxALL, bsize);
   compareSizer->Add(20, 5, 0, wxALIGN_LEFT|wxALL, bsize);
   compareSizer->Add(saveFileLabel, 0, wxALIGN_LEFT|wxALL, bsize);
   compareSizer->Add(saveGridSizer, 0, wxALIGN_LEFT|wxALL, bsize);
   
   //------------------------------------------------------
   // add to page sizer
   //------------------------------------------------------
   wxBoxSizer *pageBoxSizer = new wxBoxSizer(wxVERTICAL);
   
   pageBoxSizer->Add(baseSizer, 0, wxALIGN_CENTRE|wxALL|wxGROW, bsize);
   pageBoxSizer->Add(compareDirsSizer, 0, wxALIGN_CENTRE|wxALL|wxGROW, bsize);
   pageBoxSizer->Add(compareSizer, 0, wxALIGN_CENTRE|wxALL|wxGROW, bsize);
   
   theMiddleSizer->Add(pageBoxSizer, 0, wxALIGN_CENTRE|wxALL, bsize);
}


//------------------------------------------------------------------------------
// virtual void LoadData()
//------------------------------------------------------------------------------
void CompareTextDialog::LoadData()
{
   wxString str;
   str.Printf(wxT("%d"), mNumFilesToCompare);
   mNumFilesToCompareTextCtrl->SetValue(str);
   
   str.Printf(wxT("%d"), mNumDirsToCompare);
   mNumDirsToCompareTextCtrl->SetValue(str);


   FileManager *fm = FileManager::Instance();
   mBaseDirectory = fm->GetFullPathname(FileManager::OUTPUT_PATH).c_str();
   mCompareDirs[0] = fm->GetFullPathname(FileManager::OUTPUT_PATH).c_str();
   mCompareDirs[1] = fm->GetFullPathname(FileManager::OUTPUT_PATH).c_str();
   mCompareDirs[2] = fm->GetFullPathname(FileManager::OUTPUT_PATH).c_str();
   mCompareDirsComboBox->SetSelection(0);
   mSaveFileName = mBaseDirectory + wxT("CompareTextResults.txt");
   mBaseDirTextCtrl->SetValue(mBaseDirectory);
   mCompareDirTextCtrl->SetValue(mCompareDirs[0]);
   mSaveFileTextCtrl->SetValue(mSaveFileName);

   // update file info in directory 1 and 2
   UpdateFileInfo(0, true);
   UpdateFileInfo(1, false);
   
   mSaveResultCheckBox->Enable();
   mSaveFileTextCtrl->Disable();
   mSaveBrowseButton->Disable();
   
   theOkButton->Enable();
}


//------------------------------------------------------------------------------
// virtual void SaveData()
//------------------------------------------------------------------------------
void CompareTextDialog::SaveData()
{
   long longNum;
   canClose = true;
      
   if (!mNumFilesToCompareTextCtrl->GetValue().ToLong(&longNum))
   {
      wxMessageBox(wxT("Invalid number of scripts to run entered."));
      canClose = false;
      return;
   }
   
   mNumFilesToCompare = longNum;
   
   if (!mNumDirsToCompareTextCtrl->GetValue().ToLong(&longNum))
   {
      wxMessageBox(wxT("Invalid number of scripts to run entered."));
      canClose = false;
      return;
   }
   
   mNumDirsToCompare = longNum;
   mSaveFileName = mSaveFileTextCtrl->GetValue();
   
   mCompareFiles = true;
   if (mNumFilesToCompare <= 0)
   {
      wxMessageBox(wxT("There are no specific report files to compare.\n")
                       wxT("Please check file names to compare."),
                   wxT("GMAT Warning"));
      canClose = false;
      mCompareFiles = false;
   }
   
   mSaveCompareResults = mSaveResultCheckBox->GetValue();
   
   #if DEBUG_COMPARE_TEXT_DIALOG
   MessageInterface::ShowMessage
      (wxT("CompareTextDialog::SaveData() mNumFilesToCompare=%d, ")
       wxT("mCompareFiles=%d\n   mBaseDirectory=%s, mCompareDirs[0]=%s\n"),
       mNumFilesToCompare, mCompareFiles,
       mBaseDirectory.c_str(), mCompareDirs[0].c_str());
   #endif
}


//------------------------------------------------------------------------------
// virtual void ResetData()
//------------------------------------------------------------------------------
void CompareTextDialog::ResetData()
{
   canClose = true;
   mCompareFiles = false;
}


//------------------------------------------------------------------------------
// void OnButtonClick(wxCommandEvent& event)
//------------------------------------------------------------------------------
void CompareTextDialog::OnButtonClick(wxCommandEvent& event)
{
   if (event.GetEventObject() == mBaseDirButton)
   {
      wxDirDialog dialog(this, wxT("Select a base directory"), mBaseDirectory);
      
      if (dialog.ShowModal() == wxID_OK)
      {
         mBaseDirectory = dialog.GetPath();
         mBaseDirTextCtrl->SetValue(mBaseDirectory);
         mSaveFileTextCtrl->SetValue(mBaseDirectory + wxT("/CompareTextResults.txt"));
         UpdateFileInfo(0, true);
         
         #if DEBUG_COMPARE_TEXT_DIALOG
         MessageInterface::ShowMessage
            (wxT("CompareTextDialog::OnButtonClick() mBaseDirectory=%s\n"),
             mBaseDirectory.c_str());
         #endif
      }
   }
   else if (event.GetEventObject() == mCompareDirButton)
   {
      int dirIndex = mCompareDirsComboBox->GetSelection();
      wxDirDialog dialog(this, wxT("Select a compare dierctory"), mCompareDirs[dirIndex]);
      
      if (dialog.ShowModal() == wxID_OK)
      {
         if (dirIndex == 0)
            mHasDir1 = true;
         else if (dirIndex == 1)
            mHasDir2 = true;
         else if (dirIndex == 2)
            mHasDir3 = true;
         
         mCompareDirs[dirIndex] = dialog.GetPath();
         mCompareDirTextCtrl->SetValue(mCompareDirs[dirIndex]);
         UpdateFileInfo(dirIndex, false);
         
         // update number of directories to compare
         int numDirs = 0;
         if (mHasDir1) numDirs++;
         if (mHasDir2) numDirs++;
         if (mHasDir3) numDirs++;
         mNumDirsToCompare = numDirs;
         
         wxString str;
         str.Printf(wxT("%d"), mNumDirsToCompare);
         mNumDirsToCompareTextCtrl->SetValue(str);
         
         #if DEBUG_COMPARE_TEXT_DIALOG
         MessageInterface::ShowMessage
            (wxT("CompareTextDialog::OnButtonClick() mCompareDirs[%d]=%s\n"),
             dirIndex, mCompareDirs[dirIndex].c_str());
         #endif
      }
   }
   else if (event.GetEventObject() == mBasePrefixButton)
   {
      mBasePrefix = mBasePrefixTextCtrl->GetValue();
      UpdateFileInfo(0, true);
   }
   else if (event.GetEventObject() == mComparePrefixButton)
   {
      int dirIndex = mCompareDirsComboBox->GetSelection();
      mComparePrefixes[dirIndex] = mComparePrefixTextCtrl->GetValue();
      UpdateFileInfo(dirIndex, false);
   }
   else if (event.GetEventObject() == mSaveBrowseButton)
   {
      wxString filename =
         wxFileSelector(wxT("Choose a file to save"), mBaseDirectory, wxT(""), wxT("txt"),
                        wxT("Report files (*.report)|*.report|Text files (*.txt)|*.txt"),
                        wxSAVE);
      
      if (!filename.empty())
      {
         mSaveFileTextCtrl->SetValue(filename);
         
         #if DEBUG_COMPARE_TEXT_DIALOG
         MessageInterface::ShowMessage
            (wxT("CompareTextDialog::OnButtonClick() savefile=%s\n"), filename.c_str());
         #endif
      }
      
   }
}


//------------------------------------------------------------------------------
// void OnCheckBoxChange(wxCommandEvent& event)
//------------------------------------------------------------------------------
void CompareTextDialog::OnCheckBoxChange(wxCommandEvent& event)
{
   if (mSaveResultCheckBox->IsChecked())
   {
      mSaveFileTextCtrl->Enable();
      mSaveBrowseButton->Enable();
   }
   else
   {
      mSaveFileTextCtrl->Disable();
      mSaveBrowseButton->Disable();
   }
}


//------------------------------------------------------------------------------
// void OnComboBoxChange(wxCommandEvent& event)
//------------------------------------------------------------------------------
void CompareTextDialog::OnComboBoxChange(wxCommandEvent& event)
{
   if (event.GetEventObject() == mCompareDirsComboBox)
   {
      int currDir = mCompareDirsComboBox->GetSelection();
      mCompareDirTextCtrl->SetValue(mCompareDirs[currDir]);
      mComparePrefixTextCtrl->SetValue(mComparePrefixes[currDir]);
   }
}


//------------------------------------------------------------------------------
// void OnTextEnterPress(wxCommandEvent& event)
//------------------------------------------------------------------------------
void CompareTextDialog::OnTextEnterPress(wxCommandEvent& event)
{
}


//------------------------------------------------------------------------------
// void UpdateFileInfo(Integer dir)
//------------------------------------------------------------------------------
void CompareTextDialog::UpdateFileInfo(Integer dir, bool isBaseDir)
{
   #if DEBUG_COMPARE_TEXT_DIALOG   
   MessageInterface::ShowMessage
      (wxT("CompareTextDialog::UpdateFileInfo() entered, dir=%d, isBaseDir=%d\n"),
       dir, isBaseDir);
   #endif
   
   if (isBaseDir)
   {
      mFileNamesInBaseDir = GetFilenames(mBaseDirectory, mBasePrefix);
      mNumFilesInBaseDir = mFileNamesInBaseDir.GetCount();
      mNumFilesInBaseDirTextCtrl->SetValue(wxT(""));
      *mNumFilesInBaseDirTextCtrl << mNumFilesInBaseDir;
   }
   else
   {
      mFileNamesInCompareDir = GetFilenames(mCompareDirs[dir], mComparePrefixes[dir]);
      mNumFilesInCompareDir = mFileNamesInCompareDir.GetCount();
      mNumFilesInCompareDirTextCtrl->SetValue(wxT(""));
      *mNumFilesInCompareDirTextCtrl << mNumFilesInCompareDir;
   }
   
   // number of files to compare
   mNumFilesToCompareTextCtrl->SetValue(wxT(""));
   if (mNumFilesInBaseDir == 0 || mNumFilesInCompareDir == 0)
      *mNumFilesToCompareTextCtrl << 0;
   else
      *mNumFilesToCompareTextCtrl << mNumFilesInBaseDir;
   
}


//------------------------------------------------------------------------------
// wxArrayString GetFilenames(const wxString &dirname, const wxString &prefix)
//------------------------------------------------------------------------------
wxArrayString CompareTextDialog::GetFilenames(const wxString &dirname,
                                              const wxString &prefix)
{
   #if DEBUG_COMPARE_TEXT_DIALOG   
   MessageInterface::ShowMessage
      (wxT("CompareTextDialog::GetFilenames() entered, dirname='%s', ")
       wxT("prefix='%s'\n"), dirname.c_str(), prefix.c_str());
   #endif
   
   wxDir dir(dirname);
   wxString filename;
   wxString filepath;
   wxArrayString fileNames;
   size_t prefixLen = prefix.Len();
   
   bool cont = dir.GetFirst(&filename);
   while (cont)
   {
      if (filename.Contains(wxT(".report")) || filename.Contains(wxT(".txt")) ||
          filename.Contains(wxT(".data")) || filename.Contains(wxT(".script")) ||
          filename.Contains(wxT(".eph")))
      {
         // if it has prefix
         if (filename.Left(prefixLen) == prefix)
         {
            // if not backup files
            if (filename.Last() == 't' || filename.Last() == 'a' ||
                filename.Last() == 'h')
            {
               filepath = dirname + wxT("/") + filename;
               fileNames.Add(filepath);
            }
         }
      }
      
      cont = dir.GetNext(&filename);
   }
   
   return fileNames;
}

