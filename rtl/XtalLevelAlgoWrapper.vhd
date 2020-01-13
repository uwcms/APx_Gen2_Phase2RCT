

library IEEE;
use IEEE.STD_LOGIC_1164.all;
use ieee.numeric_std.all;

use work.StdRtlPkg.all;
use work.AxiStreamPkg.all;

entity XtalLevelAlgoWrapper is
  generic (
    N_INPUT_STREAMS  : integer := 34;
    N_OUTPUT_STREAMS : integer := 34
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
    axiStreamIn  : in  AxiStreamMasterArray(0 to N_INPUT_STREAMS-1);
    axiStreamOut : out AxiStreamMasterArray(0 to N_OUTPUT_STREAMS-1)
    );
end XtalLevelAlgoWrapper;

architecture Behavioral of XtalLevelAlgoWrapper is


  signal axiStreamOutInt   : AxiStreamMasterArray(0 to N_OUTPUT_STREAMS-1);
  signal axiStreamOutEta   : AxiStreamMasterArray(0 to N_OUTPUT_STREAMS-1);
  signal axiStreamOutPhiS1 : AxiStreamMasterArray(0 to N_OUTPUT_STREAMS-1);
  signal axiStreamOutPhiS2 : AxiStreamMasterArray(0 to N_OUTPUT_STREAMS-1);

  signal algoRstD1   : sl;
  signal algoStartD1 : sl;

  attribute max_fanout                           : integer;
  attribute max_fanout of algoRstD1, algoStartD1 : signal is 1;

begin

  algoRstD1   <= algoRst   when rising_edge(algoClk);
  algoStartD1 <= algoStart when rising_edge(algoClk);

  gen_Ecal : for i in 0 to N_INPUT_STREAMS-1 generate

    U_ecalWrapper : entity work.ecalWrapper
      port map(
        algoClk   => algoClk,
        algoRst   => algoRstD1,
        algoStart => algoStartD1,
        algoDone  => open,
        algoIdle  => open,
        algoReady => open,

        -- AXI-Stream In/Out Ports
        axiStreamIn  => axiStreamIn(i),
        axiStreamOut => axiStreamOutInt(i)
        );

  end generate;

  gen_stitch_eta : for i in 0 to N_INPUT_STREAMS/2-1 generate
    stitchTowersWrapperEta : entity work.stitchTowersWrapper
      generic map(
        G_STITCH_DIR => '1'             -- stitch in eta
        )
      port map(
        algoClk   => algoClk,
        algoRst   => algoRstD1,
        algoStart => algoStartD1,
        algoDone  => open,
        algoIdle  => open,
        algoReady => open,

        -- AXI-Stream In/Out Ports
        axiStreamIn(0) => axiStreamOutInt(i*2),
        axiStreamIn(1) => axiStreamOutInt(i*2+1),

        axiStreamOut(0) => axiStreamOutEta(i*2),
        axiStreamOut(1) => axiStreamOutEta(i*2+1)
        );

  end generate;

  gen_stitch_phiS1 : for i in 0 to N_INPUT_STREAMS/4-1 generate
    stitchTowersWrapperPhi1 : entity work.stitchTowersWrapper
      generic map(
        G_STITCH_DIR => '0'             -- stitch in phi
        )
      port map(
        algoClk   => algoClk,
        algoRst   => algoRstD1,
        algoStart => algoStartD1,
        algoDone  => open,
        algoIdle  => open,
        algoReady => open,

        -- AXI-Stream In/Out Ports
        axiStreamIn(0) => axiStreamOutEta(i*4),
        axiStreamIn(1) => axiStreamOutEta(i*4+2),

        axiStreamOut(0) => axiStreamOutPhiS1(i*4),
        axiStreamOut(1) => axiStreamOutPhiS1(i*4+2)
        );

    stitchTowersWrapperPhi2 : entity work.stitchTowersWrapper
      generic map(
        G_STITCH_DIR => '0'             -- stitch in phi
        )
      port map(
        algoClk   => algoClk,
        algoRst   => algoRstD1,
        algoStart => algoStartD1,
        algoDone  => open,
        algoIdle  => open,
        algoReady => open,

        -- AXI-Stream In/Out Ports
        axiStreamIn(0) => axiStreamOutEta(i*4+1),
        axiStreamIn(1) => axiStreamOutEta(i*4+3),

        axiStreamOut(0) => axiStreamOutPhiS1(i*4+1),
        axiStreamOut(1) => axiStreamOutPhiS1(i*4+3)
        );
  end generate;

  U_AxiStreamPipeline_Eta_32 : entity work.AxiStreamPipeline
    generic map(
      PIPE_STAGES_G => 1
      )
    port map(
      -- Clock and Reset
      axisClk     => algoClk,
      axisRst     => '0',
      -- Slave Port
      sAxisMaster => axiStreamOutEta(32),
      sAxisSlave  => open,
      -- Master Port
      mAxisMaster => axiStreamOutPhiS1(32),
      mAxisSlave  => AXI_STREAM_SLAVE_FORCE_C);

  U_AxiStreamPipeline_Eta_33 : entity work.AxiStreamPipeline
    generic map(
      PIPE_STAGES_G => 1
      )
    port map(
      -- Clock and Reset
      axisClk     => algoClk,
      axisRst     => '0',
      -- Slave Port
      sAxisMaster => axiStreamOutEta(33),
      sAxisSlave  => open,
      -- Master Port
      mAxisMaster => axiStreamOutPhiS1(33),
      mAxisSlave  => AXI_STREAM_SLAVE_FORCE_C);




  gen_stitch_phiS2 : for i in 0 to N_INPUT_STREAMS/4-1 generate
    stitchTowersWrapperPhi1 : entity work.stitchTowersWrapper
      generic map(
        G_STITCH_DIR => '0'             -- stitch in phi
        )
      port map(
        algoClk   => algoClk,
        algoRst   => algoRstD1,
        algoStart => algoStartD1,
        algoDone  => open,
        algoIdle  => open,
        algoReady => open,

        -- AXI-Stream In/Out Ports
        axiStreamIn(0) => axiStreamOutPhiS1(i*4+2),
        axiStreamIn(1) => axiStreamOutPhiS1(i*4+4),

        axiStreamOut(0) => axiStreamOutPhiS2(i*4+2),
        axiStreamOut(1) => axiStreamOutPhiS2(i*4+4)
        );

    stitchTowersWrapperPhi2 : entity work.stitchTowersWrapper
      generic map(
        G_STITCH_DIR => '0'             -- stitch in phi
        )
      port map(
        algoClk   => algoClk,
        algoRst   => algoRstD1,
        algoStart => algoStartD1,
        algoDone  => open,
        algoIdle  => open,
        algoReady => open,

        -- AXI-Stream In/Out Ports
        axiStreamIn(0) => axiStreamOutPhiS1(i*4+3),
        axiStreamIn(1) => axiStreamOutPhiS1(i*4+5),

        axiStreamOut(0) => axiStreamOutPhiS2(i*4+3),
        axiStreamOut(1) => axiStreamOutPhiS2(i*4+5)
        );
  end generate;

  U_AxiStreamPipeline_PhiS2_0 : entity work.AxiStreamPipeline
    generic map(
      PIPE_STAGES_G => 1
      )
    port map(
      -- Clock and Reset
      axisClk     => algoClk,
      axisRst     => '0',
      -- Slave Port
      sAxisMaster => axiStreamOutPhiS1(0),
      sAxisSlave  => open,
      -- Master Port
      mAxisMaster => axiStreamOutPhiS2(0),
      mAxisSlave  => AXI_STREAM_SLAVE_FORCE_C);

  U_AxiStreamPipeline_PhiS2_1 : entity work.AxiStreamPipeline
    generic map(
      PIPE_STAGES_G => 1
      )
    port map(
      -- Clock and Reset
      axisClk     => algoClk,
      axisRst     => '0',
      -- Slave Port
      sAxisMaster => axiStreamOutPhiS1(1),
      sAxisSlave  => open,
      -- Master Port
      mAxisMaster => axiStreamOutPhiS2(1),
      mAxisSlave  => AXI_STREAM_SLAVE_FORCE_C);

  axiStreamOut <= axiStreamOutPhiS2;

end Behavioral;
