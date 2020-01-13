library IEEE;
use IEEE.STD_LOGIC_1164.all;

use work.StdRtlPkg.all;
use work.AxiStreamPkg.all;


entity stitchTowersWrapper is
  generic (
    G_STITCH_DIR : sl := '1'            -- '1' -> eta stitch, '0' -> phi stitch
    );
  port (
    -- Algo Control/Status Signals
    algoClk   : in  sl;
    algoRst   : in  sl;
    algoStart : in  sl;
    algoDone  : out sl := '0';
    algoIdle  : out sl := '0';
    algoReady : out sl := '0';

    -- AXI-Stream In/Out Ports
    axiStreamIn  : in  AxiStreamMasterArray(0 to 1);
    axiStreamOut : out AxiStreamMasterArray(0 to 1)
    );
end stitchTowersWrapper;

architecture rtl of stitchTowersWrapper is

  signal algoRstN : sl;

  signal algoRstND1  : sl;
  signal algoStartD1 : sl;

begin

  algoRstN <= not algoRst;

  algoRstND1  <= algoRstN  when rising_edge(algoClk);
  algoStartD1 <= algoStart when rising_edge(algoClk);

  U_stitchTowers : entity work.stitchTowers
    port map(
      ap_clk   => algoClk,
      ap_rst_n => algoRstND1,
      ap_start => algoStartD1,
      ap_done  => algoDone,
      ap_idle  => algoIdle,
      ap_ready => algoReady,

      stitchDir => G_STITCH_DIR,

      link_in_0_TDATA    => axiStreamIn(0).tData(31 downto 0),
      link_in_0_TVALID   => axiStreamIn(0).tValid,
      link_in_0_TREADY   => open,
      link_in_0_TUSER    => axiStreamIn(0).tUser(7 downto 0),
      link_in_0_TLAST(0) => axiStreamIn(0).tLast,

      link_in_1_TDATA    => axiStreamIn(1).tData(31 downto 0),
      link_in_1_TVALID   => axiStreamIn(1).tValid,
      link_in_1_TREADY   => open,
      link_in_1_TUSER    => axiStreamIn(1).tUser(7 downto 0),
      link_in_1_TLAST(0) => axiStreamIn(1).tLast,

      link_out_0_TREADY   => '1',
      link_out_0_TDATA    => axiStreamOut(0).tData(31 downto 0),
      link_out_0_TVALID   => axiStreamOut(0).tValid,
      link_out_0_TUSER    => axiStreamOut(0).tUser(7 downto 0),
      link_out_0_TLAST(0) => axiStreamOut(0).tLast,

      link_out_1_TREADY   => '1',
      link_out_1_TDATA    => axiStreamOut(1).tData(31 downto 0),
      link_out_1_TVALID   => axiStreamOut(1).tValid,
      link_out_1_TUSER    => axiStreamOut(1).tUser(7 downto 0),
      link_out_1_TLAST(0) => axiStreamOut(1).tLast

      );

end rtl;
