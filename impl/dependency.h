#ifndef DOUX_DEPENDENCY_H
#define DOUX_DEPENDENCY_H

#include <unordered_map>

namespace doux {

class Dependency {
public:
    Dependency() {}

    uint64_t Find(uint64_t num) {
        if (parent.find(num) == parent.end()) {
            parent[num] = num;
        }

        if (parent[num] != num) {
            parent[num] = Find(parent[num]);
        }

        return parent[num];
    }

    bool IsSameSet(uint64_t num1, uint64_t num2) {
        return parent[num1] == parent[num2];
    }

    void Union(uint64_t num1, uint64_t num2) {
        uint64_t root1 = Find(num1);
        uint64_t root2 = Find(num2);

        if (root1 == root2) {
            return;
        }

        parent[root2] = root1;
    }

private:
    std::unordered_map<uint64_t, uint64_t> parent;
};

}


#endif  // DOUX_DEPENDENCY_H