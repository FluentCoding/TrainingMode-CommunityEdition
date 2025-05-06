@echo off
setlocal

REM set directory to location of this file. Needed when drag n dropping across directories.
cd /d %~dp0

set ISO="%~1"

if %ISO%=="" (
    echo ERROR: iso path not passed
    echo USAGE: build_windows.bat vanilla_melee.iso
    goto end
) else (
    echo iso: %ISO%
)

if not exist %ISO% (
    echo ERROR: iso does not exist
    echo USAGE: build_windows.bat vanilla_melee.iso
    goto end
)

if not exist "C:/devkitPro/devkitPPC" (
    echo ERROR: devkitPro not found at "C:/devkitPro/devkitPPC"
    echo Please install devkitPro with the GameCube package
    goto cleanup
) else (
    echo found devkitPro
    set "PATH=%PATH%;C:\devkitPro\devkitPPC\bin"
)

if not exist "TM-CE.iso" (
    copy %ISO% "TM-CE.iso"
)

echo BUILD C FILES --------------------------------------------------------

mkdir build

echo build event menu
copy "dats\eventMenu.dat" "build\eventMenu.dat"
"MexTK/MexTK.exe" -ff -i "src/events.c" -b "build" -s tmFunction -dat "build/eventMenu.dat" -t "MexTK/tmFunction.txt" -q -ow -l "MexTK/melee.link" -op 2 || ( echo ERROR: Failed to compile 'events.c' & goto cleanup )
"MexTK/MexTK.exe" -trim "build/eventMenu.dat" || ( echo ERROR: Dat file trimming failed & goto cleanup )

echo build lab event
copy "dats\lab.dat" "build\lab.dat"
"MexTK/MexTK.exe" -ff -i "src/lab.c" -b "build" -s evFunction -dat "build/lab.dat" -t "MexTK/evFunction.txt" -q -ow -l "MexTK/melee.link" -op 2 || ( echo ERROR: Failed to compile 'lab.c' & goto cleanup)
"MexTK/MexTK.exe" -trim "build/lab.dat" || ( echo ERROR: Dat file trimming failed & goto cleanup )

echo build lab css
copy "dats\labCSS.dat" "build\labCSS.dat"
"MexTK/MexTK.exe" -ff -i "src/lab_css.c" -b "build" -s cssFunction -dat "build/labCSS.dat" -t "MexTK/cssFunction.txt" -q -ow -l "MexTK/melee.link" -op 2 || ( echo ERROR: Failed to compile 'lab_css.c' & goto cleanup)
"MexTK/MexTK.exe" -trim "build/labCSS.dat" || ( echo ERROR: Dat file trimming failed & goto cleanup )

echo build lcancel event
copy "dats\lcancel.dat" "build\lcancel.dat"
"MexTK/MexTK.exe" -ff -i "src/lcancel.c" -b "build" -s evFunction -dat "build/lcancel.dat" -t "MexTK/evFunction.txt" -q -ow -l "MexTK/melee.link" -op 2 || ( echo ERROR: Failed to compile 'lcancel.c' & goto cleanup)
"MexTK/MexTK.exe" -trim "build/lcancel.dat" || ( echo ERROR: Dat file trimming failed & goto cleanup )

echo build ledgedash event
copy "dats\ledgedash.dat" "build\ledgedash.dat"
"MexTK/MexTK.exe" -ff -i "src/ledgedash.c" -b "build" -s evFunction -dat "build/ledgedash.dat" -t "MexTK/evFunction.txt" -q -ow -l "MexTK/melee.link" -op 2 || ( echo ERROR: Failed to compile 'ledgedash.c' & goto cleanup)
"MexTK/MexTK.exe" -trim "build/ledgedash.dat" || ( echo ERROR: Dat file trimming failed & goto cleanup )

echo build wavedash event
copy "dats\wavedash.dat" "build\wavedash.dat"
"MexTK/MexTK.exe" -ff -i "src/wavedash.c" -b "build" -s evFunction -dat "build/wavedash.dat" -t "MexTK/evFunction.txt" -q -ow -l "MexTK/melee.link" -op 2 || ( echo ERROR: Failed to compile 'wavedash.c' & goto cleanup)
"MexTK/MexTK.exe" -trim "build/wavedash.dat" || ( echo ERROR: Dat file trimming failed & goto cleanup )

echo build powershield event
"MexTK/MexTK.exe" -ff -i "src/powershield.c" -b "build" -s evFunction -dat "build/powershield.dat" -t "MexTK/evFunction.txt" -q -ow -l "MexTK/melee.link" -op 2 || ( echo ERROR: Failed to compile 'powershield.c' & goto cleanup)
"MexTK/MexTK.exe" -trim "build/powershield.dat" || ( echo ERROR: Dat file trimming failed & goto cleanup )

echo build edgeguard event
"MexTK/MexTK.exe" -ff -i "src/edgeguard.c" -b "build" -s evFunction -dat "build/edgeguard.dat" -t "MexTK/evFunction.txt" -q -ow -l "MexTK/melee.link" -op 2 || ( echo ERROR: Failed to compile 'edgeguard.c' & goto cleanup)
"MexTK/MexTK.exe" -trim "build/edgeguard.dat" || ( echo ERROR: Dat file trimming failed & goto cleanup )

echo BUILD ASM FILES --------------------------------------------------------

del "Additional ISO Files\codes.gct"
cd "Build TM Codeset"
echo gecko.exe build

REM gecko always returns success, so we check that the file exists after building as a replacement
gecko.exe build
cd ..
if not exist "Additional ISO Files\codes.gct" (
    echo ERROR: gecko.exe build failed
    goto cleanup
)

copy "Additional ISO Files\codes.gct" "build\"
copy "Additional ISO Files\opening.bnr" "build\"


echo BUILD START.DOL --------------------------------------------------------

for /f "delims=" %%A in ('gc_fst get-header %ISO%') do set "GAME_ID=%%A"
if "%GAME_ID%" == "GALJ01" (
	set "PATCH_FILENAME=patch_jp.xdelta"
) else (
	set "PATCH_FILENAME=patch.xdelta"
)

gc_fst read %ISO% "Start.dol" "Start.dol"
"Build TM Start.dol\xdelta.exe" -d -f -s "Start.dol" "Build TM Start.dol\%PATCH_FILENAME%" "build\Start.dol" || ( echo ERROR: Failed to patch Start.dol & goto cleanup )
del Start.dol

echo BUILD ISO --------------------------------------------------------
gc_fst fs TM-CE.iso ^
	delete MvHowto.mth ^
	delete MvOmake15.mth ^
	delete MvOpen.mth ^
	insert TM\eventMenu.dat build\eventMenu.dat ^
	insert TM\lab.dat build\lab.dat ^
	insert TM\labCSS.dat build\labCSS.dat ^
	insert TM\lcancel.dat build\lcancel.dat ^
	insert TM\ledgedash.dat build\ledgedash.dat ^
	insert TM\wavedash.dat build\wavedash.dat ^
	insert TM\powershield.dat build\powershield.dat ^
	insert TM\edgeguard.dat build\edgeguard.dat ^
	insert codes.gct build\codes.gct ^
	insert Start.dol build\Start.dol ^
	insert opening.bnr build\opening.bnr
gc_fst set-header TM-CE.iso "GTME01" "Training Mode Community Edition"


echo ############ TM-CE.iso has been created ######################

:cleanup

echo CLEANUP -----------------------------------------------------------------

if exist "build" (
    echo deleting build dir...
    rmdir /s /q build\
)

:end

REM pause if not run from command line
echo %CMDCMDLINE% | findstr /C:"/c">nul && pause
