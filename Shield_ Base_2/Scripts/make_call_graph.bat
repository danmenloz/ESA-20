@echo off
REM This script is run by MDK, so it is run in <Project Directory>
REM set PATH=%PATH;C:\Users\Alex\AppData\Local\Programs\Python\Python38
set PATH=%PATH;..\..\Tools\Calls;"C:\Program Files (x86)\Graphviz2.38\bin"
@echo on
"C:\Users\Alex\Documents\Teaching\ESA\ESA-Fall-2020\Code\Keil-uVision-Call-Graph-Generator\call_graph_generator.py" -i ..\mycallgraph.txt  -o ..\cg.gv
@echo off REM ..\..\Tools\Calls\call_graph_generator.py.lnk -i ..\mycallgraph.txt  -o ..\cg.gv
dot -O -Tpdf ..\cg.gv
