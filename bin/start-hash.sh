#!/bin/bash
HASHED='$6$1UNfGmmL8v6itaPG$j54ho2EN0nnSk14Bd0AG2JoP5Zsue1I1DT/VDOFRPAonZg1J7PdlIknsEvDB2srHXV/zCkSr10sBDIOV.PF8f/'
PROJ_PORT=30000
PROJ_IP="192.168.64.211"
FILE_PORT=8000
CHUNK_SIZE=456976

# kill old process
sudo fuser -k $PROJ_PORT/udp
sudo fuser -k $FILE_PORT/tcp

# Start hosting the worker script
python3 -m http.server $FILE_PORT -d ../ &

# Create master worker tasks
g++ -o master.o ../src/master.cpp -pthread -std=c++11

# Usages: bin port chunk_size hashed_value
./master.o $PROJ_PORT $CHUNK_SIZE $HASHED &

# Deploy a worker script
ansible-playbook -i ../src/ansible/inventory.ini ../src/ansible/deploy-tasks.yml \
    --extra-vars "proj_ip=$PROJ_IP" \
    --extra-vars "proj_port=$PROJ_PORT" \
    --extra-vars "file_port=$FILE_PORT" \
    --extra-vars "hashed=$HASHED" \
    --extra-vars "chunk_size=$CHUNK_SIZE"