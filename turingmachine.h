#ifndef TURINGMACHINE_H
#define TURINGMACHINE_H

#include <cstdio>
#include <vector>
#include <string>

enum bit {
    BLANK, ON, OFF, RANDOM
};

struct rule {
    std::string if_state;
    enum bit if_bit;
    enum bit set_bit;
    int offset;
    std::string set_state;
};

extern enum bit new_bit;

class TuringMachine
{
public:
    TuringMachine();

    size_t pointer = 0;
    std::string state = "q0";
    std::vector<enum bit> memory;
    std::vector<struct rule> rules;

    struct rule* step();

    void reset();
private:

};

#endif // TURINGMACHINE_H
