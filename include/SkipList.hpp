#pragma once
#include <iostream>
#include <vector>
#include <cstdlib>
#include <climits>
#include<cstdint>


struct SkipNode {
    uint32_t timestamp;
    double multiplier;
    
    SkipNode* left;
    SkipNode* right;
    SkipNode* up;
    SkipNode* down;

    SkipNode(uint32_t ts, double mult) 
        : timestamp(ts), multiplier(mult), left(nullptr), right(nullptr), up(nullptr), down(nullptr) {}
};

class TrafficTimeline {
private:
    SkipNode* head; 
    SkipNode* tail; 
    int max_levels;
    int current_levels;

    // Connects two nodes horizontally
    void link_horizontal(SkipNode* l, SkipNode* r) {
        if (l) l->right = r;
        if (r) r->left = l;
    }

    // Connects two nodes vertically
    void link_vertical(SkipNode* u, SkipNode* d) {
        if (u) u->down = d;
        if (d) d->up = u;
    }

    // Dynamically spawns a new empty level above the current top
    void push_new_level() {
        SkipNode* new_head = new SkipNode(0, 1.0); // 0 acts as -infinity timestamp
        SkipNode* new_tail = new SkipNode(UINT_MAX, 1.0); // UINT_MAX acts as +infinity

        link_horizontal(new_head, new_tail);
        link_vertical(new_head, head);
        link_vertical(new_tail, tail);

        head = new_head;
        tail = new_tail;
        current_levels++;
    }

public:
    // Regular Constructor
    TrafficTimeline(int max_lvls = 4) : max_levels(max_lvls), current_levels(1) {
        head = new SkipNode(0, 1.0);
        tail = new SkipNode(UINT_MAX, 1.0);
        link_horizontal(head, tail);
    }

    // Move Constructor (Transfers ownership safely during graph transpositions)
    TrafficTimeline(TrafficTimeline&& other) noexcept 
        : head(other.head), tail(other.tail), max_levels(other.max_levels), current_levels(other.current_levels) {
        other.head = nullptr;
        other.tail = nullptr;
        other.current_levels = 0;
    }

    // Move Assignment Operator
    TrafficTimeline& operator=(TrafficTimeline&& other) noexcept {
        if (this != &other) {
            // Invoke clear routine on existing memory allocation first
            this->~TrafficTimeline();

            head = other.head;
            tail = other.tail;
            max_levels = other.max_levels;
            current_levels = other.current_levels;

            other.head = nullptr;
            other.tail = nullptr;
            other.current_levels = 0;
        }
        return *this;
    }

    // Disable copy semantics explicitly to block shallow-pointer cloning traps
    TrafficTimeline(const TrafficTimeline&) = delete;
    TrafficTimeline& operator=(const TrafficTimeline&) = delete;

    // Destructor
    ~TrafficTimeline() {
        if (!head) return; // Skip if resource ownership was already moved out
        
        // Drop down to the absolute bottom layer
        SkipNode* level_runner = head;
        while (level_runner->down) {
            level_runner = level_runner->down;
        }

        // Clean up individual nodes level-by-level going upwards
        while (level_runner) {
            SkipNode* next_level_up = level_runner->up;
            SkipNode* curr = level_runner;
            while (curr) {
                SkipNode* next_node = curr->right;
                delete curr;
                curr = next_node;
            }
            level_runner = next_level_up;
        }
    }

    // Inserts a traffic anomaly into the timeline using the coin toss progression
    void insert_anomaly(uint32_t ts, double mult) {
        SkipNode* curr = head;
        std::vector<SkipNode*> drop_path;

        // Trace search path downwards and collect links for tower construction
        while (curr) {
            while (curr->right && curr->right->timestamp < ts) {
                curr = curr->right;
            }
            if (curr->right && curr->right->timestamp == ts) {
                // Node already present at this exact timestamp; refresh value downwards
                SkipNode* updater = curr->right;
                while (updater) {
                    updater->multiplier = mult;
                    updater = updater->down;
                }
                return;
            }
            drop_path.push_back(curr);
            curr = curr->down;
        }

        SkipNode* down_node = nullptr;
        bool promote = true;

        // Build tower upwards based on real coin tosses
        while (promote && !drop_path.empty()) {
            SkipNode* insert_after = drop_path.back();
            drop_path.pop_back();

            SkipNode* new_node = new SkipNode(ts, mult);
            SkipNode* insert_before = insert_after->right;

            // Splicing links horizontally
            link_horizontal(insert_after, new_node);
            link_horizontal(new_node, insert_before);

            // Splicing links vertically
            if (down_node) {
                link_vertical(new_node, down_node);
            }
            down_node = new_node;

            // Simulate the pure random coin toss loop promotion
            promote = (std::rand() % 2 == 1);

            // Expand levels if the coin keeps hit heads and limits aren't exceeded
            if (promote && drop_path.empty() && current_levels < max_levels) {
                push_new_level();
                drop_path.push_back(head);
            }
        }
    }

    // Searches the index structure for the most relevant active traffic condition
    double get_multiplier_at(uint32_t ts) const {
        SkipNode* curr = head;
        double active_multiplier = 1.0;

        while (curr) {
            while (curr->right && curr->right->timestamp <= ts) {
                curr = curr->right;
                active_multiplier = curr->multiplier;
            }
            curr = curr->down;
        }
        return active_multiplier;
    }
};