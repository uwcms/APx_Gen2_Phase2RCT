# Load RUCKUS library
source -quiet $::env(RUCKUS_DIR)/vivado_proc.tcl

# # Load Source Code
loadSource -dir "$::DIR_PATH/rtl/"

loadSource -dir "$::DIR_PATH/hls/ecal/proj/solution1/impl/ip/hdl/vhdl/"
loadSource -dir "$::DIR_PATH/hls/stitchTowers/proj/solution1/impl/ip/hdl/vhdl/"