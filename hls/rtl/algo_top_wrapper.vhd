library IEEE;
use IEEE.STD_LOGIC_1164.all;

use work.algo_pkg.all;

entity algo_top_wrapper is
  generic (
    N_INPUT_LINKS  : integer := 24;
    N_OUTPUT_LINKS : integer := 24
    );
  port (
    -- Algo Control/Status Signals
    ap_clk   : in  std_logic;
    ap_rst   : in  std_logic;
    ap_start : in  std_logic;
    ap_done  : out std_logic;
    ap_idle  : out std_logic;
    ap_ready : out std_logic;

    -- AXI-Stream Input Links 
    link_in_master : in  LinkMasterArrType(0 to N_INPUT_LINKS-1) := (others => LINK_MASTER_NULL_C);
    link_in_slave  : out LinkSlaveArrType(0 to N_INPUT_LINKS-1)  := (others => LINK_SLAVE_NULL_C);

    -- AXI-Stream Output Links 
    link_out_master : out LinkMasterArrType(0 to N_OUTPUT_LINKS-1) := (others => LINK_MASTER_NULL_C);
    link_out_slave  : in  LinkSlaveArrType(0 to N_OUTPUT_LINKS-1)  := (others => LINK_SLAVE_NULL_C)
    );
end algo_top_wrapper;

architecture rtl of algo_top_wrapper is
begin

end rtl;
