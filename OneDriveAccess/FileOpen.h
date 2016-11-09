#pragma once

using namespace Platform;
using namespace::Windows::Storage;
using namespace concurrency;

ref class PickedFile sealed
{
	Windows::Storage::StorageFile^ m_File;
	Windows::Storage::FileProperties::BasicProperties^ m_FileProperty;
public:
	PickedFile(Windows::Storage::StorageFile^ file, Windows::Storage::FileProperties::BasicProperties^ prop)
	{
		m_File = file;
		m_FileProperty = prop;
	}
	property Windows::Storage::StorageFile^ File
	{
		Windows::Storage::StorageFile^ get()
		{
			return m_File;
		}
	}
	property Windows::Storage::FileProperties::BasicProperties^ FileProperty
	{
		Windows::Storage::FileProperties::BasicProperties^ get()
		{
			return m_FileProperty;
		}
	}
};
task<Windows::Storage::StorageFolder^> SelectFolder();
void SaveFile(Windows::Storage::StorageFolder^ folder, String^ FileName, BYTE* pBuffer, UINT64 ui64FileSize);
