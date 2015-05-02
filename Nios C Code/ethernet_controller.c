/*
Author: Ross Johnson
File: ethernet_controller.c
Version: 1.2

An Ethernet controller that can control a NI USRP-2920
*/
#include <altera_avalon_sgdma.h>
#include <altera_avalon_sgdma_descriptor.h>
#include <altera_avalon_sgdma_regs.h>
#include "sys/alt_stdio.h"
#include "sys/alt_irq.h"
#include <unistd.h>
#include <alt_types.h>
#include "altera_avalon_pio_regs.h"
#include "packet_lookup.h"
#include "system.h"

#define PACKET_LENGTH 2048

void setup(void);
void rx_ethernet_interupt(void *context);
void create_ARP(void);
void create_UHD(int number_of_packets);

// Create SGDMA transmit and receive devices
alt_sgdma_dev *sgdma_tx_device;
alt_sgdma_dev *sgdma_rx_device;

// Allocate descriptors in the descriptor_memory (onchip memory)
alt_sgdma_descriptor tx_descriptor		__attribute__((section(".descriptor_memory")));
alt_sgdma_descriptor tx_descriptor_end	__attribute__((section(".descriptor_memory")));
alt_sgdma_descriptor rx_descriptor  	__attribute__((section(".descriptor_memory")));
alt_sgdma_descriptor rx_descriptor_end  __attribute__((section(".descriptor_memory")));

unsigned char tx_frame[PACKET_LENGTH];
int arp_complete_flag = 0;
unsigned char sdr_mac_address[6];
extern const int *packet_lookup[];
extern const int number_of_rows;

// Create a receive frame
unsigned char rx_frame[PACKET_LENGTH] = { 0 };

int main(void)
{	    
    int number_of_packets = 0;    
    setup(); //setups the SGDMA devices and the TSE megacore
    alt_printf("Starting ARP\n");

    while(arp_complete_flag == 0)
    {	
        create_ARP();
   
        //send packet into SGDMA for transmit
        alt_avalon_sgdma_construct_mem_to_stream_desc(&tx_descriptor, &tx_descriptor_end, (alt_u32 *)tx_frame, 44, 0, 1, 1, 0);
        alt_avalon_sgdma_do_async_transfer(sgdma_tx_device, &tx_descriptor);    
        while(alt_avalon_sgdma_check_descriptor_status(&tx_descriptor) != 0);
        
        alt_printf("Transmitting ARP\n");
        
        usleep(5000000);        
    }
    
    alt_printf("Received ARP\n");
    
    while(number_of_packets < number_of_rows)
    {                
        create_UHD(number_of_packets);
                        
        //send packet into SGDMA for transmit
        alt_avalon_sgdma_construct_mem_to_stream_desc(&tx_descriptor, &tx_descriptor_end, (alt_u32 *)tx_frame, packet_lookup[number_of_packets][0] + 44, 0, 1, 1, 0);
        alt_avalon_sgdma_do_async_transfer(sgdma_tx_device, &tx_descriptor);    
        while(alt_avalon_sgdma_check_descriptor_status(&tx_descriptor) != 0);
        
        usleep(10000);
        number_of_packets++;
    }
    
    alt_printf("SDR Configured\n");
    
    return 0;
}

void setup(void)
{
    sgdma_tx_device = alt_avalon_sgdma_open("/dev/sgdma_tx");
	if(sgdma_tx_device == NULL)
    {
		alt_printf("Error: could not open SGDMA transmit device\n");
        return;
	}
    else
    {
        alt_printf("Opened SGDMA transmit device\n");
    }
    
    // Open the sgdma receive device
	sgdma_rx_device = alt_avalon_sgdma_open("/dev/sgdma_rx");
	if(sgdma_rx_device == NULL)
    {
		alt_printf("Error: could not open SGDMA receive device\n");
        return;
	}
    else
    {
        alt_printf("Opened SGDMA receive device\n");
    }
    
    	
	alt_avalon_sgdma_register_callback(sgdma_rx_device, (alt_avalon_sgdma_callback) rx_ethernet_interupt, 0x00000014, NULL); // Set interrupts for the sgdma receive device
	alt_avalon_sgdma_construct_stream_to_mem_desc(&rx_descriptor, &rx_descriptor_end, rx_frame, 0, 0); // Create sgdma receive descriptor
	alt_avalon_sgdma_do_async_transfer(sgdma_rx_device, &rx_descriptor); // Set up non-blocking transfer of sgdma receive descriptor
    
        
    // TSE core base address pointer
	volatile int *tse_address = (int *) TSE_MAC_BASE;
    //setup TSE IP core
	*(tse_address + 0x03) = 0x116E6001;
	*(tse_address + 0x04) = 0x00000F02; 
	*(tse_address + 0x0F) = 0x10; 
	*(tse_address + 0x10) = 0x11;	
	*(tse_address + 0x94) = 0x4000;
	*(tse_address + 0xB0) = *(tse_address + 0xB0) | 0x60;
	*(tse_address + 0xB4) = *(tse_address + 0xB4) | 0x82;	
	*(tse_address + 0xA0) = *(tse_address + 0xA0) | 0x8000;
	while(*(tse_address + 0xA0) & 0x8000);	    
	*(tse_address + 2) = *(tse_address + 2) | 0x0000004B;
    alt_printf("TSE address is correct!\n");
    return;
}

void rx_ethernet_interupt(void *context)
{    
    int i, data_point;
	// Wait until receive descriptor transfer is complete
	while(alt_avalon_sgdma_check_descriptor_status(&rx_descriptor) != 0);
    
    if((rx_frame[14] == 0x08) && (rx_frame[15] == 0x06)) //ARP received
    {
        arp_complete_flag = 1;
        for(i = 0; i < 6; i++)
        {
            //set MAC address variable
            sdr_mac_address[i] = rx_frame[24 + i];
        }
    }
    //VITA 49 received
    else if(rx_frame[44] == 0x14)
    {
        //data ready line set to 0
        IOWR_ALTERA_AVALON_PIO_DATA(DATA_READY_BASE, 0);   
        //create data point
        data_point = rx_frame[60] * 16777216 + rx_frame[61] * 65536 + rx_frame[62] * 256 + rx_frame[63];
        //write data out
        IOWR_ALTERA_AVALON_PIO_DATA(DATA_LINES_BASE, data_point);
        //data ready line set to 1
        IOWR_ALTERA_AVALON_PIO_DATA(DATA_READY_BASE, 1);        
    }
    
    for(i = 0; i < 6; i++)
    {
        sdr_mac_address[i] = rx_frame[24 + i];
    }
    IOWR_ALTERA_AVALON_PIO_DATA(DATA_READY_BASE, 0);
    
	//create next receive SGDMA
	alt_avalon_sgdma_construct_stream_to_mem_desc(&rx_descriptor, &rx_descriptor_end, rx_frame, 0, 0); // Create new receive sgdma descriptor	
	alt_avalon_sgdma_do_async_transfer(sgdma_rx_device, &rx_descriptor); // Set up non-blocking transfer of sgdma receive descriptor
}
//This defines an ARP packet
void create_ARP(void)
{
    tx_frame[0] = 0x00;       
    tx_frame[1] = 0x00;
    tx_frame[2] = 0xFF;
    tx_frame[3] = 0xFF;
    tx_frame[4] = 0xFF;
    tx_frame[5] = 0xFF;
    tx_frame[6] = 0xFF;
    tx_frame[7] = 0xFF;
    tx_frame[8] = 0x01;
    tx_frame[9] = 0x60;
    tx_frame[10] = 0x6E;
    tx_frame[11] = 0x11;
    tx_frame[12] = 0x02;
    tx_frame[13] = 0x0F;
    tx_frame[14] = 0x08;
    tx_frame[15] = 0x06;
    tx_frame[16] = 0x00;
    tx_frame[17] = 0x01;
    tx_frame[18] = 0x08;
    tx_frame[19] = 0x00;
    tx_frame[20] = 0x06;
    tx_frame[21] = 0x04;
    tx_frame[22] = 0x00;
    tx_frame[23] = 0x01;
    
    tx_frame[24] = tx_frame[8];
    tx_frame[25] = tx_frame[9]; 
    tx_frame[26] = tx_frame[10];
    tx_frame[27] = tx_frame[11];
    tx_frame[28] = tx_frame[12];
    tx_frame[29] = tx_frame[13];
    tx_frame[30] = 0xC0;
    tx_frame[31] = 0xA8;
    tx_frame[32] = 0x0A;
    tx_frame[33] = 0x01;
    tx_frame[34] = 0x00;
    tx_frame[35] = 0x00;
    tx_frame[36] = 0x00;
    tx_frame[37] = 0x00;
    tx_frame[38] = 0x00;
    tx_frame[39] = 0x00;
    tx_frame[40] = 0xC0;
    tx_frame[41] = 0xA8;
    tx_frame[42] = 0x0A;
    tx_frame[43] = 0x03;
    
    return;
}
// this fiunction creates a UHD packet from 
void create_UHD(int number_of_packets)
{
    int i;
    //clear previous packet
    for(i = 0; i < PACKET_LENGTH; i++)
    {        
        tx_frame[i] = 0;
    }
    //link layer
    tx_frame[0] = 0x00;
    tx_frame[1] = 0x00;
    tx_frame[2] = sdr_mac_address[0];
    tx_frame[3] = sdr_mac_address[1];
    tx_frame[4] = sdr_mac_address[2];
    tx_frame[5] = sdr_mac_address[3];
    tx_frame[6] = sdr_mac_address[4];
    tx_frame[7] = sdr_mac_address[5];
    tx_frame[8] = 0x01;
    tx_frame[9] = 0x60;
    tx_frame[10] = 0x6E;
    tx_frame[11] = 0x11;
    tx_frame[12] = 0x02;
    tx_frame[13] = 0x0F;
    tx_frame[14] = 0x08;
    tx_frame[15] = 0x00;
    //IPv4
    tx_frame[16] = 0x45;
    tx_frame[17] = 0x00;
    tx_frame[18] = (int)((packet_lookup[number_of_packets][0] + 44 - 16) / 256);
    tx_frame[19] = (int)((packet_lookup[number_of_packets][0] + 44 - 16) % 256);
    tx_frame[20] = 0x00;
    tx_frame[21] = 0x00;
    tx_frame[22] = 0x40;
    tx_frame[23] = 0x00;
    tx_frame[24] = 0x40;
    tx_frame[25] = 0x11;
    tx_frame[26] = 0x00;
    tx_frame[27] = 0x00;
    tx_frame[28] = 0xC0;
    tx_frame[29] = 0xA8;
    tx_frame[30] = 0x0A;
    tx_frame[31] = 0x01;
    tx_frame[32] = 0xC0;
    tx_frame[33] = 0xA8;
    tx_frame[34] = 0x0A;
    tx_frame[35] = 0x03;
    //UDP
    tx_frame[36] = 0xE8;
    tx_frame[37] = 0x3A;
    tx_frame[38] = packet_lookup[number_of_packets][1];
    tx_frame[39] = packet_lookup[number_of_packets][2];
    tx_frame[40] = (int)((packet_lookup[number_of_packets][0] + 44 - 36) / 256);
    tx_frame[41] = (int)((packet_lookup[number_of_packets][0] + 44 - 36) % 256);
    tx_frame[42] = 0x00;
    tx_frame[43] = 0x00;
    
    //fill UHD data    
    for(i = 0; i < packet_lookup[number_of_packets][0]; i++)
    {        
        tx_frame[i + 44] = packet_lookup[number_of_packets][i + 3];
    }
    
    return;
}