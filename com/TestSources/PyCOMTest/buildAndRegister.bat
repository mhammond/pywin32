msbuild %~dp0\PyCOMTest.sln -property:Configuration=Release
regsvr32 %~dp0\x64\Release\PyCOMTest.dll %*
