/**
 * algo_top_parameters.h
 *
 * Defines the algorithm related configuration parameters.
 */

#ifndef __ALGO_TOP_PARAMETERS_H__
#define __ALGO_TOP_PARAMETERS_H__

/** Common algorithm definitions, do not remove **/
// Un-comment to have algo as a passthrough for testing purposes
//#define ALGO_PASSTHROUGH

// Number of data words per processing cycle/frame
const int N_WORDS_PER_FRAME	= 6;

/** Algorithm specific parameters **/
#define TOWERS_IN_ETA 4
#define TOWERS_IN_PHI 3

#define TOTAL_CLUSTERS (TOWERS_IN_ETA * TOWERS_IN_PHI)
#define MAX_CLUSTERS_PER_LINK (N_WORDS_PER_FRAME * 64 / 32)
#define REQUIRED_OUTPUT_LINKS \
	((TOTAL_CLUSTERS%MAX_CLUSTERS_PER_LINK)? \
			(TOTAL_CLUSTERS / MAX_CLUSTERS_PER_LINK + 1) : \
			TOTAL_CLUSTERS / MAX_CLUSTERS_PER_LINK)

/** More common algorithm definitions, do not remove **/
#define N_INPUT_LINKS	(TOWERS_IN_ETA * TOWERS_IN_PHI * 2)
#define N_OUTPUT_LINKS	REQUIRED_OUTPUT_LINKS

#define N_INPUT_WORDS_PER_FRAME N_WORDS_PER_FRAME
#define N_OUTPUT_WORDS_PER_FRAME (MAX_CLUSTERS_PER_LINK/2)


#endif /* !__ALGO_TOP_PARAMETERS_H__ */
