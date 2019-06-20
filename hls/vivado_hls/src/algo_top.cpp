#include "algo_top_parameters.h"
#include "algo_top.h"
#include <algorithm>
#include <utility>

using namespace std;
using namespace algo;

/* ECAL crystal object definition */
struct Crystal {
	Crystal() : energy(0), timing(0), spike(false) {};

	Crystal(ap_uint<14> i) {
		this->energy = i;
		this->timing = i >> 10;
		this->spike = i >> 13;
	}

	inline operator uint16_t() {
		return  ((uint16_t)(this->spike) << 13) |
				((uint16_t)(this->timing) << 10) |
				this->energy;
	}

#ifndef __SYNTHESIS__
	string toString() {
		return "energy = " + to_string(this->energy) + ", timing = " + to_string(this->timing) + ", spike = " + to_string(this->spike);
	}
#endif

	ap_uint<10> energy;
	ap_uint<3> timing;
	bool spike;
};

/* ECAL cluster object definition */
struct Cluster {
	Cluster() : et(0), tower_phi(0), tower_eta(0), peak_phi(0), peak_eta(0) {};

// Even thought it is practical, this kind of initialization should be avoided to prevent mistakes.
//	Cluster(uint16_t et, ap_uint<4> tower_phi, ap_uint<6> tower_eta, ap_uint<3> peak_phi, ap_uint<3> peak_eta) :
//		et(et), tower_phi(tower_phi), tower_eta(tower_eta), peak_phi(peak_phi), peak_eta(peak_eta) {};

	Cluster(uint32_t i) {
		this->et = i;
		this->tower_phi = i >> 16;
		this->tower_eta = i >> 20;
		this->peak_phi = i >> 26;
		this->peak_eta = i >> 29;
	}

	inline friend bool operator >(const Cluster& c1, const Cluster& c2) {
		if (c1.et > c2.et) return true;
		else return false;
	}

	inline operator uint32_t() {
#pragma HLS INLINE
		return  ((uint32_t)(this->peak_eta) << 29) |
				((uint32_t)(this->peak_phi) << 26) |
				((uint32_t)(this->tower_eta) << 20) |
				((uint32_t)(this->tower_phi) << 16) |
				(uint32_t)this->et;
	}

	static void stitchEtaNeigbours(const Cluster Ai, const Cluster Bi, Cluster &Ao, Cluster &B);
	static void stitchPhiNeigbours(const Cluster Ai, const Cluster Bi, Cluster &Ao, Cluster &B);

	template <size_t ETA, size_t PHI>
	static void stitchRegion(const Cluster in[ETA][PHI], Cluster out[ETA][PHI]);

#ifndef __SYNTHESIS__
	string toString() const {
		return "Cluster [" + to_string(this->tower_eta) + "(" + to_string(this->peak_eta) + ")," + to_string(this->tower_phi) + "(" + to_string(this->peak_phi) + ")]: " + to_string(this->et);
	}
#endif

	uint16_t et;
	ap_uint<4> tower_phi;
	ap_uint<6> tower_eta;
	ap_uint<3> peak_phi;
	ap_uint<3> peak_eta;
};

void Cluster::stitchEtaNeigbours(const Cluster Ai, const Cluster Bi, Cluster &Ao, Cluster &Bo) {
// For some reason if this pragma is set then the code doesn't synthetize properly...
//#pragma HLS INLINE
	// Assuming clusters are already next to each other
	if (Ai.peak_eta == 4 && Bi.peak_eta == 0) {
		Ao = Ai;
		Bo = Bi;
		Ao.et = (Ai.et > Bi.et)? Ai.et + Bi.et : 0;
		Bo.et = (Ai.et > Bi.et)? 0 : Ai.et + Bi.et;
#ifndef __SYNTHESIS__
		cout << "Stitching cluster cluster [" + to_string(Ai.tower_eta) + "," + to_string(Ai.tower_phi) + "] with [" <<
				to_string(Bi.tower_eta) + "," + to_string(Bi.tower_phi) + "] in ETA" << endl;
#endif
	} else {
		Ao = Ai;
		Bo = Bi;
	}
}

void Cluster::stitchPhiNeigbours(const Cluster Ai, const Cluster Bi, Cluster &Ao, Cluster &Bo) {
// For some reason if this pragma is set then the code doesn't synthetize properly...
//#pragma HLS INLINE
	// Assuming clusters are already next to each other
	if (Ai.peak_phi == 4 && Bi.peak_phi == 0) {
		Ao = Ai;
		Bo = Bi;
		Ao.et = (Ai.et > Bi.et)? Ai.et + Bi.et : 0;
		Bo.et = (Ai.et > Bi.et)? 0 : Ai.et + Bi.et;
#ifndef __SYNTHESIS__
		cout << "Stitching cluster cluster [" + to_string(Ai.tower_eta) + "," + to_string(Ai.tower_phi) + "] with [" <<
				to_string(Bi.tower_eta) + "," + to_string(Bi.tower_phi) + "] in PHI" << endl;
#endif
	} else {
		Ao = Ai;
		Bo = Bi;
	}
}

template <size_t ETA, size_t PHI>
void Cluster::stitchRegion(const Cluster in[ETA][PHI], Cluster out[ETA][PHI]) {
#pragma HLS ARRAY_PARTITION variable=in complete dim=0
#pragma HLS ARRAY_PARTITION variable=out complete dim=0

	// Stitch in eta
	for (size_t phi = 0; phi < PHI; phi++) {
#pragma LOOP UNROLL
		Cluster eta_stitch[ETA];

		for (size_t eta = 0; eta < ETA; eta++) {
#pragma LOOP UNROLL
			eta_stitch[eta] = in[eta][phi];
		}

		Cluster eta_stitch_step1[ETA];

		for (size_t eta = 0; eta < ETA-1; eta+=2) {
#pragma LOOP UNROLL
			Cluster::stitchEtaNeigbours(eta_stitch[eta], eta_stitch[eta+1], eta_stitch_step1[eta], eta_stitch_step1[eta+1]);
		}
		if (ETA % 2) eta_stitch_step1[ETA-1] = eta_stitch[ETA-1];

		Cluster eta_stitch_step2[ETA];

		eta_stitch_step2[0] = eta_stitch_step1[0];
		for (size_t eta = 1; eta < ETA; eta+=2) {
#pragma LOOP UNROLL
			Cluster::stitchEtaNeigbours(eta_stitch_step1[eta], eta_stitch_step1[eta+1], eta_stitch_step2[eta], eta_stitch_step2[eta+1]);
		}

		for (size_t eta = 0; eta < ETA; eta++) {
#pragma LOOP UNROLL
			out[eta][phi] = eta_stitch_step2[eta];
		}
	}

	// Stitch in phi
	for (size_t eta = 0; eta < ETA; eta++) {
#pragma LOOP UNROLL
		Cluster phi_stitch[PHI];

		for (size_t phi = 0; phi < PHI; phi++) {
#pragma LOOP UNROLL
			phi_stitch[phi] = in[eta][phi];
		}

		Cluster phi_stitch_step1[PHI];

		for (size_t phi = 0; phi < PHI-1; phi+=2) {
#pragma LOOP UNROLL
			Cluster::stitchPhiNeigbours(phi_stitch[phi], phi_stitch[phi+1], phi_stitch_step1[phi], phi_stitch_step1[phi+1]);
		}
		if (PHI % 2) phi_stitch_step1[PHI-1] = phi_stitch[PHI-1];

		Cluster phi_stitch_step2[PHI];

		phi_stitch_step2[0] = phi_stitch_step1[0];
		for (size_t phi = 1; phi < PHI; phi+=2) {
#pragma LOOP UNROLL
			Cluster::stitchPhiNeigbours(phi_stitch_step1[phi], phi_stitch_step1[phi+1], phi_stitch_step2[phi], phi_stitch_step2[phi+1]);
		}

		for (size_t phi = 0; phi < PHI; phi++) {
#pragma LOOP UNROLL
			out[eta][phi] = phi_stitch_step2[phi];
		}
	}
}

/* Tower object definition */
struct Tower {
	Tower() {
#pragma HLS ARRAY_PARTITION variable=crystals complete dim=0
		for (size_t i = 0; i < 5; i++) {
#pragma LOOP UNROLL
			for (size_t j = 0; j < 5; j++) {
#pragma LOOP UNROLL
				this->crystals[i][j] = Crystal();
			}
		}
	}

	Cluster computeCluster(const ap_uint<6> towerEta, const ap_uint<4> towerPhi, uint16_t &towerEt);

#ifndef __SYNTHESIS__
	string toString() {
		ostringstream str;
		for (size_t i = 0; i < 5; i++) {
			for (size_t k = 0; k < 5; k++) {
				str << setfill('0') << setw(4) << hex << this->crystals[i][k].energy << " ";
			}
			str << endl;
		}

		return str.str();
	}
#endif

	Crystal crystals[5][5];

protected:
	inline uint16_t getPeakBinOf5(const uint16_t et[5], const uint16_t etSum);
};

Cluster Tower::computeCluster(const ap_uint<6> towerEta, const ap_uint<4> towerPhi, uint16_t &towerEt) {
#pragma HLS ARRAY_PARTITION variable=crystals complete dim=0

	uint16_t phi_strip[5], eta_strip[5];

	// Compute strips
	for (size_t eta = 0; eta < 5; eta++) {
#pragma LOOP UNROLL
		eta_strip[eta] = 0;
		for (size_t phi = 0; phi < 5; phi++) {
			eta_strip[eta] += this->crystals[eta][phi].energy;
		}
	}

	for (size_t phi = 0; phi < 5; phi++) {
#pragma LOOP UNROLL
		phi_strip[phi] = 0;
		for (size_t eta = 0; eta < 5; eta++) {
			phi_strip[phi] += this->crystals[eta][phi].energy;
		}
	}

	// Compute tower energy
	uint16_t tet = 0;
	for (size_t eta = 0; eta < 5; eta++) {
#pragma LOOP UNROLL
		tet += eta_strip[eta];
	}
	towerEt = tet;

	// Compute peak locations
	ap_uint<3> peakEta = getPeakBinOf5(eta_strip, tet);
	ap_uint<3> peakPhi = getPeakBinOf5(phi_strip, tet);

	// Small cluster ET is just the 3x5 around the peak
	uint16_t clusterEt = 0;
	for (int dEta = -1; dEta <= 1; dEta++) {
		int eta = peakEta + dEta;
		clusterEt = (eta >= 0 && eta < 5)? clusterEt + eta_strip[eta] : clusterEt;
	}

	Cluster cluster;
	cluster.et = clusterEt;
	cluster.tower_eta = towerEta;
	cluster.tower_phi = towerPhi;
	cluster.peak_eta = peakEta;
	cluster.peak_phi = peakPhi;

	return cluster;
}

inline uint16_t Tower::getPeakBinOf5(const uint16_t et[5], const uint16_t etSum) {
#pragma HLS ARRAY_PARTITION variable=et complete dim=0
#pragma HLS INLINE
	uint16_t iEtSum =
		(et[0] >> 1)                +  // 0.5xet[0]
		(et[1] >> 1) + et[1]        +  // 1.5xet[1]
		(et[2] >> 1) + (et[2] << 1) +  // 2.5xet[2]
		(et[3] << 2) - (et[3] >> 1) +  // 3.5xet[3]
		(et[4] << 2) + (et[4] >> 1) ;  // 4.5xet[4]
	uint16_t iAve = 0xBEEF;
	if(     iEtSum <= etSum) iAve = 0;
	else if(iEtSum <= (etSum << 1)) iAve = 1;
	else if(iEtSum <= (etSum + (etSum << 1))) iAve = 2;
	else if(iEtSum <= (etSum << 2)) iAve = 3;
	else iAve = 4;
	return iAve;
}

// Each input link carries the information of a 5x5 region
// Last 16-bits are reserved for CRC
inline Tower unpackInputLink(hls::stream<axiword> &link) {
#pragma HLS INTERFACE axis port=link
#pragma HLS INLINE

	Tower tower;

	uint8_t carry = 0;

	for (size_t i = 0; i < N_WORDS_PER_FRAME; i++) {
#ifndef __SYNTHESIS__
		// Avoid simulation warnings
		if (link.empty()) continue;
#endif

		uint64_t data = link.read().data;

		switch (i) {
		case 0: {
			for (size_t k = 0; k < 4; k++) {
#pragma LOOP UNROLL
				tower.crystals[0][k] = Crystal(data >> (k * 14));
			}
		} break;
		case 1: {
			tower.crystals[0][4] = Crystal((data << 8) | carry);
			for (size_t k = 0; k < 4; k++) {
#pragma LOOP UNROLL
				tower.crystals[1][k] = Crystal(data >> (k * 14 + 6));
			}
		} break;
		case 2: {
			tower.crystals[1][4] = Crystal(data);
			for (size_t k = 0; k < 3; k++) {
#pragma LOOP UNROLL
				tower.crystals[2][k] = Crystal(data >> (k * 14 + 14));
			}
		} break;
		case 3: {
			tower.crystals[2][3] = Crystal((data << 8) | carry);
			tower.crystals[2][4] = Crystal(data >> 6);
			for (size_t k = 0; k < 3; k++) {
#pragma LOOP UNROLL
				tower.crystals[3][k] = Crystal(data >> (k * 14 + 20));
			}
		} break;
		case 4: {
			for (size_t k = 0; k < 2; k++) {
#pragma LOOP UNROLL
				tower.crystals[3][k+3] = Crystal(data >> (k * 14));
			}
			for (size_t k = 0; k < 2; k++) {
#pragma LOOP UNROLL
				tower.crystals[4][k] = Crystal(data >> (k * 14 + 28));
			}
		} break;
		case 5: {
			tower.crystals[4][2] = Crystal((data << 8) | carry);
			for (size_t k = 0; k < 2; k++) {
#pragma LOOP UNROLL
				tower.crystals[4][k+3] = Crystal(data >> (k * 14 + 6));
			}
		} break;
		default: break;
		}

		// Remaining data to be used on the next cycle
		carry = data >> 56;
	}

	return tower;
}

void algo_top(hls::stream<axiword> link_in[N_INPUT_LINKS], hls::stream<axiword> link_out[N_OUTPUT_LINKS]) {
#pragma HLS INTERFACE axis port=link_in
#pragma HLS INTERFACE axis port=link_out
#pragma HLS PIPELINE II=N_WORDS_PER_FRAME
#pragma HLS ARRAY_PARTITION variable=link_in complete dim=0
#pragma HLS ARRAY_PARTITION variable=link_out complete dim=0

#ifndef ALGO_PASSTHROUGH

	// Step 1: Unpack links
	Tower ecalTowers[TOWERS_IN_ETA][TOWERS_IN_PHI];
#pragma HLS ARRAY_PARTITION variable=ecalTowers complete dim=0

	for (size_t i = 0; i < TOWERS_IN_ETA; i++) {
#pragma LOOP UNROLL
		for (size_t j = 0; j < TOWERS_IN_PHI; j++) {
#pragma LOOP UNROLL
			const size_t linkn = (i * TOWERS_IN_PHI + j);
			ecalTowers[i][j] = unpackInputLink(link_in[linkn]);
		}
	}

	// Step 2: Compute clusters
	Cluster ecalClusters[TOWERS_IN_ETA][TOWERS_IN_PHI];
#pragma HLS ARRAY_PARTITION variable=ecalClusters complete dim=0

	for (size_t i = 0; i < TOWERS_IN_ETA; i++) {
#pragma LOOP UNROLL
		for (size_t j = 0; j < TOWERS_IN_PHI; j++) {
#pragma LOOP UNROLL
			uint16_t towerEt = 0;
			ecalClusters[i][j] = ecalTowers[i][j].computeCluster(j, i, towerEt);
#ifndef __SYNTHESIS__
			cout << "Clustering: " << ecalClusters[i][j].toString() << endl;
#endif
		}
	}

	// Step 3: Merge neighbor clusters
	Cluster ecalClustersStitched[TOWERS_IN_ETA][TOWERS_IN_PHI];
#pragma HLS ARRAY_PARTITION variable=ecalClustersStitched complete dim=0

	Cluster::stitchRegion<TOWERS_IN_ETA,TOWERS_IN_PHI>(ecalClusters, ecalClustersStitched);

#ifndef __SYNTHESIS__
	for (size_t i = 0; i < TOWERS_IN_ETA; i++) {
		for (size_t j = 0; j < TOWERS_IN_PHI; j++) {
			cout << "Stitched: " << ecalClustersStitched[i][j].toString() << endl;
		}
	}
#endif

	// Step 5: Pack the outputs
	for (size_t i = 0; i < N_OUTPUT_LINKS; i++) {
#pragma LOOP UNROLL
		const size_t start_eta = i * 16 / TOWERS_IN_PHI;

		for (size_t j = 0; j < N_OUTPUT_WORDS_PER_FRAME-1; j++) {
#pragma LOOP UNROLL
			const size_t eta_offset = (j/2) % 4;
			const size_t phi = (j*2) % 4;

			axiword r; r.last = 0; r.user = 0;

			if (start_eta + eta_offset >= TOWERS_IN_ETA) {
				r.data = 0;
			} else {
				r.data = ((uint64_t)ecalClustersStitched[start_eta + eta_offset][phi+1] << 32) |
						 ((uint64_t)ecalClustersStitched[start_eta + eta_offset][phi]);
			}

			link_out[i].write(r);

		}

		// Last word is CRC
		axiword r; r.last = 0; r.user = 0; r.data = 0;
		link_out[i].write(r);
	}

#else
	// Algo passthrough (for testing)
	for (size_t i = 0; i < N_WORDS_PER_FRAME; i++) {
		axiword r[N_INPUT_LINKS];

		// Read all inputs
		for (size_t l = 0; l < N_INPUT_LINKS; l++)
			r[l] = link_in[l].read();

		// Write inputs to outputs
		for (size_t l = 0; l < N_OUTPUT_LINKS; l++) {
			if (l >= N_INPUT_LINKS) {
				link_out[l].write(r[N_INPUT_LINKS-1]);
			} else {
				link_out[l].write(r[l]);
			}
		}
	}


#endif /* !ALGO_PASSTHROUGH */

}
