Make sure you have the Tracelog tool on your target computer. The tool is located in the <install_folder>Windows Kits\10\Tools\<arch> folder of the WDK.

Open a Command Window and run as administrator.

Type the following command:

```
tracelog -start MyTrace -guid #c8d7d28f-5d1a-4ae1-96c2-7b02bc5412d1 -flag 0xFFFF -level 7-rt -kd
```

The command starts a trace session named MyTrace. In the Debugger Immediate Window you will see _Hello there!_

You can stop the trace session by typing the following command:

```
tracelog -stop MyTrace
```
