// wxWidgets "Hello World" Program

#include "CMakeWxHelloWorld.h"
#include "wx\dialog.h"

enum
{
    ID_Hello = 1,
    ID_SelFolder = 2,
    ID_SelFile = 3
};

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit()
{
    MyFrame *frame = new MyFrame();
    frame->Show(true);
    return true;
}

MyFrame::MyFrame()
    : wxFrame(NULL, wxID_ANY, "Hello World")
{
    wxMenu *menuFile = new wxMenu;
    menuFile->Append(ID_Hello, "&Hello...\tCtrl-H",
                     "Help string shown in status bar for this menu item");
    menuFile->AppendSeparator();
    menuFile->Append(ID_SelFolder, "Select Path");
    menuFile->Append(ID_SelFile, "Select File");
    menuFile->Append(wxID_EXIT);
    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuHelp, "&Help");
    SetMenuBar( menuBar );
    CreateStatusBar();
    SetStatusText("Welcome to wxWidgets!");
    Bind(wxEVT_MENU, &MyFrame::OnHello, this, ID_Hello);
    Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);
    Bind(wxEVT_MENU, &MyFrame::OnSelFolder, this, ID_SelFolder);
    Bind(wxEVT_MENU, &MyFrame::OnSelFile, this, ID_SelFile);
}

void MyFrame::OnExit(wxCommandEvent& event)
{
    Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("This is a wxWidgets Hello World example",
                 "About Hello World", wxOK | wxICON_INFORMATION);
}

void MyFrame::OnHello(wxCommandEvent& event)
{
    wxLogMessage("Hello world from wxWidgets!");
}

void MyFrame::OnSelFolder(wxCommandEvent& event)
{
    wxString s;
    wxDirDialog folder(NULL, "Select a folder", "\\\\.", wxDD_DIR_MUST_EXIST);
    if (folder.ShowModal() == wxID_OK) {
        s = folder.GetPath();
    }
    wxLogMessage(s);
}

void MyFrame::OnSelFile(wxCommandEvent& event)
{
    wxString s;
    wxFileDialog path(NULL, "Select a folder", "\\\\.");
    if (path.ShowModal() == wxID_OK) {
        s = path.GetPath();
    }
    wxLogMessage(s);
}
