RFD-900x / Video Shortcut

Downloading the Programs
    1.	Download the file named 'pythonScript.zip'.
    2.	Right-click on the downloaded zip file and select 'Extract All'.
    3.	In the dialog box that appears, change the extraction destination to 'C: \Users\ Public' and click on 'Extract'.
    4.	Once the extraction is complete, you need to create shortcuts for the 'exe' programs present in the extracted files.

Creating Shortcuts
    1.	Navigate to the 'Config/RFD' folder within the extracted files. The path should be 
    ... pythonScript\dist\Config' or '... pythonScript\dist\RFD-900x'.
    2.	Right-click on the desired 'exe' file and select 'Create Shortcut'
    3.	Right-click on the newly created shortcut and select 'Cut'.
    4.	Open a new File Explorer window and navigate to 
    'Local Disk\ Users\Public\Public Desktop' or 'Local Disk\Users\Public\Desktop'.
        •	If you do not see the 'Public Desktop' folder, click on the address bar at the top that displays your current location.
        •	Append a backslash ‘\’ at the end of the current location so that it looks like
            'C: \Users\Public\'. Now click on 'Desktop'.
    5.	Paste the shortcut into the desired location to make the program accessible on the desktop for all users.


Editing Code:

    1.	Download Python
        •	Visit the official Python website through this link to download Python.
        •	https://www.python.org/downloads/
        •	During installation, select the checkboxes for 'Installer Launcher for all Users and 'Add python.exe to PATH'.
        •	Click on 'Install Now'.
    2.	Downloading Python Libraries
        •	Open Command Prompt as Administrator
        •	Type in the following commands in order and press enter after each one
            o	python.exe -m pip install -upgrade pip
            o	python -m pip install pyserial
            o	pip install tk
            o	pip install pyinstaller

    3.	Copy and Paste the source code into a new folder where you can edit the program
        •	For the Config/Video shortcut program, ensure you also copy and paste the “IPChange.bat” file into the new folder as well


Formatting Edited code to an EXE Program
    1.	Move the folder containing the edited codes into the public folder. “Local Disk\Users\Public\”.
    2.	Go into the folder with the edited code, right-click on it, select 'Properties', and copy the location.
    3.	Select properties and copy the location
    4.	Open Command Prompt as an Administrator
    5.	Type in `cd `, leave a space, and then paste the copied location. Press enter.
    6.	Now type in `pyinstaller `, leave a space, and type in the name of your Python file (including the '.py' extension). Press enter. 
    7.	Once the process is complete, you can find the 'exe' file by navigating     to: 'Your Edited Code Folder\dist\Your Code Folder'. Here, you should see the 'exe' file.
        •	For the Config or Video shortcut program, remember to copy and paste the 'IPChange.bat' file into the folder containing the 'exe' file.
    8.	Repeat the steps in the 'Creating Shortcuts' section above, but for these new 'exe' files and associated directories.