//#include <cstdint>
#include <stdint.h>
#include <algorithm>
#include <utility>

#include "../../rct_common/src/rct_common.h"
#include "../../rct_common/src/adders.h"
#include "../../ecal/src/ecal.h"

#include "stitchTowers.h"

using namespace std;
using namespace stchTwr;

void stchTwr::packOutput(hls::stream<ecal::ecalOutWord> &link, ecal::ProcessedTower procTwr) {

#pragma HLS INLINE
	ecal::ecalOutWord r;
	r.user = 0;
	r.last = 1;

	r.data(31,0) = (uint32_t)procTwr;
	link.write(r);
}

ecal::ProcessedTower stchTwr::unpackInput(hls::stream<ecal::ecalOutWord> &link) {

#pragma HLS INTERFACE axis port=link
#pragma HLS INLINE
#pragma HLS PIPELINE

	ap_uint<64> data;

	data = link.read().data;

	ecal::ProcessedTower cluster((uint32_t)(data(31,0)));

	return cluster;
}


void stitch(bool stitchDir,
		ecal::ProcessedTower Ai, ecal::ProcessedTower Bi,
		ecal::ProcessedTower &Ao, ecal::ProcessedTower &Bo) {

#pragma HLS STABLE variable=stitchDir
#pragma HLS PIPELINE
#pragma HLS INLINE

	ap_uint<11> clustered_et_sum = Ai.clustered_et + Bi.clustered_et;

	ap_uint<10> clustered_et_pegged = (clustered_et_sum > 0x3FF) ? (ap_uint<10>)0x3FF : (ap_uint<10>)clustered_et_sum;

	// stitchDir == true -> eta stitch, stitchDir == false -> phi stitch
	bool etaStitch =  stitchDir && (Ai.peak_eta == 4 && Bi.peak_eta == 0) && (Ai.peak_phi == Bi.peak_phi);
	bool phiStitch = !stitchDir && (Ai.peak_phi == 4 && Bi.peak_phi == 0) && (Ai.peak_eta == Bi.peak_eta);

	if ( etaStitch || phiStitch )
	{
		if(Ai.clustered_et > Bi.clustered_et){
			Ao.clustered_et = clustered_et_pegged;
			Bo.clustered_et = 0;
		}
		else{
			Ao.clustered_et = 0;
			Bo.clustered_et = clustered_et_pegged;
		}
	}
	else{
		Ao.clustered_et = Ai.clustered_et;
		Bo.clustered_et = Bi.clustered_et;
	}

#ifndef __SYNTHESIS__
	cout << "etaStitch: "<< etaStitch << "   phiStitch: " << phiStitch << endl;
	cout << "Ai.clustered_et: " << Ai.clustered_et << "  Bi.clustered_et: " << Bi.clustered_et << endl;
	cout << "clustered_et_sum: " << clustered_et_sum << "  clustered_et_pegged: " << clustered_et_pegged << endl;
	cout << "Ao.clustered_et: " << Ao.clustered_et << "  Bo.clustered_et: " << Bo.clustered_et << endl<< endl<< endl;
#endif

	Ao.peak_eta = Ai.peak_eta;
	Ao.peak_phi = Ai.peak_phi;
	Ao.peak_time = Ai.peak_time;
	Ao.total_et = Ai.total_et;

	Bo.peak_eta = Bi.peak_eta;
	Bo.peak_phi = Bi.peak_phi;
	Bo.peak_time = Bi.peak_time;
	Bo.total_et = Bi.total_et;
}

void stitchTowers( bool stitchDir,
		hls::stream<ecal::ecalOutWord> link_in[2],
		hls::stream<ecal::ecalOutWord> link_out[2]){

#pragma HLS STABLE variable=stitchDir

#pragma HLS INTERFACE axis port=link_in
#pragma HLS INTERFACE axis port=link_out

#pragma HLS ARRAY_PARTITION variable=link_in complete dim=0
#pragma HLS ARRAY_PARTITION variable=link_out complete dim=0


#pragma HLS PIPELINE

	ecal::ProcessedTower Ai = unpackInput(link_in[0]);
	ecal::ProcessedTower Bi = unpackInput(link_in[1]);

	ecal::ProcessedTower Ao, Bo;

	stitch(stitchDir, Ai, Bi, Ao, Bo);

	stchTwr::packOutput(link_out[0],Ao);
	stchTwr::packOutput(link_out[1],Bo);
}

