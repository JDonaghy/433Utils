# About

rcswitch-pi is for controlling rc remote controlled power sockets 
with the raspberry pi. Kudos to the projects [rc-switch](http://code.google.com/p/rc-switch)
and [wiringpi](https://projects.drogon.net/raspberry-pi/wiringpi).
I just adapted the rc-switch code to use the wiringpi library instead of
the library provided by the arduino.


## Usage

First you have to install the [wiringpi](https://projects.drogon.net/raspberry-pi/wiringpi/download-and-install/) library.
After that you can compile the example program *send* by executing *make*. 
It uses wiringPi pin no 2 by default. You may want to change the used GPIO pin before compilation of the codesend.cpp source file. (Good Resource for Pin Details https://pinout.xyz/pinout/wiringpi)

```
Usage: ./RFSniffer [-h] [-q] [-l pulseLength] [-p pin] [-a codeActionFile] 
    -q: quietMode
    -a: command actions to run when code is detected. Format: code,command\n
```

## Running as a Service
Example

Create a file called `/etc/systemd/system/rf-sniffer.service`
and add the following contents (e.g.)
```
[Unit]
Description=RF sniffer service
After=multi-user.target

[Service]
Type=simple
ExecStart=/home/pi/code/433Utils/RPi_utils/RFSniffer -q -a /home/pi/code/433Utils/RPi_utils/CodeActions.csv
Restart=on-failure
RestartSec=5
KillMode=process

[Install]
WantedBy=multi-user.target
```

Next run: 
```bash 
sudo systemctl daemon-reload
sudo systemctl enable rf-sniffer.service
sudo systemctl start rf-sniffer.service
```
