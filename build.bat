@echo off
setlocal
cd /D "%~dp0"

for %%a in (%*) do set "%%a=1"

:: default behaviour is to compile in debug mode
if "%~1"=="" (
set app=1
)

set debug_build= /Od /Zi /MTd 
set release_build= /O2 /MT

if "%debug%" == "1" echo [debug] && set build_type=%debug_build%
if "%release%" == "1" echo [release] && set build_type=%release_build%

if "%build_type%" == "" echo [debug] && set build_type=%debug_build%

:: Deletes compiler artifacts
if "%clean%" == "1" (
rm *.exp
rm *.lib
rm *.obj
rm *.pdb
rm *.exe
rm *.rdi
true
)

if "%app%" == "1" echo [app] && cl /d2cgsummary /W4 /wd4100 /wd4996 /Zi /FC /Zc:strictStrings- %build_type% /I . ./main.c /link /INCREMENTAL:NO /OUT:yk.exe  user32.lib kernel32.lib gdi32.lib

:: don't run (if asked to compile and run) if compilation failed
if %errorlevel% neq 0 echo app compilation failed && exit /b

if "%run%" == "1" echo [run] && start cmd /c ".\yk.exe