#!/bin/sh

./Main blender.l1.trace >> nextLine_blender.txt;
echo "done blender"
./Main cactuBSSN.l1.trace >> nextLine_cactu.txt;
echo "done cactu"
./Main omnetpp.l1.trace >> nextLine_omnetpp.txt;
echo "done omnetpp"
