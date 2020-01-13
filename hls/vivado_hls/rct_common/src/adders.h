#ifndef __ADDERS_H__
#define __ADDERS_H__

#include <cstdlib>
#include <stdint.h>

#include <ap_int.h>


template <size_t IN_W, size_t OUT_W>
ap_uint<OUT_W> add2_noDSP(ap_uint<IN_W> op[2]);

template <size_t IN_W, size_t OUT_W>
ap_uint<OUT_W> add3_noDSP(ap_uint<IN_W> op[2]);

template <size_t IN_W, size_t OUT_W>
ap_uint<OUT_W> add4_noDSP(ap_uint<IN_W> op[2]);

template <size_t IN_W, size_t OUT_W>
ap_uint<OUT_W> add5_noDSP(ap_uint<IN_W> op[2]);

template <size_t IN_W, size_t OUT_W>
ap_uint<OUT_W> add2_noDSP(ap_uint<IN_W> op[2]) {
#pragma HLS ARRAY_PARTITION variable=op complete dim=1
#pragma HLS INLINE
#pragma HLS INTERFACE port=return register

	ap_uint<OUT_W> acc = static_cast<ap_uint<OUT_W> >(0);
//#pragma HLS RESOURCE variable=acc core=AddSub_DSP
//#pragma HLS RESOURCE variable=acc core=AddSubnS

	acc = op[0] + op[1];

	return acc;
}

template <size_t IN_W, size_t OUT_W>
ap_uint<OUT_W> add3_noDSP(ap_uint<IN_W> op[3]) {
#pragma HLS ARRAY_PARTITION variable=op complete dim=1

#pragma HLS inline
#pragma HLS INTERFACE port=return register

	ap_uint<OUT_W> acc = static_cast<ap_uint<OUT_W> >(0);
//#pragma HLS RESOURCE variable=acc core=AddSub_DSP
//#pragma HLS RESOURCE variable=acc core=AddSubnS

	acc = op[0] + op[1] + op[2];

	return acc;
}

template <size_t IN_W, size_t OUT_W>
ap_uint<OUT_W> add4_noDSP(ap_uint<IN_W> op[4]) {
#pragma HLS ARRAY_PARTITION variable=op complete dim=1

#pragma HLS inline
#pragma HLS INTERFACE port=return register

	ap_uint<OUT_W> acc = static_cast<ap_uint<OUT_W> >(0);
//#pragma HLS RESOURCE variable=acc core=AddSub_DSP
//#pragma HLS RESOURCE variable=acc core=AddSubnS

	acc = op[0] + op[1] + op[2] + op[3];

	return acc;
}

template <size_t IN_W, size_t OUT_W>
ap_uint<OUT_W> add5_noDSP(ap_uint<IN_W> op[5]) {
#pragma HLS ARRAY_PARTITION variable=op complete dim=1

#pragma HLS inline
#pragma HLS INTERFACE port=return register

	ap_uint<OUT_W> acc = static_cast<ap_uint<OUT_W> >(0);
//#pragma HLS RESOURCE variable=acc core=AddSub_DSP
//#pragma HLS RESOURCE variable=acc core=AddSubnS

	acc = op[0] + op[1] + op[2] + op[3] + op[4];

	return acc;
}


#endif /* !__ADDERS_H__ */
