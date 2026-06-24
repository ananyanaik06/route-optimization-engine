#pragma once
#include "SkipList.hpp"
#include <vector>
#include <stack>
#include <algorithm>

struct Edge {
    uint32_t target_node;
    double distance;
    char road_type; 
    TrafficTimeline timeline;

    Edge(uint32_t tgt, double dist, char type) 
        : target_node(tgt), distance(dist), road_type(type) {}
};

class MapGraph {
public:
    uint32_t total_nodes;
    std::vector<std::vector<Edge>> adj;

    MapGraph(uint32_t nodes) : total_nodes(nodes), adj(nodes) {}

    void add_road(uint32_t src, uint32_t dst, double distance, char type) {
        if (src >= total_nodes || dst >= total_nodes) return;
        adj[src].emplace_back(dst, distance, type);
    }

    void add_live_traffic(uint32_t src, uint32_t dst, uint32_t timestamp, double multiplier) {
        for (auto& edge : adj[src]) {
            if (edge.target_node == dst) {
                edge.timeline.insert_anomaly(timestamp, multiplier);
                return;
            }
        }
    }

    MapGraph transpose() const {
        MapGraph g_t(total_nodes);
        for (uint32_t u = 0; u < total_nodes; u++) {
            for (const auto& edge : adj[u]) {
                g_t.add_road(edge.target_node, u, edge.distance, edge.road_type);
            }
        }
        return g_t;
    }

private:
    void dfs_sort_pass(uint32_t u, std::vector<bool>& visited, std::stack<uint32_t>& st) const {
        visited[u] = true;
        for (const auto& edge : adj[u]) {
            if (!visited[edge.target_node]) {
                dfs_sort_pass(edge.target_node, visited, st);
            }
        }
        st.push(u);
    }

    void dfs_extract_scc(uint32_t u, std::vector<bool>& visited, std::vector<uint32_t>& comp) const {
        visited[u] = true;
        comp.push_back(u);
        for (const auto& edge : adj[u]) {
            if (!visited[edge.target_node]) {
                dfs_extract_scc(edge.target_node, visited, comp);
            }
        }
    }

public:
    std::vector<std::vector<uint32_t>> find_strongly_connected_zones() const {
        std::stack<uint32_t> st;
        std::vector<bool> visited(total_nodes, false);

        for (uint32_t i = 0; i < total_nodes; i++) {
            if (!visited[i]) dfs_sort_pass(i, visited, st);
        }

        MapGraph reversed_map = transpose();
        std::fill(visited.begin(), visited.end(), false);
        std::vector<std::vector<uint32_t>> components;

        while (!st.empty()) {
            uint32_t u = st.top();
            st.pop();

            if (!visited[u]) {
                std::vector<uint32_t> component;
                reversed_map.dfs_extract_scc(u, visited, component);
                components.push_back(component);
            }
        }
        return components;
    }
};