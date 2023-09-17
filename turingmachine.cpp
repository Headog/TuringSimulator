#include "turingmachine.h"

enum bit new_bit;

TuringMachine::TuringMachine()
{
    reset();
}

struct rule *TuringMachine::step() {
    if (memory.size() == 0) {
        memory.push_back(new_bit == RANDOM ? (std::rand() % 2 == 0 ? OFF : ON) : new_bit);
    }
    for (struct rule &rule : rules) {
        if (rule.if_bit == memory[pointer] && rule.if_state == state) {
            memory[pointer] = rule.set_bit;
            if (pointer == 0 && rule.offset == -1) {
                memory.insert(memory.begin(), new_bit == RANDOM ? (std::rand() % 2 == 0 ? OFF : ON) : new_bit);
            } else {
                pointer += rule.offset;
                if (pointer == memory.size()) {
                    memory.push_back(new_bit == RANDOM ? (std::rand() % 2 == 0 ? OFF : ON) : new_bit);
                }
            }
            state = rule.set_state;
            return &rule;
        }
    }
    return NULL;
}

void TuringMachine::reset() {
    memory.clear();
    pointer = 0;
    state = "q0";
}
