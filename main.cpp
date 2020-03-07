// frdds -- A cross platform dds viewer by Frib
//

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include "mainframe.h"

mainframe *frame;
char buffer[2048]; //buffer for entire program

class fribddsApp: public wxApp {
	public:
    virtual bool OnInit();
};

wxIMPLEMENT_APP(fribddsApp);

bool fribddsApp::OnInit()
{
    frame = new mainframe( "Frib DDS Viewer", wxPoint(50, 50), wxSize(300, 300) );
    frame->Show( true );
    return true;
}
