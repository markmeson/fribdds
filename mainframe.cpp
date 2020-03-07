#include "mainframe.h"
#include "dds.h"

extern char buffer[2048];

dds maindds;
int imousex, imousey;

wxBEGIN_EVENT_TABLE(mainframe, wxFrame)
    EVT_MENU(ID_Open,   mainframe::OnOpen)
    EVT_MENU(ID_Close, mainframe::OnClose)
    EVT_MENU(ID_Show, mainframe::OnShow)
    EVT_MENU(wxID_EXIT,  mainframe::OnExit)
		EVT_SIZE(mainframe::OnResize)
		EVT_PAINT(mainframe::OnRepaint)
		EVT_MOTION(mainframe::OnMotion)
wxEND_EVENT_TABLE()

mainframe::mainframe(const wxString& title, const wxPoint& pos, const wxSize& size)
        : wxFrame(NULL, wxID_ANY, title, pos, size)
{
    menuFile = new wxMenu;
    menuFile->Append(ID_Open, "&Open...\tCtrl-O", "Open a DDS File");
    menuFile->Append(ID_Close, "&Close...\tCtrl-X", "Close DDS File");
		menuFile->Enable(ID_Close, 0);
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);

		menuView = new wxMenu;
		menuView->Append(ID_Show, "&Show header info", "Show the header info for the currently loaded image");
		menuView->Enable(ID_Show, 0);

    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
		menuBar->Append(menuView, "&View");
    SetMenuBar(menuBar);
    CreateStatusBar();
    SetStatusText("Welcome to Frib DDS Viewer!");
		SetMinClientSize(wxSize(300, 300));
		SetMaxClientSize(wxSize(300, 300));

		m_idrawx = 0;
		m_idrawy = 0;
}

void mainframe::OnExit(wxCommandEvent& event) {
	Close(true);
}

void mainframe::OnClose(wxCommandEvent& event) {
	close();
}

void mainframe::close() {
	maindds.close();
	menuFile->Enable(ID_Close, 0);
	menuView->Enable(ID_Show, 0);
	m_bImageset = false;
	SetMinClientSize(wxSize(300, 300));
	SetMaxClientSize(wxSize(300, 300));
	SetLabel("Frib DDS Viewer");
}

void mainframe::OnMotion(wxMouseEvent& event) {
	event.GetPosition(&m_imousex, &m_imousey);
	if(m_bImageset) {
		sprintf(buffer, "%d, %d", m_imousex - m_idrawx, m_imousey - m_idrawy);
		SetStatusText(buffer);
	}
}

void mainframe::OnOpen(wxCommandEvent& event) {
	wxFileDialog fdlg(this, "Open DDS File", "", "", "DDS files (*.dds;*.DDS)|*.dds;*.DDS", wxFD_OPEN|wxFD_FILE_MUST_EXIST);

	if (fdlg.ShowModal() == wxID_CANCEL) {
		SetStatusText("File open canceled...");
		return;	
	}
	open(fdlg.GetPath(), fdlg.GetFilename());
}

void mainframe::open(const char *path, const char *f) {
	if(!maindds.setfile(path, f)) {
		wxLogMessage("Error setting dds file name");
		return;
	}
	int openresult;
	if((openresult = maindds.open())) {
		switch(openresult) {
			case FILENOTFOUND:
				wxLogMessage("DDS File Not Found!");
				break;
			case FILEOPENERR:
				wxLogMessage("Error opening DDS File!");
				break;
			case INVALIDFILE:
				wxLogMessage("File is not a valid DDS file!");
				break;
			case UNSUPPFMT:
				wxLogMessage("DDS Type Not Supported! See File Info (View -> File Info)");
				menuView->Enable(ID_Show, 1);
				break;
		}
		return;
	}

	//open image succeeded
	DDS_HEADER *hdr = maindds.getheader();

	m_bImageset = true;
	int pxlsize = maindds.getpxlsize();
	if(pxlsize == 2) {
		if(maindds.isalpha())
			m_img = wxImage(hdr->dwWidth, hdr->dwHeight, (unsigned char *)maindds.getimgdata(), (unsigned char *)maindds.getalphadata());
		else
			m_img = wxImage(hdr->dwWidth, hdr->dwHeight, (unsigned char *)maindds.getimgdata());
	} else if(pxlsize == 3)
		m_img = wxImage(hdr->dwWidth, hdr->dwHeight, (unsigned char *)maindds.getimgdata());
	else if(pxlsize == 4)
		m_img = wxImage(hdr->dwWidth, hdr->dwHeight, (unsigned char *)maindds.getimgdata(), (unsigned char *)maindds.getalphadata());
	else {
		m_bImageset = false;
		printf("Unacceptable pixel size (%d), only loading image info, not the image itself\n", pxlsize);
	}

	if(m_bImageset) {
		if(pxlsize == 4)
			m_bmp = wxBitmap(m_img, 4 * 8);
		else
			m_bmp = wxBitmap(m_img, 3 * 8);

		wxPaintDC dc(this);
		dc.DrawBitmap(m_bmp, 0, 0, true);

		SetMinClientSize(wxSize(0, 0));
		if(hdr->dwWidth > 500 && hdr->dwHeight > 500) {
			SetMaxClientSize(wxSize(hdr->dwWidth, hdr->dwHeight));
			SetMinClientSize(wxSize(hdr->dwWidth, hdr->dwHeight));
			m_idrawx = m_idrawy = 0;
		} else {
			SetMaxClientSize(wxSize(500, 500));
			SetMinClientSize(wxSize(500, 500));
			m_idrawx = 250 - hdr->dwWidth / 2;
			m_idrawy = 250 - hdr->dwHeight / 2;
		}
	} else {
		SetMaxClientSize(wxSize(300, 300));
		SetMinClientSize(wxSize(300, 300));
	}
	sprintf(buffer, "Frib DDS Viewer - %s", static_cast<const char *>(strlen(f) > 0 ? f : path));
	SetLabel(buffer);
	menuFile->Enable(ID_Close, 1);
	menuView->Enable(ID_Show, 1);
}

void mainframe::setstatus(const char *st) {
	SetStatusText(st);
}

void mainframe::OnShow(wxCommandEvent& event) {
	maindds.showinfo();
}

void mainframe::OnResize(wxSizeEvent& event) {
	if(!m_bImageset) {
		wxPaintDC dc(this);
		dc.Clear();
		return;
	}
	wxPaintDC dc(this);
	dc.DrawBitmap(m_bmp, 0, 0, true);
}

void mainframe::OnRepaint(wxPaintEvent& event) {
	if(!m_bImageset) {
		wxPaintDC dc(this);
		dc.Clear();
		return;
	}
	wxPaintDC dc(this);
	dc.DrawBitmap(m_bmp, m_idrawx, m_idrawy, true);
}
