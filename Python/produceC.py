# Author: Ross Johnson
# File: produceC.py
# Version: 1.5
# This Python script creates the C code for the Nios II processor 
#that contains all the packets to be sent.

def start_script():
    input_file = open("output.txt", "r")
    output_file = open("packet_lookup.c", "w")
    
    counter = 0
    #iterate through every line of the input file.
    for line in input_file:
        #each row of the input file is made an array of constant integers
        output_file.write("const int row")  
        output_file.write(str(counter))  
        output_file.write("[] = {") 
        
        output_file.write(str(int((len(line) - 5) / 2)) + ",")
        
        port_number_string = line[0] + line[1] + line[2] + line[3] + line[4] 
        port_number_hex_string = hex(int(port_number_string))
        
        output_file.write(str(int(port_number_hex_string[2:4], 16)))
        output_file.write(",")
        output_file.write(str(int(port_number_hex_string[4:6], 16)))
        output_file.write(",")
        
        #write all values into the array for that row
        for i in range(0, int((len(line) - 5) / 2)):
            string = str(line[(i * 2) + 5]) + str(line[(i * 2) + 6])
            output_file.write(str(int(string, 16)))
            
            if(i != int((len(line) - 5) / 2) - 1):
                output_file.write(",")

        output_file.write("};\n")        

        counter = counter + 1
        
    #create an array of pointers to all rows
    output_file.write("const int *packet_lookup[] = {")
    
    #fill array with pointers
    for i in range(0, counter):
        output_file.write("row" +str(i))
        
        if(i != counter - 1):
            output_file.write(",")
            
    output_file.write("};\n")        
    #create a constant int that is the length of the array of pointers
    output_file.write("const int number_of_rows = " + str(counter) + ";")
        
    input_file.close()
    output_file.close()
    
    print("done!")
#start the script
start_script()





