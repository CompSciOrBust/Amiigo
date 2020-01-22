@echo off

set /p IP=<C:/My-Switch-ip.txt

dir /b *.nro>%temp%\filete.txt
set /p File=<%temp%\filete.txt
del "%temp%\filete.txt"
title -%IP% - %File%
make -j7
set a=%errorlevel%
echo ------------------------------------------
rem if %a% neq 0 color 04
if %a% equ 0 "C:\devkitPro\tools\bin\nxlink.exe" -a %IP% %File%

echo -----------------------------------

%systemroot%\system32\timeout.exe 55

