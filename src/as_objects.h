//-----------------------------------------------------------------------------
//
//	ActionScript Objects
//
//
//-----------------------------------------------------------------------------
#pragma once

#ifndef uint8
	typedef		unsigned __int8			uint8;				
	typedef 	unsigned __int16		uint16;			
	typedef 	unsigned __int32		uint32;				
	typedef 	unsigned __int64		uint64;				
	typedef 	__int8					int8;				
	typedef 	__int16					int16;				
	typedef 	__int32					int32;				
	typedef		__int64					int64;				
#endif


namespace Flash
{

	//-------------------------------------------------------------------------
	//
	//	ActionScript objekty
	//
	//-------------------------------------------------------------------------

	#define ENDMARKER	0x09

	class AS_Bytestream
	{
	public:
		uint8		*pos;
		uint8		*end;		
	public:
		inline void DumpBytes(int n) { pos += n; }
	
		// show only
		inline uint8  UPeek8() 	{ return pos[0]; }
		inline uint16 UPeek16()	{ return ((pos[0] <<  8) | (pos[1])); }
		inline uint32 UPeek24()	{ return ((pos[0] << 16) | (pos[1] <<  8) | (pos[2])); }
		inline uint32 UPeek32()	{ return ((pos[0] << 24) | (pos[1] << 16) | (pos[2] << 8) | (pos[3])); }
		inline uint64 UPeek64() 
		{
			uint64	p; uint8* pp = (uint8*)&p;
			pp[0] = pos[7]; pp[1] = pos[6]; 
			pp[2] = pos[5]; pp[3] = pos[4]; 
			pp[4] = pos[3]; pp[5] = pos[2]; 
			pp[6] = pos[1];	pp[7] = pos[0];
			return p;
		}
		
		inline int8   SPeek8() 	{ return (int8)pos[0]; }
		inline int16  SPeek16()	{ return (int16)((pos[0] <<  8) | (pos[1]     )); }
		inline int32  SPeek24()	{ return (int32)((pos[0] << 16) | (pos[1] <<  8) | (pos[2])); }
		inline int32  SPeek32()	{ return (int32)((pos[0] << 24) | (pos[1] << 16) | (pos[2] << 8) | (pos[3])); }
		inline int64  SPeek64() 
		{
			int64	p; uint8* pp = (uint8*)&p;
			pp[0] = pos[7]; pp[1] = pos[6]; 
			pp[2] = pos[5]; pp[3] = pos[4]; 
			pp[4] = pos[3]; pp[5] = pos[2]; 
			pp[6] = pos[1];	pp[7] = pos[0];
			return p;
		}
		
		inline float	PeekFloat()  { uint32 val = UPeek32(); float  f = *((float*)&val); return f; }
		inline double	PeekDouble() { uint64 val = UPeek64(); double d = *((double*)&val); return d; }
		inline uint32   PeekEncodedU32() 
		{
			uint32 r = pos[0];
			if (!(r & 0x00000080)) return r;			r = (r & 0x0000007f) | pos[1]<<7;
			if (!(r & 0x00004000)) return r;			r = (r & 0x00003fff) | pos[2]<<14;
			if (!(r & 0x00200000)) return r;			r = (r & 0x001fffff) | pos[3]<<21;
			if (!(r & 0x10000000)) return r;			r = (r & 0x0fffffff) | pos[4]<<28;
			return r;
		}
		
		// get
		inline uint8	UGet8()		{ uint8 val = UPeek8(); DumpBytes(1); return val; }
		inline uint16	UGet16()	{ uint16 val = UPeek16(); DumpBytes(2); return val; }
		inline uint32	UGet24()	{ uint32 val = UPeek24(); DumpBytes(3); return val; }
		inline uint32	UGet32()	{ uint32 val = UPeek32(); DumpBytes(4); return val; }
		inline uint64	UGet64()	{ uint64 val = UPeek64(); DumpBytes(8); return val; }		
		inline int8		SGet8()		{ int8 val = SPeek8(); DumpBytes(1); return val; }
		inline int16	SGet16()	{ int16 val = SPeek16(); DumpBytes(2); return val; }
		inline int32	SGet24()	{ int32 val = SPeek24(); DumpBytes(3); return val; }
		inline int32	SGet32()	{ int32 val = SPeek32(); DumpBytes(4); return val; }
		inline int64	SGet64()	{ int64 val = SPeek64(); DumpBytes(8); return val; }

		inline float	GetFloat()  
		{ 	
			uint32 val = UGet32(); 
			float  rf;
			memcpy(&rf, &val, sizeof(val));
			return rf;
		}
		inline double GetDouble() 
		{ 	
			uint64 val = UGet64();
			double	v;
			memcpy(&v, &val, sizeof(val));
			return v;
		}

		inline uint32 GetEncodedU32() 
		{
			uint32 r = pos[0];			pos++;
			if (!(r & 0x00000080)) return r;	r = (r & 0x0000007f) | pos[0]<<7;		pos++;
			if (!(r & 0x00004000)) return r;	r = (r & 0x00003fff) | pos[0]<<14;		pos++;
			if (!(r & 0x00200000)) return r;	r = (r & 0x001fffff) | pos[0]<<21;		pos++;
			if (!(r & 0x10000000)) return r;	r = (r & 0x0fffffff) | pos[0]<<28;		pos++;
			return r;
		}
		
		inline CString GetString()
		{
			int			len = UGet16();			

			// TODO: some UTF-8 ?
			CStringA	sa((char*)pos, len);
			CString		s;
			s = sa;
			pos += len;
			return s;
		}
		
		inline CString GetLongString()
		{
			uint32		len = UGet32();			
			CStringA	sa((char*)pos, len);
			CString		s;

			s = sa;
			pos += len;
			return s;
		}
		
		// set
		inline void	USet8(uint8 val) { pos[0] = val; DumpBytes(1); }
		inline void	USet16(uint16 val) { pos[0] = val >> 8; pos[1] = val >> 0; DumpBytes(2); }
		inline void	USet24(uint32 val) { pos[0] = val >> 16; pos[1] = val >> 8; pos[2] = val >> 0; DumpBytes(3); }
		inline void	USet32(uint32 val) { pos[0] = val >> 24; pos[1] = val >> 16; pos[2] = val >> 8; pos[3] = val >> 0; DumpBytes(4); }
		inline void	USet64(uint64 val) { pos[0] = val >> 56; pos[1] = val >> 48; pos[2] = val >> 40; pos[3] = val >> 32; pos[4] = val >> 24; pos[5] = val >> 16; pos[6] = val >> 8; pos[7] = val >> 0; DumpBytes(8); }		
		inline void	SSet8(uint8 val) { USet8(val); }
		inline void	SSet16(int16 val) { USet16(val); }
		inline void	SSet24(int32 val) { USet24(val); }
		inline void	SSet32(int32 val) { USet32(val); }
		inline void	SSet64(int64 val) { USet64(val); }		
		
		inline void	SetFloat(float val)  
		{ 	
			uint32 v;
			memcpy(&v, &val, sizeof(v));
			USet32(v);
		}
		
		inline void	SetDouble(double val) 
		{ 	
			uint64 v;
			memcpy(&v, &val, sizeof(v));
			USet64(v);
		}

		inline void SetString(CString val)
		{
			CStringA		stra;
			
			// TODO: perhaps some UTF-8 conversion ?
			stra = val;

			USet16(stra.GetLength());

			memcpy(pos, stra.GetBuffer(), stra.GetLength());
			pos += stra.GetLength();
		}
		
		inline void SetLongString(CString val)
		{
			CStringA		stra;
			// TODO: perhaps some UTF-8 conversion ?
			stra = val;


			USet32(stra.GetLength());
			memcpy(pos, stra.GetBuffer(), stra.GetLength());
			pos += stra.GetLength();
		}
	};


	class AS_Value
	{
	public:	
	
		CString					name;
		int						type;
		
		// hodnota bude zalezat na odvodenej triede
		
		// child objekty
		vector<AS_Value*>	child;
	
	public:
		AS_Value();
		virtual ~AS_Value();
				
		void Clear();
		void AddValue(AS_Value *var);
		int GetCount();
		
		virtual int Write(AS_Bytestream &b);
		virtual int Read(AS_Bytestream &b); 
		virtual int Size();
		
		// vytvarame objekty
		virtual int CreateType(int type, AS_Value **obj);
		
		// vraciame hodnoty
		virtual CString AsString();
		virtual double AsDouble();
		virtual bool AsBool();
		virtual int AsInt();
		virtual void EchoTrace();
		
		AS_Value *Find(CString byname);
	};
	

	class AS_DataObject : public AS_Value
	{
	public:
	
	public:
		AS_DataObject();
		virtual ~AS_DataObject();
		virtual int Read(AS_Bytestream &b);
		virtual int Write(AS_Bytestream &b);
		virtual int Size();
	};

	class AS_ECMA_Array : public AS_Value
	{
	public:
		AS_ECMA_Array();
		virtual ~AS_ECMA_Array();		
		virtual int Read(AS_Bytestream &b);
		virtual int Write(AS_Bytestream &b);
		virtual int Size();
		virtual void EchoTrace();
	};
	
	class AS_Strict_Array : public AS_Value
	{
	public:
		AS_Strict_Array();
		virtual ~AS_Strict_Array();		
		virtual int Read(AS_Bytestream &b);
		virtual int Write(AS_Bytestream &b);
		virtual int Size();
	};
	
	class AS_NullType : public AS_Value
	{
	public:
		AS_NullType();
		virtual ~AS_NullType();		
		virtual int Read(AS_Bytestream &b);
		virtual int Write(AS_Bytestream &b);
		virtual int Size();
	};
	
	class AS_AMF3Number : public AS_Value
	{
	public:
		uint32	value;
		uint8	size;
	public:
		AS_AMF3Number();
		AS_AMF3Number(uint32 number);
		virtual ~AS_AMF3Number();		
		virtual int Read(AS_Bytestream &b);
		virtual int Write(AS_Bytestream &b);
		virtual int Size();
	};

	class AS_Variable : public AS_Value
	{
	public:
		AS_Value	*value;
		bool		hasname;		
	public:
		AS_Variable();
		AS_Variable(AS_Value* value);
		AS_Variable(CString key, CString value);
		AS_Variable(CString key, AS_Value* value);
		virtual ~AS_Variable();		
		virtual int Read(AS_Bytestream &b);
		virtual int Write(AS_Bytestream &b);
		virtual int Size();
		virtual void EchoTrace();
		
		virtual CString AsString() { return (value == NULL ? AS_Value::AsString() : value->AsString()); }
		virtual double AsDouble() { return (value == NULL ? AS_Value::AsDouble() : value->AsDouble()); }
		virtual bool AsBool() { return (value == NULL ? AS_Value::AsBool() : value->AsBool()); }
		virtual int AsInt() { return (value == NULL ? AS_Value::AsInt() : value->AsInt()); }
	};
	
	class AS_Date : public AS_Value
	{
	public:
		uint64		datetime;
		int			local_offset;
	public:
		AS_Date();
		virtual ~AS_Date();		
		virtual int Read(AS_Bytestream &b);
		virtual int Write(AS_Bytestream &b);
		virtual int Size();
	};
	
	class AS_Object : public AS_Value
	{
	public:
		AS_Object();
		virtual ~AS_Object();		
		virtual int Read(AS_Bytestream &b);
		virtual int Write(AS_Bytestream &b);
		virtual int Size();
	};

	//-------------------------------------------------------------------------
	//
	//	Typy AS hodnot
	//
	//-------------------------------------------------------------------------

#define AS_CLASS_TYPE(classname, astypename, def, t)							\
	class classname : public AS_Value											\
	{																			\
	public:																		\
		astypename	value;														\
	public:																		\
		classname() : AS_Value(), value(def) { type = t; }						\
		classname(astypename val) : AS_Value(), value(val) { type = t; }		\
		virtual ~classname() { }												\
		virtual int Read(AS_Bytestream &b); 									\
		virtual int Write(AS_Bytestream &b); 									\
		virtual int Size();														\
	

	AS_CLASS_TYPE(AS_Double, double, 0.0, 0)
		virtual double AsDouble() { return value; }
		virtual void EchoTrace();
	};
	
	AS_CLASS_TYPE(AS_UInt8, uint8, 0, 1)
		virtual int AsInt() { return value; }
		virtual bool AsBool() { return (value == 0 ? false : true); }
		virtual void EchoTrace();
	};

	AS_CLASS_TYPE(AS_UInt16, uint16, 0, 7)
		virtual int AsInt() { return value; }
		virtual void EchoTrace();
	};
	
	AS_CLASS_TYPE(AS_String, CString, _T(""), 2)
		virtual CString AsString() { return value; }
		virtual void EchoTrace();
	};
	
	AS_CLASS_TYPE(AS_LongString, CString, _T(""), 12)
		virtual CString AsString() { return value; }
		virtual void EchoTrace();
	};

#undef AS_CLASS_TYPE


};


