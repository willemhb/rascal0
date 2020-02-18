#!/bin/bash

gcc -Wall rsc/rascal.c rsc/util.c rsc/object.c rsc/reader.c rsc/printer.c rsc/eval.c rsc/gc.c -lm -o rascal
