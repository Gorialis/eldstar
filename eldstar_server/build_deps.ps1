
# Change to UTF-8
$OutputEncoding = [System.Text.Encoding]::Unicode;
chcp 65001;

Set-Variable -Name "PROJECT_DIR" -Value (Get-Item -Path ".\").FullName;

$CONFIGS = @("Debug", "Release");
$COMPILERS = @("Visual Studio 15 2017", "Visual Studio 15 2017 Win64");

Set-Location deps;

ForEach ($CONFIG In $CONFIGS) {
ForEach ($COMPILER In $COMPILERS) {

Set-Variable -Name TARGET -Value "${CONFIG} - ${COMPILER}";
Set-Variable -Name TARGET_DIR -Value "${PROJECT_DIR}/deps/${TARGET}";

mkdir $TARGET;
Set-Location $TARGET;

mkdir zlib;
Set-Location zlib;
cmake ../../sources/zlib/zlib-1.2.11 -G ${COMPILER} -DCMAKE_INSTALL_PREFIX:PATH="${TARGET_DIR}/zlib/out";
cmake --build . --target INSTALL --config ${CONFIG} -- /nologo;
Set-Location ..;

mkdir libpng;
Set-Location libpng;
cmake ../../sources/libpng/lpng1637 -G ${COMPILER} -DCMAKE_INSTALL_PREFIX:PATH="${TARGET_DIR}/libpng/out" -DZLIB_INCLUDE_DIR:PATH="${TARGET_DIR}/zlib/out/include" -DZLIB_LIBRARY_DEBUG:FILEPATH="${TARGET_DIR}/zlib/out/lib/zlibstaticd.lib" -DZLIB_LIBRARY_RELEASE:FILEPATH="${TARGET_DIR}/zlib/out/lib/zlibstatic.lib";
cmake --build . --target INSTALL --config ${CONFIG} -- /nologo;
Set-Location ..;

mkdir glfw;
Set-Location glfw;
cmake ../../sources/glfw/glfw-3.3 -G ${COMPILER} -DCMAKE_INSTALL_PREFIX:PATH="${TARGET_DIR}/glfw/out";
cmake --build . --target INSTALL --config ${CONFIG} -- /nologo;
Set-Location ..;

mkdir glm;
Set-Location glm;
cmake ../../sources/glm/glm -G ${COMPILER} -DCMAKE_INSTALL_PREFIX:PATH="${TARGET_DIR}/glm/out";
cmake --build . --target INSTALL --config ${CONFIG} -- /nologo;
Set-Location ..;

mkdir harfbuzz;
Set-Location harfbuzz;

if (${COMPILER} -Match "64") {
    Set-Variable -Name FREETYPE_DIST_TYPE -Value "win64";
} else {
    Set-Variable -Name FREETYPE_DIST_TYPE -Value "win32";
}

cmake ../../sources/harfbuzz/harfbuzz-2.6.1 -G ${COMPILER} -DCMAKE_INSTALL_PREFIX:PATH="${TARGET_DIR}/harfbuzz/out" -DHB_HAVE_FREETYPE:BOOL="1" -DFREETYPE_INCLUDE_DIR_freetype2:PATH="${PROJECT_DIR}/deps/dists/freetype/include" -DFREETYPE_INCLUDE_DIR_ft2build:PATH="${PROJECT_DIR}/deps/dists/freetype/include" -DFREETYPE_LIBRARY_DEBUG:FILEPATH="${PROJECT_DIR}/deps/dists/freetype/${FREETYPE_DIST_TYPE}/freetype.lib" -DFREETYPE_LIBRARY_RELEASE:FILEPATH="${PROJECT_DIR}/deps/dists/freetype/${FREETYPE_DIST_TYPE}/freetype.lib";
cmake --build . --target INSTALL --config ${CONFIG} -- /nologo;
Set-Location ..;

mkdir freetype;
Set-Location freetype;
cmake ../../sources/freetype/freetype-2.10.1 -G ${COMPILER} -DCMAKE_INSTALL_PREFIX:PATH="${TARGET_DIR}/freetype/out" -DFT_WITH_HARFBUZZ:BOOL="1" -DHARFBUZZ_INCLUDE_DIRS:PATH="${TARGET_DIR}/harfbuzz/out/include/harfbuzz" -DHARFBUZZ_LIBRARIES:FILEPATH="${TARGET_DIR}/harfbuzz/out/lib/harfbuzz.lib" -DFT_WITH_ZLIB:BOOL="1" -DZLIB_INCLUDE_DIR:PATH="${TARGET_DIR}/zlib/out/include" -DZLIB_LIBRARY_DEBUG:FILEPATH="${TARGET_DIR}/zlib/out/lib/zlibstaticd.lib" -DZLIB_LIBRARY_RELEASE:FILEPATH="${TARGET_DIR}/zlib/out/lib/zlibstatic.lib" -DFT_WITH_PNG:BOOL="1" -DPNG_PNG_INCLUDE_DIR:PATH="${TARGET_DIR}/libpng/out/include" -DPNG_LIBRARY_DEBUG:FILEPATH="${TARGET_DIR}/libpng/out/lib/libpng16_staticd.lib" -DPNG_LIBRARY_RELEASE:FILEPATH="${TARGET_DIR}/libpng/out/lib/libpng16_static.lib";
cmake --build . --target INSTALL --config ${CONFIG} -- /nologo;
Set-Location ..;

Set-Location ..;

}
}

Set-Location ..;
