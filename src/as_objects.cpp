//-----------------------------------------------------------------------------
//
//	ActionScript Objects
//
//
//-----------------------------------------------------------------------------
#include "stdafx.h"

namespace Flash
{
	
	//-------------------------------------------------------------------------
	//
	//	ActionScript objekty
	//
	//-------------------------------------------------------------------------

	AS_Value::AS_Value()
	{
		name = _T("");
		type = -1;
	}	
	
	AS_Value::~AS_Value()
	{
		Clear();
	}
	
	void AS_Value::Clear()
	{
		for (int i=0; i<child.size(); i++) {
			AS_Value *var = child[i];
			if (var) {
				delete var;
			}
		}
		child.clear();
	}
			
	void AS_Value::AddValue(AS_Value *var)
	{
	
		child.push_back(var);
	}
	
	int AS_Value::GetCount()
	{
		return child.size();
	}

	void AS_Value::EchoTrace()					{ }
	int AS_Value::Write(AS_Bytestream &b)		{ return -1; }
	int AS_Value::Read(AS_Bytestream &b)		{ return -1; }
	int AS_Value::Size()						{ return 0; }		

	CString AS_Value::AsString()		{ return _T(""); }
	double AS_Value::AsDouble()		{ return 0; }
	bool AS_Value::AsBool()			{ return false; }
	int AS_Value::AsInt()			{ return 0; }
	
	AS_Value *AS_Value::Find(CString byname)
	{
		for (int i=0; i<child.size(); i++) {
			if (child[i]->name == byname) return child[i];
		}
		return NULL;
	}
	
	// vytvarame objekty
	int AS_Value::CreateType(int type, AS_Value **obj)
	{
		if (!obj) return -1;
		
		switch (type) {
		case 0:		*obj = new AS_Double(); break;
		case 1:		*obj = new AS_UInt8(); break;
		case 2:		*obj = new AS_String(); break;		
		case 3:		*obj = new AS_Object(); break;
		case 4:		*obj = new AS_String(); break;		
		case 5:		*obj = new AS_NullType(); break;
		case 6:		*obj = NULL; break;					//new AS_Undefined(); break;
		case 7:		*obj = new AS_UInt16(); break;
		case 8:		*obj = new AS_ECMA_Array(); break;
		case 9:		*obj = NULL; break;
		case 10:	*obj = new AS_Strict_Array(); break;
		case 11:	*obj = new AS_Date(); break;
		case 12:	*obj = new AS_LongString(); break;
		default:	*obj = NULL;
		}
		
		return 0;
	}
	
	//-------------------------------------------------------------------------
	//
	//	Typy
	//
	//-------------------------------------------------------------------------

	int AS_Double::Read(AS_Bytestream &b)
	{
		type  = b.UGet8();		
		value = b.GetDouble();		
		return 0;
	}

	int AS_Double::Write(AS_Bytestream &b)
	{
		b.USet8(type);	
		b.SetDouble(value);		
		return 0;
	}
	
	int AS_Double::Size()
	{
		return 1 + 8;
	}

	void AS_Double::EchoTrace()
	{
		TRACE(_T("(Double) %s : %f\n"), name.GetBuffer(), (float)value);
	}

	int AS_UInt8::Read(AS_Bytestream &b)
	{
		type  = b.UGet8();
		value = b.UGet8();
		return 0;
	}
	
	int AS_UInt8::Write(AS_Bytestream &b)
	{
		b.USet8(type);		
		b.USet8(value);		
		return 0;
	}
	
	int AS_UInt8::Size()
	{
		return 1 + 1;
	}

	void AS_UInt8::EchoTrace()
	{
		TRACE(_T("(UInt8) %s : %d\n"), name.GetBuffer(), (int)value);
	}

	int AS_String::Read(AS_Bytestream &b)
	{
		type = b.UGet8();
		value = b.GetString();
		return 0;
	}

	int AS_String::Write(AS_Bytestream &b)
	{
		b.USet8(type);		
		b.SetString(value);		
		return 0;
	}
	
	int AS_String::Size()
	{
		return 1 + 2 + value.GetLength();
	}

	void AS_String::EchoTrace()
	{
		TRACE(_T("(String) %s : %s\n"), name.GetBuffer(), value.GetBuffer());
	}

	int AS_LongString::Read(AS_Bytestream &b)
	{
		type  = b.UGet8();
		value = b.GetLongString();
		return 0;
	}
	
	int AS_LongString::Write(AS_Bytestream &b)
	{
		b.USet8(type);		
		b.SetLongString(value);		
		return 0;
	}
	
	int AS_LongString::Size()
	{
		return 1 + 4 + value.GetLength();
	}

	void AS_LongString::EchoTrace()
	{
		TRACE(_T("(LongString) %s : %s\n"), name.GetBuffer(), value.GetBuffer());
	}


	int AS_UInt16::Read(AS_Bytestream &b)
	{
		type  = b.UGet8();
		value = b.UGet16();
		return 0;
	}
	
	int AS_UInt16::Write(AS_Bytestream &b)
	{
		b.USet8(type);		
		b.USet16(value);		
		return 0;
	}
	
	int AS_UInt16::Size()
	{
		return 1 + 2;
	}

	void AS_UInt16::EchoTrace()
	{
		TRACE(_T("(UInt16) %s : %d\n"), name.GetBuffer(), (int)value);
	}


	AS_ECMA_Array::AS_ECMA_Array() :
		AS_Value()
	{
		type = 8;
	}
	
	AS_ECMA_Array::~AS_ECMA_Array()
	{
	}
	
	int AS_ECMA_Array::Read(AS_Bytestream &b)
	{
		type  = b.UGet8();
		int len = b.UGet32();
		
		
		// loadujeme objekty
		for (int i=0; i<len; i++) {
			AS_Variable	*var = new AS_Variable();
			int ret = var->Read(b);
			if (ret < 0) {
				delete var;
			} else {
				AddValue(var);
			}			
		}
		
		// niekedy tam byva, niekedy nie, fetaci...
		uint32 em = b.UPeek24();
		if (em == 3) b.UGet24();
		
		return 0;
	}

	int AS_ECMA_Array::Write(AS_Bytestream &b)
	{

		int len = GetCount();	
		b.USet8(type);
		b.USet32(len);
		
		// loadujeme objekty
		for (int i=0; i<len; i++) {
			AS_Value *var = child[i];
			var->Write(b);
		}
		b.USet24(ENDMARKER);
		
		return 0;
	}

	int AS_ECMA_Array::Size()
	{
		int len = GetCount();
		int result(0);
		result += 1;
		result += 4;
		
		for (int i=0; i<len; i++) {
			AS_Value *var = child[i];
			int s = var->Size();
			result += s;
		}
		result += 3;
		
		return result;
	}
	
	void AS_ECMA_Array::EchoTrace()
	{
		TRACE(_T("(ECMA_Array - Begin) : %s\n"), name.GetBuffer());
		int len = GetCount();
		for (int i=0; i<len; i++) {
			AS_Value *var = child[i];
			var->EchoTrace();
		}
		TRACE(_T("--- ECMA Array End\n"));
	}

	AS_Object::AS_Object() :
		AS_Value()
	{
		type = 3;
	}
	
	AS_Object::~AS_Object()
	{
	}
	
	int AS_Object::Read(AS_Bytestream &b)
	{
		type  = b.UGet8();
		do {
			int		w = b.UPeek24();
			if (w == ENDMARKER) {
				b.UGet24();
				break;
			}
			
			AS_Variable	*var = new AS_Variable();
			int ret = var->Read(b);
			if (ret < 0) {
				delete var;
				return -1;
			} else {
				AddValue(var);
			}						
		} while (true);		
		return 0;
	}
	
	int AS_Object::Write(AS_Bytestream &b)
	{
		b.USet8(type);
		for (int i=0; i<GetCount(); i++) {
			AS_Value *var = child[i];
			var->Write(b);
		}
		b.USet24(ENDMARKER);
		
		return 0;
	}

	int AS_Object::Size()
	{
		int result(0);
		result += 1;
		for (int i=0; i<GetCount(); i++) {
			AS_Value *var = child[i];
			result += var->Size();
		}
		result += 3;
		
		return result;
	}

	AS_Strict_Array::AS_Strict_Array() :
		AS_Value()
	{
		type = 10;
	}
	
	AS_Strict_Array::~AS_Strict_Array()
	{
	}
	
	int AS_Strict_Array::Read(AS_Bytestream &b)
	{		
		int i = 0;
		int ret = 0;
		
		type  = b.UGet8();
		int len = b.UGet32();

		// loadujeme objekty
		for (int i=0; i<len; i++) {		
			AS_Variable	*var = new AS_Variable();
			var->hasname = false;
			int ret = var->Read(b);
			if (ret < 0) {
				delete var;
			} else {
				AddValue(var);
			}			
		}
		
		return ret;
	}

	int AS_Strict_Array::Write(AS_Bytestream &b)
	{
		int len = GetCount();
		b.USet8(type);
		b.USet32(len);
		
		// loadujeme objekty
		for (int i=0; i<len; i++) {
			AS_Variable *var = (AS_Variable*)child[i];
			var->hasname = false;
			var->Write(b);
		}
		
		return 0;
	}

	int AS_Strict_Array::Size()
	{
		int result(0);
		result += 1;
		result += 4;
		for (int i=0; i<GetCount(); i++) {
			AS_Value *var = child[i];
			result += var->Size();
		}
		return result;
	}


	AS_Date::AS_Date() :
		AS_Value()
	{
		type = 11;
	}
	
	AS_Date::~AS_Date()
	{
	}
	
	int AS_Date::Read(AS_Bytestream &b)
	{
		type = b.UGet8();
		datetime = b.UGet64();
		local_offset = b.SGet16();
		return 0;
	}

	int AS_Date::Write(AS_Bytestream &b)
	{
		b.USet8(type);
		b.USet64(datetime);
		b.SSet16(local_offset);
		return 0;
	}

	int AS_Date::Size()
	{
		return 1 + 8 + 2;
	}

	AS_NullType::AS_NullType() :
		AS_Value()
	{
		type = 5;
	}
	
	AS_NullType::~AS_NullType()
	{
	}
	
	int AS_NullType::Read(AS_Bytestream &b)
	{
		type = b.UGet8();
		return 0;
	}

	int AS_NullType::Write(AS_Bytestream &b)
	{
		b.USet8(type);
		return 0;
	}

	int AS_NullType::Size()
	{
		return 1;
	}

	AS_AMF3Number::AS_AMF3Number() :
		AS_Value(),
		value(0),
		size(1)
	{
		type = 0x11;
	}
	
	AS_AMF3Number::AS_AMF3Number(uint32 number) :
		AS_Value(),
		value(0),
		size(1)
	{
		type = 0x11;
		
		value = 0;
		if (number <= 0x7F) {
			value = number;
			size = 1;
		} else if (number < 0x3fff) {
			value = 0x8000 | ((number << 1) & 0x7f00) | (number & 0x7f);
			size = 2;
		} else if (number < 0x1fffff) {
			value = 0x808000 | ((number << 2) & 0x7f0000) | ((number << 1) & 0x7f00) | (number & 0x7f);
			size = 3;
		} else if (number < 0x1fffffff) {
			value = 0x80808000 | ((number << 3) & 0x7f000000) | ((number << 2) & 0x7f0000) | ((number << 1) & 0x7f00) | (number & 0xff);
			size = 4;
		}
		
	}
	
	AS_AMF3Number::~AS_AMF3Number()
	{
	}
	
	int AS_AMF3Number::Read(AS_Bytestream &b)
	{
		// TODO: 
//		type = b.UGet8();
		return 0;
	}

	int AS_AMF3Number::Write(AS_Bytestream &b)
	{
		b.USet8(type);
		b.USet8(0x04);
		switch (size) {
		case 1: b.USet8(value); break;
		case 2: b.USet16(value); break;
		case 3: b.USet24(value); break;
		case 4: b.USet32(value); break;
		}
		return 0;
	}

	int AS_AMF3Number::Size()
	{
		return 1 + 1 + size;
	}

	//-------------------------------------------------------------------------
	//
	//	ActionScript Typy
	//
	//-------------------------------------------------------------------------

	AS_DataObject::AS_DataObject() :
		AS_Value()
	{
		name = "";
	}
	
	AS_DataObject::~AS_DataObject()
	{
	}
	
	int AS_DataObject::Read(AS_Bytestream &b)
	{
		// error
		int	type = b.UPeek8();
		if (type != 2) return -1;				
		b.UGet8();		
		
		// loadujeme		
		name = b.GetString();				
		type = b.UPeek8();
		
		AS_Value *value = NULL;
		int ret = CreateType(type, &value);
		if (ret == 0 && value) {
			value->name = name;
			ret = value->Read(b);
			if (ret < 0) {
				delete value;
			} else {
				AddValue(value);
			}			
		} else {
			return -1;
		}
				
		uint32 end_marker = b.UGet32();
		return ret;
	}
	
	int AS_DataObject::Write(AS_Bytestream &b)
	{
		b.USet8(2);		
		
		// loadujeme		
		b.SetString(name);				

		AS_Value *value = child[0];
		value->Write(b);
				
		return 0;
	}

	int AS_DataObject::Size()
	{
		if (name.GetLength() <= 0) return 0;

		int result(0);
		result += 1;

		result += 2 + name.GetLength();		

		AS_Value *value = child[0];
		result += value->Size();

		return result;
	}

	
	AS_Variable::AS_Variable() :
		AS_Value(),
		value(NULL),
		hasname(true)
	{	
	}

	AS_Variable::AS_Variable(AS_Value* value) :
		AS_Value(),
		hasname(false)
	{
		name = "";
		this->value = value;
	}
	
	AS_Variable::AS_Variable(CString key, CString value) :
		AS_Value(),
		hasname(true)
	{	
		name = key;
		AS_String* s = new AS_String();
		s->value = value;
		this->value = s;
	}

	AS_Variable::AS_Variable(CString key, AS_Value* value) :
		AS_Value(),
		hasname(true)
	{
		name = key;
		this->value = value;
	}
	
	AS_Variable::~AS_Variable()
	{
		if (value) {
			delete value;
			value = NULL;
		}
	}
	
	int AS_Variable::Read(AS_Bytestream &b)
	{		
		if (hasname) name = b.GetString();			
		int		 	vtype 	= b.UPeek8();
				
		AS_Value	*v 		= NULL;
		int			ret		= CreateType(vtype, &v);
		
		if (ret == 0 && v) {
			ret = v->Read(b);
			if (ret < 0) {
				delete v;
				return -1;
			} else {
				value = v;
			}		
		} else {
			return -1;
		}			
		
		return 0;
	}
	
	int AS_Variable::Write(AS_Bytestream &b)
	{
		if (hasname) {
			b.SetString(name);
		}
		value->Write(b);
		
		return 0;
	}

	int AS_Variable::Size()
	{
		int result(0);
		if (hasname) {
			result += 2 + name.GetLength();
		}
		result += value->Size();
		
		return result;
	}

	void AS_Variable::EchoTrace()
	{
		TRACE(_T("(Variable) %s = \n"), name.GetBuffer());

		if (value) value->EchoTrace();
	}

};






