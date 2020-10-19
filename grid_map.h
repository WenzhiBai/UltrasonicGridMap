#pragma once

#include <Eigen/Core>
#include <iostream>

#include "grid_cell.h"


// Default parameter values.
const float RESOLUTION = 0.1;   // 0.1m/cell
const int WIDTH = 300;          // 30m / 0.1m/cell = 300cells
const int HEIGHT = 300;         // 30m / 0.1m/cell = 300cells
const int EXT_ZONE = 50;        // 5m / 0.1m/cell = 50cells

enum ExtZoneType
{
    NONE = 0,
    TOP = 1 << 0,
    LEFT = 1 << 1,
    DOWN = 1 << 2,
    RIGHT = 1 << 3
};

class GridMap
{
private:
    float _resolution;          // The map resolution [m/cell].
    int _width;                 // Map width [cells].
    int _height;                // Map height [cells].
    Eigen::Vector2d _origin;    // The origin of the map [m, m].
                                // This is the real-world position of the left-down of cell (0,0) in the map.
    GridCell* _map_data;        // The map data, in row-major order, starting with (0,0), width priority.

public:
    GridMap(float resolution = RESOLUTION, 
        float width = WIDTH, 
        float height = HEIGHT,
        const Eigen::Vector3d & center_pos = Eigen::Vector3d(0.0, 0.0, 0.0))
    {
        _map_data = nullptr;
        init(resolution, width, height, center_pos);
    }

    ~GridMap()
    {
        delete_map_data();
    }

    GridMap(const GridMap& grid_map)
    {
        _resolution = grid_map.resolution();
        _width = grid_map.width();
        _height = grid_map.height();
        _origin = grid_map.origin();

        int size = _width * _height;
        _map_data = new GridCell[size];
        if (_map_data != nullptr) {
            for (int i = 0; i < size; ++i) {
                _map_data[i] = grid_map(i);
            }
        }
    }

    GridMap& operator=(const GridMap& grid_map)
    {
        if (&grid_map == this) return *this;

        if (grid_map.width() != _width || grid_map.height() != _height) {
            delete_map_data();
            _map_data = new GridCell[grid_map.height() * grid_map.width()];
        }

        _resolution = grid_map.resolution();
        _width = grid_map.width();
        _height = grid_map.height();
        _origin = grid_map.origin();
        int size = _width * _height;

        for (int i = 0; i < size; ++i) {
            _map_data[i] = grid_map(i);
        }

        return *this;
    }

    void init(float resolution, float width, float height, 
        const Eigen::Vector3d & center_pos)
    {
        _resolution = resolution;
        _width = width;
        _height = height;
        set_origin(center_pos);
        delete_map_data();
        allocate_map_data();
        reset_map_data();
    }

    int index_map(int idx_x, int idx_y) const
    {
        return idx_y * _width + idx_x;
    }
    
    void index_map(int idx, int & idx_x, int & idx_y) const
    {
        idx_y = idx / _width;
        idx_x = idx - idx_y * _width;
    }

    float resolution() const { return _resolution; }
    int width() const { return _width; }
    int height() const { return _height; }
    Eigen::Vector2d origin() const { return _origin; }

    GridCell& operator()(int idx_x, int idx_y)
    {
        return (*this)(index_map(idx_x, idx_y));
    }
    
    const GridCell& operator()(int idx_x, int idx_y) const
    {
        return (*this)(index_map(idx_x, idx_y));
    }

    GridCell& operator()(int idx)
    {
        return _map_data[idx];
    }

    const GridCell& operator()(int idx) const
    {
        return _map_data[idx];
    }

    void set_origin(const Eigen::Vector3d & pos)
    {
        _origin << pos.x() - _resolution * _width / 2,
                   pos.y() - _resolution * _height / 2;
    }

    // Allocates memory for the two dimensional pointer array for map representation.
    void allocate_map_data()
    {
        _map_data = new GridCell[_width * _height];
        if (nullptr == _map_data) {
            std::cout << "Allocate memory error in grid mapping.";
        }
    }

    void delete_map_data()
    {
        if (_map_data != nullptr) {
            delete[] _map_data;
            _map_data = nullptr;
        }
    }

    void reset_map_data()
    {
        int size = _width * _height;

        if (_map_data != nullptr) {
            for (int i = 0; i < size; ++i) {
                _map_data[i].reset_value();
            }
        }
    }

    bool is_in_border(const int idx_x, const int idx_y) const
    {
        return (idx_x >= 0 && idx_x < _width && idx_y >= 0 && idx_y < _height);
    }

    bool is_in_border(const double x, const double y) const
    {
        int idx_x = 0;
        int idx_y = 0;
        return xy_to_idx(x, y, idx_x, idx_y);
    }

    bool is_in_border(const Eigen::Vector3d & pos) const
    {
        return is_in_border(pos.x(), pos.y());
    }

    bool idx_to_xy(const int idx_x, const int idx_y, double &x, double &y) const
    {
        x = _origin.x() + _resolution * (idx_x + 0.5);
        y = _origin.y() + _resolution * (idx_y + 0.5);
        return is_in_border(idx_x, idx_y);
    }

    bool xy_to_idx(const double x, const double y, int &idx_x, int &idx_y) const
    {
        idx_x = (x - _origin.x()) / _resolution;
        idx_y = (y - _origin.y()) / _resolution;
        if (is_in_border(idx_x, idx_y)) {
            return true;
        } else {
            idx_x = std::max(0, std::min(idx_x, _width - 1));
            idx_y = std::max(0, std::min(idx_y, _height - 1));
            return false;
        }
    }

    bool pos_to_idx(const Eigen::Vector3d & pos, int &idx_x, int &idx_y) const
    {
        return xy_to_idx(pos.x(), pos.y(), idx_x, idx_y);
    }

    bool idx_to_pos(const int idx_x, const int idx_y, Eigen::Vector3d &pos) const
    {
        pos.setZero();
        return idx_to_xy(idx_x, idx_y, pos.x(), pos.y());
    }

    uint8_t is_in_ext_zone(const Eigen::Vector3d & pos)
    {
        uint8_t ext_zone_type = ExtZoneType::NONE;
        int idx_x = 0;
        int idx_y = 0;
        if (pos_to_idx(pos, idx_x, idx_y)) {
            if (idx_x < EXT_ZONE) {
                ext_zone_type |= ExtZoneType::LEFT;
            }
            if (idx_x >= (_width - EXT_ZONE)) {
                ext_zone_type |= ExtZoneType::RIGHT;
            }
            if (idx_y < EXT_ZONE) {
                ext_zone_type |= ExtZoneType::DOWN;
            }
            if (idx_y >= (_height - EXT_ZONE)) {
                ext_zone_type |= ExtZoneType::TOP;
            }
        }
        return ext_zone_type;
    }

    void extend_map(uint8_t ext_zone_type)
    {
        if (!ext_zone_type)
            return;

        if (ext_zone_type & ExtZoneType::LEFT) {
            _origin.x() -= EXT_ZONE * _resolution;
            for (int y = 0; y < _height; y++) {
                for (int x = _width - 1; x >= 0; x--) {
                    if (x < EXT_ZONE) {
                        _map_data[y * _width + x].reset_value();
                        continue;
                    }
                    _map_data[y * _width + x]._log_odds_val
                    = _map_data[y * _width + (x - EXT_ZONE)]._log_odds_val;
                }
            }
        }
        if (ext_zone_type & ExtZoneType::RIGHT) {
            _origin.x() += EXT_ZONE * _resolution;
            for (int y = 0; y < _height; y++) {
                for (int x = 0; x < _width; x++) {
                    if (x >= _width - EXT_ZONE) {
                        _map_data[y * _width + x].reset_value();
                        continue;
                    }
                    _map_data[y * _width + x]._log_odds_val
                    = _map_data[y * _width + (x + EXT_ZONE)]._log_odds_val;
                }
            }
        }
        if (ext_zone_type & ExtZoneType::DOWN) {
            _origin.y() -= EXT_ZONE * _resolution;
            for (int y = _height - 1; y >= 0; y--) {
                for (int x = 0; x < _width; x++) {
                    if (y < EXT_ZONE) {
                        _map_data[y * _width + x].reset_value();
                        continue;
                    }
                    _map_data[y * _width + x]._log_odds_val
                    = _map_data[(y - EXT_ZONE) * _width + x]._log_odds_val;
                }
            }
        }
        if (ext_zone_type & ExtZoneType::TOP) {
            _origin.y() += EXT_ZONE * _resolution;
            for (int y = 0; y < _height; y++) {
                for (int x = 0; x < _width; x++) {
                    if (y >= _height - EXT_ZONE) {
                        _map_data[y * _width + x].reset_value();
                        continue;
                    }
                    _map_data[y * _width + x]._log_odds_val
                    = _map_data[(y + EXT_ZONE) * _width + x]._log_odds_val;
                }
            }
        }
    }

    void reset_map(const Eigen::Vector3d & pos)
    {
        set_origin(pos);
        delete_map_data();
        allocate_map_data();
        reset_map_data();
    }
};

