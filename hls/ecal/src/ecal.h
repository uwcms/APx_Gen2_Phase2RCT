#ifndef __ECAL_H__
#define __ECAL_H__

#include <cstdlib>
#include <sstream>

#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "../../rct_common/src/rct_common.h"

namespace ecal {

/* ECAL crystal object definition */
class Crystal {
public:
	Crystal() :
			energy(0), peak_time(0), spike(false) {
	};

	Crystal(ap_uint<15> i) {
		this->energy = i;
		this->peak_time = i >> 10;
		this->spike = i >> 14;
	}

	inline operator uint16_t() {
		return ((uint16_t) (this->spike) << 14)
				| ((uint16_t) (this->peak_time) << 10) | this->energy;
	}

	ap_uint<10> energy;
	ap_uint<4> peak_time;
	bool spike;
};

class RawTower {
public:
	RawTower() {
#pragma HLS ARRAY_PARTITION variable=crystals complete dim=0
		for (size_t eta = 0; eta < 5; eta++) {
#pragma LOOP UNROLL
			for (size_t phi = 0; phi < 5; phi++) {
#pragma LOOP UNROLL
				this->crystals[eta][phi] = Crystal();
			}
		}
	}

#ifndef __SYNTHESIS__
	std::string toString() {
		std::ostringstream str;
		for (size_t eta = 0; eta < 5; eta++) {
			for (size_t phi = 0; phi < 5; phi++) {
				str << "0x" << std::setfill('0') << std::setw(4) << std::hex
						<< (this->crystals[eta][phi].energy).to_int() << " ("
						<< eta << "," << phi << ")  ";
			}
			str << std::endl;
		}

		return str.str();
	}
#endif

	Crystal crystals[5][5];
};

class ProcessedTower {
public:
	ProcessedTower() :
			clustered_et(0), total_et(0), peak_phi(0), peak_eta(0), peak_time(0) {
	};

	ProcessedTower(uint32_t i) {
		this->clustered_et = i;
		this->total_et = i >> 10;
		this->peak_phi = i >> 20;
		this->peak_eta = i >> 23;
		this->peak_time = i >> 26;
	}

	inline friend bool operator >(const ProcessedTower& c1,
			const ProcessedTower& c2) {
		if (c1.total_et > c2.total_et)
			return true;
		else
			return false;
	}

	inline operator uint32_t() {
#pragma HLS INLINE

		return ((uint32_t) (this->peak_time) << 26)
				| ((uint32_t) (this->peak_eta) << 23)
				| ((uint32_t) (this->peak_phi) << 20)
				| ((uint32_t) (this->total_et) << 10)
				| (uint32_t) this->clustered_et;
	}

	ap_uint<10> clustered_et;
	ap_uint<10> total_et;
	ap_uint<3> peak_phi;
	ap_uint<3> peak_eta;
	ap_uint<4> peak_time;
};

typedef struct ecalInWord {
	ap_uint<64> data;
	ap_uint<8> user;
	ap_uint<1> last;
} ecalInWord;

typedef struct ecalOutWord {
	ap_uint<32> data;
	ap_uint<8> user;
	ap_uint<1> last;
} ecalOutWord;

void packOutput(hls::stream<ecalOutWord> &link, ProcessedTower procTwr);

RawTower unpackInput(hls::stream<ecalInWord> &link);

} // namespace ecal

void processEcalLink(hls::stream<ecal::ecalInWord> link_in[0], hls::stream<ecal::ecalOutWord> link_out[0]);


#endif /* !__ECAL_H__ */
