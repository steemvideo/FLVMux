//-----------------------------------------------------------------------------
//
//	MONOGRAM Multimedia FLV Mux
//
//
//-----------------------------------------------------------------------------
#pragma once


#define		FLV_CODEC_NONE		-1
#define		FLV_CODEC_FLV1		2			// Sorenson H.263
#define		FLV_CODEC_FLV4		4			// On2 VP6
#define		FLV_CODEC_H264		7			// H.264

#define		FLV_CODEC_MP3		2			// MP3
#define		FLV_CODEC_AAC		10			// AAC

//-----------------------------------------------------------------------------
//
//	ExtraData class
//
//-----------------------------------------------------------------------------

class ExtraData
{
public:

	// naparsovane extradata
	char		*data;
	int			size;

	int ReadFromWaveFormatEx(WAVEFORMATEX *wfx);
	void ReadBuf(void *buf, int s);
public:
	ExtraData();
	virtual ~ExtraData();

	void Free();

	// parsovanie
	virtual int Read(CMediaType *mt);

};

//-----------------------------------------------------------------------------
//
//	FLVIO class
//
//-----------------------------------------------------------------------------

class FLVIO
{
public:

	CComPtr<IStream>	stream;
	__int64				position;

public:
	FLVIO();
	virtual ~FLVIO();

	void Attach(IStream *str);
	void Detach();

	__int64 Write(BYTE *buffer, int size);
	__int64 Seek(__int64 offset);
	__int64 GetPosition();

};


//-----------------------------------------------------------------------------
//
//	FLVWriter class
//
//-----------------------------------------------------------------------------

class FLVWriter
{
public:
	
	int			video_codec;
	int			audio_codec;

	// info for the metadata
	int			video_width;
	int			video_height;
	double		video_fps;
	int			audio_channels;
	int			audio_samplerate;

	// time helpers
	bool		is_first;					
	__int64		time_first_ms;				// timestamp of the first packet
	__int64		time_last_ms;				// timestamp of the last packet
	__int64		duration_ms;				// calculated duration (for the onMetaData tag)
	__int64		video_raw_size;				// size of all video packets
	__int64		video_frames;				// total video frame count
	__int64		file_size;					

	int			last_tag_size;				// helper for writing FLV file

	// I/O object
	FLVIO		*io;			

	// metadata helpers
	__int64		metadatapos;				// byte position in the file

	ExtraData	video_extradata;
	ExtraData	audio_extradata;

public:
	FLVWriter();
	virtual ~FLVWriter();

	// stream config
	int Reset();
	int ConfigStream(CMediaType *pmt);
	int Start(FLVIO *flvio);
	int Stop();

	// write the data
	int WriteExtradata(GUID type);
	int WritePacket(CMuxInterPacket *packet, CMediaType *pmt);

	// metadata helpers
	int WriteMetaData();

	static int MakeAVCc(char* data, int size, char *output_data, int output_size);
};
