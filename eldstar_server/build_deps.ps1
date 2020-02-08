
# Change to UTF-8
$OutputEncoding = [System.Text.Encoding]::Unicode;

Set-Variable -Name "PROJECT_DIR" -Value (Get-Item -Path ".\").FullName;

$CONFIGS = @("Debug", "Release");
$ARCHITECTURES = @("Win32", "x64");

if ($null -eq ${env:CMAKE_GENERATOR}) {
    Set-Variable -Name COMPILER -Value "Visual Studio 16 2019";
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
cmake ../../sources/zlib/zlib-1.2.11 -G ${COMPILER} -A ${ARCHITECTURE} -DCMAKE_INSTALL_PREFIX:PATH="${TARGET_DIR}/zlib/out";
cmake --build . --target INSTALL --config ${CONFIG} -- /nologo;
Set-Location ..;

Write-Host "[${TARGET}] Building libpng...";

mkdir libpng;
Set-Location libpng;
cmake ../../sources/libpng/lpng1637 -G ${COMPILER} -A ${ARCHITECTURE} -DCMAKE_INSTALL_PREFIX:PATH="${TARGET_DIR}/libpng/out" -DZLIB_INCLUDE_DIR:PATH="${TARGET_DIR}/zlib/out/include" -DZLIB_LIBRARY_DEBUG:FILEPATH="${TARGET_DIR}/zlib/out/lib/zlibstaticd.lib" -DZLIB_LIBRARY_RELEASE:FILEPATH="${TARGET_DIR}/zlib/out/lib/zlibstatic.lib" -DPNG_TESTS:BOOL="0" -DPNG_SHARED:BOOL="0";
cmake --build . --target INSTALL --config ${CONFIG} -- /nologo;
Set-Location ..;

Write-Host "[${TARGET}] Building glfw...";

mkdir glfw;
Set-Location glfw;
cmake ../../sources/glfw/glfw-3.3.2 -G ${COMPILER} -A ${ARCHITECTURE} -DCMAKE_INSTALL_PREFIX:PATH="${TARGET_DIR}/glfw/out" -DGLFW_BUILD_EXAMPLES:BOOL="0" -DGLFW_BUILD_DOCS:BOOL="0" -DGLFW_BUILD_TESTS:BOOL="0";
cmake --build . --target INSTALL --config ${CONFIG} -- /nologo;
Set-Location ..;

Write-Host "[${TARGET}] Building glm...";

mkdir glm;
Set-Location glm;
cmake ../../sources/glm/glm -G ${COMPILER} -A ${ARCHITECTURE} -DCMAKE_INSTALL_PREFIX:PATH="${TARGET_DIR}/glm/out" -DGLM_QUIET:BOOL="1" -DGLM_TEST_ENABLE:BOOL="0";
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

cmake ../../sources/harfbuzz/harfbuzz-2.6.4 -G ${COMPILER} -A ${ARCHITECTURE} -DCMAKE_INSTALL_PREFIX:PATH="${TARGET_DIR}/harfbuzz/out" -DHB_HAVE_FREETYPE:BOOL="1" -DFREETYPE_INCLUDE_DIR_freetype2:PATH="${PROJECT_DIR}/deps/dists/freetype/include" -DFREETYPE_INCLUDE_DIR_ft2build:PATH="${PROJECT_DIR}/deps/dists/freetype/include" -DFREETYPE_LIBRARY_DEBUG:FILEPATH="${PROJECT_DIR}/deps/dists/freetype/${FREETYPE_DIST_TYPE}/freetype.lib" -DFREETYPE_LIBRARY_RELEASE:FILEPATH="${PROJECT_DIR}/deps/dists/freetype/${FREETYPE_DIST_TYPE}/freetype.lib" -DHB_BUILD_TESTS:BOOL="0";
cmake --build . --target INSTALL --config ${CONFIG} -- /nologo;
Set-Location ..;

Write-Host "[${TARGET}] Building freetype...";

mkdir freetype;
Set-Location freetype;
cmake ../../sources/freetype/freetype-2.10.1 -G ${COMPILER} -A ${ARCHITECTURE} -DCMAKE_INSTALL_PREFIX:PATH="${TARGET_DIR}/freetype/out" -DFT_WITH_HARFBUZZ:BOOL="1" -DHARFBUZZ_INCLUDE_DIRS:PATH="${TARGET_DIR}/harfbuzz/out/include/harfbuzz" -DHARFBUZZ_LIBRARIES:FILEPATH="${TARGET_DIR}/harfbuzz/out/lib/harfbuzz.lib" -DFT_WITH_ZLIB:BOOL="1" -DZLIB_INCLUDE_DIR:PATH="${TARGET_DIR}/zlib/out/include" -DZLIB_LIBRARY_DEBUG:FILEPATH="${TARGET_DIR}/zlib/out/lib/zlibstaticd.lib" -DZLIB_LIBRARY_RELEASE:FILEPATH="${TARGET_DIR}/zlib/out/lib/zlibstatic.lib" -DFT_WITH_PNG:BOOL="1" -DPNG_PNG_INCLUDE_DIR:PATH="${TARGET_DIR}/libpng/out/include" -DPNG_LIBRARY_DEBUG:FILEPATH="${TARGET_DIR}/libpng/out/lib/libpng16_staticd.lib" -DPNG_LIBRARY_RELEASE:FILEPATH="${TARGET_DIR}/libpng/out/lib/libpng16_static.lib";
cmake --build . --target INSTALL --config ${CONFIG} -- /nologo;
Set-Location ..;

Set-Location ..;

Write-Host "Done building for ${TARGET}.";

}
}

Set-Location ..;
