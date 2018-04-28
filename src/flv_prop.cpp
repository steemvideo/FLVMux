//-----------------------------------------------------------------------------
//
//	MONOGRAM Multimedia FLV Mux
//
//
//-----------------------------------------------------------------------------
#include "stdafx.h"


void MakeNiceSize(__int64 size, CString &v)
{
	int r=0;
	__int64	c=size;
	LPCTSTR		rady[] = {
		_T("bytes"),
		_T("KB"),
		_T("MB"),
		_T("GB"),
		_T("TB")
	};

	// spocitame rad
	while (c > 1024 && r<4) {
		r++;
		c = c / 1024;
	}

	c=size;
	for (int i=1; i<r; i++) { c = c/1024; }
	double d=c / 1024.0;

	v.Format(_T("%5.3f %s"), (float)d, rady[r]);
}

void MakeNiceSpeed(__int64 bps, CString &v)
{
	int r=0;
	__int64	c=bps;
	LPCTSTR		rady[] = {
		_T("bps"),
		_T("kbps"),
		_T("mbps"),
		_T("gbps"),
		_T("tbps")
	};

	// spocitame rad
	while (c > 1000 && r<4) {
		r++;
		c = c / 1000;
	}

	c=bps;
	for (int i=1; i<r; i++) { c = c/1000; }
	double d=c / 1000.0;

	v.Format(_T("%5.3f %s"), (float)d, rady[r]);
}

void MakeNiceTime(int64 time_ms, CString &v)
{
	int64		ms = time_ms%1000;	
	time_ms -= ms;
	time_ms /= 1000;

	int		h, m, s;
	h = time_ms / 3600;		time_ms -= h*3600;
	m = time_ms / 60;		time_ms -= m*60;
	s = time_ms;

	v.Format(_T("%.2d:%.2d:%.2d,%.3d"), h, m, s, ms);
}


#define MAX_TEXT_LENGTH			1024
#define WM_UPDATE_VISUALS		(WM_USER + 1003)
#define ITEM(x)					(GetDlgItem(m_Dlg, x))

//-----------------------------------------------------------------------------
//
//	CFLVPage class
//
//-----------------------------------------------------------------------------

CUnknown *CFLVPage::CreateInstance(LPUNKNOWN lpUnk, HRESULT *phr)
{
	return new CFLVPage(lpUnk, phr);
}

CFLVPage::CFLVPage(LPUNKNOWN lpUnk, HRESULT *phr) :
	CBasePropertyPage(NAME("FLV Mux Property Page"), lpUnk, IDD_PAGE_FLV, IDS_PAGE_FLV),
	mux(NULL)
{
}

CFLVPage::~CFLVPage()
{
}

HRESULT CFLVPage::OnConnect(IUnknown *pUnknown)
{
	ASSERT(!mux);

	HRESULT hr = pUnknown->QueryInterface(IID_IMonogramFLVMux, (void**)&mux);
	if (FAILED(hr)) return hr;

	// okej
	return NOERROR;
}

HRESULT CFLVPage::OnDisconnect()
{
	if (mux) {
		mux->Release();
		mux = NULL;
	}
	return NOERROR;
}

HRESULT CFLVPage::OnActivate()
{
	SetTimer(m_Dlg, 0, 300, NULL);
	Update();
	return NOERROR;
}

HRESULT CFLVPage::OnDeactivate()
{
	KillTimer(m_Dlg, 0);
	return NOERROR;
}

INT_PTR CFLVPage::OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_TIMER:
		{
			Update();
		}
		break;
	}
	return __super::OnReceiveMessage(hwnd, uMsg, wParam, lParam);
}

void CFLVPage::Update()
{
	if (!mux) return ;

	FLV_Mux_Info		info;
	mux->GetMuxInfo(&info);

	// stream descriptions
	Static_SetText(ITEM(IDC_STATIC_AUDIO), info.audio_stream);
	Static_SetText(ITEM(IDC_STATIC_VIDEO), info.video_stream);

	double	rate = 0;
	if (info.duration_sec > 0) {
		rate = info.file_size * 8.0 / info.duration_sec;
	}

	CString		s;
	MakeNiceSpeed((__int64)rate, s);
	Static_SetText(ITEM(IDC_STATIC_DATARATE), s);

	// duration
	__int64 ms = (__int64)(info.duration_sec * 1000.0);
	MakeNiceTime(ms, s);
	Static_SetText(ITEM(IDC_STATIC_DURATION), s);

	// size
	MakeNiceSize(info.file_size, s);
	Static_SetText(ITEM(IDC_STATIC_SIZE), s);
}

