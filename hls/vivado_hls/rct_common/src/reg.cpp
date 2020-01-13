#include <stdint.h>
#include <ap_int.h>

template<typename OBJ_T, int NOBJ>
void reg_1d_arr(OBJ_T obj[NOBJ], OBJ_T obj_out[NOBJ]) {
	OBJ_T obj_tmp[NOBJ];
#pragma HLS DATA_PACK variable=obj_tmp
#pragma HLS ARRAY_PARTITION variable=obj_tmp complete
	for (int iobj = 0; iobj < NOBJ; ++iobj) {
#pragma HLS latency min=1
#pragma HLS LOOP UNROLL
		obj_tmp[iobj] = obj[iobj];
	}
	for (int iobj = 0; iobj < NOBJ; ++iobj) {
#pragma HLS latency min=1
#pragma HLS LOOP UNROLL
		obj_out[iobj] = obj_tmp[iobj];
	}
}

template<typename OBJ_T, int NOBJ_X, int NOBJ_Y>
void reg_2d_arr(OBJ_T obj[NOBJ_X][NOBJ_Y], OBJ_T obj_out[NOBJ_X][NOBJ_Y]) {
	OBJ_T obj_tmp[NOBJ_X][NOBJ_Y];
#pragma HLS DATA_PACK variable=obj_tmp
#pragma HLS ARRAY_PARTITION variable=obj_tmp complete
	for (int iobj = 0; iobj < NOBJ_X; ++iobj) {
#pragma HLS latency min=1
#pragma HLS LOOP UNROLL
		for (int jobj = 0; jobj < NOBJ_Y; ++jobj) {
#pragma HLS LOOP UNROLL
			obj_tmp[iobj][jobj] = obj[iobj][jobj];
		}
	}
	for (int iobj = 0; iobj < NOBJ_X; ++iobj) {
#pragma HLS latency min=1
#pragma HLS LOOP UNROLL
		for (int jobj = 0; jobj < NOBJ_Y; ++jobj) {
#pragma HLS LOOP UNROLL
			obj_out[iobj][jobj] = obj_tmp[iobj][jobj];
		}
	}
}

template<class T>
T reg_scalar(T in) {
	//#pragma HLS INTERFACE ap_ctrl_none register port=return
#pragma HLS INTERFACE port=return register
#pragma HLS LATENCY min=1 max=1
#pragma HLS PIPELINE
#pragma HLS INLINE off
	return in;
}
