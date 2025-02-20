// Copyright 2015 ETH Zurich and University of Bologna.
// Copyright and related rights are licensed under the Solderpad Hardware
// License, Version 0.51 (the “License”); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://solderpad.org/licenses/SHL-0.51. Unless required by applicable law
// or agreed to in writing, software, hardware and materials distributed under
// this License is distributed on an “AS IS” BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.

`include "axi_bus.sv"
`include "config.sv"

module core_region
#(
	parameter AXI_ADDR_WIDTH       = 32,  // Largeur d'adresse AXI
	parameter AXI_DATA_WIDTH       = 64,  // Largeur de données AXI
	parameter AXI_ID_MASTER_WIDTH  = 10,  // Largeur de l'ID maître AXI
	parameter AXI_ID_SLAVE_WIDTH   = 10,   // Largeur de l'ID esclave AXI
	parameter AXI_USER_WIDTH       = 0,   // Largeur des signaux utilisateur AXI
	parameter DATA_RAM_SIZE        = 32768, // Taille de la RAM de données en octets
	parameter INSTR_RAM_SIZE       = 32768  // Taille de la RAM d'instructions en octets
)
(
	// Horloge et Reset
	input	logic 			clk,				// Horloge d'entrée
	input	logic 			rst_n,				// Reset actif bas

	input	logic 			testmode_i,			// Mode test
	input	logic 			fetch_enable_i,		// Autorisation de fetch
	input	logic [31:0]	irq_i,				// Interruptions
	output	logic			core_busy_o,		// Signal indiquant que le coeur est occupé
	input	logic			clock_gating_i,		// Signal de gating d'horloge
	input	logic [31:0] 	boot_addr_i, 		// Adresse de boot

	AXI_BUS.Master			core_master,		// Interface maître AXI pour le coeur
	AXI_BUS.Master			dbg_master,			// Interface maître AXI pour le debug
	AXI_BUS.Slave			data_slave,			// Interface esclave AXI pour les données
	AXI_BUS.Slave			instr_slave,		// Interface esclave AXI pour les instructions
	DEBUG_BUS.Slave			debug,				// Interface esclave de debug

	// Signaux JTAG
	input	logic			tck_i,				// Horloge JTAG
	input	logic			trstn_i,			// Reset JTAG
	input	logic			tms_i,				// Mode sélection JTAG
	input	logic			tdi_i,				// Entrée de données JTAG
	output	logic			tdo_o				// Sortie de données JTAG
);

	localparam INSTR_ADDR_WIDTH = $clog2(INSTR_RAM_SIZE)+1; // Largeur d'adresse pour la RAM d'instructions
	localparam DATA_ADDR_WIDTH  = $clog2(DATA_RAM_SIZE);    // Largeur d'adresse pour la RAM de données

	localparam AXI_B_WIDTH      = $clog2(AXI_DATA_WIDTH/8);  // Largeur d'un octet AXI

	// Signaux vers/depuis le coeur
	logic			core_instr_req;			// Requête d'instruction depuis le coeur
	logic			core_instr_gnt;			// Accord d'instruction vers le coeur
	logic			core_instr_rvalid;  	// Validité de la réponse d'instruction
	logic [31:0]	core_instr_addr;		// Adresse de l'instruction
	logic [31:0]	core_instr_rdata;		// Données d'instruction lues

	logic			core_lsu_req;			// Requête de l'unité de chargement/stockage
	logic			core_lsu_gnt;			// Accord de l'unité de chargement/stockage
	logic			core_lsu_rvalid;		// Validité de la réponse de l'unité de chargement/stockage
	logic [31:0]	core_lsu_addr;			// Adresse de l'unité de chargement/stockage
	logic 			core_lsu_we;			// Écriture activée pour l'unité de chargement/stockage
	logic [3:0]		core_lsu_be;			// Byte enable pour l'unité de chargement/stockage
	logic [31:0]	core_lsu_rdata;			// Données lues par l'unité de chargement/stockage
	logic [31:0]	core_lsu_wdata;			// Données à écrire par l'unité de chargement/stockage

	logic			core_data_req;			// Requête de données depuis le coeur
	logic			core_data_gnt;			// Accord de données vers le coeur
	logic			core_data_rvalid;		// Validité de la réponse de données
	logic [31:0]	core_data_addr;			// Adresse des données
	logic			core_data_we;			// Écriture activée pour les données
	logic [3:0]		core_data_be;			// Byte enable pour les données
	logic [31:0]	core_data_rdata;		// Données lues
	logic [31:0]	core_data_wdata;		// Données à écrire

	// Signaux vers/depuis la mémoire AXI
	logic							is_axi_addr;		// Indique si l'adresse est dans l'espace AXI
	logic							axi_mem_req;		// Requête de mémoire AXI
	logic [DATA_ADDR_WIDTH-1:0] 	axi_mem_addr;		// Adresse de la mémoire AXI
	logic							axi_mem_we;			// Écriture activée pour la mémoire AXI
	logic [AXI_DATA_WIDTH/8-1:0]	axi_mem_be;			// Byte enable pour la mémoire AXI
	logic [AXI_DATA_WIDTH-1:0]		axi_mem_rdata;		// Données lues depuis la mémoire AXI
	logic [AXI_DATA_WIDTH-1:0]		axi_mem_wdata;		// Données à écrire dans la mémoire AXI

	// Signaux vers/depuis la mémoire d'instructions AXI
	logic 							axi_instr_req;		// Requête d'instructions AXI
	logic [INSTR_ADDR_WIDTH-1:0]	axi_instr_addr;		// Adresse des instructions AXI
	logic							axi_instr_we;		// Écriture activée pour les instructions AXI
	logic [AXI_DATA_WIDTH/8-1:0]	axi_instr_be;		// Byte enable pour les instructions AXI
	logic [AXI_DATA_WIDTH-1:0]		axi_instr_rdata;	// Données d'instructions lues
	logic [AXI_DATA_WIDTH-1:0]		axi_instr_wdata;	// Données d'instructions à écrire

	// Signaux vers/depuis la mémoire d'instructions
	logic							instr_mem_en;		// Activation de la mémoire d'instructions
	logic [INSTR_ADDR_WIDTH-1:0]	instr_mem_addr;		// Adresse de la mémoire d'instructions
	logic							instr_mem_we;		// Écriture activée pour la mémoire d'instructions
	logic [AXI_DATA_WIDTH/8-1:0]	instr_mem_be;		// Byte enable pour la mémoire d'instructions
	logic [AXI_DATA_WIDTH-1:0]		instr_mem_rdata;	// Données lues depuis la mémoire d'instructions
	logic [AXI_DATA_WIDTH-1:0]		instr_mem_wdata;	// Données à écrire dans la mémoire d'instructions

	// Signaux vers/depuis la mémoire de données
	logic 							data_mem_en;		// Activation de la mémoire de données
	logic [DATA_ADDR_WIDTH-1:0]		data_mem_addr;		// Adresse de la mémoire de données
	logic 							data_mem_we;		// Écriture activée pour la mémoire de données
	logic [AXI_DATA_WIDTH/8-1:0]	data_mem_be;		// Byte enable pour la mémoire de données
	logic [AXI_DATA_WIDTH-1:0]		data_mem_rdata;		// Données lues depuis la mémoire de données
	logic [AXI_DATA_WIDTH-1:0]		data_mem_wdata;		// Données à écrire dans la mémoire de données

	enum logic [0:0] { AXI, RAM } lsu_resp_CS, lsu_resp_NS;  // États de réponse de l'unité de chargement/stockage

	// Signaux vers/depuis le wrapper core2axi
	logic			core_axi_req;			// Requête AXI depuis le coeur
	logic			core_axi_gnt;			// Accord AXI vers le coeur
	logic			core_axi_rvalid;		// Validité de la réponse AXI
	logic [31:0]	core_axi_addr;			// Adresse AXI
	logic			core_axi_we;			// Écriture activée pour AXI
	logic [3:0]		core_axi_be;			// Byte enable pour AXI
	logic [31:0]	core_axi_rdata;			// Données lues depuis AXI
	logic [31:0]	core_axi_wdata;			// Données à écrire dans AXI

`ifdef DIFT
	// Signaux de tag pour le mode DIFT
	logic			core_lsu_gnt_tag;
	logic 			core_lsu_we_tag;
	logic [3:0]		core_lsu_rdata_tag;
	logic			core_lsu_rvalid_tag;
	logic			core_data_we_tag;
	logic			core_data_wdata_tag;
	logic [3:0]		core_data_rdata_tag;
`endif

	AXI_BUS
	#(
		.AXI_ADDR_WIDTH   ( AXI_ADDR_WIDTH      ),
		.AXI_DATA_WIDTH   ( AXI_DATA_WIDTH      ),
		.AXI_ID_WIDTH     ( AXI_ID_MASTER_WIDTH ),
		.AXI_USER_WIDTH   ( AXI_USER_WIDTH      )
	)
	core_master_int();  // Instance du maître AXI interne

	//----------------------------------------------------------------------------//
	// Instanciation du coeur
	//----------------------------------------------------------------------------//

	riscv_core
	#(
		.N_EXT_PERF_COUNTERS ( 0 )  // Nombre de compteurs de performance externes
	)
	RISCV_CORE
	(
		.clk_i           ( clk               ),  // Horloge
		.rst_ni          ( rst_n             ),  // Reset
		.clock_en_i      ( clock_gating_i    ),  // Activation de l'horloge
		.test_en_i       ( testmode_i        ),  // Mode test
		.boot_addr_i     ( boot_addr_i       ),  // Adresse de boot
		.core_id_i       ( 4'h0              ),  // ID du coeur
		.cluster_id_i    ( 6'h0              ),  // ID du cluster
		.instr_addr_o    ( core_instr_addr   ),  // Adresse de l'instruction
		.instr_req_o     ( core_instr_req    ),  // Requête d'instruction
		.instr_rdata_i   ( core_instr_rdata  ),  // Données d'instruction lues
		.instr_gnt_i     ( core_instr_gnt    ),  // Accord d'instruction
		.instr_rvalid_i  ( core_instr_rvalid ),  // Validité de la réponse d'instruction
		.data_addr_o     ( core_lsu_addr     ),  // Adresse des données
		.data_wdata_o    ( core_lsu_wdata    ),  // Données à écrire
		.data_we_o       ( core_lsu_we       ),  // Écriture activée
		.data_req_o      ( core_lsu_req      ),  // Requête de données
		.data_be_o       ( core_lsu_be       ),  // Byte enable
		.data_rdata_i    ( core_lsu_rdata    ),  // Données lues
		.data_gnt_i      ( core_lsu_gnt      ),  // Accord de données
		.data_rvalid_i   ( core_lsu_rvalid   ),  // Validité de la réponse de données
		.data_err_i      ( 1'b0              ),  // Erreur de données
`ifdef DIFT
		// Connexions de tag pour le mode DIFT
		.data_gnt_i_tag    ( core_lsu_gnt_tag    ),
		.data_rdata_i_tag  ( core_lsu_rdata_tag  ),
		.data_rvalid_i_tag ( core_lsu_rvalid_tag ),
		.data_we_o_tag     ( core_lsu_we_tag     ),
		.data_wdata_o_tag  ( core_lsu_wdata_tag  ),
`endif
		.irq_i           ( irq_i             ),  // Interruptions
		.debug_req_i     ( debug.req         ),  // Requête de debug
		.debug_gnt_o     ( debug.gnt         ),  // Accord de debug
		.debug_rvalid_o  ( debug.rvalid      ),  // Validité de la réponse de debug
		.debug_addr_i    ( debug.addr        ),  // Adresse de debug
		.debug_we_i      ( debug.we          ),  // Écriture activée pour le debug
		.debug_wdata_i   ( debug.wdata       ),  // Données à écrire pour le debug
		.debug_rdata_o   ( debug.rdata       ),  // Données lues pour le debug
		.debug_halted_o  (                   ),  // Cœur arrêté
		.debug_halt_i    ( 1'b0              ),  // Arrêt de debug
		.debug_resume_i  ( 1'b0              ),  // Reprise de debug
		.fetch_enable_i  ( fetch_enable_i    ),  // Autorisation de fetch
		.core_busy_o     ( core_busy_o       ),  // Cœur occupé
		.ext_perf_counters_i (               )	 // Compteurs de performance externes
	);

	core2axi_wrap
	#(
		.AXI_ADDR_WIDTH   ( AXI_ADDR_WIDTH      ),
		.AXI_DATA_WIDTH   ( AXI_DATA_WIDTH      ),
		.AXI_ID_WIDTH     ( AXI_ID_MASTER_WIDTH ),
		.AXI_USER_WIDTH   ( AXI_USER_WIDTH      ),
		.REGISTERED_GRANT ( "FALSE"             )
	)
	core2axi_i
	(
		.clk_i         ( clk             ),  // Horloge
		.rst_ni        ( rst_n           ),  // Reset
		.data_req_i    ( core_axi_req    ),  // Requête de données
		.data_gnt_o    ( core_axi_gnt    ),  // Accord de données
		.data_rvalid_o ( core_axi_rvalid ),  // Validité de la réponse de données
		.data_addr_i   ( core_axi_addr   ),  // Adresse des données
		.data_we_i     ( core_axi_we     ),  // Écriture activée
		.data_be_i     ( core_axi_be     ),  // Byte enable
		.data_rdata_o  ( core_axi_rdata  ),  // Données lues
		.data_wdata_i  ( core_axi_wdata  ),  // Données à écrire
		.master        ( core_master_int )  // Maître AXI interne
	);

	//----------------------------------------------------------------------------//
	// Tranches AXI
	//----------------------------------------------------------------------------//

	axi_slice_wrap
	#(
		.AXI_ADDR_WIDTH ( AXI_ADDR_WIDTH       ),
		.AXI_DATA_WIDTH ( AXI_DATA_WIDTH       ),
		.AXI_USER_WIDTH ( AXI_USER_WIDTH       ),
		.AXI_ID_WIDTH   ( AXI_ID_MASTER_WIDTH  ),
		.SLICE_DEPTH    ( 2                    )
	)
	axi_slice_core2axi
	(
		.clk_i      ( clk             ),  // Horloge
		.rst_ni     ( rst_n           ),  // Reset
		.test_en_i  ( testmode_i      ),  // Mode test
		.axi_slave  ( core_master_int ),  // Esclave AXI interne
		.axi_master ( core_master     )  // Maître AXI
	);

	//----------------------------------------------------------------------------//
	// DEMUX
	//----------------------------------------------------------------------------//
	assign is_axi_addr		= (core_lsu_addr[31:20] != 12'h001);  // Détermine si l'adresse est dans l'espace AXI
	assign core_data_req	= (~is_axi_addr) & core_lsu_req;  // Requête de données pour la RAM
	assign core_axi_req		=   is_axi_addr  & core_lsu_req;  // Requête de données pour AXI

	assign core_data_addr	= core_lsu_addr;			// Adresse des données
	assign core_data_we		= core_lsu_we;				// Écriture activée pour les données
	assign core_data_be		= core_lsu_be;				// Byte enable pour les données
	assign core_data_wdata	= core_lsu_wdata;			// Données à écrire

`ifdef DIFT
	assign core_data_we_tag    = core_lsu_we_tag;		// Tag d'écriture pour le mode DIFT
	assign core_data_wdata_tag = core_lsu_wdata_tag;	// Tag de données à écrire pour le mode DIFT
`endif

	assign core_axi_addr	= core_lsu_addr;			// Adresse AXI
	assign core_axi_we		= core_lsu_we;				// Écriture activée pour AXI
	assign core_axi_be		= core_lsu_be;				// Byte enable pour AXI
	assign core_axi_wdata	= core_lsu_wdata;			// Données à écrire pour AXI

	always_ff @(posedge clk, negedge rst_n)
	begin
		if (rst_n == 1'b0)
			lsu_resp_CS <= RAM;  // Réinitialisation de l'état de réponse
		else
			lsu_resp_CS <= lsu_resp_NS;  // Mise à jour de l'état de réponse
	end

	// Détermine la source de la prochaine réponse
	always_comb
	begin
		lsu_resp_NS  = lsu_resp_CS;  // État de réponse par défaut
		core_lsu_gnt = 1'b0;  // Accord par défaut

`ifdef DIFT
		core_lsu_gnt_tag = 1'b0;  // Tag d'accord pour le mode DIFT
`endif

		if (core_axi_req)
		begin
			core_lsu_gnt = core_axi_gnt;  // Accord AXI
			lsu_resp_NS  = AXI;  // Prochaine réponse depuis AXI
		end
		else if (core_data_req)
		begin
			core_lsu_gnt = core_data_gnt;  // Accord RAM

`ifdef DIFT
			core_lsu_gnt_tag = core_data_gnt;  // Tag d'accord pour le mode DIFT
`endif

			lsu_resp_NS = RAM;  // Prochaine réponse depuis RAM
		end
	end

	// Route la réponse vers l'unité de chargement/stockage
	assign core_lsu_rdata  = (lsu_resp_CS == AXI) ? core_axi_rdata : core_data_rdata;  // Données lues
	assign core_lsu_rvalid = core_axi_rvalid | core_data_rvalid;  // Validité de la réponse

`ifdef DIFT
	assign core_lsu_rdata_tag  = (lsu_resp_CS == AXI) ? 4'h0 : core_data_rdata_tag;  // Tag de données lues pour le mode DIFT
	assign core_lsu_rvalid_tag = core_data_rvalid;  // Tag de validité de la réponse pour le mode DIFT
`endif

	//----------------------------------------------------------------------------//
	// RAM d'instructions
	//----------------------------------------------------------------------------//

	instr_ram_wrap
	#(
		.RAM_SIZE   ( INSTR_RAM_SIZE ),  // Taille de la RAM d'instructions
		.DATA_WIDTH ( AXI_DATA_WIDTH )  // Largeur de données
	)
	instr_mem
	(
		.clk         ( clk             ),  // Horloge
		.rst_n       ( rst_n           ),  // Reset
		.en_i        ( instr_mem_en    ),  // Activation de la mémoire d'instructions
		.addr_i      ( instr_mem_addr  ),  // Adresse de la mémoire d'instructions
		.wdata_i     ( instr_mem_wdata ),  // Données à écrire
		.rdata_o     ( instr_mem_rdata ),  // Données lues
		.we_i        ( instr_mem_we    ),  // Écriture activée
		.be_i        ( instr_mem_be    ),  // Byte enable
		.bypass_en_i ( testmode_i      )  // Mode bypass
	);

	axi_mem_if_SP_wrap
	#(
		.AXI_ADDR_WIDTH  ( AXI_ADDR_WIDTH         ),
		.AXI_DATA_WIDTH  ( AXI_DATA_WIDTH         ),
		.AXI_ID_WIDTH    ( AXI_ID_SLAVE_WIDTH     ),
		.AXI_USER_WIDTH  ( AXI_USER_WIDTH         ),
		.MEM_ADDR_WIDTH  ( INSTR_ADDR_WIDTH       )
	)
	instr_mem_axi_if
	(
		.clk         ( clk               ),  // Horloge
		.rst_n       ( rst_n             ),  // Reset
		.test_en_i   ( testmode_i        ),  // Mode test
		.mem_req_o   ( axi_instr_req     ),  // Requête de mémoire AXI
		.mem_addr_o  ( axi_instr_addr    ),  // Adresse de la mémoire AXI
		.mem_we_o    ( axi_instr_we      ),  // Écriture activée pour la mémoire AXI
		.mem_be_o    ( axi_instr_be      ),  // Byte enable pour la mémoire AXI
		.mem_rdata_i ( axi_instr_rdata   ),  // Données lues depuis la mémoire AXI
		.mem_wdata_o ( axi_instr_wdata   ),  // Données à écrire dans la mémoire AXI
		.slave       ( instr_slave       )  // Esclave AXI
	);

	ram_mux
	#(
		.ADDR_WIDTH ( INSTR_ADDR_WIDTH ),
		.IN0_WIDTH  ( AXI_DATA_WIDTH   ),
		.IN1_WIDTH  ( 32               ),
		.OUT_WIDTH  ( AXI_DATA_WIDTH   )
	)
	instr_ram_mux_i
	(
		.clk            ( clk               ),  // Horloge
		.rst_n          ( rst_n             ),  // Reset
		.port0_req_i    ( axi_instr_req     ),  // Requête de mémoire AXI
		.port0_gnt_o    (                   ),  // Accord de mémoire AXI
		.port0_rvalid_o (                   ),  // Validité de la réponse de mémoire AXI
		.port0_addr_i   ( {axi_instr_addr[INSTR_ADDR_WIDTH-AXI_B_WIDTH-1:0], {AXI_B_WIDTH{1'b0}}} ),  // Adresse de la mémoire AXI
		.port0_we_i     ( axi_instr_we      ),  // Écriture activée pour la mémoire AXI
		.port0_be_i     ( axi_instr_be      ),  // Byte enable pour la mémoire AXI
		.port0_rdata_o  ( axi_instr_rdata   ),  // Données lues depuis la mémoire AXI
		.port0_wdata_i  ( axi_instr_wdata   ),  // Données à écrire dans la mémoire AXI
		.port1_req_i    ( core_instr_req    ),  // Requête de mémoire depuis le coeur
		.port1_gnt_o    ( core_instr_gnt    ),  // Accord de mémoire vers le coeur
		.port1_rvalid_o ( core_instr_rvalid ),  // Validité de la réponse de mémoire vers le coeur
		.port1_addr_i   ( core_instr_addr[INSTR_ADDR_WIDTH-1:0] ),  // Adresse de la mémoire depuis le coeur
		.port1_we_i     ( 1'b0              ),  // Écriture désactivée pour le coeur
		.port1_be_i     ( '1                ),  // Byte enable pour le coeur
		.port1_rdata_o  ( core_instr_rdata  ),  // Données lues pour le coeur
		.port1_wdata_i  ( '0                ),  // Données à écrire pour le coeur
		.ram_en_o       ( instr_mem_en      ),  // Activation de la mémoire d'instructions
		.ram_addr_o     ( instr_mem_addr    ),  // Adresse de la mémoire d'instructions
		.ram_we_o       ( instr_mem_we      ),  // Écriture activée pour la mémoire d'instructions
		.ram_be_o       ( instr_mem_be      ),  // Byte enable pour la mémoire d'instructions
		.ram_rdata_i    ( instr_mem_rdata   ),  // Données lues depuis la mémoire d'instructions
		.ram_wdata_o    ( instr_mem_wdata   )  // Données à écrire dans la mémoire d'instructions
	);

	//----------------------------------------------------------------------------//
	// RAM de données
	//----------------------------------------------------------------------------//
	sp_ram_wrap
	#(
		.RAM_SIZE   ( DATA_RAM_SIZE  ),  // Taille de la RAM de données
		.DATA_WIDTH ( AXI_DATA_WIDTH )  // Largeur de données
	)
	data_mem
	(
		.clk         ( clk                 ),  // Horloge
		.rstn_i      ( rst_n               ),  // Reset
		.en_i        ( data_mem_en         ),  // Activation de la mémoire de données
		.addr_i      ( data_mem_addr       ),  // Adresse de la mémoire de données
		.wdata_i     ( data_mem_wdata      ),  // Données à écrire
		.rdata_o     ( data_mem_rdata      ),  // Données lues
		.we_i        ( data_mem_we         ),  // Écriture activée
		.be_i        ( data_mem_be         ),  // Byte enable

`ifdef DIFT
		.we_i_tag    ( core_data_we_tag    ),  // Tag d'écriture pour le mode DIFT
		.wdata_i_tag ( core_data_wdata_tag ),  // Tag de données à écrire pour le mode DIFT
		.rdata_o_tag ( core_data_rdata_tag ),  // Tag de données lues pour le mode DIFT
`endif

		.bypass_en_i ( testmode_i          )  // Mode bypass
	);

	axi_mem_if_SP_wrap
	#(
		.AXI_ADDR_WIDTH  ( AXI_ADDR_WIDTH     ),
		.AXI_DATA_WIDTH  ( AXI_DATA_WIDTH     ),
		.AXI_ID_WIDTH    ( AXI_ID_SLAVE_WIDTH ),
		.AXI_USER_WIDTH  ( AXI_USER_WIDTH     ),
		.MEM_ADDR_WIDTH  ( DATA_ADDR_WIDTH    )
	)
	data_mem_axi_if
	(
		.clk         ( clk               ),  // Horloge
		.rst_n       ( rst_n             ),  // Reset
		.test_en_i   ( testmode_i        ),  // Mode test
		.mem_req_o   ( axi_mem_req       ),  // Requête de mémoire AXI
		.mem_addr_o  ( axi_mem_addr      ),  // Adresse de la mémoire AXI
		.mem_we_o    ( axi_mem_we        ),  // Écriture activée pour la mémoire AXI
		.mem_be_o    ( axi_mem_be        ),  // Byte enable pour la mémoire AXI
		.mem_rdata_i ( axi_mem_rdata     ),  // Données lues depuis la mémoire AXI
		.mem_wdata_o ( axi_mem_wdata     ),  // Données à écrire dans la mémoire AXI
		.slave       ( data_slave        )  // Esclave AXI
	);

	ram_mux
	#(
		.ADDR_WIDTH ( DATA_ADDR_WIDTH ),
		.IN0_WIDTH  ( AXI_DATA_WIDTH  ),
		.IN1_WIDTH  ( 32              ),
		.OUT_WIDTH  ( AXI_DATA_WIDTH  )
	)
	data_ram_mux_i
	(
		.clk            ( clk              ),  // Horloge
		.rst_n          ( rst_n            ),  // Reset
		.port0_req_i    ( axi_mem_req      ),  // Requête de mémoire AXI
		.port0_gnt_o    (                  ),  // Accord de mémoire AXI
		.port0_rvalid_o (                  ),  // Validité de la réponse de mémoire AXI
		.port0_addr_i   ( {axi_mem_addr[DATA_ADDR_WIDTH-AXI_B_WIDTH-1:0], {AXI_B_WIDTH{1'b0}}} ),  // Adresse de la mémoire AXI
		.port0_we_i     ( axi_mem_we       ),  // Écriture activée pour la mémoire AXI
		.port0_be_i     ( axi_mem_be       ),  // Byte enable pour la mémoire AXI
		.port0_rdata_o  ( axi_mem_rdata    ),  // Données lues depuis la mémoire AXI
		.port0_wdata_i  ( axi_mem_wdata    ),  // Données à écrire dans la mémoire AXI
		.port1_req_i    ( core_data_req    ),  // Requête de mémoire depuis le coeur
		.port1_gnt_o    ( core_data_gnt    ),  // Accord de mémoire vers le coeur
		.port1_rvalid_o ( core_data_rvalid ),  // Validité de la réponse de mémoire vers le coeur
		.port1_addr_i   ( core_data_addr[DATA_ADDR_WIDTH-1:0] ),  // Adresse de la mémoire depuis le coeur
		.port1_we_i     ( core_data_we     ),  // Écriture activée pour le coeur
		.port1_be_i     ( core_data_be     ),  // Byte enable pour le coeur
		.port1_rdata_o  ( core_data_rdata  ),  // Données lues pour le coeur
		.port1_wdata_i  ( core_data_wdata  ),  // Données à écrire pour le coeur
		.ram_en_o       ( data_mem_en      ),  // Activation de la mémoire de données
		.ram_addr_o     ( data_mem_addr    ),  // Adresse de la mémoire de données
		.ram_we_o       ( data_mem_we      ),  // Écriture activée pour la mémoire de données
		.ram_be_o       ( data_mem_be      ),  // Byte enable pour la mémoire de données
		.ram_rdata_i    ( data_mem_rdata   ),  // Données lues depuis la mémoire de données
		.ram_wdata_o    ( data_mem_wdata   )  // Données à écrire dans la mémoire de données
	);

	//----------------------------------------------------------------------------//
	// Unité de debug avancée
	//----------------------------------------------------------------------------//

	// TODO: retirer les connexions de debug vers le coeur
	adv_dbg_if
	#(
		.NB_CORES           ( 1                   ),  // Nombre de coeurs
		.AXI_ADDR_WIDTH     ( AXI_ADDR_WIDTH      ),
		.AXI_DATA_WIDTH     ( AXI_DATA_WIDTH      ),
		.AXI_USER_WIDTH     ( AXI_USER_WIDTH      ),
		.AXI_ID_WIDTH       ( AXI_ID_MASTER_WIDTH )
	)
	adv_dbg_if_i
	(
		.tms_pad_i   ( tms_i           ),  // Signal TMS JTAG
		.tck_pad_i   ( tck_i           ),  // Horloge JTAG
		.trstn_pad_i ( trstn_i         ),  // Reset JTAG
		.tdi_pad_i   ( tdi_i           ),  // Entrée de données JTAG
		.tdo_pad_o   ( tdo_o           ),  // Sortie de données JTAG
		.test_mode_i ( testmode_i      ),  // Mode test
		.cpu_addr_o  (                 ),  // Adresse CPU
		.cpu_data_i  ( '0              ),  // Données CPU
		.cpu_data_o  (                 ),  // Données CPU
		.cpu_bp_i    ( '0              ),  // Point d'arrêt CPU
		.cpu_stall_o (                 ),  // Stall CPU
		.cpu_stb_o   (                 ),  // Strobe CPU
		.cpu_we_o    (                 ),  // Écriture activée pour le CPU
		.cpu_ack_i   ( '1              ),  // Accusé de réception CPU
		.cpu_rst_o   (                 ),  // Reset CPU
		.axi_aclk             ( clk                  ),  // Horloge AXI
		.axi_aresetn          ( rst_n                ),  // Reset AXI
		.axi_master_aw_valid  ( dbg_master.aw_valid  ),  // Validité de l'adresse AXI
		.axi_master_aw_addr   ( dbg_master.aw_addr   ),  // Adresse AXI
		.axi_master_aw_prot   ( dbg_master.aw_prot   ),  // Protection AXI
		.axi_master_aw_region ( dbg_master.aw_region ),  // Région AXI
		.axi_master_aw_len    ( dbg_master.aw_len    ),  // Longueur AXI
		.axi_master_aw_size   ( dbg_master.aw_size   ),  // Taille AXI
		.axi_master_aw_burst  ( dbg_master.aw_burst  ),  // Burst AXI
		.axi_master_aw_lock   ( dbg_master.aw_lock   ),  // Lock AXI
		.axi_master_aw_cache  ( dbg_master.aw_cache  ),  // Cache AXI
		.axi_master_aw_qos    ( dbg_master.aw_qos    ),  // Qualité de service AXI
		.axi_master_aw_id     ( dbg_master.aw_id     ),  // ID AXI
		.axi_master_aw_user   ( dbg_master.aw_user   ),  // Utilisateur AXI
		.axi_master_aw_ready  ( dbg_master.aw_ready  ),  // Prêt AXI
		.axi_master_ar_valid  ( dbg_master.ar_valid  ),  // Validité de lecture AXI
		.axi_master_ar_addr   ( dbg_master.ar_addr   ),  // Adresse de lecture AXI
		.axi_master_ar_prot   ( dbg_master.ar_prot   ),  // Protection de lecture AXI
		.axi_master_ar_region ( dbg_master.ar_region ),  // Région de lecture AXI
		.axi_master_ar_len    ( dbg_master.ar_len    ),  // Longueur de lecture AXI
		.axi_master_ar_size   ( dbg_master.ar_size   ),  // Taille de lecture AXI
		.axi_master_ar_burst  ( dbg_master.ar_burst  ),  // Burst de lecture AXI
		.axi_master_ar_lock   ( dbg_master.ar_lock   ),  // Lock de lecture AXI
		.axi_master_ar_cache  ( dbg_master.ar_cache  ),  // Cache de lecture AXI
		.axi_master_ar_qos    ( dbg_master.ar_qos    ),  // Qualité de service de lecture AXI
		.axi_master_ar_id     ( dbg_master.ar_id     ),  // ID de lecture AXI
		.axi_master_ar_user   ( dbg_master.ar_user   ),  // Utilisateur de lecture AXI
		.axi_master_ar_ready  ( dbg_master.ar_ready  ),  // Prêt de lecture AXI
		.axi_master_w_valid   ( dbg_master.w_valid   ),  // Validité d'écriture AXI
		.axi_master_w_data    ( dbg_master.w_data    ),  // Données d'écriture AXI
		.axi_master_w_strb    ( dbg_master.w_strb    ),  // Strobe d'écriture AXI
		.axi_master_w_user    ( dbg_master.w_user    ),  // Utilisateur d'écriture AXI
		.axi_master_w_last    ( dbg_master.w_last    ),  // Dernière écriture AXI
		.axi_master_w_ready   ( dbg_master.w_ready   ),  // Prêt d'écriture AXI
		.axi_master_r_valid   ( dbg_master.r_valid   ),  // Validité de lecture AXI
		.axi_master_r_data    ( dbg_master.r_data    ),  // Données de lecture AXI
		.axi_master_r_resp    ( dbg_master.r_resp    ),  // Réponse de lecture AXI
		.axi_master_r_last    ( dbg_master.r_last    ),  // Dernière lecture AXI
		.axi_master_r_id      ( dbg_master.r_id      ),  // ID de lecture AXI
		.axi_master_r_user    ( dbg_master.r_user    ),  // Utilisateur de lecture AXI
		.axi_master_r_ready   ( dbg_master.r_ready   ),  // Prêt de lecture AXI
		.axi_master_b_valid   ( dbg_master.b_valid   ),  // Validité de réponse AXI
		.axi_master_b_resp    ( dbg_master.b_resp    ),  // Réponse AXI
		.axi_master_b_id      ( dbg_master.b_id      ),  // ID de réponse AXI
		.axi_master_b_user    ( dbg_master.b_user    ),  // Utilisateur de réponse AXI
		.axi_master_b_ready   ( dbg_master.b_ready   )  // Prêt de réponse AXI
	);

	//----------------------------------------------------------------------------//
	// Code de test
	//----------------------------------------------------------------------------//

	// Introduire des stalls aléatoires pour l'accès aux données afin de stresser l'unité de chargement/stockage
`ifdef DATA_STALL_RANDOM
	random_stalls data_stalls_i
	(
		.clk           ( clk                     ),  // Horloge
		.core_req_i    ( RISCV_CORE.data_req_o   ),  // Requête de données depuis le coeur
		.core_addr_i   ( RISCV_CORE.data_addr_o  ),  // Adresse des données
		.core_we_i     ( RISCV_CORE.data_we_o    ),  // Écriture activée
		.core_be_i     ( RISCV_CORE.data_be_o    ),  // Byte enable
		.core_wdata_i  ( RISCV_CORE.data_wdata_o ),  // Données à écrire
		.core_gnt_o    (                         ),  // Accord de données
		.core_rdata_o  (                         ),  // Données lues
		.core_rvalid_o (                         ),  // Validité de la réponse de données
		.data_req_o    (                         ),  // Requête de données pour la RAM
		.data_addr_o   (                         ),  // Adresse des données pour la RAM
		.data_we_o     (                         ),  // Écriture activée pour la RAM
		.data_be_o     (                         ),  // Byte enable pour la RAM
		.data_wdata_o  (                         ),  // Données à écrire pour la RAM
		.data_gnt_i    ( core_lsu_gnt            ),  // Accord de données depuis la RAM
		.data_rdata_i  ( core_lsu_rdata          ),  // Données lues depuis la RAM
		.data_rvalid_i ( core_lsu_rvalid         )  // Validité de la réponse de données depuis la RAM
	);

	initial begin
		force RISCV_CORE.data_gnt_i    = data_stalls_i.core_gnt_o;  // Forcer l'accord de données
		force RISCV_CORE.data_rvalid_i = data_stalls_i.core_rvalid_o;  // Forcer la validité de la réponse de données
		force RISCV_CORE.data_rdata_i  = data_stalls_i.core_rdata_o;  // Forcer les données lues
		force core_lsu_req   = data_stalls_i.data_req_o;  // Forcer la requête de données
		force core_lsu_addr  = data_stalls_i.data_addr_o;  // Forcer l'adresse des données
		force core_lsu_we    = data_stalls_i.data_we_o;  // Forcer l'écriture activée
		force core_lsu_be    = data_stalls_i.data_be_o;  // Forcer le byte enable
		force core_lsu_wdata = data_stalls_i.data_wdata_o;  // Forcer les données à écrire
	end
`endif

	// Introduire des stalls aléatoires pour l'accès aux instructions afin de stresser le fetcher d'instructions
`ifdef INSTR_STALL_RANDOM
	random_stalls instr_stalls_i
	(
		.clk           ( clk                     ),  // Horloge
		.core_req_i    ( RISCV_CORE.instr_req_o  ),  // Requête d'instructions depuis le coeur
		.core_addr_i   ( RISCV_CORE.instr_addr_o ),  // Adresse des instructions
		.core_we_i     (                         ),  // Écriture désactivée
		.core_be_i     (                         ),  // Byte enable désactivé
		.core_wdata_i  (                         ),  // Données à écrire désactivées
		.core_gnt_o    (                         ),  // Accord d'instructions
		.core_rdata_o  (                         ),  // Données d'instructions lues
		.core_rvalid_o (                         ),  // Validité de la réponse d'instructions
		.data_req_o    (                         ),  // Requête d'instructions pour la RAM
		.data_addr_o   (                         ),  // Adresse des instructions pour la RAM
		.data_we_o     (                         ),  // Écriture désactivée pour la RAM
		.data_be_o     (                         ),  // Byte enable désactivé pour la RAM
		.data_wdata_o  (                         ),  // Données à écrire désactivées pour la RAM
		.data_gnt_i    ( core_instr_gnt          ),  // Accord d'instructions depuis la RAM
		.data_rdata_i  ( core_instr_rdata        ),  // Données d'instructions lues depuis la RAM
		.data_rvalid_i ( core_instr_rvalid       )  // Validité de la réponse d'instructions depuis la RAM
	);

	initial begin
		force RISCV_CORE.instr_gnt_i    = instr_stalls_i.core_gnt_o;  // Forcer l'accord d'instructions
		force RISCV_CORE.instr_rvalid_i = instr_stalls_i.core_rvalid_o;  // Forcer la validité de la réponse d'instructions
		force RISCV_CORE.instr_rdata_i  = instr_stalls_i.core_rdata_o;  // Forcer les données d'instructions lues
		force core_instr_req   = instr_stalls_i.data_req_o;  // Forcer la requête d'instructions
		force core_instr_addr  = instr_stalls_i.data_addr_o;  // Forcer l'adresse des instructions
	end
`endif

endmodule

