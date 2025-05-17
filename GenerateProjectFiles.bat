rd /s /q Bin
rd /s /q Bin-Intermediate
rd /s /q .vs
del TimeEngine.sln

Vendor\bin\Premake\premake5.exe vs2022
PAUSE