@ECHO OFF

Rem Construit la version "jeu" de TRON dans le dossier TRON\ :
Rem   TRON.exe (sans console) + shaders\ + assets\
Rem Le dossier peut ensuite être zippé et lancé sur n'importe quel PC Windows (double-clic sur TRON.exe).

cd %~dp0
cd ..

ECHO Configuration (build_release/)
cmake -B build_release -DTRON_RELEASE=ON || goto :error

ECHO.
ECHO Compilation
cmake --build build_release --config RelWithDebInfo || goto :error

ECHO.
ECHO Copie des fichiers dans TRON\
if exist TRON\ rmdir /s /q TRON
mkdir TRON
copy /y build_release\bin\project.exe TRON\TRON.exe >NUL || goto :error
xcopy /e /i /q shaders TRON\shaders >NUL || goto :error
xcopy /e /i /q assets TRON\assets >NUL || goto :error

ECHO.
ECHO ===============================
ECHO Jeu pret dans le dossier TRON\
ECHO ===============================
PAUSE
exit /b 0

:error
ECHO.
ECHO La construction a echoue, voir les messages ci-dessus.
PAUSE
exit /b 1
