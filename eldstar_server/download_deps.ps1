
mkdir deps;
Set-Location deps;

[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12;
Invoke-WebRequest -Uri "https://zlib.net/zlib1211.zip" -OutFile 'zlib.zip';
Invoke-WebRequest -Uri "https://phoenixnap.dl.sourceforge.net/project/libpng/libpng16/1.6.37/lpng1637.zip" -OutFile 'libpng.zip';
Invoke-WebRequest -Uri "https://github.com/glfw/glfw/releases/download/3.3/glfw-3.3.zip" -OutFile 'glfw.zip';
Invoke-WebRequest -Uri "https://github.com/g-truc/glm/releases/download/0.9.9.5/glm-0.9.9.5.zip" -OutFile 'glm.zip';
Invoke-WebRequest -Uri "https://github.com/harfbuzz/harfbuzz/archive/2.6.1.zip" -OutFile 'harfbuzz.zip';
Invoke-WebRequest -Uri "https://download.savannah.gnu.org/releases/freetype/ft2101.zip" -OutFile 'freetype.zip';
Invoke-WebRequest -Uri "https://github.com/ubawurinna/freetype-windows-binaries/releases/download/v2.10.1/freetype.zip" -OutFile 'freetype-dist.zip';

mkdir sources;
mkdir dists;

Expand-Archive -Path 'zlib.zip' -DestinationPath 'sources/zlib';
Expand-Archive -Path 'libpng.zip' -DestinationPath 'sources/libpng';
Expand-Archive -Path 'glfw.zip' -DestinationPath 'sources/glfw';
Expand-Archive -Path 'glm.zip' -DestinationPath 'sources/glm';
Expand-Archive -Path 'harfbuzz.zip' -DestinationPath 'sources/harfbuzz';
Expand-Archive -Path 'freetype.zip' -DestinationPath 'sources/freetype';
Expand-Archive -Path 'freetype-dist.zip' -DestinationPath 'dists/freetype';

Remove-Item 'zlib.zip' -Force;
Remove-Item 'libpng.zip' -Force;
Remove-Item 'glfw.zip' -Force;
Remove-Item 'glm.zip' -Force;
Remove-Item 'harfbuzz.zip' -Force;
Remove-Item 'freetype.zip' -Force;
Remove-Item 'freetype-dist.zip' -Force;

Set-Location ..;
