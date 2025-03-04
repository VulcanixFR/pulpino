// Copyright 2015 ETH Zurich and University of Bologna.
// Copyright and related rights are licensed under the Solderpad Hardware
// License, Version 0.51 (the “License”); you may not use this file except in
// compliance with the License.  You may obtain a copy of the License at
// http://solderpad.org/licenses/SHL-0.51. Unless required by applicable law
// or agreed to in writing, software, hardware and materials distributed under
// this License is distributed on an “AS IS” BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.

`include "config.sv"

module sp_ram_wrap
	#(
		parameter RAM_SIZE   = 32768,              // in bytes
		parameter ADDR_WIDTH = $clog2(RAM_SIZE),
		parameter DATA_WIDTH = 32
	)(
		// Clock and Reset
		input  logic                    clk,
		input  logic                    rstn_i,
		input  logic                    en_i,
		input  logic [ADDR_WIDTH-1:0]   addr_i,
		input  logic [DATA_WIDTH-1:0]   wdata_i,
		output logic [DATA_WIDTH-1:0]   rdata_o,
		input  logic                    we_i,
		input  logic [DATA_WIDTH/8-1:0] be_i,
`ifdef DIFT
		input  logic                    we_i_tag,
		input  logic                    wdata_i_tag,
		output logic [DATA_WIDTH/8-1:0] rdata_o_tag,
`endif
		input  logic                    bypass_en_i
	);

`ifdef DIFT
	logic read_en;

	assign read_en = we_i_tag ? 1'b0 : en_i;
`endif

`ifdef PULP_FPGA_EMUL
	xilinx_mem_8192x32
	sp_ram_i
	(
		.clka   ( clk                    ),
		.rsta   ( 1'b0                   ), // reset is active high

		.ena    ( en_i                   ),
		.addra  ( addr_i[ADDR_WIDTH-1:2] ),
		.din	.NUM_BANKS  ( RAM_SIZE/4096 ),
		.Ba		( wdata_i                ),
		.douta  ( rdata_o                ),
		.wea    ( be_i & {4{we_i}}       )
	);

`ifdef DIFT
	xil_block_ram_8192x4b_1w1r
	sp_rap_i_tag
	(
		.CLK     ( clk                    ),
		.CE0     ( en_i                   ),
		.A0      ( addr_i[ADDR_WIDTH-1:2] ),
		.D0      ( {4{wdata_i_tag}}       ),
		.WE0     ( we_i_tag               ),
		.WEM0    ( be_i                   ),
		.CE1     ( read_en                ),
		.A1      ( addr_i[ADDR_WIDTH-1:2] ),
		.Q1      ( rdata_o_tag            )
	);
`endif

`elsif ASIC
	// RAM bypass logic
	logic [31:0] ram_out_int;
	// assign rdata_o = (bypass_en_i) ? wdata_i : ram_out_int;
	assign rdata_o = ram_out_int;

	sp_ram_bank
	#(
		.NUM_BANKS  ( RAM_SIZE/4096 ),
		.BANK_SIZE  ( 1024          )
	)
	sp_ram_bank_i
	(
		.clk_i   ( clk                     ),
		.rstn_i  ( rstn_i                  ),
		.en_i    ( en_i                    ),
		.addr_i  ( addr_i                  ),
		.wdata_i ( wdata_i                 ),
		.rdata_o ( ram_out_int             ),
		.we_i    ( (we_i & ~bypass_en_i)   ),
		.be_i    ( be_i                    )
	);

`else
	sp_ram
	#(
		.ADDR_WIDTH ( ADDR_WIDTH ),
		.DATA_WIDTH ( DATA_WIDTH ),
		.NUM_WORDS  ( RAM_SIZE   )
	)
	sp_ram_i
	(
		.clk     ( clk       ),

		.en_i    ( en_i      ),
		.addr_i  ( addr_i    ),
		.wdata_i ( wdata_i   ),
		.rdata_o ( rdata_o   ),
		.we_i    ( we_i      ),
		.be_i    ( be_i      )
	);

`ifdef DIFT
	xil_block_ram_8192x4b_1w1r
	sp_rap_i_tag
	(
		.CLK     ( clk                    ),
		.CE0     ( en_i                   ),
		.A0      ( addr_i[ADDR_WIDTH-1:2] ),
		.D0      ( {4{wdata_i_tag}}       ),
		.WE0     ( we_i_tag               ),
		.WEM0    ( be_i                   ),
		.CE1     ( read_en                ),
		.A1      ( addr_i[ADDR_WIDTH-1:2] ),
		.Q1      ( rdata_o_tag            )
	);
`endif

`endif

endmodule
