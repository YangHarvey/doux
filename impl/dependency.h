#ifndef DOUX_DEPENDENCY_H
#define DOUX_DEPENDENCY_H

#include <unordered_map>
#include <iostream>

namespace doux {

struct Dependency {
    Dependency() {}

    uint64_t FindParent(uint64_t child) {
        if (dep_map_.find(child) == dep_map_.end()) {
            dep_map_[child] = child;
        }

        if (dep_map_[child] != child) {
            dep_map_[child] = FindParent(dep_map_[child]);
        }

        return dep_map_[child];
    }

    void SetParent(uint64_t parent, uint64_t child) {
        uint64_t root1 = FindParent(parent);
        uint64_t root2 = FindParent(child);

        if (root1 == root2) {
            return;
        }
        dep_map_[root2] = root1;
    }

    void PrintAll() {
        for (const auto& it : dep_map_) {
            std::cout << "Dep: " << it.first << " -> " <<  it.second << std::endl;
        }
    }

    std::unordered_map<uint64_t, uint64_t> dep_map_;
};

}


#endif  // DOUX_DEPENDENCY_H