#!/bin/sh

./Main 531.deepsjeng_r_llc.mem_trace >> arc_deep.txt;
echo "done deep"
./Main 541.leela_r_llc.mem_trace >> arc_leela.txt;
echo "done leela"
./Main 548.exchange2_r_llc.mem_trace >> arc_exchange.txt;
echo "done exchange"
