setlocal

md BuildResults
md %temp%\AutoMenus
for %%a in (5.2 5.3 5.4) do call :build %%a
rd /s /q %temp%\AutoMenus
goto :eof

:build
set epic=
for /f "tokens=2* skip=2" %%a in ('reg query "HKLM\SOFTWARE\EpicGames\Unreal Engine\%1" /v "InstalledDirectory"') do set epic=%%b
if "%epic%"=="" goto :eof
md %temp%\AutoMenus\%1
call "%epic%\Engine\Build\BatchFiles\RunUAT.bat" BuildPlugin -Plugin="%CD%\AutoMenus.uplugin" -Package="%temp%\AutoMenus\%1" -Rocket >BuildResults\LogBuild-%1.txt
rd /s /q %temp%\AutoMenus\%1\Binaries
rd /s /q %temp%\AutoMenus\%1\Intermediate
md %temp%\AutoMenus\%1\Config
copy Config\FilterPlugin.ini %temp%\AutoMenus\%1\Config\FilterPlugin.ini
del BuildResults\AutoMenus-%1.zip
tar -a -c -f BuildResults\AutoMenus-%1.zip -C %temp%\AutoMenus\%1\ *
goto :eof
