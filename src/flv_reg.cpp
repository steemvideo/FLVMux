//-----------------------------------------------------------------------------
//
//	MONOGRAM Multimedia FLV Mux
//
//
//-----------------------------------------------------------------------------
#include "stdafx.h"

#pragma warning(disable: 4996)

//-----------------------------------------------------------------------------
//
//	Registry Information
//
//-----------------------------------------------------------------------------


const AMOVIESETUP_MEDIATYPE sudPinTypes[] = { 
	{ &MEDIATYPE_Video,  &MEDIASUBTYPE_FLV1 },
	{ &MEDIATYPE_Video,  &MEDIASUBTYPE_FLV4 },
	{ &MEDIATYPE_Video,  &MEDIASUBTYPE_AVC },
	{ &MEDIATYPE_Audio,	 &MEDIASUBTYPE_MP3 },
	{ &MEDIATYPE_Audio,	 &MEDIASUBTYPE_AAC },
	{ &MEDIATYPE_Stream, &GUID_NULL }
};

const AMOVIESETUP_PIN psudPins[] = { 
	{ L"In",  FALSE, FALSE, FALSE, TRUE,  &CLSID_NULL, NULL, 5, &sudPinTypes[0] }, 
	{ L"Out", FALSE, TRUE,  FALSE, FALSE, &CLSID_NULL, NULL, 1, &sudPinTypes[5]	} 
};   

const AMOVIESETUP_FILTER sudFLVMux = { 
	&CLSID_MonogramFLVMux, L"MONOGRAM FLV Mux", MERIT_NORMAL, 2, psudPins
};                     

CFactoryTemplate g_Templates[]=	{
	{ L"MONOGRAM FLV Mux", &CLSID_MonogramFLVMux, CFLVMux::CreateInstance, NULL, &sudFLVMux },
	{ L"MONOGRAM FLV Mux", &CLSID_MonogramFLVPage, CFLVPage::CreateInstance }
};


int g_cTemplates = sizeof(g_Templates)/sizeof(g_Templates[0]);

//-----------------------------------------------------------------------------
//
//	DLL Entry Points
//
//-----------------------------------------------------------------------------

STDAPI DllRegisterServer() 
{
	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
	return AMovieDllRegisterServer2(FALSE);
}

