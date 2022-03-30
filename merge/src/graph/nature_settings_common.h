#ifndef _nature_settings_common
#define _nature_settings_common

/*************************************************************************
 * This will be the settings type that contains all settings
 * for the graph to minimise the number of arguments being passed to the 
 * constructor
 * 
 *************************************************************************/

typedef struct _nature_settings_t
{
    int fifo_cap; // communication fifo size
    int scheduler; // scheduler type
    int num_calc_actors; // number of separate bbox pair calculation actors
} nature_settings_t;

#endif