//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of main frame for displaying game
//
// This file is part of Gambit
// Copyright (c) 2004, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#include <fstream>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // WX_PRECOMP

#include "gambit.h"
#include "game-frame.h"

#include "dialog-about.h"        // for "About..." dialog

#include "tree-display.h"
#include "tree-print.h"
#include "table-schelling.h"
#include "table-matrix.h"
#include "panel-nash.h"
#include "panel-qre.h"

const int GBT_MENU_FILE_EXPORT = 970;
const int GBT_MENU_FILE_EXPORT_BMP = 971;
const int GBT_MENU_FILE_EXPORT_JPG = 972;
const int GBT_MENU_FILE_EXPORT_PNG = 973;

const int GBT_MENU_VIEW_EFG = 998;
const int GBT_MENU_VIEW_NFG = 999;

const int GBT_MENU_VIEW_ZOOMIN = 997;
const int GBT_MENU_VIEW_ZOOMOUT = 996;

const int GBT_MENU_TOOLS_EQM = 1000;
const int GBT_MENU_TOOLS_QRE = 1001;

BEGIN_EVENT_TABLE(gbtGameFrame, wxFrame)
  EVT_MENU(wxID_NEW, gbtGameFrame::OnFileNew)
  EVT_MENU(wxID_OPEN, gbtGameFrame::OnFileOpen)
  EVT_MENU(wxID_CLOSE, gbtGameFrame::OnFileClose)
  EVT_MENU(wxID_SAVE, gbtGameFrame::OnFileSave)
  EVT_MENU(wxID_PRINT_SETUP, gbtGameFrame::OnFilePageSetup)
  EVT_MENU(wxID_PREVIEW, gbtGameFrame::OnFilePrintPreview)
  EVT_MENU(wxID_PRINT, gbtGameFrame::OnFilePrint)
  EVT_MENU(GBT_MENU_FILE_EXPORT_BMP, gbtGameFrame::OnFileExportBMP)
  EVT_MENU(GBT_MENU_FILE_EXPORT_JPG, gbtGameFrame::OnFileExportJPG)
  EVT_MENU(GBT_MENU_FILE_EXPORT_PNG, gbtGameFrame::OnFileExportPNG)
  EVT_MENU(wxID_EXIT, gbtGameFrame::OnFileExit)
  EVT_MENU_RANGE(wxID_FILE1, wxID_FILE9, gbtGameFrame::OnFileMRU)
  EVT_MENU(wxID_UNDO, gbtGameFrame::OnEditUndo)
  EVT_MENU(wxID_REDO, gbtGameFrame::OnEditRedo)
  EVT_MENU(GBT_MENU_VIEW_EFG, gbtGameFrame::OnViewEfg)
  EVT_MENU(GBT_MENU_VIEW_NFG, gbtGameFrame::OnViewNfg)
  EVT_MENU(GBT_MENU_VIEW_ZOOMIN, gbtGameFrame::OnViewZoomIn)
  EVT_MENU(GBT_MENU_VIEW_ZOOMOUT, gbtGameFrame::OnViewZoomOut)
  EVT_MENU(GBT_MENU_TOOLS_EQM, gbtGameFrame::OnToolsEquilibrium)
  EVT_MENU(GBT_MENU_TOOLS_QRE, gbtGameFrame::OnToolsQre)
  EVT_MENU(wxID_ABOUT, gbtGameFrame::OnHelpAbout)
  EVT_CLOSE(gbtGameFrame::OnCloseWindow)
END_EVENT_TABLE()

//-------------------------------------------------------------------------
//                       gbtGameFrame: Lifecycle
//-------------------------------------------------------------------------

gbtGameFrame::gbtGameFrame(wxWindow *p_parent, gbtGameDocument *p_doc)
  : wxFrame(p_parent, -1, "",
	    wxDefaultPosition, wxSize(800, 600)), 
    gbtGameView(p_doc)
{
  wxGetApp().AddWindow(this);
  MakeMenu();

  if (p_doc->GetGame()->HasTree()) {
    m_treeDisplay = new gbtTreeDisplay(this, p_doc);
  }
  else {
    m_treeDisplay = 0;
  }

  if (p_doc->GetGame()->NumPlayers() == 2) {
    m_tablePanel = new gbtTableSchelling(this, p_doc);
  }
  else {
    m_tablePanel = new gbtTableMatrix(this, p_doc);
  }

  m_algorithmPanel = new gbtNashPanel(this, p_doc);
  m_qrePanel = new gbtQrePanel(this, p_doc);

  wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
  if (m_treeDisplay) {
    sizer->Add(m_treeDisplay, 1, wxEXPAND, 0);
  }
  sizer->Add(m_tablePanel, 1, wxEXPAND, 0);
  sizer->Show(m_tablePanel, !m_doc->GetGame()->HasTree());

  sizer->Add(m_algorithmPanel, 1, wxEXPAND, 0);
  sizer->Show(m_algorithmPanel, false);

  sizer->Add(m_qrePanel, 1, wxEXPAND, 0);
  sizer->Show(m_qrePanel, false);

  SetSizer(sizer);
  Layout();

  OnUpdate();
  Show(true);
}

gbtGameFrame::~gbtGameFrame()
{
  wxGetApp().GetFileHistory()->RemoveMenu(GetMenuBar()->GetMenu(0));
  wxGetApp().RemoveWindow(this);
}

//-------------------------------------------------------------------------
//          gbtGameFrame: Auxiliary functions for setting up frame
//-------------------------------------------------------------------------

void gbtGameFrame::MakeMenu(void)
{
  wxMenu *fileMenu = new wxMenu;
  fileMenu->Append(wxID_NEW, _("&New"), _("Create a new game"));
  fileMenu->Append(wxID_OPEN, _("&Open"), _("Open a saved game"));
  fileMenu->Append(wxID_CLOSE, _("&Close"), _("Close this window"));
  fileMenu->AppendSeparator();
  fileMenu->Append(wxID_SAVE, _("&Save"), _("Save this game"));
  
  wxMenu *fileExportMenu = new wxMenu;
  fileExportMenu->Append(GBT_MENU_FILE_EXPORT_BMP, _("&BMP"),
			 _("Export game as a Windows bitmap graphic"));
  fileExportMenu->Append(GBT_MENU_FILE_EXPORT_JPG, _("&JPG"),
			 _("Export game as a JPEG graphic"));
  fileExportMenu->Append(GBT_MENU_FILE_EXPORT_PNG, _("&PNG"),
			 _("Export game as a PNG graphic"));
  fileMenu->Append(GBT_MENU_FILE_EXPORT, _("&Export"),
		   fileExportMenu, _("Export the game in a different format"));
  fileMenu->AppendSeparator();

  fileMenu->Append(wxID_PRINT_SETUP, _("Page Se&tup"),
		   _("Set up preferences for printing"));
  fileMenu->Append(wxID_PREVIEW, _("Print Pre&view"),
		   _("View a preview of the game printout"));
  fileMenu->Append(wxID_PRINT, _("&Print"), _("Print this game"));
  fileMenu->AppendSeparator();

  fileMenu->Append(wxID_EXIT, _("E&xit"), _("Exit Gambit"));

  wxMenu *editMenu = new wxMenu;
  editMenu->Append(wxID_UNDO, _("&Undo"), _("Undo the last action"));
  editMenu->Append(wxID_REDO, _("&Redo"), _("Redo the last undone action"));

  wxMenu *viewMenu = new wxMenu;
  viewMenu->Append(GBT_MENU_VIEW_EFG, _("&Extensive form"),
		   _("Show the extensive form of this game"), true);
  viewMenu->Append(GBT_MENU_VIEW_NFG, _("&Normal form"),
		   _("Show the normal form of this game"), true);
  viewMenu->AppendSeparator();
  viewMenu->Append(GBT_MENU_VIEW_ZOOMIN, _("Zoom &in"),
		   _("Increase magnification of tree"));
  viewMenu->Append(GBT_MENU_VIEW_ZOOMOUT, _("Zoom &out"),
		   _("Decrease magnification of tree"));

  wxMenu *toolsMenu = new wxMenu;	
  toolsMenu->Append(GBT_MENU_TOOLS_EQM, _("&Equilibrium"),
		    _("Compute Nash equilibria of the game"), true);
  toolsMenu->Append(GBT_MENU_TOOLS_QRE, _("&QRE"),
		    _("Compute quantal response equilibria of the game"),
		    true);
  
  wxMenu *helpMenu = new wxMenu;
  helpMenu->Append(wxID_ABOUT, _("&About"), _("About Gambit"));

  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(fileMenu, _("&File"));
  menuBar->Append(editMenu, _("&Edit"));
  menuBar->Append(viewMenu, _("&View"));
  menuBar->Append(toolsMenu, _("&Tools"));
  menuBar->Append(helpMenu, _("&Help"));

  wxGetApp().GetFileHistory()->UseMenu(fileMenu);
  wxGetApp().GetFileHistory()->AddFilesToMenu(fileMenu);

  SetMenuBar(menuBar);
}

void gbtGameFrame::OnCloseWindow(wxCloseEvent &p_event)
{
  if (p_event.CanVeto() && m_doc->IsModified()) {
    if (wxMessageBox("Game has been modified.  Close anyway?", "Warning",
		     wxOK | wxCANCEL) == wxCANCEL) {
      p_event.Veto();
      return;
    }
  }
  p_event.Skip();
}

//-------------------------------------------------------------------------
//                 gbtGameFrame: Menu command handlers
//-------------------------------------------------------------------------

void gbtGameFrame::OnFileNew(wxCommandEvent &)
{
  gbtArray<int> dim(2);  dim[1] = dim[2] = 2;
  gbtGame nfg = NewNfg(dim);
  nfg->SetLabel("Untitled normal form game");
  (void) new gbtGameFrame(0, new gbtGameDocument(nfg));
}


void gbtGameFrame::OnFileOpen(wxCommandEvent &)
{
  wxFileDialog dialog(this, _("Choose file"), wxT(""), wxT(""), 
		      _("Gambit files (*.gbt)|*.gbt|"
			"Extensive form games (*.efg)|*.efg|"
			"Normal form games (*.nfg)|*.nfg|"
			"All files|*.*"));

  if (dialog.ShowModal() == wxID_OK) {
    if (dialog.GetPath().substr(dialog.GetPath().length() - 3, 3) == "gbt") {
      gbtGameDocument *doc = new gbtGameDocument(0);
      doc->Load(dialog.GetPath());
      doc->SetFilename(dialog.GetPath());
      wxGetApp().GetFileHistory()->AddFileToHistory(dialog.GetPath());
      (void) new gbtGameFrame(0, doc);
      return; 
    }

    std::ifstream file(dialog.GetPath().mb_str());

    if (!file.is_open()) {
      wxMessageBox(wxString::Format(_("Could not open '%s' for reading"),
				    (const char *) dialog.GetPath().mb_str()),
		   _("Error"), wxOK, 0);
      return;
    }

    gbtGame game;

    try {
      game = ReadNfg(file);
      if (file.bad()) {
	wxMessageBox(wxString::Format(_("An error occured in reading '%s'"),
				      (const char *) dialog.GetPath().mb_str()),
		     _("Error"), wxOK, 0);
	return;
      }
      gbtGameDocument *doc = new gbtGameDocument(game);
      doc->SetFilename(dialog.GetPath());
      wxGetApp().GetFileHistory()->AddFileToHistory(dialog.GetPath());
      (void) new gbtGameFrame(0, doc);
      return;
    }
    catch (gbtNfgParserException &) {
      // Not a normal form file; eat exception and try extensive form
    }

    try {
      file.seekg(0);
      game = ReadEfg(file);
      game->Canonicalize();
      if (file.bad()) {
	wxMessageBox(wxString::Format(_("An error occured in reading '%s'"),
				      (const char *) dialog.GetPath().mb_str()),
		     _("Error"), wxOK, 0);
	return;
      }
    }
    catch (gbtEfgParserException &) {
      wxMessageBox(wxString::Format(_("File '%s' not in a recognized format"),
				    (const char *) dialog.GetPath().mb_str()),
		   _("Error"), wxOK, 0);

      return;
    }

    gbtGameDocument *doc = new gbtGameDocument(game);
    doc->SetFilename(dialog.GetPath());
    wxGetApp().GetFileHistory()->AddFileToHistory(dialog.GetPath());
    (void) new gbtGameFrame(0, doc);
  }
}

void gbtGameFrame::OnFileMRU(wxCommandEvent &p_event)
{
  wxString filename(wxGetApp().GetFileHistory()->GetHistoryFile(p_event.GetId() - wxID_FILE1));

  std::ifstream file(filename.mb_str());

  if (!file.is_open()) {
    wxMessageBox(wxString::Format(_("Could not open '%s' for reading"),
				  (const char *) filename.mb_str()),
		 _("Error"), wxOK, 0);
    return;
  }

  gbtGame game;

  try {
    game = ReadNfg(file);
    if (file.bad()) {
      wxMessageBox(wxString::Format(_("An error occured in reading '%s'"),
				    (const char *) filename.mb_str()),
		   _("Error"), wxOK, 0);
      return;
    }

    gbtGameDocument *doc = new gbtGameDocument(game);
    doc->SetFilename(filename);
    wxGetApp().GetFileHistory()->AddFileToHistory(filename);
    (void) new gbtGameFrame(0, doc);
    return;
  }
  catch (gbtNfgParserException &) {
    // Not a normal form file; eat exception and try extensive form
  }

  try {
    file.seekg(0);
    game = ReadEfg(file);
    game->Canonicalize();
    if (file.bad()) {
      wxMessageBox(wxString::Format(_("An error occured in reading '%s'"),
				    (const char *) filename.mb_str()),
		   _("Error"), wxOK, 0);
      return;
    }
  }
  catch (gbtEfgParserException &) {
    wxMessageBox(wxString::Format(_("File '%s' not in a recognized format"),
				  (const char *) filename.mb_str()),
		 _("Error"), wxOK, 0);
    
    return;
  }
  gbtGameDocument *doc = new gbtGameDocument(game);
  doc->SetFilename(filename);
  wxGetApp().GetFileHistory()->AddFileToHistory(filename);
  (void) new gbtGameFrame(0, doc);

}

void gbtGameFrame::OnFileClose(wxCommandEvent &)
{
  Close();
}

void gbtGameFrame::OnFileSave(wxCommandEvent &)
{
  wxFileDialog dialog(this, _("Choose file"),
		      wxPathOnly(m_doc->GetFilename()),
		      wxFileNameFromPath(m_doc->GetFilename()), 
		      _T("Gambit files (*.gbt)|*.gbt"),
		      wxSAVE | wxOVERWRITE_PROMPT);

  switch (dialog.ShowModal()) {
  case wxID_OK:
    m_doc->SetFilename(dialog.GetPath());
    break;
  case wxID_CANCEL:
  default:
    return;
  }

  m_doc->Save(dialog.GetPath());
  m_doc->SetModified(false);
}

void gbtGameFrame::OnFileExportBMP(wxCommandEvent &)
{
  wxFileDialog dialog(this, _("Choose output file"), 
		      _T(""), _T(""),
		      _T("Windows bitmap files (*.bmp)|*.bmp"), 
		      wxSAVE | wxOVERWRITE_PROMPT);

  if (dialog.ShowModal() == wxID_OK) {
    wxMemoryDC dc;
    wxBitmap bitmap(m_treeDisplay->GetLayout().GetMaxX(),
		    m_treeDisplay->GetLayout().GetMaxY());
    dc.SelectObject(bitmap);
    dc.Clear();
    m_treeDisplay->GetLayout().DrawTree(dc);
    if (!bitmap.SaveFile(dialog.GetPath(), wxBITMAP_TYPE_BMP)) {
      wxMessageBox(wxString::Format(_("An error occurred in writing '%s'."),
				    (const char *) dialog.GetPath().mb_str()),
		   _("Error"), wxOK, this);
    }
  }
}

void gbtGameFrame::OnFileExportJPG(wxCommandEvent &)
{
  wxFileDialog dialog(this, _("Choose output file"), 
		      _T(""), _T(""),
		      _T("JPEG image files (*.jpg)|*.jpg|"
			 "JPEG image files (*.jpeg)|*.jpeg|"), 
		      wxSAVE | wxOVERWRITE_PROMPT);

  if (dialog.ShowModal() == wxID_OK) {
    wxMemoryDC dc;
    wxBitmap bitmap(m_treeDisplay->GetLayout().GetMaxX(),
		    m_treeDisplay->GetLayout().GetMaxY());
    dc.SelectObject(bitmap);
    dc.Clear();
    m_treeDisplay->GetLayout().DrawTree(dc);
    if (!bitmap.SaveFile(dialog.GetPath(), wxBITMAP_TYPE_JPEG)) {
      wxMessageBox(wxString::Format(_("An error occurred in writing '%s'."),
				    (const char *) dialog.GetPath().mb_str()),
		   _("Error"), wxOK, this);
    }
  }
}

void gbtGameFrame::OnFileExportPNG(wxCommandEvent &)
{
  wxFileDialog dialog(this, _("Choose output file"), 
		      _T(""), _T(""),
		      _T("PNG image files (*.png)|*.png"), 
		      wxSAVE | wxOVERWRITE_PROMPT);

  if (dialog.ShowModal() == wxID_OK) {
    wxMemoryDC dc;
    wxBitmap bitmap(m_treeDisplay->GetLayout().GetMaxX(),
		    m_treeDisplay->GetLayout().GetMaxY());
    dc.SelectObject(bitmap);
    dc.Clear();
    m_treeDisplay->GetLayout().DrawTree(dc);
    if (!bitmap.SaveFile(dialog.GetPath(), wxBITMAP_TYPE_PNG)) {
      wxMessageBox(wxString::Format(_("An error occurred in writing '%s'."),
				    (const char *) dialog.GetPath().mb_str()),
		   _("Error"), wxOK, this);
    }
  }
}

void gbtGameFrame::OnFilePageSetup(wxCommandEvent &)
{
  wxPageSetupDialog dialog(this, &m_pageSetupData);
  if (dialog.ShowModal() == wxID_OK) {
    m_printData = dialog.GetPageSetupData().GetPrintData();
    m_pageSetupData = dialog.GetPageSetupData();
  }
}

void gbtGameFrame::OnFilePrintPreview(wxCommandEvent &)
{
  wxPrintDialogData data(m_printData);
  wxPrintPreview *preview = 
    new wxPrintPreview(new gbtTreePrintout(m_treeDisplay->GetLayout(),
					   wxString::Format(_T("%s"), 
							    m_doc->GetGame()->GetLabel().c_str())),
		       new gbtTreePrintout(m_treeDisplay->GetLayout(),
					   wxString::Format(_T("%s"), 
							    m_doc->GetGame()->GetLabel().c_str())),
		       &data);

  if (!preview->Ok()) {
    delete preview;
    return;
  }

  wxPreviewFrame *frame = new wxPreviewFrame(preview, this,
					     _("Print Preview"),
					     wxPoint(100, 100),
					     wxSize(600, 650));
  frame->Initialize();
  frame->Show(true);
}

void gbtGameFrame::OnFilePrint(wxCommandEvent &)
{
  wxPrintDialogData data(m_printData);
  wxPrinter printer(&data);
  gbtTreePrintout printout(m_treeDisplay->GetLayout(),
			   wxString::Format(_T("%s"), 
					    m_doc->GetGame()->GetLabel().c_str()));

  if (!printer.Print(this, &printout, true)) {
    if (wxPrinter::GetLastError() == wxPRINTER_ERROR) {
      wxMessageBox(_("There was an error in printing"), _("Error"), wxOK);
    }
    // Otherwise, user hit "cancel"; just be quiet and return.
    return;
  }
  else {
    m_printData = printer.GetPrintDialogData().GetPrintData();
  }
}

void gbtGameFrame::OnFileExit(wxCommandEvent &)
{
  for (int i = 1; i <= wxGetApp().NumWindows(); i++) {
    if (wxGetApp().GetWindow(i)->GetDocument()->IsModified()) {
      if (wxMessageBox("There are modified games.  Exit anyway?", "Warning",
		       wxOK | wxCANCEL) == wxCANCEL) {
	return;
      }
      else {
	break;
      }
    }
  }

  while (wxGetApp().GetTopWindow()) {
    delete wxGetApp().GetTopWindow();
  }
}

void gbtGameFrame::OnEditUndo(wxCommandEvent &)
{
  m_doc->Undo();
}

void gbtGameFrame::OnEditRedo(wxCommandEvent &)
{
  m_doc->Redo();
}

void gbtGameFrame::OnViewEfg(wxCommandEvent &)
{
  if (GetSizer()->IsShown(m_treeDisplay)) {
    GetMenuBar()->Check(GBT_MENU_VIEW_EFG, true);
    return;
  }
  
  GetSizer()->Show(m_treeDisplay, true);
  GetSizer()->Show(m_tablePanel, false);
  GetMenuBar()->Check(GBT_MENU_VIEW_NFG, false);
  Layout();
  OnUpdate();
}

void gbtGameFrame::OnViewNfg(wxCommandEvent &)
{
  if (GetSizer()->IsShown(m_tablePanel)) {
    GetMenuBar()->Check(GBT_MENU_VIEW_NFG, true);
    return;
  }

  // At this point, we know that there must be a tree view to hide
  GetSizer()->Show(m_treeDisplay, false);
  GetSizer()->Show(m_tablePanel, true);
  GetMenuBar()->Check(GBT_MENU_VIEW_EFG, false);
  Layout();
  OnUpdate();
}

void gbtGameFrame::OnViewZoomIn(wxCommandEvent &)
{
  m_doc->SetTreeZoom(m_doc->GetTreeZoom() * 1.1);
}

void gbtGameFrame::OnViewZoomOut(wxCommandEvent &)
{
  m_doc->SetTreeZoom(m_doc->GetTreeZoom() / 1.1);
}

void gbtGameFrame::OnToolsEquilibrium(wxCommandEvent &)
{
  if (GetSizer()->IsShown(m_qrePanel)) {
    GetSizer()->Show(m_qrePanel, false);
    GetMenuBar()->Check(GBT_MENU_TOOLS_QRE, false);
  }

  GetSizer()->Show(m_algorithmPanel,
		   !GetSizer()->IsShown(m_algorithmPanel));
  Layout();
}

void gbtGameFrame::OnToolsQre(wxCommandEvent &)
{
  if (GetSizer()->IsShown(m_algorithmPanel)) {
    GetSizer()->Show(m_algorithmPanel, false);
    GetMenuBar()->Check(GBT_MENU_TOOLS_EQM, false);
  }

  GetSizer()->Show(m_qrePanel, !GetSizer()->IsShown(m_qrePanel));
  Layout();
}

void gbtGameFrame::OnHelpAbout(wxCommandEvent &)
{
  gbtAboutDialog(this).ShowModal();
}


//-------------------------------------------------------------------------
//           gbtGameFrame: Implementation of view interface
//-------------------------------------------------------------------------

void gbtGameFrame::OnUpdate(void)
{
  if (m_doc->GetFilename() == "") {
    SetTitle(wxString::Format("Gambit: [<no file>%s] %s",
			      (m_doc->IsModified()) ? "*" : "",
			      m_doc->GetGame()->GetLabel().c_str()));
  }
  else {
    SetTitle(wxString::Format("Gambit: [%s%s] %s",
			      m_doc->GetFilename().c_str(),
			      (m_doc->IsModified()) ? "*" : "",
			      m_doc->GetGame()->GetLabel().c_str()));
  }

  GetMenuBar()->Enable(GBT_MENU_FILE_EXPORT,
		       m_treeDisplay && GetSizer()->IsShown(m_treeDisplay));
  GetMenuBar()->Enable(wxID_PRINT_SETUP,
		       m_treeDisplay && GetSizer()->IsShown(m_treeDisplay));
  GetMenuBar()->Enable(wxID_PREVIEW,
		       m_treeDisplay && GetSizer()->IsShown(m_treeDisplay));
  GetMenuBar()->Enable(wxID_PRINT,
		       m_treeDisplay && GetSizer()->IsShown(m_treeDisplay));

  GetMenuBar()->Enable(wxID_UNDO, m_doc->CanUndo());
  if (m_doc->CanUndo()) {
    GetMenuBar()->SetLabel(wxID_UNDO, "Undo " + m_doc->GetUndoDescription());
  }
  else {
    GetMenuBar()->SetLabel(wxID_UNDO, "Undo");
  }

  GetMenuBar()->Enable(wxID_REDO, m_doc->CanRedo());
  if (m_doc->CanRedo()) {
    GetMenuBar()->SetLabel(wxID_REDO, "Redo " + m_doc->GetRedoDescription());
  }
  else {
    GetMenuBar()->SetLabel(wxID_REDO, "Redo");
  }
  
  
  GetMenuBar()->Check(GBT_MENU_VIEW_EFG, 
		      m_treeDisplay && GetSizer()->IsShown(m_treeDisplay));
  GetMenuBar()->Enable(GBT_MENU_VIEW_EFG, m_doc->GetGame()->HasTree());
  GetMenuBar()->Check(GBT_MENU_VIEW_NFG, 
		      GetSizer()->IsShown(m_tablePanel));
  GetMenuBar()->Enable(GBT_MENU_VIEW_ZOOMIN,
		       m_treeDisplay && GetSizer()->IsShown(m_treeDisplay));
  GetMenuBar()->Enable(GBT_MENU_VIEW_ZOOMOUT,
		       m_treeDisplay && GetSizer()->IsShown(m_treeDisplay));
}