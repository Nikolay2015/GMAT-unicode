//$Id: ViewTextFrame.cpp 9585 2011-06-10 19:54:28Z lindajun $
//------------------------------------------------------------------------------
//                                 ViewTextFrame
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number S-67573-G
//
// Author: Linda Jun
// Created: 2003/11/03
//
/**
 * Shows text.
 */
//------------------------------------------------------------------------------
#include "ViewTextFrame.hpp"
#include "GmatAppData.hpp"
#include "FileManager.hpp"
#include "GmatBaseException.hpp"
#include "MessageInterface.hpp"

BEGIN_EVENT_TABLE(ViewTextFrame, wxFrame)
   EVT_MENU(VIEW_TEXT_CLEAR, ViewTextFrame::OnClear)
   EVT_MENU(VIEW_TEXT_SAVE_AS, ViewTextFrame::OnSaveAs)
   EVT_MENU(VIEW_TEXT_EXIT, ViewTextFrame::OnExit)
END_EVENT_TABLE()

//------------------------------
// public methods
//------------------------------

//------------------------------------------------------------------------------
// ViewTextFrame(wxFrame *frame, const wxString& title,
//              int x, int y, int w, int h, const wxString &mode)
//------------------------------------------------------------------------------
ViewTextFrame::ViewTextFrame(wxFrame *frame, const wxString& title,
                             int x, int y, int w, int h, const wxString &mode,
                             const wxString &type)
   : wxFrame(frame, -1, title, wxPoint(x, y), wxSize(w, h),
             wxDEFAULT_FRAME_STYLE | wxFRAME_FLOAT_ON_PARENT)
{
   CreateStatusBar(2);
   mWindowMode = mode;
   mTextType = type;

   // Set additional style wxTE_RICH to Ctrl + mouse scroll wheel to decrease or
   // increase text size(loj: 2009.02.05)
   mTextCtrl = new wxTextCtrl(this, -1, wxT(""), wxPoint(0, 0), wxSize(0, 0),
                              wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP | wxTE_RICH);
   mTextCtrl->SetMaxLength(320000);
   mTextCtrl->SetFont(GmatAppData::Instance()->GetFont());

#if wxUSE_MENUS
   // create a menu bar
   SetMenuBar(CreateMainMenu());
#endif // wxUSE_MENUS

   // Set icon if icon file is in the start up file
   FileManager *fm = FileManager::Instance();
   try
   {
      wxString iconfile = fm->GetFullPathname(wxT("MAIN_ICON_FILE")).c_str();
      #if defined __WXMSW__
         SetIcon(wxIcon(iconfile, wxBITMAP_TYPE_ICO));
      #elif defined __WXGTK__
         SetIcon(wxIcon(iconfile, wxBITMAP_TYPE_XPM));
      #elif defined __WXMAC__
         SetIcon(wxIcon(iconfile, wxBITMAP_TYPE_PICT_RESOURCE));
      #endif
   }
   catch (GmatBaseException &)
   {
      //MessageInterface::ShowMessage(e.GetMessage());
   }

   CenterOnScreen(wxBOTH);
}


//------------------------------------------------------------------------------
// ~ViewTextFrame()
//------------------------------------------------------------------------------
ViewTextFrame::~ViewTextFrame()
{
   if (mWindowMode != wxT("Temporary"))
   {
      GmatAppData::Instance()->SetCompareWindow(NULL);
   }
}


//------------------------------------------------------------------------------
// void AppendText(const wxString& text)
//------------------------------------------------------------------------------
void ViewTextFrame::AppendText(const wxString& text)
{
   mTextCtrl->AppendText(text);
}


//------------------------------------------------------------------------------
// void OnClear(wxCommandEvent& WXUNUSED(event))
//------------------------------------------------------------------------------
void ViewTextFrame::OnClear(wxCommandEvent& WXUNUSED(event))
{
   mTextCtrl->Clear();
}


//------------------------------------------------------------------------------
// void OnSaveAs(wxCommandEvent& WXUNUSED(event))
//------------------------------------------------------------------------------
void ViewTextFrame::OnSaveAs(wxCommandEvent& WXUNUSED(event))
{
   //MessageInterface::ShowMessage("ViewTextFrame::OnSaveAs() entered\n");

   wxString filename;
   if (mTextType == wxT("Script"))
   {
      filename =
         wxFileSelector(wxT("Choose a file to save"), wxT(""), wxT(""), wxT("script"),
                        wxT("Script files (*.script)|*.script"), wxSAVE);
   }
   else
   {
      filename =
         wxFileSelector(wxT("Choose a file to save"), wxT(""), wxT(""), wxT("txt"),
                        wxT("Report files (*.report)|*.report|Text files (*.txt)|*.txt"),
                        wxSAVE);
   }

   if (!filename.empty())
      mTextCtrl->SaveFile(filename);

}


//------------------------------------------------------------------------------
// void OnExit(wxCommandEvent& WXUNUSED(event) )
//------------------------------------------------------------------------------
void ViewTextFrame::OnExit(wxCommandEvent& WXUNUSED(event) )
{
   if (mWindowMode == wxT("Temporary"))
      Close(true);
   else
      Show(false);
}

//-------------------------------
// private methods
//-------------------------------

//------------------------------------------------------------------------------
// wxMenuBar* CreateMainMenu()
//------------------------------------------------------------------------------
wxMenuBar* ViewTextFrame::CreateMainMenu()
{
   // Make a menubar
   wxMenuBar *menuBar = new wxMenuBar;

   wxMenu *menuFile = new wxMenu;
   menuFile->Append(VIEW_TEXT_CLEAR, wxT("&Clear"));
   menuFile->AppendSeparator();
   menuFile->Append(VIEW_TEXT_SAVE_AS, wxT("&Save As..."));
   menuFile->AppendSeparator();
   menuFile->Append(VIEW_TEXT_EXIT, wxT("E&xit"));
   menuBar->Append(menuFile, wxT("&File"));

   return menuBar;
}
