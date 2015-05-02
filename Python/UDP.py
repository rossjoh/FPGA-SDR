# Author: Ross Johnson
# File: UDP.py
# Version: 1.1
# The python script sends UDP messages from the computer
# to the SDR by using the script make by PacketPasser.py

import socket
from time import sleep

#destination IP address
udp_ip = "192.168.10.3"

#function to send message over Ethernet
def send_udp(message, port):
 
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.sendto(message, (udp_ip, port))

def read_file():
    #open file from PacketPasser.py 
    input_file = open("output.txt", "r")
    
    counter = 0
    
    #iterate over the input file
    for line in input_file:    
        integers = []
        byte = []
        
        line_without_port = line[5:len(line)]
        port = int(line[0:5], 10)
        
        #print port number for each packet to show it is working
        print(port)
        
        for i in range(int(len(line_without_port) / 2)):
            pair = line_without_port[(i * 2)] + line_without_port[(i * 2) + 1]               
            integers.append(int(pair, 16))
        message = bytearray(integers) 
        
        send_udp(message, port)
        counter = counter + 1
        #sleep to allow the SDR to acknowledge data.
        sleep(0.01)
        
    input_file.close()


read_file();


