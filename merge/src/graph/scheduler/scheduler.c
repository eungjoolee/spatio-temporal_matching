#include "scheduler.h"

#include <stdio.h>
#include <string.h>

/* Expects detectors to be in descending order of W over C */

/* This is a non-optimal O(n) approximation approach to this problem since the 0-1 knapsack
 * problem is np-complete.
 */

void update_v_power_focused(usage_stats_t *stats)
{
    memset(stats->v, 0, stats->n);
    float c_w = 0.0F;

    for (int i = 0; i < stats->n; i++)
    {
        if (c_w + stats->w[i] < stats->target_w)
        {
            stats->v[i] = 1;
            c_w += stats->w[i];
        }
    }

    /* If no detectors are enabled, then enable the one with the least weight */
    if (c_w == 0)
    {
        int min = 0;
        for (int i = 0; i < stats->n; i++)
        {
            if (stats->w[i] < stats->w[min])
            {
                min = i;
            }
        }

        stats->v[min] = 1;
    }
}

/* All but the first detector is disabled unless detections are found in the previous frame */
void update_v_frame_rate_focused(usage_stats_t *stats)
{
    if (stats->num_detections > 0)
    {
        memset(stats->v, 1, stats->n);
    } 
    else 
    {
        memset(stats->v, 0, stats->n); 
    }
}