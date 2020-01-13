library IEEE;
use IEEE.STD_LOGIC_1164.all;

use work.StdRtlPkg.all;
use work.AxiStreamPkg.all;

entity ecalWrapper is
  port (
    -- Algo Control/Status Signals
    algoClk   : in  sl;
    algoRst   : in  sl;
    algoStart : in  sl;
    algoDone  : out sl := '0';
    algoIdle  : out sl := '0';
    algoReady : out sl := '0';

    -- AXI-Stream In/Out Ports
    axiStreamIn  : in  AxiStreamMasterType;
    axiStreamOut : out AxiStreamMasterType
    );
end ecalWrapper;

architecture rtl of ecalWrapper is

  signal algoRstN : sl;

  signal algoRstND1  : sl;
  signal algoStartD1 : sl;

begin

  algoRstN <= not algoRst;

  algoRstND1  <= algoRstN  when rising_edge(algoClk);
  algoStartD1 <= algoStart when rising_edge(algoClk);

  U_processEcalLink : entity work.processEcalLink
    port map(
      ap_clk   => algoClk,
      ap_rst_n => algoRstND1,
      ap_start => algoStartD1,
      ap_done  => algoDone,
      ap_idle  => algoIdle,
      ap_ready => algoReady,

      link_in_TDATA    => axiStreamIn.tData(63 downto 0),
      link_in_TVALID   => axiStreamIn.tValid,
      link_in_TREADY   => open,
      link_in_TUSER    => axiStreamIn.tUser(7 downto 0),
      link_in_TLAST(0) => axiStreamIn.tLast,

      link_out_TREADY   => '1',
      link_out_TDATA    => axiStreamOut.tData(31 downto 0),
      link_out_TVALID   => axiStreamOut.tValid,
      link_out_TUSER    => axiStreamOut.tUser(7 downto 0),
      link_out_TLAST(0) => axiStreamOut.tLast

      );

end rtl;
