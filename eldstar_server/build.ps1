
Set-Variable -Name "PROJECT_DIR" -Value (Get-Item -Path ".\").FullName;

$CONFIGS = @("Debug", "Release");
$ARCHITECTURES = @("Win32", "x64");

if (${env:CMAKE_GENERATOR} -eq $null) {
    Set-Variable -Name COMPILER -Value "Visual Studio 15 2017";
} else {
    Set-Variable -Name COMPILER -Value "${env:CMAKE_GENERATOR}";
}

mkdir dists;
Set-Location dists;

ForEach ($CONFIG In $CONFIGS) {
ForEach ($ARCHITECTURE In $ARCHITECTURES) {

Set-Variable -Name TARGET -Value "${CONFIG} - ${ARCHITECTURE}";
Set-Variable -Name TARGET_DEPS -Value "${PROJECT_DIR}/deps/${TARGET}";

Write-Host "Building for ${TARGET}...";

if (${CONFIG} -Match "Debug") {
    Set-Variable -Name DEBUG_SUFFIX -Value "d";
} else {
    Set-Variable -Name DEBUG_SUFFIX -Value "";
}

mkdir $TARGET;
Set-Location $TARGET;
cmake ../.. -G ${COMPILER} -A ${ARCHITECTURE} -DFREETYPE_INCLUDE_DIR_freetype2:PATH="${TARGET_DEPS}/freetype/out/include/freetype2" -DFREETYPE_INCLUDE_DIR_ft2build:PATH="${TARGET_DEPS}/freetype/out/include/freetype2" -DFREETYPE_LIBRARY_DEBUG:FILEPATH="${TARGET_DEPS}/freetype/out/lib/freetype${DEBUG_SUFFIX}.lib" -DFREETYPE_LIBRARY_RELEASE:FILEPATH="${TARGET_DEPS}/freetype/out/lib/freetype${DEBUG_SUFFIX}.lib" -Dglfw3_DIR:PATH="${TARGET_DEPS}/glfw/out/lib/cmake/glfw3" -Dglm_DIR:PATH="${TARGET_DEPS}/glm/out/lib/cmake/glm" -Dharfbuzz_DIR:PATH="${TARGET_DEPS}/harfbuzz/out/lib/cmake/harfbuzz" -DHARFBUZZ_INCLUDE_DIRS:PATH="${TARGET_DEPS}/harfbuzz/out/include/harfbuzz/" -DHARFBUZZ_LIBRARIES:FILEPATH="${TARGET_DEPS}/harfbuzz/out/lib/harfbuzz.lib" -DPNG_PNG_INCLUDE_DIR:PATH="${TARGET_DEPS}/libpng/out/include" -DPNG_LIBRARY_DEBUG:FILEPATH="${TARGET_DEPS}/libpng/out/lib/libpng16_static${DEBUG_SUFFIX}.lib" -DPNG_LIBRARY_RELEASE:FILEPATH="${TARGET_DEPS}/libpng/out/lib/libpng16_static${DEBUG_SUFFIX}.lib" -DZLIB_INCLUDE_DIR:PATH="${TARGET_DEPS}/zlib/out/include/" -DZLIB_LIBRARY_DEBUG:FILEPATH="${TARGET_DEPS}/zlib/out/lib/zlibstatic${DEBUG_SUFFIX}.lib" -DZLIB_LIBRARY_RELEASE:FILEPATH="${TARGET_DEPS}/zlib/out/lib/zlibstatic${DEBUG_SUFFIX}.lib";
cmake --build . --target eldstar_server --config $CONFIG -- /nologo;
Set-Location ..;

Write-Host "Done building for ${TARGET}.";

}
}

Set-Location ..;
