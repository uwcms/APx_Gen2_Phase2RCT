# Set the top level module

# Add source code

# Add testbed files
add_files -tb ../rct_common/src/rct_common.h
add_files -tb ../ecal/src/ecal.h
add_files -tb ../ecal/src/ecal.cpp
add_files -tb ../stitchTowers/src/stitchTowers.cpp
add_files -tb ../stitchTowers/src/stitchTowers.h
add_files -tb ../../common/APxLinkData.hh
add_files -tb ../../common/APxLinkData.cpp

add_files -tb src/rct_top_tb.cpp

# Add test input files
add_files -tb data/test_in.txt
