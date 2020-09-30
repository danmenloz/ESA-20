@echo off
set PATH=%PATH;..\Tools\GetRegions\
@echo on
echo Need to edit next line with name of AXF file!
GetRegions.exe ..\Objects\file.axf -c -s -o..\Source\Profiler\region.c