#ifndef __BRANCH_PREDICTOR_HH__
#define __BRANCH_PREDICTOR_HH__

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "Instruction.h"

// Predictor type
// #define TWO_BIT_LOCAL
#define TOURNAMENT

// saturating counter
typedef struct Sat_Counter
{
    unsigned counter_bits;
    uint8_t max_val;
    uint8_t counter;
}Sat_Counter;

typedef struct Branch_Predictor
{
    #ifdef TWO_BIT_LOCAL
    unsigned local_predictor_sets; // Number of entries in a local predictor
    unsigned index_mask;

    Sat_Counter *local_counters;
    #endif

    #ifdef TOURNAMENT
    unsigned local_predictor_size;
    unsigned local_predictor_mask;
    Sat_Counter *local_counters;

    unsigned local_history_table_size;
    unsigned local_history_table_mask;
    unsigned *local_history_table;

    unsigned global_predictor_size;
    unsigned global_history_mask;
    Sat_Counter *global_counters;

    unsigned choice_predictor_size;
    unsigned choice_history_mask;
    Sat_Counter *choice_counters;

    uint64_t global_history;
    unsigned history_register_mask;
    #endif
}Branch_Predictor;

// Initialization function
Branch_Predictor *initBranchPredictor();

// Counter functions
void initSatCounter(Sat_Counter *sat_counter, unsigned counter_bits);
void incrementCounter(Sat_Counter *sat_counter);
void decrementCounter(Sat_Counter *sat_counter);

// Branch predictor functions
bool predict(Branch_Predictor *branch_predictor, Instruction *instr);

unsigned getIndex(uint64_t branch_addr, unsigned index_mask);
bool getPrediction(Sat_Counter *sat_counter);

// Utility
int checkPowerofTwo(unsigned x);

#endif
