# Set the top level module
set_top stchTwr::stitchTowers 

# Add source code
add_files ../rct_common/src/rct_common.h
add_files ../ecal/src/ecal.h
add_files src/stitchTowers.cpp
add_files src/stitchTowers.h

# Add testbed files
add_files -tb src/stitchTowers_tb.cpp
add_files -tb ../ecal/src/ecal.cpp
add_files -tb ../../../common/APxLinkData.hh
add_files -tb ../../../common/APxLinkData.cpp

# Add test input files
add_files -tb data/test_in.txt
