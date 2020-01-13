## Set the top level module
set_top ecal::processEcalLink

#### Add source code
add_files ../rct_common/src/rct_common.h
add_files ../rct_common/src/adders.h
add_files src/ecal.h
add_files src/ecal.cpp

### Add testbed files
add_files -tb src/ecal_tb.cpp
add_files -tb ../../../common/APxLinkData.hh
add_files -tb ../../../common/APxLinkData.cpp

### Add test input files
add_files -tb data/test_in.txt
