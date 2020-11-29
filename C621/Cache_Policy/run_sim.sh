#!/bin/sh

./Main blender.l1.trace >> nextLine_blender.txt;
echo "done deep"
./Main cactuBSSN.l1.trace >> nextLine_cactu.txt;
echo "done leela"
./Main omnetpp.l1.trace >> nextLine_omnetpp.txt;
echo "done exchange"
