# Escape room camera stream

## Author

This project is created by Thibault Feraux in extension of the code of [Random Nerd Tutorials](https://randomnerdtutorials.com/video-streaming-with-raspberry-pi-camera)

## Explanation

Streams the camera on the Raspberry Pi via the local IP address so you can watch it from another device on the same network.

## Usage

1. Clone or download this repository in the `/home/pi` folder

2. In the terminal run the command `sudo nano /etc/rc.local` to edit the startup scripts

3. Right before the line `exit 0` Add the following command
    ```
    sudo python3 /home/pi/camera_stream/camera_stream.py
    ```

4. To save and exit the file type `Ctrl-x` and than `Y`

5. Now reboot the Pi to test it:
    ```
    sudo reboot
    ```
