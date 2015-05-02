# Author: Ross Johnson
# File: fft.py
# Version: 1.0
# This python script plots an FFT of data given by the SDR.
# It uses matplotlib to plot the FFT.
import numpy as np 
import struct
import matplotlib.pyplot as plt

input_file = open("data.txt", "r")

array_of_16_bits_comp = []
array_of_16_bits_abs = []

#iterate over all files in the data file.
for line in input_file:
    for i in range(0, len(line), 8):
    
        real = line[i:i + 4]
        real_int = struct.unpack('>h', bytes.fromhex(real))[0]              
        imag = line[i + 4:i + 8]
        imag_int = struct.unpack('>h', bytes.fromhex(imag))[0]        
        
        comp = complex(real_int, imag_int)
        array_of_16_bits_comp.append(comp)
        array_of_16_bits_abs.append(abs(comp))
        
    print(array_of_16_bits_comp)
    print(array_of_16_bits_abs)
    
    #plot FFT.
    fft_data = np.fft.fft(array_of_16_bits_comp)

   # freq = np.fft.fftfreq(t.shape[-1])
    plt.plot(abs(fft_data))
    plt.show()