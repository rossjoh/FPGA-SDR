/*
Author: Ross Johnson
File: data_processing.v
Version: 1.1

This takes a received data point and 
takes the absolute value of the real part.
*/
module data_processing
(
    input          sys_clk,
    input          reset,
    input   [31:0] data,
    input          data_ready,
    output  [17:0] display_lines
);

reg [15:0] real_data_abs;

reg read_finished;

//combinatorial logic
always @ (*)
begin
    if(data[31:16] > 16'h7fff)
        //turn real negative number positive
        real_data_abs = (~data[31:16]) + 1;
    else
        //take the real number
        real_data_abs = data[31:16];
end
//output to LEDs
assign display_lines[15:0] = real_data_abs;

endmodule