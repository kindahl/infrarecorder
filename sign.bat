@echo off

set    cert_file=   %~dp0christian_kindahl.pfx
set /p cert_pass= < %~dp0christian_kindahl.psw
set    cert_sats=   "http://timestamp.comodoca.com/authenticode"

set path_w32r=%~dp0bin\win32\release\
set path_w32p=%~dp0bin\win32\releasep\
set path_x64r=%~dp0bin\x64\release\
set path_x64p=%~dp0bin\x64\releasep\

rem set path_dist=%~dp0dist\
set path_smoke_w32=%~dp0dep\smoke\win32\
set path_smoke_x64=%~dp0dep\smoke\x64\

set path_cdrtools=%~dp0dep\cdrtools\
set path_sndfile_w32=%~dp0dep\libsndfile\win32\
set path_sndfile_x64=%~dp0dep\libsndfile\x64\

if "%~1" NEQ "" goto single_file

signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_smoke_w32%smoke.exe
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_smoke_x64%smoke.exe
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_sndfile_w32%libsndfile-1.dll
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_sndfile_x64%libsndfile-1.dll

signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_cdrtools%cdda2wav.exe
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_cdrtools%cdrecord.exe
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_cdrtools%readcd.exe
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_cdrtools%cygwin1.dll

signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_w32r%infrarecorder.exe
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_w32r%shell.dll
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_w32r%codecs\lame.irc
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_w32r%codecs\sndfile.irc
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_w32r%codecs\wave.irc
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_w32r%codecs\wma.irc
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_w32r%codecs\vorbis.irc

signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_w32p%infrarecorder.exe
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_w32p%codecs\lame.irc
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_w32p%codecs\sndfile.irc
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_w32p%codecs\wave.irc
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_w32p%codecs\wma.irc
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_w32p%codecs\vorbis.irc

signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_x64r%infrarecorder.exe
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_x64r%shell.dll
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_x64r%codecs\lame.irc
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_x64r%codecs\sndfile.irc
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_x64r%codecs\wave.irc
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_x64r%codecs\wma.irc
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_x64r%codecs\vorbis.irc

signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_x64p%infrarecorder.exe
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_x64p%codecs\lame.irc
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_x64p%codecs\sndfile.irc
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_x64p%codecs\wave.irc
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_x64p%codecs\wma.irc
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_x64p%codecs\vorbis.irc

rem signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_dist%ir.exe
rem signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_dist%ir_x64.msi
goto end

:single_file

signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %1
goto end

:end

pause
