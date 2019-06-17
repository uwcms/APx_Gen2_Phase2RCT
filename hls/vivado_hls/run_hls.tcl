# Using part of the build script for HLS4ML project authored by Vladimir Loncar (vloncar)
# https://github.com/hls-fpga-machine-learning/hls4ml

array set opt {
    csim   1
    synth  1
    cosim  1
    export 1
    tv test_in.txt
}

foreach arg $::argv {
    foreach o [lsort [array names opt]] {
         regexp "$o=+(\\w+)$" $arg unused opt($o)
    }
}

proc report_time { op_name time_start time_end  } {
    set time_taken [expr $time_end - $time_start]
    set time_s [expr ($time_taken / 1000) % 60]
    set time_m [expr ($time_taken / (1000*60)) % 60]
    set time_h [expr ($time_taken / (1000*60*60)) % 24]
    puts "***** ${op_name} COMPLETED IN ${time_h}h${time_m}m${time_s}s *****"
}

# open the project, don't forget to reset
open_project -reset proj

#Add sources and specify top function
source "./sources.tcl"

#reset the solution
open_solution -reset "solution1"

#Specify FPGA and clock constraints
source "./solution.tcl"

#Add Vivado HLS project directives
source "./directives.tcl"

if {$opt(csim)} {
   puts "***** C SIMULATION *****"
   set time_start [clock clicks -milliseconds]
   csim_design -argv "-r $opt(tv) -v"
   set time_end [clock clicks -milliseconds]
   report_time "C SIMULATION" $time_start $time_end
}

if {$opt(synth)} {
   puts "***** C/RTL SYNTHESIS *****"
   set time_start [clock clicks -milliseconds]
   csynth_design
   set time_end [clock clicks -milliseconds]
   report_time "C/RTL SYNTHESIS" $time_start $time_end
   if {$opt(cosim)} {
       puts "***** C/RTL SIMULATION *****"
       set time_start [clock clicks -milliseconds]
       cosim_design -argv "-r $opt(tv) -v" -trace_level all
       set time_end [clock clicks -milliseconds]
       report_time "C/RTL SIMULATION" $time_start $time_end
   }
   if {$opt(export)} {
      puts "***** EXPORT IP *****"
      set time_start [clock clicks -milliseconds]
      export_design -format ip_catalog
      set time_end [clock clicks -milliseconds]
      report_time "EXPORT IP" $time_start $time_end

      puts "***** GENERATING WRAPPERS *****"
      puts [exec python ../wrapper_generator.py src/algo_top_parameters.h --wrapper ../rtl/algo_top_wrapper.vhd]
   }
}

exit
