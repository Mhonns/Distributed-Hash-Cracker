#!/bin/sh

# Function to extract IPs from inventory file
get_ips() {
    grep -v '^#\|^$\|\[' ../src/ansible/inventory.ini | grep -oE '[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}'
}

# Ask for username
echo "Enter username for SSH: "
read USERNAME

# Process each IP
for ip in $(get_ips); do
    echo "Copying SSH key to $ip..."
    ssh-copy-id "$USERNAME@$ip"
    
    if [ $? -eq 0 ]; then
        echo "Successfully copied key to $ip"
    else
        echo "Failed to copy key to $ip"
    fi
    echo "------------------------"
done

echo "SSH key copy process completed"