//#include <cstdint>
#include <stdint.h>
#include <algorithm>
#include <utility>

#include "../../rct_common/src/rct_common.h"
#include "../../rct_common/src/adders.h"

#include "ecal.h"

using namespace std;
using namespace ecal;


ap_uint<3> getPeakBinOf5(const ap_uint<12> et[5], const ap_uint<16> etSum) {

#pragma HLS ARRAY_PARTITION variable=et complete dim=0
#pragma HLS INLINE

	uint16_t iEtSum =
			(et[0] >> 1)                +  // 0.5xet[0]
			(et[1] >> 1) + et[1]        +  // 1.5xet[1]
			(et[2] >> 1) + (et[2] << 1) +  // 2.5xet[2]
			(et[3] << 2) - (et[3] >> 1) +  // 3.5xet[3]
			(et[4] << 2) + (et[4] >> 1) ;  // 4.5xet[4]

	ap_uint<3> iAve = 0;
	if(     iEtSum <= etSum) iAve = 0;
	else if(iEtSum <= (etSum << 1)) iAve = 1;
	else if(iEtSum <= (etSum + (etSum << 1))) iAve = 2;
	else if(iEtSum <= (etSum << 2)) iAve = 3;
	else iAve = 4;
	return iAve;
}

ProcessedTower processTower(RawTower rawTwr) {

#pragma HLS ARRAY_PARTITION variable=rawTwr->crystals complete dim=0
#pragma HLS INLINE
#pragma HLS PIPELINE II=6

	ap_uint<12> phi_strip[5], eta_strip[5];

	// Compute eta strips
	etaStripLoop: for (size_t phi = 0; phi < 5; phi++) {
		ap_uint<10> a[5];
		a[0] =  rawTwr.crystals[0][phi].energy;
		a[1] =  rawTwr.crystals[1][phi].energy;
		a[2] =  rawTwr.crystals[2][phi].energy;
		a[3] =  rawTwr.crystals[3][phi].energy;
		a[4] =  rawTwr.crystals[4][phi].energy;

		eta_strip[phi] = add5_noDSP<10,12>(a);
	}

	// Compute phi strips
	phiStripLoop: for (size_t eta = 0; eta < 5; eta++) {
		ap_uint<10> a[5];
		a[0] =  rawTwr.crystals[eta][0].energy;
		a[1] =  rawTwr.crystals[eta][1].energy;
		a[2] =  rawTwr.crystals[eta][2].energy;
		a[3] =  rawTwr.crystals[eta][3].energy;
		a[4] =  rawTwr.crystals[eta][4].energy;

		phi_strip[eta] = add5_noDSP<10,12>(a);
	}

	// Compute tower energy, based on precomputed strips
	ap_uint<16> towerEt = add5_noDSP<12,16>(eta_strip);

	// Compute energy-weighted peak locations
	ap_uint<3> peakEta = getPeakBinOf5(eta_strip, towerEt);
	ap_uint<3> peakPhi = getPeakBinOf5(phi_strip, towerEt);

	// Small cluster ET is just the 3x5 around the peak
	ap_uint<14> subClusteredEt[5];
	ap_uint<14> ClusteredEt;

// precompute all 5 sub-clusters
	subClusteredEt[0]=add2_noDSP<12,14>(&eta_strip[0]); // 0-1
	subClusteredEt[1]=add3_noDSP<12,14>(&eta_strip[0]); // 0-1-2
	subClusteredEt[2]=add3_noDSP<12,14>(&eta_strip[1]); // 1-2-3
	subClusteredEt[3]=add3_noDSP<12,14>(&eta_strip[2]); // 2-3-4
	subClusteredEt[4]=add2_noDSP<12,14>(&eta_strip[3]); // 3-4

	// pick the sub-cluster around around the peak
	switch (peakEta) {
		case 0: ClusteredEt = subClusteredEt[0]; break;
		case 1: ClusteredEt = subClusteredEt[1]; break;
		case 2: ClusteredEt = subClusteredEt[2]; break;
		case 3: ClusteredEt = subClusteredEt[3]; break;
		default: ClusteredEt = subClusteredEt[4];break;
	}

	ProcessedTower pt;

	pt.clustered_et = (ClusteredEt > 0x3FF) ? (ap_uint<10>)0x3FF : (ap_uint<10>)ClusteredEt;
	pt.total_et = (towerEt > (ap_uint<16>)0x3FF) ? (ap_uint<10>)0x3FF : (ap_uint<10>)towerEt;

	pt.peak_eta = peakEta;
	pt.peak_phi = peakPhi;

	pt.peak_time = rawTwr.crystals[peakEta][peakPhi].peak_time;

#ifndef __SYNTHESIS__
	cout << "peakEta: " << peakEta << " peakPhi: " << peakPhi << endl;

	cout << "eta strips: " << eta_strip[0] << " "<< eta_strip[1] << " "<< eta_strip[2] << " "<< eta_strip[3] << " "<< eta_strip[4] << endl;
	cout << "phi strips: " << phi_strip[0] << " "<< phi_strip[1] << " "<< phi_strip[2] << " "<< phi_strip[3] << " "<< phi_strip[4] << endl;

	cout << "Sub-clusters: " << subClusteredEt[0] << " "<< subClusteredEt[1] << " "<< subClusteredEt[2] << " "<< subClusteredEt[3] << " "<< subClusteredEt[4] << endl;
	cout << "Cluster: " << pt.clustered_et << " " << pt.total_et << " " << pt.peak_eta << " " << pt.peak_phi << endl;
#endif

	return pt;
}

RawTower ecal::unpackInput(hls::stream<ecalInWord> &link) {

#pragma HLS INTERFACE axis port=link
#pragma HLS INLINE
#pragma HLS PIPELINE II=N_WORDS_PER_FRAME

	RawTower rt;

	ap_uint<64> data[6];
#pragma HLS ARRAY_PARTITION variable=data complete dim=1

	for (size_t i = 0; i < N_WORDS_PER_FRAME; i++) {
#ifndef __SYNTHESIS__
		// Avoid simulation warnings
		if (link.empty()) continue;
#endif
		data[i] = link.read().data;
	}

	rt.crystals[0][0]= Crystal(data[0](14,0));  // X0
	rt.crystals[0][1]= Crystal(data[0](29,15)); // X1
	rt.crystals[0][2]= Crystal(data[0](44,30)); // X2
	rt.crystals[0][3]= Crystal(data[0](59,45)); // X3
	rt.crystals[0][4]= Crystal((data[1](10,0) << 4) | data[0](63,60)); // X4

	rt.crystals[1][0]= Crystal(data[1](25,11)); // X5
	rt.crystals[1][1]= Crystal(data[1](40,26)); // X6
	rt.crystals[1][2]= Crystal(data[1](55,41)); // X7
	rt.crystals[1][3]= Crystal((data[2](6,0) << 8) | data[1](63,56)); // X8
	rt.crystals[1][4]= Crystal(data[2](21,7)); // X9

	rt.crystals[2][0]= Crystal(data[2](36,22));// X10
	rt.crystals[2][1]= Crystal(data[2](51,37));// X11
	rt.crystals[2][2]= Crystal((data[3](2,0) << 12) | data[2](63,52));// X12
	rt.crystals[2][3]= Crystal(data[3](17,3));// X13
	rt.crystals[2][4]= Crystal(data[3](32,18));// X14

	rt.crystals[3][0]= Crystal(data[3](47,33));// X15
	rt.crystals[3][1]= Crystal(data[3](62,48));// X16
	rt.crystals[3][2]= Crystal((data[4](13,0) << 1) | data[3](63,63));// X17
	rt.crystals[3][3]= Crystal(data[4](28,14));// X18
	rt.crystals[3][4]= Crystal(data[4](43,29));// X19

	rt.crystals[4][0]= Crystal(data[4](58,44));// X20
	rt.crystals[4][1]= Crystal((data[5](9,0) << 5) | data[4](63,59));// X21
	rt.crystals[4][2]= Crystal(data[5](24,10));// X22
	rt.crystals[4][3]= Crystal(data[5](39,25));// X23
	rt.crystals[4][4]= Crystal(data[5](54,40));// X24

	return rt;
}


void ecal::packOutput(hls::stream<ecalOutWord> &link, ProcessedTower procTwr) {

#pragma HLS INLINE

	ecalOutWord r;
	r.user = 0;
	r.last = 1;
	r.data(31,0) = (uint32_t)procTwr;

	link.write(r);
}


void ecal::processEcalLink(hls::stream<ecalInWord> link_in[1], hls::stream<ecalOutWord> link_out[1]) {

//#pragma HLS INTERFACE ap_ctrl_hs register port=return
#pragma HLS INTERFACE axis port=link_in
#pragma HLS INTERFACE axis port=link_out

#pragma HLS PIPELINE II=N_WORDS_PER_FRAME

	RawTower rawTwr = ecal::unpackInput(link_in[0]);

	ProcessedTower procTwr= processTower(rawTwr);

	packOutput(link_out[0], procTwr);
}

