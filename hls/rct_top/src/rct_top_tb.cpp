#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>

#include <argp.h>

#include "../../ecal/src/ecal.h"

#include "../../stitchTowers/src/stitchTowers.h"

#include "../../../common/APxLinkData.hh"


using namespace std;
using namespace ecal;
using namespace stchTwr;

static char doc[] =
    "APx HLS Algo C/RTL simulation";

static struct argp_option options[] = {
    {"read", 'r', "FILE", 0, "Reads target file and writes contexts to AXI streams", 0},
    {"write", 'w', "FILE", 0, "Writes algorithm results to target file", 0},
	{"compare", 'c', "FILE", 0, "Compare the algorithm output to target file", 0},
    {"verbose", 'v', 0, 0, "Produce verbose output", 0},
    {0},
};

struct arguments {
    char *readfile;
    char *writefile;
    char *cmpfile;
    bool verbose;
};

struct arguments arguments = {0};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = (struct arguments *)state->input;

    switch (key) {
    case 'r': arguments->readfile = arg; break;
    case 'w': arguments->writefile = arg; break;
    case 'c': arguments->cmpfile = arg; break;
    case 'v': arguments->verbose = true; break;
    default: return ARGP_ERR_UNKNOWN; break;
    }

    return 0;
}

static struct argp argp = { options, parse_opt, NULL, doc };

#define N_INPUT_LINKS 34
#define N_OUTPUT_LINKS 34

#define ETA_STITCH 1
#define PHI_STITCH 0


void rct_top(hls::stream<ecalInWord> link_in[N_INPUT_LINKS],
		hls::stream<ecalOutWord> link_out[N_OUTPUT_LINKS]) {

	hls::stream<ecalOutWord> link_twr[N_INPUT_LINKS];
	hls::stream<ecalOutWord> link_eta[N_INPUT_LINKS];
	hls::stream<ecalOutWord> link_phiS1[N_INPUT_LINKS];
	hls::stream<ecalOutWord> link_phiS2[N_INPUT_LINKS];

	// Process ECAL inputs
	for (int i = 0 ; i<N_INPUT_LINKS; i++) {
		processEcalLink(&link_in[i], &link_twr[i]);
    }

	// Stitch towers in eta direction
	for (int i = 0 ; i<N_INPUT_LINKS/2; i++) {
		stitchTowers(ETA_STITCH, &link_twr[i*2], &link_eta[i*2]);
    }

	// Stitch towers in phi direction, stage 1
	for (int i = 0 ; i<N_INPUT_LINKS/4; i++)
	{
		hls::stream<ecalOutWord> link_in_tmp[2];
		hls::stream<ecalOutWord> link_out_tmp[2];

		{
			link_in_tmp[0].write (link_eta[i*4].read());
			link_in_tmp[1].write (link_eta[i*4+2].read());

			stitchTowers(PHI_STITCH, link_in_tmp, link_out_tmp);

			link_phiS1[i*4].write (link_out_tmp[0].read());
			link_phiS1[i*4+2].write (link_out_tmp[1].read());
		}

		{
			link_in_tmp[0].write (link_eta[i*4+1].read());
			link_in_tmp[1].write (link_eta[i*4+3].read());

			stitchTowers(PHI_STITCH, link_in_tmp, link_out_tmp);

			link_phiS1[i*4+1].write (link_out_tmp[0].read());
			link_phiS1[i*4+3].write (link_out_tmp[1].read());
		}
	}

	// Pass-through links 32 and 33 after phi stitch stage 1
	link_phiS1[32].write(link_eta[32].read());
	link_phiS1[33].write(link_eta[33].read());


	// Stitch towers in phi direction, stage 2
	for (int i = 0 ; i<N_INPUT_LINKS/4; i++)
	{
		hls::stream<ecalOutWord> link_in_tmp[2];
		hls::stream<ecalOutWord> link_out_tmp[2];

		{
			link_in_tmp[0].write (link_phiS1[i*4+2].read());
			link_in_tmp[1].write (link_phiS1[i*4+4].read());

			stitchTowers(PHI_STITCH, link_in_tmp, link_out_tmp);

			link_phiS2[i*4+2].write (link_out_tmp[0].read());
			link_phiS2[i*4+4].write (link_out_tmp[1].read());
		}

		{
			link_in_tmp[0].write (link_phiS1[i*4+3].read());
			link_in_tmp[1].write (link_phiS1[i*4+5].read());

			stitchTowers(PHI_STITCH, link_in_tmp, link_out_tmp);

			link_phiS2[i*4+3].write (link_out_tmp[0].read());
			link_phiS2[i*4+5].write (link_out_tmp[1].read());
		}
	}

	// Pass-through links 0 and 1 after phi stitch stage 2
	link_phiS2[0].write(link_phiS1[0].read());
	link_phiS2[1].write(link_phiS1[1].read());

	for (int i = 0 ; i<N_INPUT_LINKS; i++) {
		link_out[i].write (link_phiS2[i].read());
    }
}


int main(int argc, char **argv) {

	argp_parse(&argp, argc, argv, 0, 0, &arguments);

	hls::stream<ecalInWord> link_in[N_INPUT_LINKS];
	hls::stream<ecalOutWord> link_in_tmp[N_INPUT_LINKS];
	hls::stream<ecalOutWord> link_out[N_OUTPUT_LINKS];
	size_t loop_count = 1;

	if (arguments.readfile) {
		APxLinkData datafile_in(N_INPUT_LINKS);

		datafile_in.read(arguments.readfile);

		// Copy the file data to the stream
		for (size_t i = 0; i < datafile_in.getCycles(); i++) {
			for (size_t k = 0; k < N_INPUT_LINKS; k++) {
				APxLinkData::LinkValue v;
				if (datafile_in.get(i, k, v)) {
					link_in[k].write({v.data, v.user, 0});
				}
			}
		}

		if (arguments.verbose) {
			cout << "Input data:" << endl;
			datafile_in.print();
		}

		// Calculate how many times the algorithm will have to run
		loop_count = datafile_in.getCycles() / N_WORDS_PER_FRAME;
		if (datafile_in.getCycles() % N_WORDS_PER_FRAME) loop_count++;
	}

	// Run the algorithm
	for (size_t i = 0; i < loop_count; i++) {
        rct_top(link_in, link_out);
	}

	APxLinkData datafile_out(N_OUTPUT_LINKS);

	for (size_t i = 0; i < N_WORDS_PER_FRAME * loop_count; i++) {
		for (size_t k = 0; k < N_OUTPUT_LINKS; k++) {
			if (!link_out[k].empty()) {
				ecalOutWord r = link_out[k].read();
				datafile_out.add(i, k, {r.user, r.data});
			}
		}
	}

	if (arguments.verbose) {
		cout << "Output data:" << endl;
		datafile_out.print();
	}

	if (arguments.writefile) {
		datafile_out.write(arguments.writefile);
	}

	if (arguments.cmpfile) {
		APxLinkData datafile_cmp(N_OUTPUT_LINKS);
		datafile_cmp.read(arguments.cmpfile);

		if (datafile_cmp != datafile_out) {
			cout << "HLS output DOES NOT match!" << endl;
			return -1;
		} else {
			cout << "HLS output matches!" << endl;
		}
	}

	return 0;
}

