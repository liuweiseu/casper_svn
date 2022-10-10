/* callbacks.h: public interface for set of callbacks. */
#include "check_gap_lib.h"

/* load_new_tables_from_test: 
 *
 * This is a ResultHandler for check_gap's run_test(). 
 * The results of the check gap test are passed to this function,
 * which reloads them into the histograms. */
int load_new_tables_from_test (CheckGapData *d, int cur_channel);
