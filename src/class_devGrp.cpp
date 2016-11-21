#include "class_devGrp.h"

#include "res/check2.xpm"
#include "res/check3.xpm"

BEGIN_EVENT_TABLE(DevGrp, wxGrid)
END_EVENT_TABLE()

DevGrp::DevGrp(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
:wxGrid(parent, id, pos, size, style, name)
{
	chkStatus = false;
}

DevGrp::~DevGrp(void)
{
}


void DevGrp::DrawColLabel( wxDC& dc, int col )
{
	if ( GetColWidth(col) <= 0 || m_colLabelHeight <= 0 )
		return;

	int colLeft = GetColLeft(col);

	wxRect rect;

#if 0
	def __WXGTK20__
		rect.SetX( colLeft + 1 );
	rect.SetY( 1 );
	rect.SetWidth( GetColWidth(col) - 2 );
	rect.SetHeight( m_colLabelHeight - 2 );

	wxWindowDC *win_dc = (wxWindowDC*) &dc;

	wxRendererNative::Get().DrawHeaderButton( win_dc->m_owner, dc, rect, 0 );
#else
	int colRight = GetColRight(col) - 1;

	dc.SetPen( wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DSHADOW), 1, wxSOLID) );
	dc.DrawLine( colRight, 0, colRight, m_colLabelHeight - 1 );
	dc.DrawLine( colLeft, 0, colRight, 0 );
	dc.DrawLine( colLeft, m_colLabelHeight - 1,
		colRight + 1, m_colLabelHeight - 1 );

	dc.SetPen( *wxWHITE_PEN );
	dc.DrawLine( colLeft, 1, colLeft, m_colLabelHeight - 1 );
	dc.DrawLine( colLeft, 1, colRight, 1 );
#endif

	if (col == 0)
	{
		if (chkStatus)
		{
			dc.DrawBitmap(wxBitmap(check3_xpm), wxPoint(1, m_colLabelHeight/2-8), true);
		}
		else
		{
			dc.DrawBitmap(wxBitmap(check2_xpm), wxPoint(1, m_colLabelHeight/2-8), true);
		}
	}

	dc.SetBackgroundMode( wxTRANSPARENT );
	dc.SetTextForeground( GetLabelTextColour() );
	dc.SetFont( GetLabelFont() );

	int hAlign, vAlign, orient;
	GetColLabelAlignment( &hAlign, &vAlign );
	orient = GetColLabelTextOrientation();

	rect.SetX( colLeft + 2 );
	rect.SetY( 2 );
	rect.SetWidth( GetColWidth(col) - 4 );
	rect.SetHeight( m_colLabelHeight - 4 );
	DrawTextRectangle( dc, GetColLabelValue( col ), rect, hAlign, vAlign, orient );

}


void DevGrp::unCheck(bool zuruecksetzen)
{
	if (zuruecksetzen)
	{
		chkStatus = false;
	}
	else
	{
		chkStatus = !chkStatus;
		this->Refresh();
	}
}


bool DevGrp::getCheckStatus()
{
	return chkStatus;
}