#pragma once

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Data::Json;
using namespace Windows::Foundation::Collections;
using namespace Windows::Security::Authentication::OnlineId;

namespace OneDriveAccess
{
	ref class OneDriveService sealed
	{
		OnlineIdAuthenticator^ m_Authenticator = nullptr;
		DWORD	m_dwThreadID = 0;
		bool	m_bIsRequestComplted = false;
		bool	m_bIsBusy = false;

		Platform::String^ m_AccessToken = nullptr;
		Platform::String^ m_UserName = nullptr;

		
		const WCHAR* ROOT_URL = L"https://apis.live.net/v5.0";
		const WCHAR* BEARER = L"Bearer";
		
		FileItem^	m_CurFolder = nullptr;
		Vector<FileItem^>^	m_CurItems = nullptr;

		String^ GetUserName(String^ jsonStr);

		FileItem^ CreateFileItem(IMapView<Platform::String^, Windows::Data::Json::IJsonValue^>^ view);
		Vector<FileItem^>^ CreateFileItemsInfoFromJson(String^ jsonStr,FileItem^ ParentFolder);
		FileItem^ CreateFileItemInfoFromJson(String^ jsonStr, FileItem^ ParentFolder);
		
	public:
	
		
		OneDriveService(String^ ClientID);
	internal:
		void OnSignIn(String^ token,String^ UserName);
		String^ GetAccessToken()
		{
			return m_AccessToken;
		}
		IVectorView<FileItem^>^ GetItemsView() 
		{
			auto view = m_CurItems->GetView();
			return view;
		}
	
		bool	IsBusy()	{return m_bIsBusy;}
		void	SetBusy();
		void	SetNotBusy();
		String^ GetCurFolderName();
		String^ GetUserName()	{return m_UserName;}
		void SignIn();
		void SignOut();
		
		
		void RequestFolder(FileItem^ folder);
		void RequestParentFolder();
		void DownloadFile(FileItem^ file);
		void UploadFile(String^ FileName, Windows::Storage::StorageFile^ file, UINT64 ui64FileSize);
		

	};
};
