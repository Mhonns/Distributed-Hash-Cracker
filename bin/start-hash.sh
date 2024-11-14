#!/bin/

# Case baaaaaa
# HASHED='\$6\$M2pjffoHE4V/J2W0\$jzcT/b6/VIpP0UaCpIh7TVGB1eJ.8vCq5cOmQJTJp7BSXMAhzICPyJ59sL0hFtDVpFk9p.CgMWrZWHK3vOxCD0'
# Case aaabaaa
# HASHED='\$6\$.gK6Njlm2G17CumU\$3vB12Jo0PhuwIubHfKI8bGqjFrtghTj6eMM7uUV2F5VypgwQ3KDfGSj3ezF2cWJf5CHWNUQ4xuRrBZEY9N1g01'
# Case aabaaaa
HASHED='\$6\$biVMS251oM0lGF96\$eKSB95hcgjpUTpYMGeFZqrKfAAtXUGlPmqixxdUnoJ23jjSgneIl6c2a2qaEkMeuYF1/sc/cJzDyPRtCJuZgp1'
PROJ_PORT=30000
PROJ_IP="192.168.64.211"
FILE_PORT=8000
CHUNK_SIZE=17576 # 26 ^ 3
# CHUNK_SIZE=456976 # 26 ^ 4

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