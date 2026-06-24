#include "../include/Graph.hpp"
#include "../include/RoutingEngine.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <memory>

int main() {
    std::srand(42); 

    std::string filename = "map_data.txt";
    std::ifstream infile(filename);
    
    if (!infile.is_open()) {
        std::cerr << "[ERROR] Could not open script file: " << filename << std::endl;
        return 1;
    }

    std::unique_ptr<MapGraph> city = nullptr;
    std::unique_ptr<Router> navigation_engine = nullptr;
    
    CarDriver car_profile;
    Walker walk_profile;

    // Epoch tracking parameters
    std::vector<std::vector<double>> cached_fw_matrix;
    uint32_t last_cache_sync_time = 0;
    uint32_t last_traffic_update_time = 0;
    bool has_traffic = false;

    std::string line;
    std::cout << "[ENGINE] Initializing Epoch-Based Cache Routing Ingestion...\n" << std::endl;

    while (std::getline(infile, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::stringstream ss(line);
        std::string command;
        ss >> command;

        if (command == "VERTICES") {
            uint32_t num_nodes;
            ss >> num_nodes;
            city = std::make_unique<MapGraph>(num_nodes);
            navigation_engine = std::make_unique<Router>(*city);
            std::cout << "[INIT] Graph framework allocated with " << num_nodes << " nodes." << std::endl;
        } 
        else if (command == "ROAD") {
            uint32_t src, dst;
            double distance;
            char type;
            ss >> src >> dst >> distance >> type;
            city->add_road(src, dst, distance, type);
        } 
        else if (command == "TRAFFIC") {
            uint32_t src, dst, timestamp;
            double multiplier;
            ss >> src >> dst >> timestamp >> multiplier;
            city->add_live_traffic(src, dst, timestamp, multiplier);
            
            has_traffic = true;
            last_traffic_update_time = timestamp; // Track precisely when layout mutated
            std::cout << "[EVENT] Traffic Event Registered at t=" << timestamp << " (Edge " << src << "->" << dst << ")" << std::endl;
        } 
        else if (command == "SYNC_CACHE") {
            uint32_t current_system_time;
            ss >> current_system_time;
            
            // Background daemon thread triggers Floyd-Warshall to recalculate baseline
            cached_fw_matrix = navigation_engine->run_floyd_warshall();
            last_cache_sync_time = current_system_time;
            
            std::cout << "[CACHE_SYNC] Background Floyd-Warshall computed and flushed at System Time = " << last_cache_sync_time << std::endl;
        }
        else if (command == "QUERY_DIJKSTRA") {
            uint32_t src, dst, departure_time;
            std::string profile_str;
            ss >> src >> dst >> departure_time >> profile_str;

            const Profile* active_profile = &car_profile;
            if (profile_str == "WALK") active_profile = &walk_profile;

            // --- ADVANCED EPOCH CONDITION CHECK ---
            // If there's no traffic at all, OR if the most recent traffic update happened BEFORE our last cache sweep,
            // AND we're running a standard profile, we drop right into the cached table matrix!
            if (profile_str == "CAR" && (!has_traffic || last_traffic_update_time <= last_cache_sync_time)) {
                std::cout << "[FAST-PATH] Short-circuiting to Floyd-Warshall matrix lookup (Cache is Up-To-Date)..." << std::endl;
                std::cout << "  -> Calculated Cost [" << src << " -> " << dst << "]: " << cached_fw_matrix[src][dst] << " base units." << std::endl;
            } 
            else {
                // Traffic occurred after the last sync; our snapshot is stale, use full dynamic Dijkstra
                std::cout << "[DYNAMIC-PATH] Cache is stale (New traffic detected since last sync). Running Dijkstra..." << std::endl;
                auto optimal_path = navigation_engine->run_dijkstra(src, dst, departure_time, *active_profile);

                std::cout << "  -> Route Calculated: ";
                if (optimal_path.empty()) {
                    std::cout << "NO VIABLE ROUTE FOUND" << std::endl;
                } else {
                    for (size_t i = 0; i < optimal_path.size(); i++) {
                        std::cout << optimal_path[i] << (i == optimal_path.size() - 1 ? "" : " -> ");
                    }
                    std::cout << std::endl;
                }
            }
        } 
    }

    infile.close();
    std::cout << "\n[ENGINE] Script stream execution cleanly terminated." << std::endl;
    return 0;
}