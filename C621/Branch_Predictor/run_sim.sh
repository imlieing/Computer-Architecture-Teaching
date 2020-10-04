#!/bin/sh

./Main 531.deepsjeng_r_branches.cpu_trace >> tour_deep.txt;
echo "done deep"
./Main 541.leela_r_branches.cpu_trace >> tour_leela.txt;
echo "done leela"
./Main 548.exchange2_r_branches.cpu_trace >> tour_exchange.txt;
echo "done exchange"