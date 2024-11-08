# ping task
ansible all -i inventory.ini -m ping

# install sudo
ansible-playbook -i inventory.ini install-sudo.yml
