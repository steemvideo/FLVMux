//-----------------------------------------------------------------------------
//
//	MONOGRAM Multimedia FLV Mux
//
//
//-----------------------------------------------------------------------------

// {C1FB436D-3A1A-4f7b-8D69-B8B8BC652718}
static const GUID CLSID_MonogramFLVMux = 
{ 0xc1fb436d, 0x3a1a, 0x4f7b, { 0x8d, 0x69, 0xb8, 0xb8, 0xbc, 0x65, 0x27, 0x18 } };

// {314382F7-2122-408f-A315-35353CADD93B}
static const GUID IID_IMonogramFLVMux = 
{ 0x314382f7, 0x2122, 0x408f, { 0xa3, 0x15, 0x35, 0x35, 0x3c, 0xad, 0xd9, 0x3b } };

// {E37411E7-8EE1-4471-AAC7-40AE4FF4677A}
static const GUID CLSID_MonogramFLVPage = 
{ 0xe37411e7, 0x8ee1, 0x4471, { 0xaa, 0xc7, 0x40, 0xae, 0x4f, 0xf4, 0x67, 0x7a } };


/*
	List of our supported media types
*/

// {000000FF-0000-0010-8000-00AA00389B71}
static const GUID MEDIASUBTYPE_AAC =
{ 0x000000ff, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };

// {00000055-0000-0010-8000-00AA00389B71}
static const GUID MEDIASUBTYPE_MP3 =
{ 0x00000055, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };

// {31435641-0000-0010-8000-00AA00389B71}
static const GUID MEDIASUBTYPE_AVC =
{ 0x31435641, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };

// {31564C46-0000-0010-8000-00AA00389B71}
static const GUID MEDIASUBTYPE_FLV1 =
{ 0x31564C46, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };

// {34564C46-0000-0010-8000-00AA00389B71}
static const GUID MEDIASUBTYPE_FLV4 =
{ 0x34564C46, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };


struct FLV_Mux_Info
{
	TCHAR		audio_stream[256];				// description string
	TCHAR		video_stream[256];
	double		duration_sec;					// written so far
	__int64		file_size;						// written so far
};

DECLARE_INTERFACE_(IMonogramFLVMux, IUnknown)
{
	STDMETHOD(GetMuxInfo)(FLV_Mux_Info *info);
};
