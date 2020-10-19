#pragma once
#include <cmath>

// Provides a log odds of occupancy probability representation for cells in a occupancy grid map.
class GridCell
{
public:
    float _log_odds_val; // The log odds representation of occupancy probability.

public:
    // Reset Cell to prior probability.
    void reset_value()
    {
        _log_odds_val = 0.0f;
    }

    bool is_occupied() const
    {
        return _log_odds_val > s_log_odds_occupied_thre;
    }

    bool is_free() const
    {
        return _log_odds_val < s_log_odds_free_thre;
    }

    void update(float mea_log_odds)
    {
        // It will be too big, so it`s meaningless for cal probability
        if ((mea_log_odds > 0.0f && _log_odds_val < 50.0f) 
            || (mea_log_odds < 0.0f && _log_odds_val > -50.0f)) {   
            _log_odds_val += mea_log_odds;
        }
    }

    static float prob_to_log_odds(float prob)
    {
        float odds = prob / (1.0f - prob);
        return std::log(odds);
    }

    static float log_odds_to_prob(float log_odds)
    {
        float odds = std::exp(log_odds);
        return odds / (odds + 1.0f);
    }
    
private:
    static const float s_log_odds_occupied_thre;
    static const float s_log_odds_free_thre;
};

