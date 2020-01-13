#ifndef __STITCH_TOWERS_H__
#define __STITCH_TOWERS_H__

#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "../../rct_common/src/rct_common.h"
#include "../../ecal/src/ecal.h"

namespace stchTwr {

void packOutput(hls::stream<ecal::ecalOutWord> &link, ecal::ProcessedTower cluster);
ecal::ProcessedTower unpackInput(hls::stream<ecal::ecalOutWord> &link);

void stitchTowers(
	bool stitchDir,
	hls::stream<ecal::ecalOutWord> link_in[2],
	hls::stream<ecal::ecalOutWord> link_out[2]);

} //namespace stchTwr

#endif /* !__STITCH_TOWERS_H__ */
