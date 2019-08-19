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
#include <iterator>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>

#include <execinfo.h>
#include <signal.h>
#include <pthread.h>

using namespace std;

//
//  Source: https://stackoverflow.com/questions/865668/how-to-parse-command-line-arguments-in-c
//
class InputParser{
    public:
        InputParser (int &argc, char **argv){
            for (int i=1; i < argc; ++i)
                this->tokens.push_back(std::string(argv[i]));
        }
        /// @author iain
        const std::string& getCmdOption(const std::string &option) const{
            std::vector<std::string>::const_iterator itr;
            itr =  std::find(this->tokens.begin(), this->tokens.end(), option);
            if (itr != this->tokens.end() && ++itr != this->tokens.end()){
                return *itr;
            }
            static const std::string empty_string("");
            return empty_string;
        }
        /// @author iain
        bool cmdOptionExists(const std::string &option) const{
            return std::find(this->tokens.begin(), this->tokens.end(), option)
                != this->tokens.end();
        }
    private:
        std::vector <std::string> tokens;
};

// soure: https://stackoverflow.com/questions/1120140/how-can-i-read-and-parse-csv-files-in-c
class CSVRow
{
    public:
        std::string const& operator[](std::size_t index) const
        {
            return m_data[index];
        }
        std::size_t size() const
        {
            return m_data.size();
        }
        void readNextRow(std::istream& str)
        {
            std::string     line;
            std::getline(str, line);

            std::stringstream   lineStream(line);
            std::string     cell;

            m_data.clear();
            while(std::getline(lineStream, cell, ','))
            {
                m_data.push_back(cell);
            }
            // This checks for a trailing comma with no data after it.
            if (!lineStream && cell.empty())
            {
                // If there was a trailing comma then add an empty element.
                m_data.push_back("");
            }
        }
    private:
        std::vector<std::string>    m_data;
};

std::vector<std::string> getNextLineAndSplitIntoTokens(std::istream& str)
{
    std::vector<std::string>   result;
    std::string        line;
    std::getline(str,line);

    std::stringstream      lineStream(line);
    std::string        cell;

    while(std::getline(lineStream,cell, ','))
    {
        result.push_back(cell);
    }
    // This checks for a trailing comma with no data after it.
    if (!lineStream && cell.empty())
    {
        // If there was a trailing comma then add an empty element.
        result.push_back("");
    }
    return result;
}

std::istream& operator>>(std::istream& str, CSVRow& data)
{
    data.readNextRow(str);
    return str;
}

RCSwitch mySwitch;

bool quietMode=false;
int PIN = 2;
int pulseLength = 0;
vector<pair<int,string>> codeActions; 

void * callback(void * arg) {
    if (mySwitch.available()) {
        int value = mySwitch.getReceivedValue();

        if (!quietMode) {
            if (value == 0) {
                cout << "Unknown encoding" << '\n';
            } else {    
                cout << "Received " << mySwitch.getReceivedValue() << '\n';
            }
        }
        vector<pair<int,string>>::iterator it;
        for (it = codeActions.begin(); it != codeActions.end(); it++ ){
            if (it->first == mySwitch.getReceivedValue()){
                cout << it->second << "\n";
                system(it->second.c_str());
                break;
            }
        }
        mySwitch.resetAvailable();
    }
    return NULL;
}

int setupRCSwitch() {
    // This pin is not the first pin on the RPi GPIO header!
    // Consult https://projects.drogon.net/raspberry-pi/wiringpi/pins/
    // for more information.

    if(wiringPiSetup() == -1) {
        return 0;
    }

    mySwitch = RCSwitch();
    if (pulseLength != 0) mySwitch.setPulseLength(pulseLength);
    mySwitch.enableReceive(PIN);  // Receiver on interrupt 0 => that is pin #2
    mySwitch.setReceiveCallback(callback);   
    return 1;
}

void readConfig(string codeActionFile) 
{
    std::ifstream file(codeActionFile);
    CSVRow row;
    while(file >> row){
        string code = row[0];
        string action = row[1];
        codeActions.push_back(make_pair(stoi(code), action));
    }
}

int main(int argc, char *argv[]) {
    InputParser input(argc, argv);
    if (input.cmdOptionExists("-h")){
	cout << "Usage: " << argv[0] << " [-h] [-q] [-l pulseLength] [-p pin] [-a codeActionFile] " << '\n';
	cout << "    -q: quietMode" << '\n';
	cout << "    -a: command actions to run when code is detected. Format: code,command\\n" << '\n';
	exit(0);
    }
    if (input.cmdOptionExists("-l")){
        pulseLength = stoi(input.getCmdOption("-l"));
    }
    if (input.cmdOptionExists("-q")){
        quietMode = true;
    }
    if (input.cmdOptionExists("-p")){
        PIN = stoi(input.getCmdOption("-p"));
    }

    const std::string &filename = input.getCmdOption("-a");
    if (!filename.empty()){
        readConfig(filename);
        if (!quietMode) {
            vector<pair<int,string>>::iterator it;
            for (it = codeActions.begin(); it != codeActions.end(); it++ ){
                cout << "Code: " << it->first << ", Action: " << it->second << '\n';
            }
        }
    }

    if (setupRCSwitch() == 0) { 
        cerr << "wiringPiSetup failed, exiting..." << '\n';
        exit (1);
    }

    while (true) {
        sleep(10000000); 
    }

    exit(0);
}

