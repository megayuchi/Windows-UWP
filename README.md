# Windows-UWP

Sample codes of Windows 10 UWP Apps by megayuchi Source code that was demonstrated at MS Techdays 2015 in SEOUL ( https://doc.co/jERsyS )

[Requirements] 1. Windows 10 build 10240 or later 2. Visual Studio 2015

•RefClass - ref class sample in UWP.

•FileOpenUWP - File Open Sample in UWP. 


•FFMPEG_UWP - DirectX 11 and ffmepg interop sample in UWP.

[Build]
1.Download latest ffmpeg build. ( https://github.com/FFmpeg/FFmpeg )
2.Build ffmpeg for WinRT. ( https://trac.ffmpeg.org/wiki/CompilationGuide/WinRT )
3.Copy ffmpeg's binary files to Windows-UWP\FFMPEG_UWP\ffmpeg folder.
4.Run copy_dlls.bat in Windows-UWP\FFMPEG_UWP folder.
5.Build Windows-UWP\FFMPEG_UWP\Decoder\Decoder.vcxproj Project.
6.Build and run Windows-UWP\FFMPEG_UWP\D3DVideoPlayer\D3DVideoPlayer.vcxproj Project.
