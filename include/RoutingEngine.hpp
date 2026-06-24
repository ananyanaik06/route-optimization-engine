#pragma once
#include "Graph.hpp"
#include <queue>
#include <vector>
#include <limits>

class Profile {
public:
    virtual ~Profile() = default;
    virtual double apply_terrain_penalty(char road_type) const = 0;
};

class CarDriver : public Profile {
public:
    double apply_terrain_penalty(char road_type) const override {
        return (road_type == 'H') ? 0.7 : 1.4; 
    }
};

class Walker : public Profile {
public:
    double apply_terrain_penalty(char road_type) const override {
        return (road_type == 'H') ? 8.0 : 1.0; 
    }
};

class Router {
private:
    const MapGraph& map;
    const double INF_VAL = std::numeric_limits<double>::infinity();

public:
    Router(const MapGraph& m) : map(m) {}

    std::vector<uint32_t> run_dijkstra(uint32_t start, uint32_t end, uint32_t time_of_day, const Profile& cost_profile) {
        std::vector<double> min_dist(map.total_nodes, INF_VAL);
        std::vector<int> tracking_parent(map.total_nodes, -1);
        
        using NodePair = std::pair<double, uint32_t>;
        std::priority_queue<NodePair, std::vector<NodePair>, std::greater<NodePair>> pq;

        min_dist[start] = 0.0;
        pq.push({0.0, start});

        while (!pq.empty()) {
            auto [current_cost, u] = pq.top();
            pq.pop();

            if (u == end) break;
            if (current_cost > min_dist[u]) continue;

            for (const auto& road : map.adj[u]) {
                double dynamic_traffic = road.timeline.get_multiplier_at(time_of_day + (uint32_t)current_cost);
                double terrain_modifier = cost_profile.apply_terrain_penalty(road.road_type);
                
                double absolute_step_cost = road.distance * dynamic_traffic * terrain_modifier;

                if (min_dist[u] + absolute_step_cost < min_dist[road.target_node]) {
                    min_dist[road.target_node] = min_dist[u] + absolute_step_cost;
                    tracking_parent[road.target_node] = (int)u;
                    pq.push({min_dist[road.target_node], road.target_node});
                }
            }
        }

        std::vector<uint32_t> complete_path;
        if (min_dist[end] == INF_VAL) return complete_path; 

        int current = (int)end;
        while (current != -1) {
            complete_path.push_back((uint32_t)current);
            current = tracking_parent[current];
        }
        std::reverse(complete_path.begin(), complete_path.end());
        return complete_path;
    }

    std::vector<std::vector<double>> run_floyd_warshall() {
        std::vector<std::vector<double>> matrix(map.total_nodes, std::vector<double>(map.total_nodes, INF_VAL));

        for (uint32_t i = 0; i < map.total_nodes; i++) {
            matrix[i][i] = 0.0;
            for (const auto& road : map.adj[i]) {
                matrix[i][road.target_node] = std::min(matrix[i][road.target_node], road.distance);
            }
        }

        for (uint32_t k = 0; k < map.total_nodes; k++) {
            for (uint32_t i = 0; i < map.total_nodes; i++) {
                for (uint32_t j = 0; j < map.total_nodes; j++) {
                    if (matrix[i][k] != INF_VAL && matrix[k][j] != INF_VAL) {
                        matrix[i][j] = std::min(matrix[i][j], matrix[i][k] + matrix[k][j]);
                    }
                }
            }
        }
        return matrix;
    }
};