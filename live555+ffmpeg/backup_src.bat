@echo off

set vardate=%date:~0,4%-%date:~5,2%-%date:~8,2%
set vartime=%time:~0,2%-%time:~3,2%-%time:~6,2%

if "%vartime:~0,1%"==" " set vartime=0%time:~1,1%-%time:~3,2%-%time:~6,2%

setlocal enabledelayedexpansion
set PWD=%~p0
set PWD=%PWD:~1,-1%
for /l %%a in (1,1,100) do (
   if "!PWD:~%%a,1!"=="\" set i=%%a
)
set /a i+=1
set PWD=!PWD:~%i%!

set varfile=bak\%PWD%_src[%vardate%_%vartime%].zip
set varrar=tool\rar.exe
set varfilelist=sln src

echo.
echo 创建备份文件 %varfile%

if not exist bak mkdir bak

if exist %varfile% goto BACK_FILE_EXIST


:FILE_BACKUP

%varrar% A "%varfile%" %varfilelist%

echo.
echo %cd%\%varfile%
echo 文件备份结束!
echo.
pause

goto EXIT_BAT

:BACK_FILE_EXIST

set /P OK= 目标文件 %varfile% 已经存在，确认要覆盖前次备份的文件吗？(Y/N): 

IF /I "%OK:~0,1%"=="Y" (

	del /f /s /q %varfile%
	goto FILE_BACKUP

) ELSE (

	goto EXIT_BAT

)

:EXIT_BAT

@echo on
