//-----------------------------------------------------------------------------
//
//	MONOGRAM Multimedia FLV Mux
//
//
//-----------------------------------------------------------------------------
#pragma once

class CFLVMux;

//-----------------------------------------------------------------------------
//
//	CFLVMux class
//
//-----------------------------------------------------------------------------

class CFLVMux : 
	public CBaseMux,
	public ISpecifyPropertyPages,
	public IMonogramFLVMux
{
public:

	CCritSec			lock_mux;				// guard the muxer access
	FLVWriter			flv;					// our writer object
	FLVIO				flvio;					// I/O object for our writer

	CString				video_desc;
	CString				audio_desc;
	bool				has_video;
	bool				has_audio;

public:
	CFLVMux(LPUNKNOWN pUnk, HRESULT *phr);
	virtual ~CFLVMux();
	static CUnknown *WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *phr);

	DECLARE_IUNKNOWN;
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

	// ISpecifyPropertyPages interface
    STDMETHODIMP GetPages(CAUUID *pPages);

	// custom pins
	virtual int CreatePin(CBaseMuxInputPin **pin, LPCWSTR name);
	virtual HRESULT SetInputType(CBaseMuxInputPin *pin, const CMediaType *pmt);
	virtual HRESULT BreakInputConnect(CBaseMuxInputPin *pin);

	// handling media types
	virtual HRESULT CheckInputType(const CMediaType *pmt);

	// handle the file
	virtual int OnStartStreaming();
	virtual int OnStopStreaming();
	virtual int OnMuxPacket(CMuxInterPacket *packet, CBaseMuxInputPin *inpin);

	// IMonogramFLVMux
	STDMETHODIMP GetMuxInfo(FLV_Mux_Info *info);
};
