@echo off

set path_w32d=%~dp0bin\win32\debug\
set path_w32r=%~dp0bin\win32\release\
set path_w32p=%~dp0bin\win32\releasep\
set path_x64d=%~dp0bin\x64\debug\
set path_x64r=%~dp0bin\x64\release\
set path_x64p=%~dp0bin\x64\releasep\

set path_trans_l=%~dp0etc\translations\software\
set path_trans_h=%~dp0etc\translations\help\
set path_cdrtools=%~dp0dep\cdrtools\
set path_sndfile=%~dp0dep\libsndfile\
set path_smoke=%~dp0dep\
set path_help=%~dp0doc\english\

rem make sure directories exists.
if not exist %path_w32d%codecs\ mkdir %path_w32d%codecs\
if not exist %path_w32r%codecs\ mkdir %path_w32r%codecs\
if not exist %path_w32p%codecs\ mkdir %path_w32p%codecs\
if not exist %path_x64d%codecs\ mkdir %path_x64d%codecs\
if not exist %path_x64r%codecs\ mkdir %path_x64r%codecs\
if not exist %path_x64p%codecs\ mkdir %path_x64p%codecs\
if not exist %path_w32d%languages\ mkdir %path_w32d%languages\
if not exist %path_w32r%languages\ mkdir %path_w32r%languages\
if not exist %path_w32p%languages\ mkdir %path_w32p%languages\
if not exist %path_x64d%languages\ mkdir %path_x64d%languages\
if not exist %path_x64r%languages\ mkdir %path_x64r%languages\
if not exist %path_x64p%languages\ mkdir %path_x64p%languages\

rem make symbolic links to cdrtools binaries.
if not exist %path_w32d%cdrtools mklink /D %path_w32d%cdrtools %path_cdrtools%
if not exist %path_w32r%cdrtools mklink /D %path_w32r%cdrtools %path_cdrtools%
if not exist %path_w32p%cdrtools mklink /D %path_w32p%cdrtools %path_cdrtools%
if not exist %path_x64d%cdrtools mklink /D %path_x64d%cdrtools %path_cdrtools%
if not exist %path_x64r%cdrtools mklink /D %path_x64r%cdrtools %path_cdrtools%
if not exist %path_x64p%cdrtools mklink /D %path_x64p%cdrtools %path_cdrtools%

rem make symbolic links to libsndfile libraries.
if not exist %path_w32d%codecs\libsndfile.dll mklink %path_w32d%codecs\libsndfile.dll %path_sndfile%win32\libsndfile.dll
if not exist %path_w32r%codecs\libsndfile.dll mklink %path_w32r%codecs\libsndfile.dll %path_sndfile%win32\libsndfile.dll
if not exist %path_w32p%codecs\libsndfile.dll mklink %path_w32p%codecs\libsndfile.dll %path_sndfile%win32\libsndfile.dll
if not exist %path_x64d%codecs\libsndfile.dll mklink %path_x64d%codecs\libsndfile.dll %path_sndfile%x64\libsndfile.dll
if not exist %path_x64r%codecs\libsndfile.dll mklink %path_x64r%codecs\libsndfile.dll %path_sndfile%x64\libsndfile.dll
if not exist %path_x64p%codecs\libsndfile.dll mklink %path_x64p%codecs\libsndfile.dll %path_sndfile%x64\libsndfile.dll

rem make symbolic links to smoke program.
if not exist %path_w32d%smoke.exe mklink %path_w32d%smoke.exe %path_smoke%smoke.exe
if not exist %path_w32r%smoke.exe mklink %path_w32r%smoke.exe %path_smoke%smoke.exe
if not exist %path_w32p%smoke.exe mklink %path_w32p%smoke.exe %path_smoke%smoke.exe
if not exist %path_x64d%smoke.exe mklink %path_x64d%smoke.exe %path_smoke%smoke.exe
if not exist %path_x64r%smoke.exe mklink %path_x64r%smoke.exe %path_smoke%smoke.exe
if not exist %path_x64p%smoke.exe mklink %path_x64p%smoke.exe %path_smoke%smoke.exe

rem make symbolic links to help file.
if not exist %path_w32d%infrarecorder.chm mklink %path_w32d%infrarecorder.chm %path_help%infrarecorder.chm
if not exist %path_w32r%infrarecorder.chm mklink %path_w32r%infrarecorder.chm %path_help%infrarecorder.chm
if not exist %path_w32p%infrarecorder.chm mklink %path_w32p%infrarecorder.chm %path_help%infrarecorder.chm
if not exist %path_x64d%infrarecorder.chm mklink %path_x64d%infrarecorder.chm %path_help%infrarecorder.chm
if not exist %path_x64r%infrarecorder.chm mklink %path_x64r%infrarecorder.chm %path_help%infrarecorder.chm
if not exist %path_x64p%infrarecorder.chm mklink %path_x64p%infrarecorder.chm %path_help%infrarecorder.chm

rem make symbolic links to translations.
if not exist %path_w32d%languages\albanian.irl             mklink %path_w32d%languages\albanian.irl             %path_trans_l%albanian.irl
if not exist %path_w32d%languages\arabic.irl               mklink %path_w32d%languages\arabic.irl               %path_trans_l%arabic.irl
if not exist %path_w32d%languages\armenian.irl             mklink %path_w32d%languages\armenian.irl             %path_trans_l%armenian.irl
if not exist %path_w32d%languages\basque.irl               mklink %path_w32d%languages\basque.irl               %path_trans_l%basque.irl
if not exist %path_w32d%languages\bosnian.irl              mklink %path_w32d%languages\bosnian.irl              %path_trans_l%bosnian.irl
if not exist %path_w32d%languages\bulgarian.irl            mklink %path_w32d%languages\bulgarian.irl            %path_trans_l%bulgarian.irl
if not exist %path_w32d%languages\catalan.irl              mklink %path_w32d%languages\catalan.irl              %path_trans_l%catalan.irl
if not exist %path_w32d%languages\chinese-simplified.irl   mklink %path_w32d%languages\chinese-simplified.irl   %path_trans_l%chinese-simplified.irl
if not exist %path_w32d%languages\chinese-traditional.irl  mklink %path_w32d%languages\chinese-traditional.irl  %path_trans_l%chinese-traditional.irl
if not exist %path_w32d%languages\chuvash.irl              mklink %path_w32d%languages\chuvash.irl              %path_trans_l%chuvash.irl
if not exist %path_w32d%languages\croatian.irl             mklink %path_w32d%languages\croatian.irl             %path_trans_l%croatian.irl
if not exist %path_w32d%languages\czech.irl                mklink %path_w32d%languages\czech.irl                %path_trans_l%czech.irl
if not exist %path_w32d%languages\danish.irl               mklink %path_w32d%languages\danish.irl               %path_trans_l%danish.irl
if not exist %path_w32d%languages\dutch.irl                mklink %path_w32d%languages\dutch.irl                %path_trans_l%dutch.irl
if not exist %path_w32d%languages\estonian.irl             mklink %path_w32d%languages\estonian.irl             %path_trans_l%estonian.irl
if not exist %path_w32d%languages\farsi.irl                mklink %path_w32d%languages\farsi.irl                %path_trans_l%farsi.irl
if not exist %path_w32d%languages\finnish.irl              mklink %path_w32d%languages\finnish.irl              %path_trans_l%finnish.irl
if not exist %path_w32d%languages\french.irl               mklink %path_w32d%languages\french.irl               %path_trans_l%french.irl
if not exist %path_w32d%languages\galician.irl             mklink %path_w32d%languages\galician.irl             %path_trans_l%galician.irl
if not exist %path_w32d%languages\german.irl               mklink %path_w32d%languages\german.irl               %path_trans_l%german.irl
if not exist %path_w32d%languages\greek.irl                mklink %path_w32d%languages\greek.irl                %path_trans_l%greek.irl
if not exist %path_w32d%languages\hebrew.irl               mklink %path_w32d%languages\hebrew.irl               %path_trans_l%hebrew.irl
if not exist %path_w32d%languages\hungarian.irl            mklink %path_w32d%languages\hungarian.irl            %path_trans_l%hungarian.irl
if not exist %path_w32d%languages\indonesian.irl           mklink %path_w32d%languages\indonesian.irl           %path_trans_l%indonesian.irl
if not exist %path_w32d%languages\italian.irl              mklink %path_w32d%languages\italian.irl              %path_trans_l%italian.irl
if not exist %path_w32d%languages\japanese.irl             mklink %path_w32d%languages\japanese.irl             %path_trans_l%japanese.irl
if not exist %path_w32d%languages\korean.irl               mklink %path_w32d%languages\korean.irl               %path_trans_l%korean.irl
if not exist %path_w32d%languages\latvian.irl              mklink %path_w32d%languages\latvian.irl              %path_trans_l%latvian.irl
if not exist %path_w32d%languages\lithuanian.irl           mklink %path_w32d%languages\lithuanian.irl           %path_trans_l%lithuanian.irl
if not exist %path_w32d%languages\macedonian.irl           mklink %path_w32d%languages\macedonian.irl           %path_trans_l%macedonian.irl
if not exist %path_w32d%languages\norwegian.irl            mklink %path_w32d%languages\norwegian.irl            %path_trans_l%norwegian.irl
if not exist %path_w32d%languages\polish.irl               mklink %path_w32d%languages\polish.irl               %path_trans_l%polish.irl
if not exist %path_w32d%languages\portuguese.irl           mklink %path_w32d%languages\portuguese.irl           %path_trans_l%portuguese.irl
if not exist %path_w32d%languages\portuguese-brazilian.irl mklink %path_w32d%languages\portuguese-brazilian.irl %path_trans_l%portuguese-brazilian.irl
if not exist %path_w32d%languages\romanian.irl             mklink %path_w32d%languages\romanian.irl             %path_trans_l%romanian.irl
if not exist %path_w32d%languages\russian.irl              mklink %path_w32d%languages\russian.irl              %path_trans_l%russian.irl
if not exist %path_w32d%languages\serbian-cyrillic.irl     mklink %path_w32d%languages\serbian-cyrillic.irl     %path_trans_l%serbian-cyrillic.irl
if not exist %path_w32d%languages\serbian-latin.irl        mklink %path_w32d%languages\serbian-latin.irl        %path_trans_l%serbian-latin.irl
if not exist %path_w32d%languages\slovak.irl               mklink %path_w32d%languages\slovak.irl               %path_trans_l%slovak.irl
if not exist %path_w32d%languages\slovenian.irl            mklink %path_w32d%languages\slovenian.irl            %path_trans_l%slovenian.irl
if not exist %path_w32d%languages\spanish.irl              mklink %path_w32d%languages\spanish.irl              %path_trans_l%spanish.irl
if not exist %path_w32d%languages\swedish.irl              mklink %path_w32d%languages\swedish.irl              %path_trans_l%swedish.irl
if not exist %path_w32d%languages\thai.irl                 mklink %path_w32d%languages\thai.irl                 %path_trans_l%thai.irl
if not exist %path_w32d%languages\turkish.irl              mklink %path_w32d%languages\turkish.irl              %path_trans_l%turkish.irl
if not exist %path_w32d%languages\ukrainian.irl            mklink %path_w32d%languages\ukrainian.irl            %path_trans_l%ukrainian.irl
if not exist %path_w32d%languages\valencian.irl            mklink %path_w32d%languages\valencian.irl            %path_trans_l%valencian.irl
if not exist %path_w32r%languages\albanian.irl             mklink %path_w32r%languages\albanian.irl             %path_trans_l%albanian.irl
if not exist %path_w32r%languages\arabic.irl               mklink %path_w32r%languages\arabic.irl               %path_trans_l%arabic.irl
if not exist %path_w32r%languages\armenian.irl             mklink %path_w32r%languages\armenian.irl             %path_trans_l%armenian.irl
if not exist %path_w32r%languages\basque.irl               mklink %path_w32r%languages\basque.irl               %path_trans_l%basque.irl
if not exist %path_w32r%languages\bosnian.irl              mklink %path_w32r%languages\bosnian.irl              %path_trans_l%bosnian.irl
if not exist %path_w32r%languages\bulgarian.irl            mklink %path_w32r%languages\bulgarian.irl            %path_trans_l%bulgarian.irl
if not exist %path_w32r%languages\catalan.irl              mklink %path_w32r%languages\catalan.irl              %path_trans_l%catalan.irl
if not exist %path_w32r%languages\chinese-simplified.irl   mklink %path_w32r%languages\chinese-simplified.irl   %path_trans_l%chinese-simplified.irl
if not exist %path_w32r%languages\chinese-traditional.irl  mklink %path_w32r%languages\chinese-traditional.irl  %path_trans_l%chinese-traditional.irl
if not exist %path_w32r%languages\chuvash.irl              mklink %path_w32r%languages\chuvash.irl              %path_trans_l%chuvash.irl
if not exist %path_w32r%languages\croatian.irl             mklink %path_w32r%languages\croatian.irl             %path_trans_l%croatian.irl
if not exist %path_w32r%languages\czech.irl                mklink %path_w32r%languages\czech.irl                %path_trans_l%czech.irl
if not exist %path_w32r%languages\danish.irl               mklink %path_w32r%languages\danish.irl               %path_trans_l%danish.irl
if not exist %path_w32r%languages\dutch.irl                mklink %path_w32r%languages\dutch.irl                %path_trans_l%dutch.irl
if not exist %path_w32r%languages\estonian.irl             mklink %path_w32r%languages\estonian.irl             %path_trans_l%estonian.irl
if not exist %path_w32r%languages\farsi.irl                mklink %path_w32r%languages\farsi.irl                %path_trans_l%farsi.irl
if not exist %path_w32r%languages\finnish.irl              mklink %path_w32r%languages\finnish.irl              %path_trans_l%finnish.irl
if not exist %path_w32r%languages\french.irl               mklink %path_w32r%languages\french.irl               %path_trans_l%french.irl
if not exist %path_w32r%languages\galician.irl             mklink %path_w32r%languages\galician.irl             %path_trans_l%galician.irl
if not exist %path_w32r%languages\german.irl               mklink %path_w32r%languages\german.irl               %path_trans_l%german.irl
if not exist %path_w32r%languages\greek.irl                mklink %path_w32r%languages\greek.irl                %path_trans_l%greek.irl
if not exist %path_w32r%languages\hebrew.irl               mklink %path_w32r%languages\hebrew.irl               %path_trans_l%hebrew.irl
if not exist %path_w32r%languages\hungarian.irl            mklink %path_w32r%languages\hungarian.irl            %path_trans_l%hungarian.irl
if not exist %path_w32r%languages\indonesian.irl           mklink %path_w32r%languages\indonesian.irl           %path_trans_l%indonesian.irl
if not exist %path_w32r%languages\italian.irl              mklink %path_w32r%languages\italian.irl              %path_trans_l%italian.irl
if not exist %path_w32r%languages\japanese.irl             mklink %path_w32r%languages\japanese.irl             %path_trans_l%japanese.irl
if not exist %path_w32r%languages\korean.irl               mklink %path_w32r%languages\korean.irl               %path_trans_l%korean.irl
if not exist %path_w32r%languages\latvian.irl              mklink %path_w32r%languages\latvian.irl              %path_trans_l%latvian.irl
if not exist %path_w32r%languages\lithuanian.irl           mklink %path_w32r%languages\lithuanian.irl           %path_trans_l%lithuanian.irl
if not exist %path_w32r%languages\macedonian.irl           mklink %path_w32r%languages\macedonian.irl           %path_trans_l%macedonian.irl
if not exist %path_w32r%languages\norwegian.irl            mklink %path_w32r%languages\norwegian.irl            %path_trans_l%norwegian.irl
if not exist %path_w32r%languages\polish.irl               mklink %path_w32r%languages\polish.irl               %path_trans_l%polish.irl
if not exist %path_w32r%languages\portuguese.irl           mklink %path_w32r%languages\portuguese.irl           %path_trans_l%portuguese.irl
if not exist %path_w32r%languages\portuguese-brazilian.irl mklink %path_w32r%languages\portuguese-brazilian.irl %path_trans_l%portuguese-brazilian.irl
if not exist %path_w32r%languages\romanian.irl             mklink %path_w32r%languages\romanian.irl             %path_trans_l%romanian.irl
if not exist %path_w32r%languages\russian.irl              mklink %path_w32r%languages\russian.irl              %path_trans_l%russian.irl
if not exist %path_w32r%languages\serbian-cyrillic.irl     mklink %path_w32r%languages\serbian-cyrillic.irl     %path_trans_l%serbian-cyrillic.irl
if not exist %path_w32r%languages\serbian-latin.irl        mklink %path_w32r%languages\serbian-latin.irl        %path_trans_l%serbian-latin.irl
if not exist %path_w32r%languages\slovak.irl               mklink %path_w32r%languages\slovak.irl               %path_trans_l%slovak.irl
if not exist %path_w32r%languages\slovenian.irl            mklink %path_w32r%languages\slovenian.irl            %path_trans_l%slovenian.irl
if not exist %path_w32r%languages\spanish.irl              mklink %path_w32r%languages\spanish.irl              %path_trans_l%spanish.irl
if not exist %path_w32r%languages\swedish.irl              mklink %path_w32r%languages\swedish.irl              %path_trans_l%swedish.irl
if not exist %path_w32r%languages\thai.irl                 mklink %path_w32r%languages\thai.irl                 %path_trans_l%thai.irl
if not exist %path_w32r%languages\turkish.irl              mklink %path_w32r%languages\turkish.irl              %path_trans_l%turkish.irl
if not exist %path_w32r%languages\ukrainian.irl            mklink %path_w32r%languages\ukrainian.irl            %path_trans_l%ukrainian.irl
if not exist %path_w32r%languages\valencian.irl            mklink %path_w32r%languages\valencian.irl            %path_trans_l%valencian.irl
if not exist %path_w32p%languages\albanian.irl             mklink %path_w32p%languages\albanian.irl             %path_trans_l%albanian.irl
if not exist %path_w32p%languages\arabic.irl               mklink %path_w32p%languages\arabic.irl               %path_trans_l%arabic.irl
if not exist %path_w32p%languages\armenian.irl             mklink %path_w32p%languages\armenian.irl             %path_trans_l%armenian.irl
if not exist %path_w32p%languages\basque.irl               mklink %path_w32p%languages\basque.irl               %path_trans_l%basque.irl
if not exist %path_w32p%languages\bosnian.irl              mklink %path_w32p%languages\bosnian.irl              %path_trans_l%bosnian.irl
if not exist %path_w32p%languages\bulgarian.irl            mklink %path_w32p%languages\bulgarian.irl            %path_trans_l%bulgarian.irl
if not exist %path_w32p%languages\catalan.irl              mklink %path_w32p%languages\catalan.irl              %path_trans_l%catalan.irl
if not exist %path_w32p%languages\chinese-simplified.irl   mklink %path_w32p%languages\chinese-simplified.irl   %path_trans_l%chinese-simplified.irl
if not exist %path_w32p%languages\chinese-traditional.irl  mklink %path_w32p%languages\chinese-traditional.irl  %path_trans_l%chinese-traditional.irl
if not exist %path_w32p%languages\chuvash.irl              mklink %path_w32p%languages\chuvash.irl              %path_trans_l%chuvash.irl
if not exist %path_w32p%languages\croatian.irl             mklink %path_w32p%languages\croatian.irl             %path_trans_l%croatian.irl
if not exist %path_w32p%languages\czech.irl                mklink %path_w32p%languages\czech.irl                %path_trans_l%czech.irl
if not exist %path_w32p%languages\danish.irl               mklink %path_w32p%languages\danish.irl               %path_trans_l%danish.irl
if not exist %path_w32p%languages\dutch.irl                mklink %path_w32p%languages\dutch.irl                %path_trans_l%dutch.irl
if not exist %path_w32p%languages\estonian.irl             mklink %path_w32p%languages\estonian.irl             %path_trans_l%estonian.irl
if not exist %path_w32p%languages\farsi.irl                mklink %path_w32p%languages\farsi.irl                %path_trans_l%farsi.irl
if not exist %path_w32p%languages\finnish.irl              mklink %path_w32p%languages\finnish.irl              %path_trans_l%finnish.irl
if not exist %path_w32p%languages\french.irl               mklink %path_w32p%languages\french.irl               %path_trans_l%french.irl
if not exist %path_w32p%languages\galician.irl             mklink %path_w32p%languages\galician.irl             %path_trans_l%galician.irl
if not exist %path_w32p%languages\german.irl               mklink %path_w32p%languages\german.irl               %path_trans_l%german.irl
if not exist %path_w32p%languages\greek.irl                mklink %path_w32p%languages\greek.irl                %path_trans_l%greek.irl
if not exist %path_w32p%languages\hebrew.irl               mklink %path_w32p%languages\hebrew.irl               %path_trans_l%hebrew.irl
if not exist %path_w32p%languages\hungarian.irl            mklink %path_w32p%languages\hungarian.irl            %path_trans_l%hungarian.irl
if not exist %path_w32p%languages\indonesian.irl           mklink %path_w32p%languages\indonesian.irl           %path_trans_l%indonesian.irl
if not exist %path_w32p%languages\italian.irl              mklink %path_w32p%languages\italian.irl              %path_trans_l%italian.irl
if not exist %path_w32p%languages\japanese.irl             mklink %path_w32p%languages\japanese.irl             %path_trans_l%japanese.irl
if not exist %path_w32p%languages\korean.irl               mklink %path_w32p%languages\korean.irl               %path_trans_l%korean.irl
if not exist %path_w32p%languages\latvian.irl              mklink %path_w32p%languages\latvian.irl              %path_trans_l%latvian.irl
if not exist %path_w32p%languages\lithuanian.irl           mklink %path_w32p%languages\lithuanian.irl           %path_trans_l%lithuanian.irl
if not exist %path_w32p%languages\macedonian.irl           mklink %path_w32p%languages\macedonian.irl           %path_trans_l%macedonian.irl
if not exist %path_w32p%languages\norwegian.irl            mklink %path_w32p%languages\norwegian.irl            %path_trans_l%norwegian.irl
if not exist %path_w32p%languages\polish.irl               mklink %path_w32p%languages\polish.irl               %path_trans_l%polish.irl
if not exist %path_w32p%languages\portuguese.irl           mklink %path_w32p%languages\portuguese.irl           %path_trans_l%portuguese.irl
if not exist %path_w32p%languages\portuguese-brazilian.irl mklink %path_w32p%languages\portuguese-brazilian.irl %path_trans_l%portuguese-brazilian.irl
if not exist %path_w32p%languages\romanian.irl             mklink %path_w32p%languages\romanian.irl             %path_trans_l%romanian.irl
if not exist %path_w32p%languages\russian.irl              mklink %path_w32p%languages\russian.irl              %path_trans_l%russian.irl
if not exist %path_w32p%languages\serbian-cyrillic.irl     mklink %path_w32p%languages\serbian-cyrillic.irl     %path_trans_l%serbian-cyrillic.irl
if not exist %path_w32p%languages\serbian-latin.irl        mklink %path_w32p%languages\serbian-latin.irl        %path_trans_l%serbian-latin.irl
if not exist %path_w32p%languages\slovak.irl               mklink %path_w32p%languages\slovak.irl               %path_trans_l%slovak.irl
if not exist %path_w32p%languages\slovenian.irl            mklink %path_w32p%languages\slovenian.irl            %path_trans_l%slovenian.irl
if not exist %path_w32p%languages\spanish.irl              mklink %path_w32p%languages\spanish.irl              %path_trans_l%spanish.irl
if not exist %path_w32p%languages\swedish.irl              mklink %path_w32p%languages\swedish.irl              %path_trans_l%swedish.irl
if not exist %path_w32p%languages\thai.irl                 mklink %path_w32p%languages\thai.irl                 %path_trans_l%thai.irl
if not exist %path_w32p%languages\turkish.irl              mklink %path_w32p%languages\turkish.irl              %path_trans_l%turkish.irl
if not exist %path_w32p%languages\ukrainian.irl            mklink %path_w32p%languages\ukrainian.irl            %path_trans_l%ukrainian.irl
if not exist %path_w32p%languages\valencian.irl            mklink %path_w32p%languages\valencian.irl            %path_trans_l%valencian.irl
if not exist %path_x64d%languages\albanian.irl             mklink %path_x64d%languages\albanian.irl             %path_trans_l%albanian.irl
if not exist %path_x64d%languages\arabic.irl               mklink %path_x64d%languages\arabic.irl               %path_trans_l%arabic.irl
if not exist %path_x64d%languages\armenian.irl             mklink %path_x64d%languages\armenian.irl             %path_trans_l%armenian.irl
if not exist %path_x64d%languages\basque.irl               mklink %path_x64d%languages\basque.irl               %path_trans_l%basque.irl
if not exist %path_x64d%languages\bosnian.irl              mklink %path_x64d%languages\bosnian.irl              %path_trans_l%bosnian.irl
if not exist %path_x64d%languages\bulgarian.irl            mklink %path_x64d%languages\bulgarian.irl            %path_trans_l%bulgarian.irl
if not exist %path_x64d%languages\catalan.irl              mklink %path_x64d%languages\catalan.irl              %path_trans_l%catalan.irl
if not exist %path_x64d%languages\chinese-simplified.irl   mklink %path_x64d%languages\chinese-simplified.irl   %path_trans_l%chinese-simplified.irl
if not exist %path_x64d%languages\chinese-traditional.irl  mklink %path_x64d%languages\chinese-traditional.irl  %path_trans_l%chinese-traditional.irl
if not exist %path_x64d%languages\chuvash.irl              mklink %path_x64d%languages\chuvash.irl              %path_trans_l%chuvash.irl
if not exist %path_x64d%languages\croatian.irl             mklink %path_x64d%languages\croatian.irl             %path_trans_l%croatian.irl
if not exist %path_x64d%languages\czech.irl                mklink %path_x64d%languages\czech.irl                %path_trans_l%czech.irl
if not exist %path_x64d%languages\danish.irl               mklink %path_x64d%languages\danish.irl               %path_trans_l%danish.irl
if not exist %path_x64d%languages\dutch.irl                mklink %path_x64d%languages\dutch.irl                %path_trans_l%dutch.irl
if not exist %path_x64d%languages\estonian.irl             mklink %path_x64d%languages\estonian.irl             %path_trans_l%estonian.irl
if not exist %path_x64d%languages\farsi.irl                mklink %path_x64d%languages\farsi.irl                %path_trans_l%farsi.irl
if not exist %path_x64d%languages\finnish.irl              mklink %path_x64d%languages\finnish.irl              %path_trans_l%finnish.irl
if not exist %path_x64d%languages\french.irl               mklink %path_x64d%languages\french.irl               %path_trans_l%french.irl
if not exist %path_x64d%languages\galician.irl             mklink %path_x64d%languages\galician.irl             %path_trans_l%galician.irl
if not exist %path_x64d%languages\german.irl               mklink %path_x64d%languages\german.irl               %path_trans_l%german.irl
if not exist %path_x64d%languages\greek.irl                mklink %path_x64d%languages\greek.irl                %path_trans_l%greek.irl
if not exist %path_x64d%languages\hebrew.irl               mklink %path_x64d%languages\hebrew.irl               %path_trans_l%hebrew.irl
if not exist %path_x64d%languages\hungarian.irl            mklink %path_x64d%languages\hungarian.irl            %path_trans_l%hungarian.irl
if not exist %path_x64d%languages\indonesian.irl           mklink %path_x64d%languages\indonesian.irl           %path_trans_l%indonesian.irl
if not exist %path_x64d%languages\italian.irl              mklink %path_x64d%languages\italian.irl              %path_trans_l%italian.irl
if not exist %path_x64d%languages\japanese.irl             mklink %path_x64d%languages\japanese.irl             %path_trans_l%japanese.irl
if not exist %path_x64d%languages\korean.irl               mklink %path_x64d%languages\korean.irl               %path_trans_l%korean.irl
if not exist %path_x64d%languages\latvian.irl              mklink %path_x64d%languages\latvian.irl              %path_trans_l%latvian.irl
if not exist %path_x64d%languages\lithuanian.irl           mklink %path_x64d%languages\lithuanian.irl           %path_trans_l%lithuanian.irl
if not exist %path_x64d%languages\macedonian.irl           mklink %path_x64d%languages\macedonian.irl           %path_trans_l%macedonian.irl
if not exist %path_x64d%languages\norwegian.irl            mklink %path_x64d%languages\norwegian.irl            %path_trans_l%norwegian.irl
if not exist %path_x64d%languages\polish.irl               mklink %path_x64d%languages\polish.irl               %path_trans_l%polish.irl
if not exist %path_x64d%languages\portuguese.irl           mklink %path_x64d%languages\portuguese.irl           %path_trans_l%portuguese.irl
if not exist %path_x64d%languages\portuguese-brazilian.irl mklink %path_x64d%languages\portuguese-brazilian.irl %path_trans_l%portuguese-brazilian.irl
if not exist %path_x64d%languages\romanian.irl             mklink %path_x64d%languages\romanian.irl             %path_trans_l%romanian.irl
if not exist %path_x64d%languages\russian.irl              mklink %path_x64d%languages\russian.irl              %path_trans_l%russian.irl
if not exist %path_x64d%languages\serbian-cyrillic.irl     mklink %path_x64d%languages\serbian-cyrillic.irl     %path_trans_l%serbian-cyrillic.irl
if not exist %path_x64d%languages\serbian-latin.irl        mklink %path_x64d%languages\serbian-latin.irl        %path_trans_l%serbian-latin.irl
if not exist %path_x64d%languages\slovak.irl               mklink %path_x64d%languages\slovak.irl               %path_trans_l%slovak.irl
if not exist %path_x64d%languages\slovenian.irl            mklink %path_x64d%languages\slovenian.irl            %path_trans_l%slovenian.irl
if not exist %path_x64d%languages\spanish.irl              mklink %path_x64d%languages\spanish.irl              %path_trans_l%spanish.irl
if not exist %path_x64d%languages\swedish.irl              mklink %path_x64d%languages\swedish.irl              %path_trans_l%swedish.irl
if not exist %path_x64d%languages\thai.irl                 mklink %path_x64d%languages\thai.irl                 %path_trans_l%thai.irl
if not exist %path_x64d%languages\turkish.irl              mklink %path_x64d%languages\turkish.irl              %path_trans_l%turkish.irl
if not exist %path_x64d%languages\ukrainian.irl            mklink %path_x64d%languages\ukrainian.irl            %path_trans_l%ukrainian.irl
if not exist %path_x64d%languages\valencian.irl            mklink %path_x64d%languages\valencian.irl            %path_trans_l%valencian.irl
if not exist %path_x64r%languages\albanian.irl             mklink %path_x64r%languages\albanian.irl             %path_trans_l%albanian.irl
if not exist %path_x64r%languages\arabic.irl               mklink %path_x64r%languages\arabic.irl               %path_trans_l%arabic.irl
if not exist %path_x64r%languages\armenian.irl             mklink %path_x64r%languages\armenian.irl             %path_trans_l%armenian.irl
if not exist %path_x64r%languages\basque.irl               mklink %path_x64r%languages\basque.irl               %path_trans_l%basque.irl
if not exist %path_x64r%languages\bosnian.irl              mklink %path_x64r%languages\bosnian.irl              %path_trans_l%bosnian.irl
if not exist %path_x64r%languages\bulgarian.irl            mklink %path_x64r%languages\bulgarian.irl            %path_trans_l%bulgarian.irl
if not exist %path_x64r%languages\catalan.irl              mklink %path_x64r%languages\catalan.irl              %path_trans_l%catalan.irl
if not exist %path_x64r%languages\chinese-simplified.irl   mklink %path_x64r%languages\chinese-simplified.irl   %path_trans_l%chinese-simplified.irl
if not exist %path_x64r%languages\chinese-traditional.irl  mklink %path_x64r%languages\chinese-traditional.irl  %path_trans_l%chinese-traditional.irl
if not exist %path_x64r%languages\chuvash.irl              mklink %path_x64r%languages\chuvash.irl              %path_trans_l%chuvash.irl
if not exist %path_x64r%languages\croatian.irl             mklink %path_x64r%languages\croatian.irl             %path_trans_l%croatian.irl
if not exist %path_x64r%languages\czech.irl                mklink %path_x64r%languages\czech.irl                %path_trans_l%czech.irl
if not exist %path_x64r%languages\danish.irl               mklink %path_x64r%languages\danish.irl               %path_trans_l%danish.irl
if not exist %path_x64r%languages\dutch.irl                mklink %path_x64r%languages\dutch.irl                %path_trans_l%dutch.irl
if not exist %path_x64r%languages\estonian.irl             mklink %path_x64r%languages\estonian.irl             %path_trans_l%estonian.irl
if not exist %path_x64r%languages\farsi.irl                mklink %path_x64r%languages\farsi.irl                %path_trans_l%farsi.irl
if not exist %path_x64r%languages\finnish.irl              mklink %path_x64r%languages\finnish.irl              %path_trans_l%finnish.irl
if not exist %path_x64r%languages\french.irl               mklink %path_x64r%languages\french.irl               %path_trans_l%french.irl
if not exist %path_x64r%languages\galician.irl             mklink %path_x64r%languages\galician.irl             %path_trans_l%galician.irl
if not exist %path_x64r%languages\german.irl               mklink %path_x64r%languages\german.irl               %path_trans_l%german.irl
if not exist %path_x64r%languages\greek.irl                mklink %path_x64r%languages\greek.irl                %path_trans_l%greek.irl
if not exist %path_x64r%languages\hebrew.irl               mklink %path_x64r%languages\hebrew.irl               %path_trans_l%hebrew.irl
if not exist %path_x64r%languages\hungarian.irl            mklink %path_x64r%languages\hungarian.irl            %path_trans_l%hungarian.irl
if not exist %path_x64r%languages\indonesian.irl           mklink %path_x64r%languages\indonesian.irl           %path_trans_l%indonesian.irl
if not exist %path_x64r%languages\italian.irl              mklink %path_x64r%languages\italian.irl              %path_trans_l%italian.irl
if not exist %path_x64r%languages\japanese.irl             mklink %path_x64r%languages\japanese.irl             %path_trans_l%japanese.irl
if not exist %path_x64r%languages\korean.irl               mklink %path_x64r%languages\korean.irl               %path_trans_l%korean.irl
if not exist %path_x64r%languages\latvian.irl              mklink %path_x64r%languages\latvian.irl              %path_trans_l%latvian.irl
if not exist %path_x64r%languages\lithuanian.irl           mklink %path_x64r%languages\lithuanian.irl           %path_trans_l%lithuanian.irl
if not exist %path_x64r%languages\macedonian.irl           mklink %path_x64r%languages\macedonian.irl           %path_trans_l%macedonian.irl
if not exist %path_x64r%languages\norwegian.irl            mklink %path_x64r%languages\norwegian.irl            %path_trans_l%norwegian.irl
if not exist %path_x64r%languages\polish.irl               mklink %path_x64r%languages\polish.irl               %path_trans_l%polish.irl
if not exist %path_x64r%languages\portuguese.irl           mklink %path_x64r%languages\portuguese.irl           %path_trans_l%portuguese.irl
if not exist %path_x64r%languages\portuguese-brazilian.irl mklink %path_x64r%languages\portuguese-brazilian.irl %path_trans_l%portuguese-brazilian.irl
if not exist %path_x64r%languages\romanian.irl             mklink %path_x64r%languages\romanian.irl             %path_trans_l%romanian.irl
if not exist %path_x64r%languages\russian.irl              mklink %path_x64r%languages\russian.irl              %path_trans_l%russian.irl
if not exist %path_x64r%languages\serbian-cyrillic.irl     mklink %path_x64r%languages\serbian-cyrillic.irl     %path_trans_l%serbian-cyrillic.irl
if not exist %path_x64r%languages\serbian-latin.irl        mklink %path_x64r%languages\serbian-latin.irl        %path_trans_l%serbian-latin.irl
if not exist %path_x64r%languages\slovak.irl               mklink %path_x64r%languages\slovak.irl               %path_trans_l%slovak.irl
if not exist %path_x64r%languages\slovenian.irl            mklink %path_x64r%languages\slovenian.irl            %path_trans_l%slovenian.irl
if not exist %path_x64r%languages\spanish.irl              mklink %path_x64r%languages\spanish.irl              %path_trans_l%spanish.irl
if not exist %path_x64r%languages\swedish.irl              mklink %path_x64r%languages\swedish.irl              %path_trans_l%swedish.irl
if not exist %path_x64r%languages\thai.irl                 mklink %path_x64r%languages\thai.irl                 %path_trans_l%thai.irl
if not exist %path_x64r%languages\turkish.irl              mklink %path_x64r%languages\turkish.irl              %path_trans_l%turkish.irl
if not exist %path_x64r%languages\ukrainian.irl            mklink %path_x64r%languages\ukrainian.irl            %path_trans_l%ukrainian.irl
if not exist %path_x64r%languages\valencian.irl            mklink %path_x64r%languages\valencian.irl            %path_trans_l%valencian.irl
if not exist %path_x64p%languages\albanian.irl             mklink %path_x64p%languages\albanian.irl             %path_trans_l%albanian.irl
if not exist %path_x64p%languages\arabic.irl               mklink %path_x64p%languages\arabic.irl               %path_trans_l%arabic.irl
if not exist %path_x64p%languages\armenian.irl             mklink %path_x64p%languages\armenian.irl             %path_trans_l%armenian.irl
if not exist %path_x64p%languages\basque.irl               mklink %path_x64p%languages\basque.irl               %path_trans_l%basque.irl
if not exist %path_x64p%languages\bosnian.irl              mklink %path_x64p%languages\bosnian.irl              %path_trans_l%bosnian.irl
if not exist %path_x64p%languages\bulgarian.irl            mklink %path_x64p%languages\bulgarian.irl            %path_trans_l%bulgarian.irl
if not exist %path_x64p%languages\catalan.irl              mklink %path_x64p%languages\catalan.irl              %path_trans_l%catalan.irl
if not exist %path_x64p%languages\chinese-simplified.irl   mklink %path_x64p%languages\chinese-simplified.irl   %path_trans_l%chinese-simplified.irl
if not exist %path_x64p%languages\chinese-traditional.irl  mklink %path_x64p%languages\chinese-traditional.irl  %path_trans_l%chinese-traditional.irl
if not exist %path_x64p%languages\chuvash.irl              mklink %path_x64p%languages\chuvash.irl              %path_trans_l%chuvash.irl
if not exist %path_x64p%languages\croatian.irl             mklink %path_x64p%languages\croatian.irl             %path_trans_l%croatian.irl
if not exist %path_x64p%languages\czech.irl                mklink %path_x64p%languages\czech.irl                %path_trans_l%czech.irl
if not exist %path_x64p%languages\danish.irl               mklink %path_x64p%languages\danish.irl               %path_trans_l%danish.irl
if not exist %path_x64p%languages\dutch.irl                mklink %path_x64p%languages\dutch.irl                %path_trans_l%dutch.irl
if not exist %path_x64p%languages\estonian.irl             mklink %path_x64p%languages\estonian.irl             %path_trans_l%estonian.irl
if not exist %path_x64p%languages\farsi.irl                mklink %path_x64p%languages\farsi.irl                %path_trans_l%farsi.irl
if not exist %path_x64p%languages\finnish.irl              mklink %path_x64p%languages\finnish.irl              %path_trans_l%finnish.irl
if not exist %path_x64p%languages\french.irl               mklink %path_x64p%languages\french.irl               %path_trans_l%french.irl
if not exist %path_x64p%languages\galician.irl             mklink %path_x64p%languages\galician.irl             %path_trans_l%galician.irl
if not exist %path_x64p%languages\german.irl               mklink %path_x64p%languages\german.irl               %path_trans_l%german.irl
if not exist %path_x64p%languages\greek.irl                mklink %path_x64p%languages\greek.irl                %path_trans_l%greek.irl
if not exist %path_x64p%languages\hebrew.irl               mklink %path_x64p%languages\hebrew.irl               %path_trans_l%hebrew.irl
if not exist %path_x64p%languages\hungarian.irl            mklink %path_x64p%languages\hungarian.irl            %path_trans_l%hungarian.irl
if not exist %path_x64p%languages\indonesian.irl           mklink %path_x64p%languages\indonesian.irl           %path_trans_l%indonesian.irl
if not exist %path_x64p%languages\italian.irl              mklink %path_x64p%languages\italian.irl              %path_trans_l%italian.irl
if not exist %path_x64p%languages\japanese.irl             mklink %path_x64p%languages\japanese.irl             %path_trans_l%japanese.irl
if not exist %path_x64p%languages\korean.irl               mklink %path_x64p%languages\korean.irl               %path_trans_l%korean.irl
if not exist %path_x64p%languages\latvian.irl              mklink %path_x64p%languages\latvian.irl              %path_trans_l%latvian.irl
if not exist %path_x64p%languages\lithuanian.irl           mklink %path_x64p%languages\lithuanian.irl           %path_trans_l%lithuanian.irl
if not exist %path_x64p%languages\macedonian.irl           mklink %path_x64p%languages\macedonian.irl           %path_trans_l%macedonian.irl
if not exist %path_x64p%languages\norwegian.irl            mklink %path_x64p%languages\norwegian.irl            %path_trans_l%norwegian.irl
if not exist %path_x64p%languages\polish.irl               mklink %path_x64p%languages\polish.irl               %path_trans_l%polish.irl
if not exist %path_x64p%languages\portuguese.irl           mklink %path_x64p%languages\portuguese.irl           %path_trans_l%portuguese.irl
if not exist %path_x64p%languages\portuguese-brazilian.irl mklink %path_x64p%languages\portuguese-brazilian.irl %path_trans_l%portuguese-brazilian.irl
if not exist %path_x64p%languages\romanian.irl             mklink %path_x64p%languages\romanian.irl             %path_trans_l%romanian.irl
if not exist %path_x64p%languages\russian.irl              mklink %path_x64p%languages\russian.irl              %path_trans_l%russian.irl
if not exist %path_x64p%languages\serbian-cyrillic.irl     mklink %path_x64p%languages\serbian-cyrillic.irl     %path_trans_l%serbian-cyrillic.irl
if not exist %path_x64p%languages\serbian-latin.irl        mklink %path_x64p%languages\serbian-latin.irl        %path_trans_l%serbian-latin.irl
if not exist %path_x64p%languages\slovak.irl               mklink %path_x64p%languages\slovak.irl               %path_trans_l%slovak.irl
if not exist %path_x64p%languages\slovenian.irl            mklink %path_x64p%languages\slovenian.irl            %path_trans_l%slovenian.irl
if not exist %path_x64p%languages\spanish.irl              mklink %path_x64p%languages\spanish.irl              %path_trans_l%spanish.irl
if not exist %path_x64p%languages\swedish.irl              mklink %path_x64p%languages\swedish.irl              %path_trans_l%swedish.irl
if not exist %path_x64p%languages\thai.irl                 mklink %path_x64p%languages\thai.irl                 %path_trans_l%thai.irl
if not exist %path_x64p%languages\turkish.irl              mklink %path_x64p%languages\turkish.irl              %path_trans_l%turkish.irl
if not exist %path_x64p%languages\ukrainian.irl            mklink %path_x64p%languages\ukrainian.irl            %path_trans_l%ukrainian.irl
if not exist %path_x64p%languages\valencian.irl            mklink %path_x64p%languages\valencian.irl            %path_trans_l%valencian.irl

rem make symbolic links to translated help.
if not exist %path_w32d%languages\czech.chm                mklink %path_w32d%languages\czech.chm                %path_trans_h%czech.chm
if not exist %path_w32d%languages\french.chm               mklink %path_w32d%languages\french.chm               %path_trans_h%french.chm
if not exist %path_w32d%languages\german.chm               mklink %path_w32d%languages\german.chm               %path_trans_h%german.chm
if not exist %path_w32d%languages\russian.chm              mklink %path_w32d%languages\russian.chm              %path_trans_h%russian.chm
if not exist %path_w32d%languages\thai.chm                 mklink %path_w32d%languages\thai.chm                 %path_trans_h%thai.chm
if not exist %path_w32d%languages\turkish.chm              mklink %path_w32d%languages\turkish.chm              %path_trans_h%turkish.chm
if not exist %path_w32d%languages\ukrainian.chm            mklink %path_w32d%languages\ukrainian.chm            %path_trans_h%ukrainian.chm
if not exist %path_w32r%languages\czech.chm                mklink %path_w32r%languages\czech.chm                %path_trans_h%czech.chm
if not exist %path_w32r%languages\french.chm               mklink %path_w32r%languages\french.chm               %path_trans_h%french.chm
if not exist %path_w32r%languages\german.chm               mklink %path_w32r%languages\german.chm               %path_trans_h%german.chm
if not exist %path_w32r%languages\russian.chm              mklink %path_w32r%languages\russian.chm              %path_trans_h%russian.chm
if not exist %path_w32r%languages\thai.chm                 mklink %path_w32r%languages\thai.chm                 %path_trans_h%thai.chm
if not exist %path_w32r%languages\turkish.chm              mklink %path_w32r%languages\turkish.chm              %path_trans_h%turkish.chm
if not exist %path_w32r%languages\ukrainian.chm            mklink %path_w32r%languages\ukrainian.chm            %path_trans_h%ukrainian.chm
if not exist %path_w32p%languages\czech.chm                mklink %path_w32p%languages\czech.chm                %path_trans_h%czech.chm
if not exist %path_w32p%languages\french.chm               mklink %path_w32p%languages\french.chm               %path_trans_h%french.chm
if not exist %path_w32p%languages\german.chm               mklink %path_w32p%languages\german.chm               %path_trans_h%german.chm
if not exist %path_w32p%languages\russian.chm              mklink %path_w32p%languages\russian.chm              %path_trans_h%russian.chm
if not exist %path_w32p%languages\thai.chm                 mklink %path_w32p%languages\thai.chm                 %path_trans_h%thai.chm
if not exist %path_w32p%languages\turkish.chm              mklink %path_w32p%languages\turkish.chm              %path_trans_h%turkish.chm
if not exist %path_w32p%languages\ukrainian.chm            mklink %path_w32p%languages\ukrainian.chm            %path_trans_h%ukrainian.chm
if not exist %path_x64d%languages\czech.chm                mklink %path_x64d%languages\czech.chm                %path_trans_h%czech.chm
if not exist %path_x64d%languages\french.chm               mklink %path_x64d%languages\french.chm               %path_trans_h%french.chm
if not exist %path_x64d%languages\german.chm               mklink %path_x64d%languages\german.chm               %path_trans_h%german.chm
if not exist %path_x64d%languages\russian.chm              mklink %path_x64d%languages\russian.chm              %path_trans_h%russian.chm
if not exist %path_x64d%languages\thai.chm                 mklink %path_x64d%languages\thai.chm                 %path_trans_h%thai.chm
if not exist %path_x64d%languages\turkish.chm              mklink %path_x64d%languages\turkish.chm              %path_trans_h%turkish.chm
if not exist %path_x64d%languages\ukrainian.chm            mklink %path_x64d%languages\ukrainian.chm            %path_trans_h%ukrainian.chm
if not exist %path_x64r%languages\czech.chm                mklink %path_x64r%languages\czech.chm                %path_trans_h%czech.chm
if not exist %path_x64r%languages\french.chm               mklink %path_x64r%languages\french.chm               %path_trans_h%french.chm
if not exist %path_x64r%languages\german.chm               mklink %path_x64r%languages\german.chm               %path_trans_h%german.chm
if not exist %path_x64r%languages\russian.chm              mklink %path_x64r%languages\russian.chm              %path_trans_h%russian.chm
if not exist %path_x64r%languages\thai.chm                 mklink %path_x64r%languages\thai.chm                 %path_trans_h%thai.chm
if not exist %path_x64r%languages\turkish.chm              mklink %path_x64r%languages\turkish.chm              %path_trans_h%turkish.chm
if not exist %path_x64r%languages\ukrainian.chm            mklink %path_x64r%languages\ukrainian.chm            %path_trans_h%ukrainian.chm
if not exist %path_x64p%languages\czech.chm                mklink %path_x64p%languages\czech.chm                %path_trans_h%czech.chm
if not exist %path_x64p%languages\french.chm               mklink %path_x64p%languages\french.chm               %path_trans_h%french.chm
if not exist %path_x64p%languages\german.chm               mklink %path_x64p%languages\german.chm               %path_trans_h%german.chm
if not exist %path_x64p%languages\russian.chm              mklink %path_x64p%languages\russian.chm              %path_trans_h%russian.chm
if not exist %path_x64p%languages\thai.chm                 mklink %path_x64p%languages\thai.chm                 %path_trans_h%thai.chm
if not exist %path_x64p%languages\turkish.chm              mklink %path_x64p%languages\turkish.chm              %path_trans_h%turkish.chm
if not exist %path_x64p%languages\ukrainian.chm            mklink %path_x64p%languages\ukrainian.chm            %path_trans_h%ukrainian.chm

pause
