#pragma once

#include <cmath>

#include "grid_cell.h"

// Default parameter values.
const float DEFAULT_HIT_FACTOR = 0.9f;      // P(z=1|s=occ)=0.9 and P(z=0|s=occ)=1-P(z=1|s=occ)=0.1
const float DEFAULT_MISS_FACTOR = 0.05f;    // P(z=1|s=free)=0.05 and P(z=0|s=free)=1-P(z=1|s=free)=0.95

// Provides functions related to a log odds of occupancy probability respresentation for cells in a occupancy grid map.
class GridSensor
{
public:
    // Constructor, sets parameters like free and occupied log odds ratios.
    GridSensor(float hit_factor = DEFAULT_HIT_FACTOR, 
        float miss_factor = DEFAULT_MISS_FACTOR)
    {
        set_update_factor(hit_factor, miss_factor);
    }

public:

    float _log_odds_hit;            // The log odds representation of probability used for updating cells as hit
    float _log_odds_miss;           // The log odds representation of probability used for updating cells as miss

    void set_update_factor(float hit_factor, float miss_factor)
    {
        _log_odds_hit = std::log(hit_factor / miss_factor);
        _log_odds_miss = std::log((1 - hit_factor) / (1 - miss_factor));
    }

    // Update cell as occupied
    void set_hit(GridCell& cell)
    {   
        cell.update(_log_odds_hit);
    }

    // Update cell as free
    void set_miss(GridCell& cell)
    {
        cell.update(_log_odds_miss);
    }
};

