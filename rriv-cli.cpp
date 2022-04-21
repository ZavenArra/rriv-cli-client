/**
 *  @example example_project.cpp
 */

#include <libserial/SerialPort.h>
#include <libserial/SerialStream.h>

#include <iostream>
#include <cstring>
#include "lib/Cmd.h"
#include <unistd.h>
#include <ctime>
#include <sstream>
#include <fstream>

LibSerial::SerialPort serial_port ;
bool processingCommandFile = false;
std::ifstream commandFile;

void help(int arg_cnt, char**args)
{
  char commands[] = "Command List:\n"
  "version\n"
  "show-warranty\n"
  "get-config\n"
  "set-config\n"
  "set-slot-config\n"
  "clear-slot\n"
  "set-rtc\n"
  "get-rtc\n"
  "restart\n"
  "set-site-name\n"
  "set-deployment-identifier\n"
  "set-interval\n"
  "set-burst-number\n"
  "set-start-up-delay\n"
  "set-burst-delay\n"
  "calibrate\n"
  "set-user-note\n"
  "set-user-value\n"
  "start-logging\n"
  "deploy-now\n"
  "interactive\n"
  "trace\n"
  "check-memory\n"
  "scan-ic2\n";

  std::cout << commands << std::endl;
}

void setRTC(int arg_cnt, char **args)
{
    std::time_t time = std::time(nullptr);
    std::cout << asctime(std::localtime(&time)) << time << std::endl;

    std::stringstream timeStringStream;
    timeStringStream << time;

    serial_port.Write("set-rtc " + timeStringStream.str() + "\r\n");
    serial_port.DrainWriteBuffer();
}

void runFile(int arg_cnt, char **args)
{
    processingCommandFile = true;
    commandFile = std::ifstream(args[1]);
}

void relay(int arg_cnt, char **args)
{
    for(int i=0; i< arg_cnt; i++)
    {
        serial_port.Write(args[i]);
        if( i < arg_cnt - 1)
        {
            serial_port.Write(" ");
        }
    }
    serial_port.Write("\r\n");

}


int main()
{
    std::cin.sync_with_stdio(false);


    char port[50] = "/dev/ttyACM0";
    std::cout << "opening " << port << std::endl;

    serial_port.Open( port ) ;
    std::cout << "opened" << std::endl;

    // Set the baud rates.
    using namespace LibSerial;
    serial_port.SetBaudRate( BaudRate::BAUD_115200 );
    serial_port.SetCharacterSize(CharacterSize::CHAR_SIZE_8);
    serial_port.SetFlowControl(FlowControl::FLOW_CONTROL_NONE);
    serial_port.SetParity(Parity::PARITY_NONE) ;
    serial_port.SetStopBits(StopBits::STOP_BITS_1) ;


    char buffer[200];

    std::cout << "connecting" << std::endl;

    cmdInit(&std::cin, &std::cout);
    cmdAdd("help", help);
    cmdAdd("set-rtc", setRTC);
    cmdAdd("run-file", runFile);
    // cmdAdd("run-test", runTest);
    // cmdAdd("load-settings", loadSettingsFromFile);
    // cmdAdd("load-slot", loadSlotFromFile);
    cmdAdd("relay", relay);

    std::cout << "cmd ready" << std::endl;


    // std::string resetCommand = "restart\r\n";
    // LibSerial::DataBuffer resetCommandBuffer(resetCommand.begin(), resetCommand.end());

    // for (char i: resetCommandBuffer)
    //     std::cout << i;
    // std::cout.flush();

    // serial_port.Write(resetCommandBuffer);

    serial_port.Write("restart\r\n");
    serial_port.DrainWriteBuffer();



    LibSerial::DataBuffer dataBuffer;

    // With SerialStream objects you can read/write to the port using iostream operators.
    while(true)
    {
        while(serial_port.IsDataAvailable()){
            try {
                serial_port.Read(dataBuffer, 200, 10);
            }
            catch (const std::exception&) { /* */ }

            if(dataBuffer.size() > 0)
            {
                // std::cout << ".> ";
                for (char i: dataBuffer)
                    std::cout << i;
                std::cout.flush();
            }    

            usleep(1000);

        }

        if(processingCommandFile)
        {
            std::string line;
            if(std::getline(commandFile, line))
            {
                serial_port.Write(line + "\r\n"); // instead pass through Cmd.cpp somehow
            }
            else
            {
                processingCommandFile = false;
                commandFile.close();
            }
        }
        else
        {
            cmdPoll();
        }

        // if(strcmp(buffer, "\u200B\u200B\u200B\u200B\r") == 0)
        // {
        //     std::cout << "will read" << std::endl;
        //     std::cin.getline(buffer, 200);
        //     std::cout << "did read" << std::endl;
        //     // if(strlen(wbuffer) > 0)
        //     // {
        //         std::cout << "done read" << std::endl;
        //         std::cout << buffer << std::endl;
        //     // }
        //     // else
        //     // {
        //         std::cout << "gotnothing" << std::endl;
        //     // }
        // }
        usleep(100000);
    }
    serial_port.Close() ;
}
