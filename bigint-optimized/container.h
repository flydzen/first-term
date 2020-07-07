//
// Created by flydzen on 06.07.2020.
//

#ifndef BIGINT__CONTAINER_H_
#define BIGINT__CONTAINER_H_

#include <cstdint>
#include <memory>
#include <variant>
#include <vector>

#define u32 uint32_t

struct wrapper{
    std::vector<u32> v;
    size_t count;
    wrapper() : count(1) {    }
    explicit wrapper(std::vector<u32>& other) : wrapper(){
        v = other;
    }
    void del(){
        count--;
        if (count == 0)
            delete this;
    }

    wrapper* add(){
        count++;
        return this;
    }

    wrapper* unshare(){
        if (count != 1) {
            --count;
            auto* nw = new wrapper;
            nw->v = v;
            return nw;
        } else {
            return this;
        }
    }
};

union myUnion {
    u32 small;
    wrapper *big;
};

class container {
 public:
     container();
     ~container();
     container(container const& other);
     explicit container(size_t i);
     void push_back(u32 v);
     void pop_back();
     u32 &operator[](size_t ind);
     u32 const &operator[](size_t ind) const;
     container &operator=(container const& other);
     u32 &back();
     size_t size() const;
     void resize(size_t sz);
     void resize(size_t sz, u32 v);
     void reverse();
     bool is_small;
     bool empty;
     myUnion data;

};
bool operator==(container const &a, container const &b);
#endif //BIGINT__CONTAINER_H_
