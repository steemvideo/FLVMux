//-----------------------------------------------------------------------------
//
//	MONOGRAM Multimedia FLV Mux
//
//
//-----------------------------------------------------------------------------
#pragma once


//-----------------------------------------------------------------------------
//
//	CFLVPage class
//
//-----------------------------------------------------------------------------
class CFLVPage : public CBasePropertyPage
{
public:

	// mux
	IMonogramFLVMux		*mux;

public:
	CFLVPage(LPUNKNOWN lpUnk, HRESULT *phr);
	virtual ~CFLVPage();
	static CUnknown *WINAPI CreateInstance(LPUNKNOWN lpUnk, HRESULT *phr);

	// CBasePropertyPage
	HRESULT OnConnect(IUnknown *pUnknown);
	HRESULT OnDisconnect();
	HRESULT OnActivate();
	HRESULT OnDeactivate();

	// message
	INT_PTR OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void Update();
};


