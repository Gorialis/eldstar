
# Change to UTF-8
$OutputEncoding = [System.Text.Encoding]::Unicode;

Set-Variable -Name "PROJECT_DIR" -Value (Get-Item -Path ".\").FullName;

$CONFIGS = @("Debug", "Release");
$ARCHITECTURES = @("Win32", "x64");

if ($null -eq ${env:CMAKE_GENERATOR}) {
    Set-Variable -Name COMPILER -Value "Visual Studio 17 2022";
} else {
    Set-Variable -Name COMPILER -Value "${env:CMAKE_GENERATOR}";
}

Set-Location deps;

ForEach ($CONFIG In $CONFIGS) {
ForEach ($ARCHITECTURE In $ARCHITECTURES) {

Set-Variable -Name TARGET -Value "${CONFIG} - ${ARCHITECTURE}";
Set-Variable -Name TARGET_DIR -Value "${PROJECT_DIR}/deps/${TARGET}";

Write-Host "Building for ${TARGET}...";

mkdir $TARGET;
Set-Location $TARGET;

Write-Host "[${TARGET}] Building zlib...";

mkdir zlib;
Set-Location zlib;
cmake ../../sources/zlib/zlib-1.2.13 -G ${COMPILER} -A ${ARCHITECTURE} -DCMAKE_INSTALL_PREFIX:PATH="${TARGET_DIR}/zlib/out";
cmake --build . --target INSTALL --config ${CONFIG} -- /nologo;
Set-Location ..;

Write-Host "[${TARGET}] Building libpng...";

mkdir libpng;
Set-Location libpng;
cmake ../../sources/libpng/lpng1639 -G ${COMPILER} -A ${ARCHITECTURE} -DCMAKE_INSTALL_PREFIX:PATH="${TARGET_DIR}/libpng/out" -DZLIB_INCLUDE_DIR:PATH="${TARGET_DIR}/zlib/out/include" -DZLIB_LIBRARY_DEBUG:FILEPATH="${TARGET_DIR}/zlib/out/lib/zlibstaticd.lib" -DZLIB_LIBRARY_RELEASE:FILEPATH="${TARGET_DIR}/zlib/out/lib/zlibstatic.lib" -DPNG_TESTS:BOOL="0" -DPNG_SHARED:BOOL="0";
cmake --build . --target INSTALL --config ${CONFIG} -- /nologo;
Set-Location ..;

Write-Host "[${TARGET}] Building glfw...";

mkdir glfw;
Set-Location glfw;
cmake ../../sources/glfw/glfw-3.3.8 -G ${COMPILER} -A ${ARCHITECTURE} -DCMAKE_INSTALL_PREFIX:PATH="${TARGET_DIR}/glfw/out" -DGLFW_BUILD_EXAMPLES:BOOL="0" -DGLFW_BUILD_DOCS:BOOL="0" -DGLFW_BUILD_TESTS:BOOL="0";
cmake --build . --target INSTALL --config ${CONFIG} -- /nologo;
Set-Location ..;

Write-Host "[${TARGET}] Building harfbuzz...";

mkdir harfbuzz;
Set-Location harfbuzz;

if (${ARCHITECTURE} -Match "x64") {
    Set-Variable -Name FREETYPE_DIST_TYPE -Value "win64";
} else {
    Set-Variable -Name FREETYPE_DIST_TYPE -Value "win32";
}
Set-Variable -Name FREETYPE_DIST_LOCATION -Value "${PROJECT_DIR}/deps/dists/freetype/freetype-windows-binaries-2.13.0";
Set-Variable -Name FREETYPE_DIST_INCLUDE -Value "${FREETYPE_DIST_LOCATION}/include";
Set-Variable -Name FREETYPE_DIST_LIBRARY -Value "${FREETYPE_DIST_LOCATION}/release static/vs2015-2022";

Set-Variable -Name CACHED_ERROR_PREFERENCE -Value "$ErrorActionPreference"
$ErrorActionPreference = 'continue';
cmake ../../sources/harfbuzz/harfbuzz-7.2.0 -G ${COMPILER} -A ${ARCHITECTURE} -DCMAKE_INSTALL_PREFIX:PATH="${TARGET_DIR}/harfbuzz/out" -DHB_HAVE_FREETYPE:BOOL="1" -DFREETYPE_INCLUDE_DIR_freetype2:PATH="${FREETYPE_DIST_INCLUDE}" -DFREETYPE_INCLUDE_DIR_ft2build:PATH="${FREETYPE_DIST_INCLUDE}" -DFREETYPE_LIBRARY_DEBUG:FILEPATH="${FREETYPE_DIST_LIBRARY}/${FREETYPE_DIST_TYPE}/freetype.lib" -DFREETYPE_LIBRARY_RELEASE:FILEPATH="${FREETYPE_DIST_LIBRARY}/${FREETYPE_DIST_TYPE}/freetype.lib" -DHB_BUILD_TESTS:BOOL="0";
$ErrorActionPreference = "${CACHED_ERROR_PREFERENCE}";
cmake --build . --target INSTALL --config ${CONFIG} -- /nologo;
Set-Location ..;

Write-Host "[${TARGET}] Building freetype...";

mkdir freetype;
Set-Location freetype;
cmake ../../sources/freetype/freetype-2.13.0 -Wno-dev -G ${COMPILER} -A ${ARCHITECTURE} -DCMAKE_INSTALL_PREFIX:PATH="${TARGET_DIR}/freetype/out" -DFT_WITH_HARFBUZZ:BOOL="1" -DHARFBUZZ_INCLUDE_DIRS:PATH="${TARGET_DIR}/harfbuzz/out/include/harfbuzz" -DHARFBUZZ_LIBRARIES:FILEPATH="${TARGET_DIR}/harfbuzz/out/lib/harfbuzz.lib" -DFT_WITH_ZLIB:BOOL="1" -DZLIB_INCLUDE_DIR:PATH="${TARGET_DIR}/zlib/out/include" -DZLIB_LIBRARY_DEBUG:FILEPATH="${TARGET_DIR}/zlib/out/lib/zlibstaticd.lib" -DZLIB_LIBRARY_RELEASE:FILEPATH="${TARGET_DIR}/zlib/out/lib/zlibstatic.lib" -DFT_WITH_PNG:BOOL="1" -DPNG_PNG_INCLUDE_DIR:PATH="${TARGET_DIR}/libpng/out/include" -DPNG_LIBRARY_DEBUG:FILEPATH="${TARGET_DIR}/libpng/out/lib/libpng16_staticd.lib" -DPNG_LIBRARY_RELEASE:FILEPATH="${TARGET_DIR}/libpng/out/lib/libpng16_static.lib";
cmake --build . --target INSTALL --config ${CONFIG} -- /nologo;
Set-Location ..;

Set-Location ..;

Write-Host "Done building for ${TARGET}.";

}
}

Set-Location ..;
