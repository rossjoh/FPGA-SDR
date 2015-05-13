# FPGA-SDR
DE2-115 based controller for NI USRP 2920

1.  Create an empty Quartus II project called final_ethernet_controller.
    
2.  Add all files in repository HDL folder to the project directory and add them in Quartus.
    
3.  Move nios_system.qsys from the repository Qsys folder into the project directory.
    
4.  Open Qsys in Quartus and open nios_system.qsys.
    
5.  Generate the HDL in Qsys.
    
6.  Add the nios_system.qip created to the project.
    
7.  Replace final_ethernet_controller.qsf in the project directory with
    final_ethernet_controller.qsf from the repository. Add the .sdc file
    from the timing repository folder and add to project.
    
8.  Open Eclipse from Quartus and create new Nios II project with BSP.
    
9.  Use the nios_system.sopc created when the Nios was built. Make sure
    the project is blank.
    
10. Drag all code from the Nios C Code repository folder into the eclipse project.

11. Build the BSP and then the project and run as Nios II hardware