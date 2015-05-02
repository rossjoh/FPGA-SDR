# Author: Ross Johnson
# File: PacketParser.py
# Version: 1.2
# This script parses the XML file output by Wireshark to create 
# an output file that another python script can use to be turned
# into C code for implementing on the Nios II. Each line in the
# output file

# REMEMBER TO REMOVE VITA AT END OF FILE AT AROUND 14000    

def start_script():
    input_file = open("wireshark.txt", "r")
    output_file = open("output.txt", "w")
    
    delete_content(output_file)
    
    counter = 1
    uhd = 0
    vita = 0
    sent_by_me = 0
    new_line = 0
    
    for line in input_file:
        if "<packet>" in line:
            if (sent_by_me == 1):
                output_file.write("\n")           
            #output_file.write(str(counter) + "\n")              
            counter = counter + 1
            
        #check the message is sent from the computer to the SDR
        if "Source: 192.168.10.1" in line: 
            sent_by_me = 1
            
        if "Source: 192.168.10.3" in line: 
            if sent_by_me == 1 and uhd == 0 and vita == 0:   
                #problem if the last packet was not a UHD or VITA packet
                print("PROBLEM")
            
            uhd = 0
            vita = 0
            sent_by_me = 0
            
        #if the packet was sent by me then analyse it
        if sent_by_me == 1:
            #check for all UHD tags within the XML file.
            if ("uhd.version" in line) or ("uhd.id" in line) or ("uhd.seq" in line) or ("uhd.i2c_addr" in line) or ("uhd.i2c_bytes" in line) or ("uhd.i2c_data" in line) or ("uhd.ip_addr" in line) or ("uhd.reg_addr" in line) or ("uhd.reg_data" in line) or ("uhd.reg_action" in line) or ("uhd.echo_len" in line): 
                        
                start = line.find('value="') + 7
                end = line.find('"/>', start)
                output_file.write(line[start:end])
                #add uhd.ip_addr part of the XML file to the output file
                if ("uhd.ip_addr" in line):
                    data_extension = ""                
                    for i in range(40):
                        data_extension = data_extension + "0"                
                    output_file.write(data_extension)  
                #add uhd.i2c_data part of the XML file to the output file
                elif ("uhd.i2c_data" in line):
                    data_extension = ""                
                    for i in range(42):
                        data_extension = data_extension + "0"                
                    output_file.write(data_extension)  
                #add uhd.reg_action part of the XML file to the output file
                elif ("uhd.reg_action" in line):
                    data_extension = ""                
                    for i in range(30):
                        data_extension = data_extension + "0"                
                    output_file.write(data_extension)
                #add uhd.echo_len part of the XML file to the output file                    
                elif ("uhd.echo_len" in line):
                    data_extension = ""                
                    for i in range(40):
                        data_extension = data_extension + "0" 
                    output_file.write(data_extension)   

                uhd = 1                            
            #if it is a vita packet
            if ("data.data" in line):
                start = line.find('value="') + 7
                end = line.find('"/>', start)
                output_file.write(line[start:end])
                vita = 1
            #write the destination port at start of each packet
            if ('proto name="udp"' in line):
                start = line.find('Dst Port: ') + 10
                output_file.write(line[start:(start + 5)])

                

    
    input_file.close()
    output_file.close()
    
    print("done!")

    #delete file contents
def delete_content(file):
    file.seek(0)
    file.truncate()

#starting point for script
start_script()





