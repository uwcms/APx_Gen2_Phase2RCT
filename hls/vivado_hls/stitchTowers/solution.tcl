set_part {xcvu9p-flgc2104-1-e}

config_interface -register_io off -trim_dangling_port=0
config_export -format ip_catalog -rtl vhdl -use_netlist none -vivado_optimization_level 2 -vivado_phys_opt all -vivado_report_level 2
config_schedule -effort high -enable_dsp_full_reg -relax_ii_for_timing -verbose

create_clock -period 240MHz -name default
set_clock_uncertainty 20% 

