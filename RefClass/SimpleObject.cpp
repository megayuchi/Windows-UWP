#include "pch.h"
#include "SimpleObject.h"


CSimpleObject::CSimpleObject()
{

}

CSimpleObject^ CSimpleObject::operator+(CSimpleObject^ obj)
{
	CSimpleObject^	result = ref new CSimpleObject();
	result->Value = Value + obj->Value;
	result->Name = _Name + L" + " + obj->Name;

	return result;

}
CSimpleObject::~CSimpleObject()
{
	String^		Message = _Name + L" destroyed\n";
	OutputDebugString(Message->Data());
}