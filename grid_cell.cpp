#include "grid_cell.h"

const float GridCell::s_log_odds_occupied_thre = GridCell::prob_to_log_odds(0.97f);
const float GridCell::s_log_odds_free_thre = GridCell::prob_to_log_odds(0.10f);

