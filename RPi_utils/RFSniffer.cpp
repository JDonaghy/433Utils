/*
  RFSniffer

  Usage: ./RFSniffer [<pulseLength>]
  [] = optional

  Hacked from http://code.google.com/p/rc-switch/
  by @justy to provide a handy RF code sniffer
*/

#include "../rc-switch/RCSwitch.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>

#include <execinfo.h>
#include <signal.h>
#include <pthread.h>

using namespace std;

RCSwitch mySwitch;
 
int PIN = 4;
int pulseLength = 0;

void * callback(void * arg) {
      if (mySwitch.available()) {
        int value = mySwitch.getReceivedValue();
    
        if (value == 0) {
          printf("Unknown encoding\n");
        } else {    
          printf("Received %i\n", mySwitch.getReceivedValue() );
        }
    
        fflush(stdout);
        mySwitch.resetAvailable();
      }
      return NULL;
}

int setup() {
  // This pin is not the first pin on the RPi GPIO header!
     // Consult https://projects.drogon.net/raspberry-pi/wiringpi/pins/
     // for more information.
     
     if(wiringPiSetup() == -1) {
       printf("wiringPiSetup failed, exiting...");
       return 0;
     }

     mySwitch = RCSwitch();
     if (pulseLength != 0) mySwitch.setPulseLength(pulseLength);
     mySwitch.enableReceive(PIN);  // Receiver on interrupt 0 => that is pin #2
     mySwitch.setReceiveCallback(callback);   
     return 1;
}

int main(int argc, char *argv[]) {
    if (argv[1] != NULL) pulseLength = atoi(argv[1]);
    setup(); 
    while (true)
      sleep(10000000); 
    exit(0);
}

