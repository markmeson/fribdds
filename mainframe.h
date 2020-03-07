#ifndef MAINFRAME_H
#define MAINFRAME_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif


enum
{
	ID_Open = 1,
	ID_Close = 2,
	ID_Show = 3
};

class mainframe : public wxFrame {
	public:
    mainframe(const wxString& title, const wxPoint& pos, const wxSize& size);
		void setstatus(const char *st);
		void open(const char *path, const char *f = "");
	private:
    void OnClose(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnMotion(wxMouseEvent& event);
    void OnOpen(wxCommandEvent& event);
    void OnShow(wxCommandEvent& event);
    void OnRepaint(wxPaintEvent& event);
    void OnResize(wxSizeEvent& event);
		void close();
    wxDECLARE_EVENT_TABLE();
		wxMenu *menuFile;
		wxMenu *menuView;
		wxImage m_img;
		wxBitmap m_bmp;
		bool m_bImageset;
		int m_idrawx;
		int m_idrawy;
		int m_imousex;
		int m_imousey;
};

#endif
