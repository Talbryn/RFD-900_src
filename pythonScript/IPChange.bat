set "interface_name=%1"

netsh interface ipv4 set address name=%interface_name% static 192.168.2.5 255.255.255.0