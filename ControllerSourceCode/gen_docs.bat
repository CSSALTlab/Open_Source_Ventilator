echo off
rmdir CodeDocs /s /q
mkdir CodeDocs
cd Tools\Doxygen
doxygen.exe
cd ..\..\
