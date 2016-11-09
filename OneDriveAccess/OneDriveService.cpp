#include "pch.h"
#include <experimental\resumable>
#include <ppl.h>
#include <pplawait.h>
#include <ppltasks.h>
#include <ppl.h>
#include "OneDriveService.h"
#include "FileItem.h"
#include "FileOpen.h"

using namespace OneDriveAccess;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::UI::Xaml::Interop;
using namespace concurrency;

using namespace Windows::Security;
using namespace Windows::UI::Xaml::Media::Imaging;
using namespace Windows::Web::Http;
using namespace Windows::Data::Json;
using namespace Windows::Security::Authentication::OnlineId;

BOOL wfopen_with_local(FILE** pp_out_fp, const WCHAR* wchFileName, WCHAR* wchIOMode);

OneDriveService::OneDriveService(String^ ClientID)
{
	m_dwThreadID = GetCurrentThreadId();
}
void OneDriveService::SetBusy()
{
	m_bIsBusy = true;

	OneDriveAccess::App^ app = safe_cast<OneDriveAccess::App^>(OneDriveAccess::App::Current);
	app->UpdateProgress();
}
void OneDriveService::SetNotBusy()
{
	DWORD	dwCurThreadID = GetCurrentThreadId();

#ifdef _DEBUG
	// Do not task_continuation_context::use_arbitrary(), when use create_task()-then(),in OneDriverService class
	//auto thread_context = task_continuation_context::use_arbitrary();
	if (m_dwThreadID != dwCurThreadID)
		__debugbreak();
#endif
	m_bIsBusy = false;

	OneDriveAccess::App^ app = safe_cast<OneDriveAccess::App^>(OneDriveAccess::App::Current);
	app->UpdateProgress();
}
void OneDriveService::SignIn()
{
	if (IsBusy())
		return;

	//GET https://apis.live.net/v5.0/me?access_token=ACCESS_TOKEN
	//GET https://apis.live.net/v5.0/folder.a6b2a7e8f2515e5e.A6B2A7E8F2515E5E!114?access_token=ACCESS_TOKEN

	String^ scope = L"wl.signin wl.basic wl.photos wl.skydrive_update";

	auto request = ref new OnlineIdServiceTicketRequest(scope, "DELEGATION");
	m_Authenticator = ref new Windows::Security::Authentication::OnlineId::OnlineIdAuthenticator();

	SetBusy();
	auto login_task = create_task(m_Authenticator->AuthenticateUserAsync(request));
	login_task.then([this](task<Windows::Security::Authentication::OnlineId::UserIdentity^> ident_task)
	{
		bool	success = false;
		UserIdentity^ identity = nullptr;
		try
		{
			identity = ident_task.get();
			if (identity)
			{
				if (identity->Tickets->Size)
				{
					success = true;
				}
			}
		}
		catch (Exception^ e)
		{
			const WCHAR* errMsg = e->Message->Data();
			WriteDebugStringW(L"Failed to SignIn()-Failed to get toeken , %s\n", errMsg);
		}
		SetNotBusy();

		if (!success)
			return;

		auto ticket = identity->Tickets->GetAt(0);
		String^ token = ticket->Value;

		HttpClient^	httpClient = ref new HttpClient();

		String^ cmd = L"https://apis.live.net/v5.0/me?access_token=" + token;
		Uri^	uri = ref new Uri(cmd);

		SetBusy();
		auto task_folder = create_task(httpClient->GetStringAsync(uri));
		task_folder.then([this, token](task<String^> task_json)
		{
			String^ json = nullptr;

			try
			{
				json = task_json.get();
			}
			catch (Exception^ e)
			{
				const WCHAR* errMsg = e->Message->Data();
				WriteDebugStringW(L"Failed to SignIn() , failed to get root , %s\n", errMsg);
			}
			SetNotBusy();

			if (!json)
			{
				return;
			}
			String^ UserName = GetUserName(json);
			OnSignIn(token, UserName);
		});
	});
}
void OneDriveService::OnSignIn(String^ token, String^ UserName)
{
	m_AccessToken = token;
	m_UserName = UserName;

	OneDriveAccess::App^ app = safe_cast<OneDriveAccess::App^>(OneDriveAccess::App::Current);
	app->UpdateSigninStatus();

}

void OneDriveService::RequestFolder(FileItem^ folder)
{
	if (IsBusy())
		return;

#ifdef _DEBUG
	if (folder)
	{
		if (folder->GetType() != SKY_FILE_TYPE_FOLDER)
			__debugbreak();
	}
#endif
	HttpClient^	httpClient = ref new HttpClient();

	auto headerAuth = ref new Windows::Web::Http::Headers::HttpCredentialsHeaderValue(ref new String(BEARER), m_AccessToken);
	httpClient->DefaultRequestHeaders->Authorization = headerAuth;

	String^ cmd = nullptr;

	if (folder)
	{
		cmd = ref new String(ROOT_URL) + L"/" + folder->ID + L"/files";
	}
	else
	{
		cmd = ref new String(ROOT_URL) + L"/me/skydrive/files";
	}

	auto uri = ref new Uri(cmd);

	SetBusy();

	//auto thread_context = task_continuation_context::use_arbitrary();
	auto task_folder = create_task(httpClient->GetStringAsync(uri));
	task_folder.then([this, folder](task<String^> task_json)
	{
		String^ json = nullptr;

		try
		{
			json = task_json.get();
		}
		catch (Exception^ e)
		{
			const WCHAR* errMsg = e->Message->Data();
			WriteDebugStringW(L"Failed to RequestFolder() , %s\n", errMsg);
		}
		SetNotBusy();

		if (!json)
		{
			return;
		}

		Vector<FileItem^>^ items = CreateFileItemsInfoFromJson(json, folder);

		m_CurFolder = folder;
		m_CurItems = items;

		OneDriveAccess::App^ app = safe_cast<OneDriveAccess::App^>(OneDriveAccess::App::Current);
		app->UpdateFolderStatus();
	});

}


void OneDriveService::RequestParentFolder()
{
	if (!m_CurFolder)
	{
		// 현재 root인 경우
		return;
	}
	FileItem^	parent = m_CurFolder->GetParentFolder();

	RequestFolder(parent);
}
void OneDriveService::DownloadFile(FileItem^ file)
{
#ifdef _DEBUG
	if (file->GetType() == SKY_FILE_TYPE_FOLDER)
		__debugbreak();
#endif

	if (IsBusy())
		return;

	// GET https://apis.live.net/v5.0/file.a6b2a7e8f2515e5e.A6B2A7E8F2515E5E!126/content?access_token=ACCESS_TOKEN


	// select folder
	auto task_folder = SelectFolder();
	task_folder.then([this, file](task<StorageFolder^> task_folder)
	{
		StorageFolder^ folder = nullptr;
		try
		{
			folder = task_folder.get();
		}
		catch (Exception^ e)
		{
			const WCHAR* errMsg = e->Message->Data();
			WriteDebugStringW(L"Failed to SaveFile() - OpenAsync(), %s\n", errMsg);
		}
		if (!folder)
			return;

		HttpClient^	httpClient = ref new HttpClient();
		String^ cmd = ref new String(ROOT_URL) + L"/" + file->ID + "/content?access_token=" + m_AccessToken;;
		auto uri = ref new Uri(cmd);

		String^ FileName = file->FileName;

		SetBusy();
		create_task(httpClient->GetAsync(uri)).then([this, folder, FileName](task<HttpResponseMessage^> task_response)
		{
			HttpResponseMessage^ response = nullptr;
			bool success = false;
			try
			{
				response = task_response.get();
				success = response->IsSuccessStatusCode;
			}
			catch (Exception^ e)
			{
				const WCHAR* errMsg = e->Message->Data();
				WriteDebugStringW(L"Failed to DownloadFile() , %s\n", errMsg);
			}

			SetNotBusy();

			if (!success)
				return;

			auto header = response->Content->Headers;
			auto type = header->ContentType;
			auto size = header->ContentLength;
			auto loc = header->ContentLocation;
			String^ locstr = loc->AbsoluteUri;

			auto content = response->Content;

			SetBusy();
			create_task(content->ReadAsBufferAsync()).then([this, folder, FileName, content](Windows::Storage::Streams::IBuffer^ buffer)
			{
				SetNotBusy();

				UINT size = buffer->Length;
				if (size > 0)
				{
					BYTE*	pDest = (BYTE*)malloc(size);
					Windows::Storage::Streams::DataReader^ dataReader = Windows::Storage::Streams::DataReader::FromBuffer(buffer);
					dataReader->ReadBytes(ArrayReference<unsigned char>(pDest, size));

					SaveFile(folder, FileName, pDest, size);
					/*
					FILE*	fp = nullptr;
					wfopen_with_local(&fp, FileName->Data(), L"wb");
					if (fp)
					{
						fwrite(pDest, size, 1, fp);
						fclose(fp);
					}
					free(pDest);
					*/
				}
			});
		});
	});
}

void OneDriveService::UploadFile(String^ FileName, Windows::Storage::StorageFile^ file, UINT64 ui64FileSize)
{
	String^ FolderID = L"me/skydrive";
	if (m_CurFolder)
	{
		FolderID = m_CurFolder->ID;
	}
	SetBusy();
	auto open_task = create_task(file->OpenSequentialReadAsync());
	open_task.then([this, FolderID, file, FileName](task<Windows::Storage::Streams::IInputStream^> task_stream)
	{
		Windows::Storage::Streams::IInputStream^ stream = nullptr;
		try
		{
			stream = task_stream.get();
		}
		catch (Exception^ e)
		{
			const WCHAR* errMsg = e->Message->Data();
			WriteDebugStringW(L"Failed to UploadFile() , %s\n", errMsg);
		}
		SetNotBusy();
		if (!stream)
			return;

		//PUT https://apis.live.net/v5.0/me/skydrive/files/HelloWorld.txt?access_token=ACCESS_TOKEN
		//POST https://apis.live.net/v5.0/me/skydrive/files?access_token=ACCESS_TOKEN

		String^ cmd = ref new String(ROOT_URL) + L"/" + FolderID + L"/files/" + FileName;
		Uri^ uri = ref new Uri(cmd);

		HttpClient^	httpClient = ref new HttpClient();
		auto headerAuth = ref new Windows::Web::Http::Headers::HttpCredentialsHeaderValue(ref new String(BEARER), m_AccessToken);
		httpClient->DefaultRequestHeaders->Authorization = headerAuth;

		HttpStreamContent^ content = ref new HttpStreamContent(stream);

		SetBusy();
		auto task_put = create_task(httpClient->PutAsync(uri, content));
		task_put.then([this](task<HttpResponseMessage^> task_response)
		{
			HttpResponseMessage^ response = nullptr;
			bool	success = false;
			try
			{
				response = task_response.get();
				success = response->IsSuccessStatusCode;
			}
			catch (Exception^ e)
			{
				const WCHAR* errMsg = e->Message->Data();
				WriteDebugStringW(L"Failed to UploadFile() - PutAsync(), %s\n", errMsg);
			}
			SetNotBusy();
			if (!response)
				return;

			create_task(response->Content->ReadAsStringAsync()).then([this](task<String^> task_json)
			{
				String^ json = nullptr;
				try
				{
					json = task_json.get();
				}
				catch (Exception^ e)
				{
					const WCHAR* errMsg = e->Message->Data();
					WriteDebugStringW(L"Failed to UploadFile() - ReadAsStringAsync() , %s\n", errMsg);
				}
				if (!json)
					return;

				FileItem^ file = CreateFileItemInfoFromJson(json, m_CurFolder);
				if (file)
				{
					m_CurItems->Append(file);
					OneDriveAccess::App^ app = safe_cast<OneDriveAccess::App^>(OneDriveAccess::App::Current);
					app->UpdateFolderStatus();
				}
			});
		});
	});
}
String^ OneDriveService::GetUserName(String^ jsonStr)
{
	String^ Name = nullptr;
	Vector<FileItem^>^	files = ref new Vector<FileItem^>();

	Windows::Data::Json::JsonObject^ tokenResponse = ref new JsonObject();

	if (!JsonObject::TryParse(jsonStr, &tokenResponse))
		return nullptr;

	auto map = tokenResponse->GetView();

	for each (auto item in map)
	{
		if (item->Key == L"name")
		{
			Name = item->Value->GetString();
			break;
		}
	}
	return Name;
}
Vector<FileItem^>^ OneDriveService::CreateFileItemsInfoFromJson(String^ jsonStr, FileItem^ ParentFolder)
{
	Vector<FileItem^>^	files = ref new Vector<FileItem^>();

	Windows::Data::Json::JsonObject^ tokenResponse = ref new JsonObject();

	if (!JsonObject::TryParse(jsonStr, &tokenResponse))
		return nullptr;

	auto map = tokenResponse->GetView();


	IJsonValue^ value = map->Lookup("data");
	String^ s = value->Stringify();

	JsonArray^ mapValue = ref new JsonArray();
	if (JsonArray::TryParse(s, &mapValue))
	{
		auto vec = mapValue->GetView();

		for each(auto item in vec)
		{
			auto vtype = item->ValueType;
			switch (vtype)
			{
			case JsonValueType::Object:
			{
				JsonObject^	obj = item->GetObject();
				FileItem^	fileItem = CreateFileItem(obj->GetView());
				fileItem->SetParentFolder(ParentFolder);
				files->Append(fileItem);
			}
			break;
			default:
				__debugbreak();
			}
		}
	}

	return files;
}
FileItem^ OneDriveService::CreateFileItemInfoFromJson(String^ jsonStr, FileItem^ ParentFolder)
{
	Windows::Data::Json::JsonObject^ tokenResponse = ref new JsonObject();

	if (!JsonObject::TryParse(jsonStr, &tokenResponse))
		return nullptr;

	auto map = tokenResponse->GetView();
	FileItem^	fileItem = CreateFileItem(map);
	if (fileItem)
	{
		fileItem->SetParentFolder(ParentFolder);
	}
	return fileItem;
}
FileItem^ OneDriveService::CreateFileItem(IMapView<Platform::String^, Windows::Data::Json::IJsonValue^>^ view)
{
	FileItem^	fileItem = nullptr;
	String^ Name = nullptr;
	String^ Title = nullptr;
	String^ ID = nullptr;
	String^ ParentID = nullptr;

	SKY_FILE_TYPE	type = SKY_FILE_TYPE_ETC;

	for each (auto item in view)
	{
		String^ key = item->Key;

		if (key == L"name")
		{
			Name = item->Value->GetString();
		}
		if (key == L"id")
		{
			ID = item->Value->GetString();
		}
		if (key == L"type")
		{
			String^ value = item->Value->GetString();
			if (L"folder" == value || L"album" == value)
			{
				type = SKY_FILE_TYPE_FOLDER;
			}
			else if (L"photo" == value)
			{
				type = SKY_FILE_TYPE_PHOTO;
			}
			else if (L"audio" == value)
			{
				type = SKY_FILE_TYPE_AUDIO;
			}
			else
			{
				type = SKY_FILE_TYPE_ETC;
			}
		}
		if (key == L"title")
		{
			auto value_type = item->Value->ValueType;
			if (value_type == JsonValueType::String)
			{
				Title = item->Value->GetString();
			}
		}
		if (key == L"parent_id")
		{
			ParentID = item->Value->GetString();
		}

	}
	if (Name && ID)
	{
		fileItem = ref new FileItem;
		fileItem->FileName = Name;
		fileItem->ID = ID;
		fileItem->ParentID = ParentID;
		fileItem->Title = Title;
		fileItem->SetType(type);
	}
	return fileItem;
}
String^ OneDriveService::GetCurFolderName()
{
	String^ Name = nullptr;
	if (m_CurFolder)
	{
		Name = m_CurFolder->FileName;
	}
	return Name;
}
void OneDriveService::SignOut()
{
	if (IsBusy())
		return;

	bool bCanSignOut = m_Authenticator->CanSignOut;

	//m_Authenticator->ApplicationId

	//if (bCanSignOut)
	{
		SetBusy();

		auto task_signout = create_task(m_Authenticator->SignOutUserAsync());
		task_signout.then([this](task<void> t)
		{
			try
			{
				t.get();
			}
			catch (Exception^ e)
			{
				const WCHAR* err = e->Message->Data();
			}
			SetNotBusy();

			OneDriveAccess::App^ app = safe_cast<OneDriveAccess::App^>(OneDriveAccess::App::Current);
			m_AccessToken = nullptr;
			m_UserName = nullptr;
			app->UpdateSigninStatus();
		});
	}
}


BOOL wfopen_with_local(FILE** pp_out_fp, const WCHAR* wchFileName, WCHAR* wchIOMode)
{
	BOOL	bResult = FALSE;
	FILE*	fp = NULL;

	Windows::ApplicationModel::Package^ package = Windows::ApplicationModel::Package::Current;
	Windows::Storage::StorageFolder^ installedLocation = package->InstalledLocation;

	// 인스톨된 폴더 쓰기 불가
	const	WCHAR*	wchInstalledLocation = installedLocation->Path->Data();

	// 로컬 폴더. 쓰기 가능
	Windows::Storage::StorageFolder^ Folder = Windows::Storage::ApplicationData::Current->LocalFolder;

	// 로컬폴더 오브젝트로부터 path 문자열을 얻는다.
	Platform::String^ Path = Folder->Path;

	const	WCHAR*	wchPath = Path->Data();

	WCHAR	wchFullPathFileName[1024];
	swprintf_s(wchFullPathFileName, L"%s\\%s", wchPath, wchFileName);


	_wfopen_s(&fp, wchFullPathFileName, wchIOMode);

	if (!fp)
		goto lb_return;

	*pp_out_fp = fp;
	bResult = TRUE;
lb_return:
	return bResult;

}