echo Compiler: %COMPILER%
echo Architecture: %MSYS2_ARCH%
echo Platform: %PLATFORM%
echo MSYS2 directory: %MSYS2_DIR%
echo MSYS2 system: %MSYSTEM%
echo Bits: %BIT%

SET "PATH=C:\%MSYS2_DIR%\%MSYSTEM%\bin;C:\%MSYS2_DIR%\usr\bin;%PATH%"

bash -lc "pacman --noconfirm -S git make tar unzip mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake"
