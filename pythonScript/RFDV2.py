# Import Packages
import serial
import time
import tkinter as tk
import csv
from datetime import datetime

# Constants

degree_sign = u"\N{DEGREE SIGN}"
fileName = "RFD900x_Data.csv"
header = ["Packet Number", "SIV", "FixType", "Latitude", "Longitude", "Altitude", "Year", "Month", "Day",
              "Hour", "Min", "Sec", "NNV", "NEV", "NDV", "Battery", "3v3 Supply", "5v Supply", "Radio Supply",
              "Analog Internal", "Analog External", "Altimeter Temp", "Digital Internal", "Digital Eternal",
              "Pressure", "Accel A", "Accel Y", "Accel z", "Pitch", "Roll", "Yaw"]

# Initialize
decoded_raw_data = []
final_data = []
packet_count = 0
average = 0
averagetime = 0
avt = 0

#Serial Port 
print("Enter the COM Port (COM4, COM5, COM9, COM12, etc.) ")
comport = str(input())
print()

ser = serial.Serial(
    port=comport,
    baudrate=57600,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    timeout=None
)

# Create or append to the CSV file
with open(fileName, "a", newline='\n') as f:
    writer = csv.writer(f, delimiter=',')
    writer.writerow(header)
print(f"{fileName} created to hold data. If file exists, data will be appended\n")

# Get current time
def get_current_time():
    now = datetime.now()
    return now.strftime("%H:%M:%S.%f")

# Get fix type string
def get_fix_type_string(fix):
    fix_types = {0: "No Fix", 1: "Dead Reckoning", 2: "2D", 3: "3D", 4: "GNSS + Dead Reckoning"}    
    return fix_types.get(int(fix), "")

# Update labels
def label_update():
    def count():
        global packet_count, average, averagetime, avt

        current_time = get_current_time()

        ser.reset_input_buffer()
        data = ser.readline().decode("utf-8").strip().split(",")

        if len(data) > 10:
            with open(fileName, "a", newline='\n') as f:
                writer = csv.writer(f, delimiter=',')
                writer.writerow(data)
                packet_count += 1

        fix_type_string = get_fix_type_string(data[2]) if len(data) >= 30 else ""
        update_gui(root, data, fix_type_string, current_time, packet_count)

        # Calculate average time
        if average < 1000:
            time_diff = (datetime.now() - datetime.strptime(current_time, "%H:%M:%S.%f")).microseconds / 1000000
            averagetime += time_diff
            average += 1
        else:
            avt = round((averagetime / 1000), 3)

        root.after(500, count)

    count()

# Update the GUI with the new data
def update_gui(ser):
    global packet_count

    # Read and decode data from the serial port
    ser.reset_input_buffer()
    raw_data = ser.readline()
    decoded_data = raw_data.decode("utf-8")
    data_list = decoded_data.split(",")

    # Save radio data
    if len(data_list) >= 30:
        packet, siv, fix, lat, lon, alt, year, month, day, hour, minute, sec, nedN, nedE, nedD, \
        bat, bat33, bat51, bat52, aint, aext, ptemp, dint, dent, pres, ax, ay, az, pitch, roll, \
        yaw = data_list[:31]

        lat = round(float(lat) * .0000001, 6)
        lon = round(float(lon) * .0000001, 6)
        alt = float(alt) / 1000
        fix = int(fix)

        fix_types = {
            0: "No Fix",
            1: "Dead Reckoning",
            2: "2D",
            3: "3D",
            4: "GNSS + Dead Reckoning"
        }
        fix_type = fix_types.get(fix, "")

        # Write a new line in the csv if there is data
        if len(data_list) > 10:
            with open(fileName, "a", newline='\n') as f:
                writer = csv.writer(f, delimiter=',')
                writer.writerow(data_list)
                packet_count += 1

        # Update GUI labels
        update_labels(packet, siv, fix_type, fix, lat, lon, alt, year, month, day, hour, minute, sec, nedN,
                      nedE, nedD, bat, bat33, bat51, bat52, aint, aext, ptemp, dint, dent, pres, ax, ay, az,
                      pitch, roll, yaw)
    else:
        update_labels(1)

    # Schedule the update_gui function to run again after 500 milliseconds
    root.after(500, update_gui, ser)


def update_labels(packet, siv="", fix_type="", fix="", lat="", lon="", alt="", year="", month="", day="", hour="",
                  minute="", sec="", nedN="", nedE="", nedD="", bat="", bat33="", bat51="", bat52="", aint="", aext="",
                  ptemp="", dint="", dent="", pres="", ax="", ay="", az="", pitch="", roll="", yaw=""):
    now = datetime.now()
    current_time = now.strftime("%H:%M:%S.%f")

    data_text_1 = (
        f'Current Packet # {packet}\n'
        f'Packets Received: {packet_count}\n'
        f'Date: {year}-{month}-{day}\n'
        f'ZULU Time: {hour}:{minute}:{sec}\n\n'
        f'Time: {current_time}\n'
        f'Battery Voltage: {bat} V\n'
        f'3.3 Voltage: {bat33} V\n'
        f'5.0 Voltage: {bat51} V\n'
        f'Radio Voltage: {bat52} V\n\n'
        f'Analog Int Temp: {aint}{degree_sign}C\n'
        f'Analog Ext Temp: {aext}{degree_sign}C\n'
        f'Digital Int Temp: {dint}{degree_sign}C\n'
        f'Digital Ext Temp: {dent}{degree_sign}C\n'
        f'Pressure: {pres} hPa'
    )

    data_text_2 = (
        f'Pitch: {pitch}{degree_sign}\n'
        f'Roll: {roll}{degree_sign}\n'
        f'Yaw: {yaw}{degree_sign}\n'
        f'Accel X: {ax} m/s²\n'
        f'Accel Y: {ay} m/s²\n'
        f'Accel Z: {az} m/s²\n\n'
        f'Lat: {lat}\n'
        f'Lon: {lon}\n'
        f'Altitude: {alt} m\n'
        f'Fix Type: {fix_type}\n'
        f'SIV: {siv}\n'
        f'NED North: {nedN} m\n'
        f'NED East: {nedE} m\n'
        f'NED Down: {nedD} m\n'
    )

    data_label_1.config(text=data_text_1)
    data_label_2.config(text=data_text_2)

# Create the GUI
root = tk.Tk()
root.title("RFD-900X")

degree_sign = u'\N{DEGREE SIGN}'

data_label_1 = tk.Label(root, text="", justify="left", anchor="nw", font=("Courier", 12))
data_label_1.pack(side="left", padx=(10, 0), pady=10)

data_label_2 = tk.Label(root, text="", justify="left", anchor="nw", font=("Courier", 12))
data_label_2.pack(side="left", padx=(10, 0), pady=10)

# Start the update_gui function
update_gui(ser)

# Run the main loop
root.mainloop()