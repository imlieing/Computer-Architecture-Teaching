#!/bin/sh

./Main 531.deepsjeng_r_branches.cpu_trace >> percep_deep.txt;
echo "done deep"
./Main 541.leela_r_branches.cpu_trace >> percep_leela.txt;
echo "done leela"
./Main 548.exchange2_r_branches.cpu_trace >> percep_exchange.txt;
echo "done exchange"