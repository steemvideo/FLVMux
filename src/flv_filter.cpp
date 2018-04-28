//-----------------------------------------------------------------------------
//
//	MONOGRAM Multimedia FLV Mux
//
//
//-----------------------------------------------------------------------------
#include "stdafx.h"

#pragma warning(disable: 4018)		// signed/usigned mismatch
#pragma warning(disable: 4267)		// '=' : conversion from 'size_t' to '....'


//-----------------------------------------------------------------------------
//
//	CFLVMux class
//
//-----------------------------------------------------------------------------

CFLVMux::CFLVMux(LPUNKNOWN pUnk, HRESULT *phr) :
	CBaseMux(_T("FLV Mux"), pUnk, phr, CLSID_MonogramFLVMux)
{
	output = new CBaseMuxOutputPin(this, phr, L"Out");

	has_video = false;
	has_audio = false;

	// we only want two pins
	AddPin();			
	AddPin();			
}

CFLVMux::~CFLVMux()
{
}

CUnknown *CFLVMux::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
	return new CFLVMux(pUnk, phr);
}

STDMETHODIMP CFLVMux::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
	if (riid == IID_ISpecifyPropertyPages) {
		return GetInterface((ISpecifyPropertyPages*)this, ppv);
	} else
	if (riid == IID_IMonogramFLVMux) {
		return GetInterface((IMonogramFLVMux*)this, ppv);
	} else
		return __super::NonDelegatingQueryInterface(riid, ppv);
}

STDMETHODIMP CFLVMux::GetPages(CAUUID *pPages)
{
    CheckPointer(pPages,E_POINTER);

    pPages->cElems = 1;
    pPages->pElems = (GUID *) CoTaskMemAlloc(pPages->cElems * sizeof(GUID));
    if (pPages->pElems == NULL) {
        return E_OUTOFMEMORY;
    }

	*(pPages->pElems) = CLSID_MonogramFLVPage;
    return NOERROR;
}

int CFLVMux::CreatePin(CBaseMuxInputPin **pin, LPCWSTR name)
{
	if (!pin) return -1;

	if (pins.size() >= 2) {
		// we don't want to have any more pins... 2 is perfectly enough for us
		return -1;
	}

	HRESULT				hr = NOERROR;
	CBaseMuxInputPin	*new_pin = new CBaseMuxInputPin(_T("InPin"), this, &hr, name);

	if (!new_pin) {	*pin = NULL; return -1;	}
	*pin = new_pin;
	return 0;
}

HRESULT CFLVMux::CheckInputType(const CMediaType *pmt)
{
	/*
		We only support a few types...
	*/

	if (pmt->majortype == MEDIATYPE_Video) {
		if (has_video) return -1;

		if (pmt->subtype == MEDIASUBTYPE_FLV1 ||
			pmt->subtype == MEDIASUBTYPE_FLV4 ||
			pmt->subtype == MEDIASUBTYPE_AVC
/*
			||
			pmt->subtype == MEDIASUBTYPE_VP60 ||
			pmt->subtype == MEDIASUBTYPE_VP61 ||
			pmt->subtype == MEDIASUBTYPE_VP62
			*/
			) return 0;
	} else
	if (pmt->majortype == MEDIATYPE_Audio) {
		if (has_audio) return -1;

		if (pmt->subtype == MEDIASUBTYPE_MP3 ||
			pmt->subtype == MEDIASUBTYPE_AAC) return 0;
	}
	return -1;
}

HRESULT CFLVMux::SetInputType(CBaseMuxInputPin *pin, const CMediaType *pmt)
{
	HRESULT		hr = __super::SetInputType(pin, pmt);
	if (FAILED(hr)) return hr;

	CString		codec;

	// we only want one of each kind
	if (pmt->majortype == MEDIATYPE_Video) { 
		has_video = true; 

		if (pmt->subtype == MEDIASUBTYPE_FLV1)	codec = _T("FLV1"); else
		if (pmt->subtype == MEDIASUBTYPE_AVC)	codec = _T("H.264"); else
		if (pmt->subtype == MEDIASUBTYPE_FLV4)	codec = _T("FLV4"); 
		/*
		else
		if (pmt->subtype == MEDIASUBTYPE_VP60)	codec = _T("VP60"); else
		if (pmt->subtype == MEDIASUBTYPE_VP61)	codec = _T("VP61"); else
		if (pmt->subtype == MEDIASUBTYPE_VP62)	codec = _T("VP62");
		*/

		int		width, height;

		// parse the video size
		if (pmt->formattype == FORMAT_VideoInfo) {
			VIDEOINFOHEADER		*vih = (VIDEOINFOHEADER*)pmt->pbFormat;
			width = vih->bmiHeader.biWidth;
			height = vih->bmiHeader.biHeight;
		} else
		if (pmt->formattype == FORMAT_VideoInfo2) {
			VIDEOINFOHEADER2	*vih = (VIDEOINFOHEADER2*)pmt->pbFormat;
			width = vih->bmiHeader.biWidth;
			height = vih->bmiHeader.biHeight;
		} else
		if (pmt->formattype == FORMAT_MPEG2Video) {
			MPEG2VIDEOINFO		*mvi = (MPEG2VIDEOINFO*)pmt->pbFormat;
			width = mvi->hdr.bmiHeader.biWidth;
			height = mvi->hdr.bmiHeader.biHeight;
		} else
		if (pmt->formattype == FORMAT_MPEGVideo) {
			MPEG1VIDEOINFO		*mvi = (MPEG1VIDEOINFO*)pmt->pbFormat;
			width = mvi->hdr.bmiHeader.biWidth;
			height = mvi->hdr.bmiHeader.biHeight;
		} 

		// and assemble the description string
		video_desc.Format(_T("%s, %dx%d"), codec.GetBuffer(), width, height);

	} else
	if (pmt->majortype == MEDIATYPE_Audio) { 
		has_audio = true; 

		if (pmt->subtype == MEDIASUBTYPE_MP3) codec = _T("MP3"); else 
		if (pmt->subtype == MEDIASUBTYPE_AAC) codec = _T("AAC");

		int		channels = 0;
		int		samplerate = 0;

		// audio parameters
		if (pmt->formattype == FORMAT_WaveFormatEx) {
			WAVEFORMATEX		*wfx = (WAVEFORMATEX*)pmt->pbFormat;
			channels	 = wfx->nChannels;
			samplerate = wfx->nSamplesPerSec;
		} 

		// FLV can only have stereo sound
		if (channels <= 0 || channels > 2) return -1;

		// FLV can only support several sample rates
		if (pmt->subtype == MEDIASUBTYPE_MP3) {
			if (samplerate != 44100 &&
				samplerate != 22050 &&
				samplerate != 11025 &&
				samplerate !=  5500
				) return -1;
		} else 
		if (pmt->subtype == MEDIASUBTYPE_AAC) {
			/*
				The specification says:

				If the SoundFormat indicates AAC, the SoundType should be set to 1 (stereo) and the
				SoundRate should be set to 3 (44 kHz). However, this does not mean that AAC audio in FLV
				is always stereo, 44 kHz data. Instead, the Flash Player ignores these values and extracts the
				channel and sample rate data is encoded in the AAC bitstream.
			*/
		}

		// and assemble the description string
		audio_desc.Format(_T("%s, %5.3f KHz"), codec.GetBuffer(), (samplerate / 1000.0));

		if (channels == 1)	audio_desc += _T(" @ mono"); else
		if (channels == 2)	audio_desc += _T(" @ stereo");
	}

	return NOERROR;
}

HRESULT CFLVMux::BreakInputConnect(CBaseMuxInputPin *pin)
{
	HRESULT		hr = __super::BreakInputConnect(pin);
	if (FAILED(hr)) return hr;

	has_video = false;
	has_audio = false;

	for (int i=0; i<pins.size(); i++) {
		CBaseMuxInputPin	*inpin = pins[i];
		if ((pin != inpin) && inpin->IsConnected()) {
			CMediaType	&mt = inpin->CurrentMediaType();

			if (mt.majortype == MEDIATYPE_Video)	has_video = true;
			if (mt.majortype == MEDIATYPE_Audio)	has_audio = true;
		}
	}

	return NOERROR;
}

int CFLVMux::OnMuxPacket(CMuxInterPacket *packet, CBaseMuxInputPin *inpin)
{
	CAutoLock	lck(&lock_mux);

	if (flv.io) {
		CMediaType	&mt = inpin->CurrentMediaType();
		flv.WritePacket(packet, &mt);
	}
	return 0;
}

int CFLVMux::OnStartStreaming()
{
	/*
		Restart the FLV Muxer instance
	*/
	
	CAutoLock	lck(&lock_mux);

	if (!outstream) return -1;

	flv.Reset();
	flvio.Detach();

	// configure each stream
	for (int i=0; i<2; i++) {
		if (pins[i]->IsConnected()) {
			CMediaType	&mt = pins[i]->CurrentMediaType();
			flv.ConfigStream(&mt);
		}
	}

	// and write something ...
	flvio.Attach(outstream);
	flv.Start(&flvio);

	return 0;
}

int CFLVMux::OnStopStreaming()
{
	CAutoLock	lck(&lock_mux);

	/*
		If there is an active muxer, finish the file.
	*/

	if (flv.io) {
		flv.Stop();
		flvio.Detach();
		flv.Reset();
	}

	return 0;
}
	
STDMETHODIMP CFLVMux::GetMuxInfo(FLV_Mux_Info *info)
{
	CAutoLock	lck(&lock_mux);

	if (!info) return E_POINTER;

	// reset everything
	memset(info, 0, sizeof(FLV_Mux_Info));

	_stprintf(info->audio_stream, _T("None"));
	_stprintf(info->video_stream, _T("None"));

	if (has_video) {
		_stprintf(info->video_stream, _T("%s"), video_desc.GetBuffer());
	}
	if (has_audio) {
		_stprintf(info->audio_stream, _T("%s"), audio_desc.GetBuffer());	
	}
	
	info->duration_sec = (double)(flv.duration_ms / 1000.0);
	info->file_size = flv.file_size;

	return 0;
}


