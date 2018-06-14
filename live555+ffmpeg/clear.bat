@echo off

set fileext1=*.sdf *.ncb *.idb *.ilk *.pdb *.exp *.obj *.o *.pch *.lastbuildstate *.res *.vcproj.*.user
set fileext2=*.log *.tlog

del /f /s /q %fileext1% %fileext2%

rmdir /s /q obj

@echo on