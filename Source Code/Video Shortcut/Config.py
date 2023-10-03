import subprocess
import re
import os
import time

def get_ethernet_interfaces():
    result = subprocess.run(["ipconfig"], capture_output=True, text=True, check=True)
    interfaces = re.findall(r"Ethernet adapter (.*?):", result.stdout)
    return interfaces

def main():
    print("Available Ethernet interfaces:")
    interfaces = get_ethernet_interfaces()
    for i, interface in enumerate(interfaces):
        print(f"{i + 1}. {interface}")

    selected = -1
    while selected < 1 or selected > len(interfaces):
        try:
            selected = int(input("Enter the number of the interface you want to configure: "))
        except ValueError:
            print("Invalid input. Please enter a valid number.")

    selected_interface = interfaces[selected - 1]

    subprocess.run(["IPChange.bat", selected_interface], check=True)

    delay_seconds = 10
    print(f"Waiting {delay_seconds} seconds before opening VLC...")
    time.sleep(delay_seconds)

    vlc_path = 'C:\\Program Files\\VideoLAN\\VLC\\vlc.exe'  # Update this path if needed

    network_stream_url = 'rtsp://192.168.2.101:8554/payload' #Here is the URL for the Network Stream

    if os.path.isfile(vlc_path):
        subprocess.Popen([vlc_path, network_stream_url])
    else: 
        print("VLC not found. Please ensure it's installed and the vlc_path variable is set correctly.")


if __name__ == "__main__":
    main()
