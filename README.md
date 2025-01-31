# Fully Distributed Hash Cracker

Distributed Hash Cracker (Multi Thread on Multi Machine Hasing program): A terminal based distributed system for cracking password hashes by leveraging multiple Linux machines in a network distributing workload on multiple cpu cores.

Notes: This project also supports automation installation using ansible playbook.

## Current Objectives

The system attempts to crack a hashed value with the following constraints:
- Original value is 7 letters long
- All letters are lowercase
- Processing is distributed across multiple machines/processors in the network
- Optimized for Linux server environments

## Overview

Please refer to the `/docs` directory for detailed documentation about the system architecture and implementation details.

## Installation and Setup [SERVER SIDE ONLY]

### Prerequisites

- Linux-based server environment
- Network connectivity between all participating machines
- SSH access to all nodes

### Installation Steps

1. Install required dependencies:
```bash
cd /init
./install-ansible.sh
./copy-pubkey.sh
```

2. Configure network settings:
   - Navigate to `/src/ansible/inventory.ini`
   - Update the machine configurations according to your network setup
  
3. Chunking and master node network settings
   - Config it on `/bin/start.sh`

## Starting the System

To initiate the hash cracking process:
```bash
/bin/start.sh
```

## Verification

The system indicates success through the following mechanisms:

1. Main System:
   - Creates `/bin/password_in.txt` upon success
   - This file contains information about which machine found the password

2. Target Machine:
   - Creates `~/success.txt` on the successful node
   - Contains the cracked password value

## Project Structure
```
.
├── /bin
│   ├── start.sh
│   └── password_in.txt (generated)
├── /docs
│   └── ...
├── /init
│   ├── install-ansible.sh
│   └── copy-pubkey.sh
└── /src
    └── ansible
        └── inventory.ini
```

## Contributing

Please read through our documentation in the `/docs` directory before making any contributions.
