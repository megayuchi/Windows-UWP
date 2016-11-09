#pragma once

namespace OneDriveAccess
{
	enum SKY_FILE_TYPE
	{
		SKY_FILE_TYPE_NONE = -1,
		SKY_FILE_TYPE_FOLDER,
		SKY_FILE_TYPE_PHOTO,
		SKY_FILE_TYPE_AUDIO,
		SKY_FILE_TYPE_ETC
	};

	[Windows::UI::Xaml::Data::Bindable]
	[Windows::Foundation::Metadata::WebHostHiddenAttribute]
	public ref class FileItem sealed
	{	
		FileItem^ m_ParentFolder = nullptr;
		Platform::String^ _Title = nullptr;
		Platform::String^ _FileName = nullptr;
		SKY_FILE_TYPE	m_Type = SKY_FILE_TYPE_NONE;
		Windows::UI::Xaml::Media::ImageSource^ _Image;

		event Windows::UI::Xaml::Data::PropertyChangedEventHandler^ _PropertyChanged;
		Windows::UI::Xaml::Input::ICommand^ m_playCommand;
		Windows::UI::Xaml::Input::ICommand^ m_downloadCommand;
		Windows::UI::Xaml::Input::ICommand^ m_deleteCommand;
		
		void	OnCommandPlay(Object^ parameter);
		bool	OnCommandCanPlaying(Object^ parameter);
		void	OnCommandDownload(Object^ parameter);
		bool	OnCommandCanDownload(Object^ parameter);
		void	OnCommandDelete(Object^ parameter);
		bool	OnCommandCanDelete(Object^ parameter);

		void SetImage();
	public:
		property	int	No;
		property	Platform::String^ ID;
		property	Platform::String^ ParentID;
		

		FileItem();
		


		void OnPropertyChanged(Platform::String^ propertyName)
		{
			Windows::UI::Xaml::Data::PropertyChangedEventArgs^ pcea = ref new  Windows::UI::Xaml::Data::PropertyChangedEventArgs(propertyName);
			_PropertyChanged(this, pcea);
		}


		//Title
		property Platform::String^ Title
		{
			Platform::String^ get()
			{
				return _Title;
			}
			void set(Platform::String^ value)
			{
				_Title = value;
				OnPropertyChanged("Title");
			}
		}



		//FileName
		property Platform::String^ FileName
		{
			Platform::String^ get()
			{
				return _FileName;
			}
			void set(Platform::String^ value)
			{
				_FileName = value;
				OnPropertyChanged("FileName");
			}
		}

		

		//Image
		property Windows::UI::Xaml::Media::ImageSource^ Image
		{
			Windows::UI::Xaml::Media::ImageSource^ get()
			{
				return _Image;
			}
			void set(Windows::UI::Xaml::Media::ImageSource^ value)
			{
				_Image = value;
				OnPropertyChanged("Image");
			}
		}

		void SetImage(Windows::Foundation::Uri^ baseUri, Platform::String^ path) //SetImage() in C# code
		{
			Windows::Foundation::Uri^ uri = ref new Windows::Foundation::Uri("ms-appx:///" + path);
			Image = ref new Windows::UI::Xaml::Media::Imaging::BitmapImage(uri);
		}
		property Windows::UI::Xaml::Input::ICommand^ PlayCommand
        {
            Windows::UI::Xaml::Input::ICommand^ get()
			{
				return m_playCommand;
			}
        }
		property Windows::UI::Xaml::Input::ICommand^ DownloadCommand
        {
            Windows::UI::Xaml::Input::ICommand^ get()
			{
				return m_downloadCommand;
			}
        }
		
		property Windows::UI::Xaml::Input::ICommand^ DeleteCommand
        {
            Windows::UI::Xaml::Input::ICommand^ get()
			{
				return m_deleteCommand;
			}
        }
		property  Windows::UI::Xaml::Visibility IsVisiblePlayButton
        {
			Windows::UI::Xaml::Visibility get()
			{
				Windows::UI::Xaml::Visibility visibility = Windows::UI::Xaml::Visibility::Collapsed;

				if (m_Type == SKY_FILE_TYPE_AUDIO)
				{
					visibility = Windows::UI::Xaml::Visibility::Visible;
				}
				return visibility;
			}

        }
		property  Windows::UI::Xaml::Visibility IsVisibleStopButton
        {
			Windows::UI::Xaml::Visibility get()
			{
				Windows::UI::Xaml::Visibility visibility = Windows::UI::Xaml::Visibility::Collapsed;

				if (m_Type == SKY_FILE_TYPE_AUDIO)
				{
					visibility = Windows::UI::Xaml::Visibility::Visible;
				}
				return visibility;
			}
        }
		property  Windows::UI::Xaml::Visibility IsVisibleDownloadButton
        {
			Windows::UI::Xaml::Visibility get()
			{
				Windows::UI::Xaml::Visibility visibility = Windows::UI::Xaml::Visibility::Collapsed;

				if (m_Type != SKY_FILE_TYPE_FOLDER)
				{
					visibility = Windows::UI::Xaml::Visibility::Visible;
				}
				return visibility;
			}
        }
		
	internal:
		void	SetParentFolder(FileItem^ folder)	{m_ParentFolder = folder;}
		FileItem^	GetParentFolder()	{return m_ParentFolder;} 
		void SetType(OneDriveAccess::SKY_FILE_TYPE type);

		OneDriveAccess::SKY_FILE_TYPE GetType()	{return m_Type;}
	};
}
