#include "pch.h"

#include <experimental\resumable>
#include <ppl.h>
#include <pplawait.h>
#include <ppltasks.h>

#include "FileOpen.h"

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Storage;
using namespace Windows::Storage::Pickers;
using namespace Windows::Storage::Streams;
using namespace concurrency;
using namespace std;

task<StorageFolder^> SelectFolder()
{
	FolderPicker^ openPicker = ref new FolderPicker();
	openPicker->ViewMode = PickerViewMode::Thumbnail;
	openPicker->SuggestedStartLocation = PickerLocationId::ComputerFolder;
		
	openPicker->FileTypeFilter->Append(".txt");
    openPicker->FileTypeFilter->Append(".jpg");
    openPicker->FileTypeFilter->Append(".jpeg");
    openPicker->FileTypeFilter->Append(".png");

	task<StorageFolder^> task_pick_folder = create_task(openPicker->PickSingleFolderAsync());
	task_pick_folder.then([](task<StorageFolder^> task_folder)
	{
		StorageFolder^ folder = nullptr;
		try
		{
			folder = task_folder.get();
		}
		catch (Exception^ e)
		{
			const WCHAR* errMsg = e->Message->Data();
			WriteDebugStringW(L"Failed to SelectFolder() - PickSingleFolderAsync() , %s\n", errMsg);
		}
		return folder;
    });
	return task_pick_folder;
}
void SaveFile(Windows::Storage::StorageFolder^ folder,String^ FileName,BYTE* pBuffer,UINT64 ui64FileSize)
{
	BOOL	bResult = FALSE;
	
	Windows::Storage::CreationCollisionOption opt = CreationCollisionOption::ReplaceExisting;
	
	CreationCollisionOption::OpenIfExists;
	create_task(folder->CreateFileAsync(FileName,opt)).then([pBuffer,ui64FileSize](task<StorageFile^> task_file)
	{
		StorageFile^ file = nullptr;
		try
		{
			file = task_file.get();

		}
		catch (Exception^ e)
		{
			const WCHAR* errMsg = e->Message->Data();
			WriteDebugStringW(L"Failed to SaveFile() - CreateFileAsync(), %s\n", errMsg);
		}
		if (!file)
			return;

		create_task(file->OpenAsync(FileAccessMode::ReadWrite)).then([pBuffer,ui64FileSize](task<IRandomAccessStream^> task_file_stream)
		{
			IRandomAccessStream^ fileStream = nullptr;
			try
			{
				fileStream = task_file_stream.get();
			}
			catch (Exception^ e)
			{
				const WCHAR* errMsg = e->Message->Data();
				WriteDebugStringW(L"Failed to SaveFile() - OpenAsync(), %s\n", errMsg);
			}
			if (!fileStream)
				return;

			DataWriter^ dataWriter = ref new DataWriter(fileStream);
			dataWriter->WriteBytes(Platform::ArrayReference<BYTE>(pBuffer, ui64FileSize));
			
			create_task(dataWriter->StoreAsync()).then([ui64FileSize](size_t size)
			{
				size_t WrittenBytes = size;
			});

		});
	});
}