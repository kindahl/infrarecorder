@echo off

set    cert_file=   christian_kindahl.pfx
set /p cert_pass= < christian_kindahl.psw
set    cert_sats=   "http://timestamp.comodoca.com/authenticode"

set path_w32r=%~dp0bin\win32\release\
set path_w32p=%~dp0bin\win32\releasep\
set path_x64r=%~dp0bin\x64\release\
set path_x64p=%~dp0bin\x64\releasep\

set path_dist=%~dp0dist\
set path_smoke=%~dp0dep\

signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_smoke%smoke.exe

signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_w32r%infrarecorder.exe
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_w32r%shell.dll
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_w32r%codecs\lame.irc
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_w32r%codecs\sndfile.irc
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_w32r%codecs\wave.irc
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_w32r%codecs\wma.irc
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_w32r%codecs\vorbis.irc

signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_w32p%infrarecorder.exe

signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_x64r%infrarecorder.exe
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_x64r%shell.dll
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_x64r%codecs\lame.irc
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_x64r%codecs\sndfile.irc
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_x64r%codecs\wave.irc
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_x64r%codecs\wma.irc
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_x64r%codecs\vorbis.irc

signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_x64p%infrarecorder.exe

signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_dist%ir.exe
signtool sign /f %cert_file% /p %cert_pass% /t %cert_sats% %path_dist%ir.msi
