#!/bin/bash

# kill old process
line1=$(head -n 1 tmp)
line2=$(head -n 1 tmp)
kill $line1
kill $line2

# Start hosting the worker script
python3 -m http.server 8000 -d ../ &
echo $! > tmp

# Create master worker tasks
g++ -o master ../src/master.cpp
# Usages: bin port chunk_size hashed_value
./master 30000 456976 "$6$1UNfGmmL8v6itaPG$j54ho2EN0nnSk14Bd0AG2JoP5Zsue1I1DT/VDOFRPAonZg1J7PdlIknsEvDB2srHXV/zCkSr10sBDIOV.PF8f/" &
echo $! >> tmp

# Deploy a worker script
sh ../init/deploy-worker.sh