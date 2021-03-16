@echo off

mkdir build
cd build/

cl.exe ../gol.c /FC /nologo /Fegol.exe

cd ../