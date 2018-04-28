//-----------------------------------------------------------------------------
//
//	MONOGRAM Multimedia FLV Mux
//
//
//-----------------------------------------------------------------------------
#include "stdafx.h"


//-------------------------------------------------------------------------
//
//	ExtraData
//
//-------------------------------------------------------------------------

ExtraData::ExtraData() :
	data(NULL),
	size(0)
{
	// nothing more
}

ExtraData::~ExtraData()
{
	Free();
}

void ExtraData::Free()
{
	if (data) free(data);
	data = NULL;
	size = 0;
}

void ExtraData::ReadBuf(void *buf, int s)
{
	data = (char*)malloc(s);
	size = s;
	memcpy(data, buf, size);
}


int ExtraData::Read(CMediaType *mt)
{
	Free();

	if (mt->formattype == FORMAT_WaveFormatEx) {
		return ReadFromWaveFormatEx((WAVEFORMATEX*)mt->pbFormat);
	} else
	if (mt->formattype == FORMAT_VideoInfo) {
		
		size = mt->cbFormat - sizeof(VIDEOINFOHEADER);
		if (size <= 0) {
			size = 0;
			data = NULL;
		} else {
			ReadBuf((char*)mt->pbFormat + sizeof(VIDEOINFOHEADER), size);
		}
		return 0;

	} else
	if (mt->formattype == FORMAT_VideoInfo2) {

		size = mt->cbFormat - sizeof(VIDEOINFOHEADER2);
		if (size <= 0) {
			size = 0;
			data = NULL;
		} else {
			ReadBuf((char*)mt->pbFormat + sizeof(VIDEOINFOHEADER2), size);
		}
		return 0;

	} else
	if (mt->formattype == FORMAT_MPEGVideo) {

		MPEG1VIDEOINFO	*mi = (MPEG1VIDEOINFO*)mt->pbFormat;
		size = mi->cbSequenceHeader;
		if (size <= 0) {
			size = 0;
			data = NULL;
		} else {
			ReadBuf((char*)mi->bSequenceHeader, size);
		}
		return 0;

	} else
	if (mt->formattype == FORMAT_MPEG2Video) {

		MPEG2VIDEOINFO	*mi = (MPEG2VIDEOINFO*)mt->pbFormat;
		size = mi->cbSequenceHeader;
		if (size <= 0) {
			size = 0;
			data = NULL;
		} else {
			ReadBuf((char*)mi->dwSequenceHeader, size);
		}
		return 0;

	}

	// error
	return -1;
}

int ExtraData::ReadFromWaveFormatEx(WAVEFORMATEX *wfx)
{
	Free();

	// najdeme data
	size = wfx->cbSize;
	if (size <= 0) return 0;

	char *src = ((char*)wfx) + sizeof(WAVEFORMATEX);

	data = (char*)malloc(size);
	memcpy(data, src, size);

	// vsetko je okej
	return 0;
}

//-----------------------------------------------------------------------------
//
//	FLVIO class
//
//-----------------------------------------------------------------------------

FLVIO::FLVIO()
{
	stream = NULL;
}

FLVIO::~FLVIO()
{
	Detach();
}

void FLVIO::Attach(IStream *str)
{
	stream = NULL;
	stream = str;
}

void FLVIO::Detach()
{
	stream = NULL;
}

__int64 FLVIO::Write(BYTE *buffer, int size)
{
	if (!stream) return 0;

	ULONG	ret;
	stream->Write((void*)buffer, (ULONG)size, &ret);
	return ret;
}

__int64 FLVIO::Seek(__int64 offset)
{
	if (!stream) return (__int64)-1;

	LARGE_INTEGER		pos;
	ULARGE_INTEGER		newpos;
	HRESULT				hr;

	pos.QuadPart = offset;

	hr = stream->Seek(pos, 0, &newpos);
	if (FAILED(hr)) return (uint64)-1;

	return newpos.QuadPart;
}

__int64 FLVIO::GetPosition()
{
	if (!stream) return (__int64)-1;

	LARGE_INTEGER		pos;
	ULARGE_INTEGER		newpos;

	pos.QuadPart = 0;
	stream->Seek(pos, STREAM_SEEK_CUR, &newpos);
	return newpos.QuadPart;
}

//-----------------------------------------------------------------------------
//
//	FLVWriter class
//
//-----------------------------------------------------------------------------

FLVWriter::FLVWriter() :
	io(NULL)
{
	video_codec = FLV_CODEC_NONE;
	audio_codec = FLV_CODEC_NONE;

	video_width = 0;
	video_height = 0;
	video_raw_size = 0;
	time_first_ms = 0;
	time_last_ms = 0;
	video_frames = 0;
	audio_channels = 0;
	audio_samplerate = 0;
	is_first = true;
	duration_ms = 0;
	video_fps = 0;
	file_size = 0;
}

FLVWriter::~FLVWriter()
{
	Reset();
}

int FLVWriter::Reset()
{
	// reset all internal states
	video_width = 0;
	video_height = 0;
	video_raw_size = 0;
	video_frames = 0;
	audio_channels = 0;
	audio_samplerate = 0;
	last_tag_size = 0;
	video_fps = 0;
	file_size = 0;

	// no streams...
	video_codec = FLV_CODEC_NONE;
	audio_codec = FLV_CODEC_NONE;

	time_first_ms = 0;
	time_last_ms = 0;
	duration_ms = 0;
	is_first = true;

	io = NULL;

	return 0;
}

int FLVWriter::ConfigStream(CMediaType *pmt)
{
	/*
		now we try to detect the stream format.
		there's a limitation - there can be only one instance of each stream type (audio, video...)
	*/

	if (pmt->majortype == MEDIATYPE_Video) {

		int	codec = FLV_CODEC_NONE;

		if (pmt->subtype == MEDIASUBTYPE_FLV1)	codec = FLV_CODEC_FLV1; else
		if (pmt->subtype == MEDIASUBTYPE_FLV4)	codec = FLV_CODEC_FLV4; else
		if (pmt->subtype == MEDIASUBTYPE_AVC) codec = FLV_CODEC_H264; else
		/*
		TODO: Find out why VP6 is not working
		if (pmt->subtype == MEDIASUBTYPE_FLV4 ||
			pmt->subtype == MEDIASUBTYPE_VP60 ||
			pmt->subtype == MEDIASUBTYPE_VP61 ||
			pmt->subtype == MEDIASUBTYPE_VP62
			)	codec = FLV_CODEC_FLV4; else
		*/
		{
			// unsupported video codec
			return -1;
		}

		// we can only have one video stream
		if (video_codec != FLV_CODEC_NONE) return -1;
		
		REFERENCE_TIME		timeperframe = 0;

		// parse the video size
		if (pmt->formattype == FORMAT_VideoInfo) {
			VIDEOINFOHEADER		*vih = (VIDEOINFOHEADER*)pmt->pbFormat;
			video_width = vih->bmiHeader.biWidth;
			video_height = vih->bmiHeader.biHeight;
			timeperframe = vih->AvgTimePerFrame;
		} else
		if (pmt->formattype == FORMAT_VideoInfo2) {
			VIDEOINFOHEADER2	*vih = (VIDEOINFOHEADER2*)pmt->pbFormat;
			video_width = vih->bmiHeader.biWidth;
			video_height = vih->bmiHeader.biHeight;
			timeperframe = vih->AvgTimePerFrame;
		} else
		if (pmt->formattype == FORMAT_MPEG2Video) {
			MPEG2VIDEOINFO		*mvi = (MPEG2VIDEOINFO*)pmt->pbFormat;
			video_width = mvi->hdr.bmiHeader.biWidth;
			video_height = mvi->hdr.bmiHeader.biHeight;
			timeperframe = mvi->hdr.AvgTimePerFrame;
		} else
		if (pmt->formattype == FORMAT_MPEGVideo) {
			MPEG1VIDEOINFO		*mvi = (MPEG1VIDEOINFO*)pmt->pbFormat;
			video_width = mvi->hdr.bmiHeader.biWidth;
			video_height = mvi->hdr.bmiHeader.biHeight;
			timeperframe = mvi->hdr.AvgTimePerFrame;
		} 

		video_fps = 0;
		if (timeperframe != 0) {
			video_fps = (double)10000000.0 / (double)timeperframe;
		}

		// we're done
		video_codec = codec;

		video_extradata.Read(pmt);

		return 0;

	} else
	if (pmt->majortype == MEDIATYPE_Audio) {

		int	codec = FLV_CODEC_NONE;

		if (pmt->subtype == MEDIASUBTYPE_MP3)	codec = FLV_CODEC_MP3; else
		if (pmt->subtype == MEDIASUBTYPE_AAC)	codec = FLV_CODEC_AAC; else
		{
			// unsupported audio codec
			return -1;
		}

		// we can only have one audio stream
		if (audio_codec != FLV_CODEC_NONE) return -1;

		// audio parameters
		if (pmt->formattype == FORMAT_WaveFormatEx) {
			WAVEFORMATEX		*wfx = (WAVEFORMATEX*)pmt->pbFormat;
			audio_channels	 = wfx->nChannels;
			audio_samplerate = wfx->nSamplesPerSec;
		} 

		// we're done
		audio_codec = codec;

		audio_extradata.Read(pmt);

		return 0;

	} else {
		// unsupported stream type
		return -1;
	}

	return 0;
}

int FLVWriter::Start(FLVIO *flvio)
{
	io = flvio;
	if (!io) return -1;

	// write the file header
	BYTE	header[9] = {
		'F','L','V',			// FLV file signature
		0x01,					// FLV file version = 1
		0,						// Flags - modified later
		0, 0, 0, 9				// size of the header
	};

	if (video_codec != FLV_CODEC_NONE)	header[4] |= 0x01;
	if (audio_codec != FLV_CODEC_NONE)	header[4] |= 0x04;

	io->Seek(0);
	file_size = 0;

	io->Write(header, sizeof(header));

	// write the onMetaData tag with duration=0
	metadatapos = io->GetPosition();
	WriteMetaData();

	file_size = io->GetPosition();

	return 0;
}

int FLVWriter::Stop()
{
	// 1. last tag size
	BYTE		prev[4];	
	prev[0] = (last_tag_size >> 24) & 0xff;
	prev[1] = (last_tag_size >> 16) & 0xff;
	prev[2] = (last_tag_size >>  8) & 0xff;
	prev[3] = (last_tag_size >>  0) & 0xff;
	io->Write(prev, 4);
	file_size = io->GetPosition();
	// and the final touch - update onMetaData duration & file position
	io->Seek(9);
	WriteMetaData();

	return 0;
}

int FLVWriter::MakeAVCc(char* data, int size, char *output_data, int output_size)
{
	if (!data || size <= 0) return -1;
	int ps_size = (data[0] << 8) | (data[1]);
	int ss_size = (data[ps_size + 2] << 8) | (data[ps_size + 3]);
	int buf_size = 6 + ps_size + 2 + 1 + ss_size + 2;

	if (buf_size > output_size) return -1;

	char* temp = data;
	char* output_temp = output_data;

	output_temp[0] = 1;
	output_temp[1] = temp[3];
	output_temp[2] = temp[4];
	output_temp[3] = temp[5];
	output_temp[4] = temp[6]/*0xff*/;
	//output_temp[4] = 0xff;
	output_temp[5] = 0xe1;
	output_temp += 6;

	memcpy(output_temp, temp, ps_size + 2);
	output_temp += ps_size + 2;
	temp += ps_size + 2;

	output_temp[0] = 1;
	output_temp += 1;

	memcpy(output_temp, temp, ss_size + 2);

	return buf_size;
}

int FLVWriter::WriteExtradata(GUID type)
{
	BYTE		prev[4];	
	BYTE		tag_hdr[16];
	int			tag_data_size(0);
	int			towrite(0);
	BYTE		*data(NULL);
	int			size(0);

	if (type == MEDIATYPE_Video && (video_extradata.size <= 0 || video_codec != FLV_CODEC_H264)) return -1;
	if (type == MEDIATYPE_Audio && (audio_extradata.size <= 0 || audio_codec != FLV_CODEC_AAC)) return -1;

	memset(tag_hdr, 0, sizeof(tag_hdr));
	prev[0] = (last_tag_size >> 24) & 0xff;
	prev[1] = (last_tag_size >> 16) & 0xff;
	prev[2] = (last_tag_size >>  8) & 0xff;
	prev[3] = (last_tag_size >>  0) & 0xff;
	io->Write(prev, 4);

	char extradata[100];
	if (type == MEDIATYPE_Video) {
		int extradata_size = MakeAVCc(video_extradata.data, video_extradata.size, extradata, sizeof(extradata));
		tag_data_size = 5 + extradata_size;
		tag_hdr[0] = 9;
		tag_hdr[11] = 0x17;
		towrite = 16;
		data = (BYTE*)extradata;
		size = extradata_size;
	} else
	if (type == MEDIATYPE_Audio) {
		tag_data_size = 2 + audio_extradata.size;
		tag_hdr[0] = 8;
		tag_hdr[11] = 0xaf;
		towrite = 13;
		data = (BYTE*)audio_extradata.data;
		size = audio_extradata.size;
	}

	tag_hdr[1] = (tag_data_size >> 16) & 0xff;
	tag_hdr[2] = (tag_data_size >>  8) & 0xff;
	tag_hdr[3] = (tag_data_size >>  0) & 0xff;

	last_tag_size = tag_data_size + 11;

	io->Write(tag_hdr, towrite);
	io->Write(data, size);

	return 0;
}

int FLVWriter::WritePacket(CMuxInterPacket *packet, CMediaType *pmt)
{
	if (!io) return 0;

	BYTE		tag_hdr[16];
	memset(tag_hdr, 0, sizeof(tag_hdr));

	// we support only two streams
	if (pmt->majortype == MEDIATYPE_Video)		tag_hdr[0] = 9; else		// video packet
	if (pmt->majortype == MEDIATYPE_Audio)		tag_hdr[0] = 8; else		// audio packet
	{
		return -1;
	}

	// tag size
	int		tag_data_size = packet->size;

	if (tag_hdr[0] == 9) {
		// VIDEO DATA follows after the tag
		switch (video_codec) {
			case FLV_CODEC_H264: tag_data_size += 5; break;
			case FLV_CODEC_FLV4: tag_data_size += 2; break;
			default: tag_data_size += 1; break;
		}
	} else
	if (tag_hdr[0] == 8) {
		// AUDIO DATA follows after the tag
		switch (audio_codec) {
			case FLV_CODEC_AAC: tag_data_size += 2; break;
			default: tag_data_size += 1; break;
		}
	}

	tag_hdr[1] = (tag_data_size >> 16) & 0xff;
	tag_hdr[2] = (tag_data_size >>  8) & 0xff;
	tag_hdr[3] = (tag_data_size >>  0) & 0xff;

	// calculate timestamp
	__int64		timestamp_ms;
	if (packet->timestamp_ex) {
		timestamp_ms = (packet->dts / 10000);
	} else {
		timestamp_ms = (packet->rtStart_10mhz / 10000);	
	}

	if (is_first) {
		is_first = false;
		time_first_ms = timestamp_ms;				// we will offset all timestamps by this value
		WriteExtradata(MEDIATYPE_Video);			// then we write extradata (only valid for H.264 and AAC)
		WriteExtradata(MEDIATYPE_Audio);
	}

	timestamp_ms -= time_first_ms;
	if (timestamp_ms < 0) return 0;					// ignore negative timestamps

	tag_hdr[4] = (timestamp_ms >> 16) & 0xff;		// TimeStamp			UI24
	tag_hdr[5] = (timestamp_ms >>  8) & 0xff;
	tag_hdr[6] = (timestamp_ms >>  0) & 0xff;
	tag_hdr[7] = (timestamp_ms >> 24) & 0xff;		// TimestampExtended	UI8

	// keep track of the last timestamp
	time_last_ms = timestamp_ms;
	duration_ms  = time_last_ms;					// for now we consider the last timestamp duration

	// StreamID = always 0
	tag_hdr[8]  = 0;
	tag_hdr[9]  = 0;
	tag_hdr[10] = 0;

	/*
		Now write the TAG
	*/

	// 1. previous tag size
	BYTE		prev[4];	
	prev[0] = (last_tag_size >> 24) & 0xff;
	prev[1] = (last_tag_size >> 16) & 0xff;
	prev[2] = (last_tag_size >>  8) & 0xff;
	prev[3] = (last_tag_size >>  0) & 0xff;
	io->Write(prev, 4);

	// 2. Tag header
	io->Write(tag_hdr, 11);

	// 3. Data Header
	if (tag_hdr[0] == 9) {
		// VIDEODATA
		switch (video_codec) {
		case FLV_CODEC_FLV1:		tag_hdr[11] = 2; break;
		case FLV_CODEC_FLV4:		tag_hdr[11] = 4; break;
		case FLV_CODEC_H264:		tag_hdr[11] = 7; break;
		}
		
		// frame type
		if (packet->sync_point) { 
			tag_hdr[11] |= 0x10;
		} else {
			tag_hdr[11] |= 0x20;
		}

		switch (video_codec) {
		case FLV_CODEC_H264:
		{
			tag_hdr[12] = 1;
			int diff;

			if (packet->timestamp_ex) {							// support for IMediaSampleEx and correct PTS and DTS values
				diff = (packet->pts - packet->dts) / 10000;
			} else {
				__int64		timestamp_ms_stop = (packet->rtStop_10mhz / 10000);
				timestamp_ms_stop -= time_first_ms;
				diff = timestamp_ms_stop - timestamp_ms;		// basically duration of frame
			}

			if (diff < 0) diff = 0;
			tag_hdr[13] = (diff >> 16) & 0xff;	
			tag_hdr[14] = (diff >> 8) & 0xff;	
			tag_hdr[15] = (diff >> 0) & 0xff;	

			io->Write(tag_hdr + 11, 5); 
		}
		break;

		case FLV_CODEC_FLV4:
		{
			tag_hdr[12] = 0x00;				// no idea what should be here, but it seems working
			io->Write(tag_hdr + 11, 2); 
		}
		break;
		default: io->Write(tag_hdr + 11, 1); break;
		}

		// update our counters
		video_raw_size += packet->size;
		video_frames += 1;

		// Annex B to NALU
		if (packet->data[0] == 0 &&
			packet->data[1] == 0 &&
			packet->data[2] == 0 &&
			packet->data[3] == 1) {
			int size = packet->size - 4;
			packet->data[0] = (size >> 24) & 0xff;
			packet->data[1] = (size >> 16) & 0xff;
			packet->data[2] = (size >> 8) & 0xff;
			packet->data[3] = (size & 0xff);
		}

	} else
	if (tag_hdr[0] == 8) {
		// AUDIODATA
		switch (audio_codec) {
		case FLV_CODEC_MP3:			tag_hdr[11] = 32; break; // (2 << 4)
		case FLV_CODEC_AAC:			tag_hdr[11] = 160; break; // (10 << 4)
		}

		// sound rate
		// AAC should be set to 44KHz
		if (audio_codec == FLV_CODEC_AAC) {
			tag_hdr[11] |= (3 << 2);
		} else {
			switch (audio_samplerate) {
			case 5500:			tag_hdr[11] |= (0 << 2); break;
			case 11025:			tag_hdr[11] |= (1 << 2); break;
			case 22050:			tag_hdr[11] |= (2 << 2); break;
			case 44100:			tag_hdr[11] |= (3 << 2); break;
			}
		}
		
		// sound rate (16 bit)
		tag_hdr[11] |= (1 << 1);

		// sound type
		if (audio_channels == 2) {
			tag_hdr[11] |= 1;
		} else {
			tag_hdr[11] |= 0;
		}

		switch (audio_codec) {
		case FLV_CODEC_MP3:			io->Write(tag_hdr + 11, 1); break;
		case FLV_CODEC_AAC:			
		{
			tag_hdr[12] = 1;
			io->Write(tag_hdr + 11, 2); 
		}
		break;
		}
		
		
	}


	// 4. Raw Data
	io->Write(packet->data, packet->size);

	// remember the size of this tag
	last_tag_size = 11 + tag_data_size;
	file_size = io->GetPosition();

	return 0;
}

int FLVWriter::WriteMetaData()
{

	/*
		We assemble some basic onMetaData structure.
	*/
	Flash::AS_String		name;
	Flash::AS_ECMA_Array	vals;

	name.value = _T("onMetaData");

	/*
		We create the following metadata
	*/

	vals.AddValue(new Flash::AS_Variable(_T("duration"), new Flash::AS_Double((double)duration_ms / 1000.0)));
	vals.AddValue(new Flash::AS_Variable(_T("filesize"), new Flash::AS_Double((double)file_size)));
	vals.AddValue(new Flash::AS_Variable(_T("metadatacreator"), new Flash::AS_String(_T("MONOGRAM FLV Mux"))));

	vals.AddValue(new Flash::AS_Variable(_T("hasCuePoints"), new Flash::AS_UInt8(0)));	
	vals.AddValue(new Flash::AS_Variable(_T("canSeekToEnd"), new Flash::AS_UInt8(0)));	
	vals.AddValue(new Flash::AS_Variable(_T("hasKeyframes"), new Flash::AS_UInt8(0)));	

	vals.AddValue(new Flash::AS_Variable(_T("lasttimestamp"), new Flash::AS_Double((double)time_last_ms / 1000.0)));

	// if there is a video stream, write its properties
	if (video_codec != FLV_CODEC_NONE) {
		vals.AddValue(new Flash::AS_Variable(_T("hasVideo"), new Flash::AS_UInt8(1)));	
		vals.AddValue(new Flash::AS_Variable(_T("width"), new Flash::AS_Double((double)video_width)));	
		vals.AddValue(new Flash::AS_Variable(_T("height"), new Flash::AS_Double((double)video_height)));	
		vals.AddValue(new Flash::AS_Variable(_T("framerate"), new Flash::AS_Double((double)video_fps)));	

		// set the proper codec
		vals.AddValue(new Flash::AS_Variable(_T("videocodecid"), new Flash::AS_Double((double)video_codec)));	

		// calculate video datarate
		double	videorate = 0;
		if (duration_ms > 0) { videorate = (video_raw_size * 8 * 1000.0) / (double)(duration_ms); }
		vals.AddValue(new Flash::AS_Variable(_T("videodatarate"), new Flash::AS_Double((double)videorate)));	


	} else {
		vals.AddValue(new Flash::AS_Variable(_T("hasVideo"), new Flash::AS_UInt8(0)));	
	}

	// if there is an audio stream, write its properties
	if (audio_codec != FLV_CODEC_NONE) {
		vals.AddValue(new Flash::AS_Variable(_T("hasAudio"), new Flash::AS_UInt8(1)));	

		// set the proper codec
		vals.AddValue(new Flash::AS_Variable(_T("audiocodecid"), new Flash::AS_Double((double)audio_codec)));				
		vals.AddValue(new Flash::AS_Variable(_T("audiosamplerate"), new Flash::AS_Double(audio_samplerate)));	
		vals.AddValue(new Flash::AS_Variable(_T("audiosamplesize"), new Flash::AS_Double(16)));	
		vals.AddValue(new Flash::AS_Variable(_T("stereo"), new Flash::AS_UInt8(  (audio_channels >= 2 ? 1.0 : 0.0) )));	

	} else {
		vals.AddValue(new Flash::AS_Variable(_T("hasAudio"), new Flash::AS_UInt8(0)));	
	}

	// should be enough for now..
	// calculate the size

	int	total_size = 0;
	total_size += name.Size();
	total_size += vals.Size();

	if (total_size > 0) {
		// prepare the buffer
		uint8	*buf = (uint8*)malloc(total_size + 32);
		if (!buf) return -1;

		Flash::AS_Bytestream	bs;
		bs.pos = buf;
		bs.end = buf + total_size;

		// write out both name and ECMA array
		name.Write(bs);
		vals.Write(bs);


		/*
			Write the metadata TAG
		*/

		BYTE		tag_hdr[] = {
			0, 0, 0, 0,							// previous tag size
			0x12,								// Type			UI8 = Script Data Tag,
			(total_size >> 16) & 0xff,			// DataSize     UI24
			(total_size >>  8) & 0xff,
			(total_size >>  0) & 0xff,
			0, 0, 0, 0,							// TimeStamp	UI24   + TimestampExtended UI8
			0, 0, 0								// StreamID		UI24  (always 0)
		};

		io->Write(tag_hdr, sizeof(tag_hdr));
		io->Write(buf, total_size);

		// remember how large the metadata block was
		last_tag_size = (sizeof(tag_hdr) + total_size - 4);			// the first 4 bytes don't count

		// done with the buffer
		free(buf);
	}

	return 0;
}















