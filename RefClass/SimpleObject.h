#pragma once

#include <Windows.h>

using namespace Platform;

ref class CSimpleObject sealed
{
	String^		_Name;
	
	~CSimpleObject();
public:	// 다른 언어로도 호출가능. 따라서 메소드 선언에 언어에 종속된 타입을 사용할 수 없다.
	property Platform::String^ Name
	{
		// property 직접 코딩
		Platform::String^ get()	
		{
			return _Name;
		}
		void set(Platform::String^ name)
		{
			_Name = name;
		}
	}
	property	int			Value;		// get(),set()자동 생성

	CSimpleObject();
	CSimpleObject(String^ name,int value)
	{
		_Name = name;
		Value = value;
	}
internal:
	CSimpleObject^ operator+(CSimpleObject^ obj);
};

