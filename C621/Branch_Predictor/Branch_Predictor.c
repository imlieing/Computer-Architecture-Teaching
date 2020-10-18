#include "Branch_Predictor.h"

const unsigned instShiftAmt = 2; // Number of bits to shift a PC by

// You can play around with these settings.
const unsigned localPredictorSize = 65536;
const unsigned localCounterBits = 2;

const unsigned localHistoryTableSize = 16384; 
const unsigned globalPredictorSize = 134217728;
const unsigned choicePredictorSize = 8192; // Keep this the same as globalPredictorSize.

const unsigned globalCounterBits = 2;
const unsigned choiceCounterBits = 2;

const unsigned numberOfPerceptrons = 64;
// Taken from page 5 of supplement one in project 2
float theta = 142;


int sign(int prediction)
{
    int output;
    if (prediction > 0)
    {
        output = 1;
    }
    else if (prediction < 0)
    {
        output = -1;
    }
    return output;
   
}
int perceptronPredict(Perceptron* perceptron, int address_bits[])
{   
    int i = 0;
    size_t n = sizeof(perceptron->weights) / sizeof(int);
    int summation = 0;

    for (i = 0; i < n; i++)
    {
        summation = summation + perceptron->weights[i] * address_bits[i];    
    }
    if (summation < 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}

void intToBinaryDigit(unsigned int in, int* out)
{
    int count = 64;
    unsigned int mask = 1U << (count-1);
    int i;
    for (i = 0; i < count; i++) {
        out[i] = (in & mask) ? 1 : 0;
        in <<= 1;
    }
}

void initPerceptron(Perceptron *perceptron)
{
    int i = 0;
    for (i = 0; i < 64; i++){
        perceptron->weights[i] = 0;
    }
}

void trainPerceptron(Perceptron* perceptron, int* address_bits, int prediction, int t)
{
    // t is -1 if the branch wasn't taken and it is 1 if it was taken
    int i = 0;
    size_t n = sizeof(perceptron->weights) / sizeof(int);

    if (t == 0)
    {
        t = -1;
    }
    
    if (sign(prediction) != t || abs(prediction) <= theta)
    {
        for (i = 0; i < n; i++)
            {
                perceptron->weights[i] = perceptron->weights[i] + t * address_bits[i];
            }
    }
}

Branch_Predictor *initBranchPredictor()
{
    Branch_Predictor *branch_predictor = (Branch_Predictor *)malloc(sizeof(Branch_Predictor));

    #ifdef TWO_BIT_LOCAL
    printf("localPredictorSize: %u\n", localPredictorSize);
    printf("localCounterBits: %u\n", localCounterBits);
    branch_predictor->local_predictor_sets = localPredictorSize;
    assert(checkPowerofTwo(branch_predictor->local_predictor_sets));

    branch_predictor->index_mask = branch_predictor->local_predictor_sets - 1;

    // Initialize sat counters
    branch_predictor->local_counters =
        (Sat_Counter *)malloc(branch_predictor->local_predictor_sets * sizeof(Sat_Counter));

    int i = 0;
    for (i; i < branch_predictor->local_predictor_sets; i++)
    {
        initSatCounter(&(branch_predictor->local_counters[i]), localCounterBits);
    }
    #endif

    #ifdef GSHARE
        assert(checkPowerofTwo(globalPredictorSize));
        branch_predictor->global_predictor_size = globalPredictorSize;
        // Initialize global counters
        branch_predictor->global_counters = 
            (Sat_Counter *)malloc(globalPredictorSize * sizeof(Sat_Counter));

        int i = 0;
        for (i = 0; i < globalPredictorSize; i++)
        {
            initSatCounter(&(branch_predictor->global_counters[i]), globalCounterBits);
        }

        branch_predictor->global_history_mask = globalPredictorSize - 1;

        // global history register
        branch_predictor->global_history = 0;

        // We assume choice predictor size is always equal to global predictor size.
    #endif

    #ifdef PERCEPTRON
        branch_predictor->perceptron_list_size = numberOfPerceptrons;
        assert(checkPowerofTwo(branch_predictor->perceptron_list_size));
        branch_predictor->perceptron_mask = branch_predictor->perceptron_list_size - 1;
        branch_predictor->perceptron_list = 
            (Perceptron *)malloc(branch_predictor->perceptron_list_size * sizeof(Perceptron));
        int i = 0;
        for (i = 0; i < branch_predictor->perceptron_list_size; i++)
        {
            initPerceptron(&(branch_predictor->perceptron_list[i]));
        }


        assert(checkPowerofTwo(globalPredictorSize));
        branch_predictor->global_predictor_size = globalPredictorSize;
        branch_predictor->global_counters = 
            (Sat_Counter *)malloc(globalPredictorSize * sizeof(Sat_Counter));

        for (i = 0; i < globalPredictorSize; i++)
        {
            initSatCounter(&(branch_predictor->global_counters[i]), globalCounterBits);
        }

        branch_predictor->global_history_mask = globalPredictorSize - 1;
        branch_predictor->global_history = 0;
    #endif

    #ifdef TOURNAMENT

        printf("localHistoryTableSize: %u\n", localHistoryTableSize);
        printf("globalPredictorSize: %u\n", globalPredictorSize);
        printf("choicePredictorSize: %u\n", choicePredictorSize);

        assert(checkPowerofTwo(localPredictorSize));
        assert(checkPowerofTwo(localHistoryTableSize));
        assert(checkPowerofTwo(globalPredictorSize));
        assert(checkPowerofTwo(choicePredictorSize));
        assert(globalPredictorSize == choicePredictorSize);

        branch_predictor->local_predictor_size = localPredictorSize;
        branch_predictor->local_history_table_size = localHistoryTableSize;
        branch_predictor->global_predictor_size = globalPredictorSize;
        branch_predictor->choice_predictor_size = choicePredictorSize;
    
        // Initialize local counters 
        branch_predictor->local_counters =
            (Sat_Counter *)malloc(localPredictorSize * sizeof(Sat_Counter));

        int i = 0;
        for (i; i < localPredictorSize; i++)
        {
            initSatCounter(&(branch_predictor->local_counters[i]), localCounterBits);
        }

        branch_predictor->local_predictor_mask = localPredictorSize - 1;

        // Initialize local history table
        branch_predictor->local_history_table = 
            (unsigned *)malloc(localHistoryTableSize * sizeof(unsigned));

        for (i = 0; i < localHistoryTableSize; i++)
        {
            branch_predictor->local_history_table[i] = 0;
        }

        branch_predictor->local_history_table_mask = localHistoryTableSize - 1;

        // Initialize global counters
        branch_predictor->global_counters = 
            (Sat_Counter *)malloc(globalPredictorSize * sizeof(Sat_Counter));

        for (i = 0; i < globalPredictorSize; i++)
        {
            initSatCounter(&(branch_predictor->global_counters[i]), globalCounterBits);
        }

        branch_predictor->global_history_mask = globalPredictorSize - 1;

        // Initialize choice counters
        branch_predictor->choice_counters = 
            (Sat_Counter *)malloc(choicePredictorSize * sizeof(Sat_Counter));

        for (i = 0; i < choicePredictorSize; i++)
        {
            initSatCounter(&(branch_predictor->choice_counters[i]), choiceCounterBits);
        }

        branch_predictor->choice_history_mask = choicePredictorSize - 1;

        // global history register
        branch_predictor->global_history = 0;

        // We assume choice predictor size is always equal to global predictor size.
        branch_predictor->history_register_mask = choicePredictorSize - 1;
    #endif

    return branch_predictor;
}

// sat counter functions
inline void initSatCounter(Sat_Counter *sat_counter, unsigned counter_bits)
{
    sat_counter->counter_bits = counter_bits;
    sat_counter->counter = 0;
    sat_counter->max_val = (1 << counter_bits) - 1;
}

inline void incrementCounter(Sat_Counter *sat_counter)
{
    if (sat_counter->counter < sat_counter->max_val)
    {
        ++sat_counter->counter;
    }
}

inline void decrementCounter(Sat_Counter *sat_counter)
{
    if (sat_counter->counter > 0) 
    {
        --sat_counter->counter;
    }
}

// Branch Predictor functions
bool predict(Branch_Predictor *branch_predictor, Instruction *instr)
{
    uint64_t branch_address = instr->PC;

    #ifdef TWO_BIT_LOCAL    
        // Step one, get prediction
        unsigned local_index = getIndex(branch_address, 
                                        branch_predictor->index_mask);

        bool prediction = getPrediction(&(branch_predictor->local_counters[local_index]));

        // Step two, update counter
        if (instr->taken)
        {
            // printf("Correct: %u -> ", branch_predictor->local_counters[local_index].counter);
            incrementCounter(&(branch_predictor->local_counters[local_index]));
            // printf("%u\n", branch_predictor->local_counters[local_index].counter);
        }
        else
        {
            // printf("Incorrect: %u -> ", branch_predictor->local_counters[local_index].counter);
            decrementCounter(&(branch_predictor->local_counters[local_index]));
            // printf("%u\n", branch_predictor->local_counters[local_index].counter);
        }

        return prediction == instr->taken;
    #endif

    #ifdef GSHARE
        // unsigned global_predictor_idx = 
        //     branch_predictor->global_history & branch_predictor->global_history_mask;
        
        unsigned global_predictor_idx = (branch_predictor->global_history ^branch_address) &branch_predictor->global_history_mask;
        // unsigned global_predictor_idx = ((branch_predictor->global_history ^branch_address) << instShiftAmt) &branch_predictor->global_history_mask;
        bool global_prediction = 
            getPrediction(&(branch_predictor->global_counters[global_predictor_idx]));

        bool prediction_correct = global_prediction == instr->taken;

        if (instr->taken)
        {
            incrementCounter(&(branch_predictor->global_counters[global_predictor_idx]));
        }
        else
        {
            decrementCounter(&(branch_predictor->global_counters[global_predictor_idx]));
        }

        branch_predictor->global_history = branch_predictor->global_history << 1 | instr->taken;
        return prediction_correct;
    #endif

    #ifdef PERCEPTRON
    // Ideally, each static branch is allocated its own perceptron to predict its outcome. 
    // Excerpt taken from supplement: A perceptron learns a target Boolean
    // function t(z1, ..., 2,) of n inputs. In our case, the 2, are the
    // bits of a global branch history shift register, and the target
    // function predicts whether a particular branch will be taken. 

        int index = branch_address % branch_predictor->perceptron_list_size;
        Perceptron* current_perceptron = &(branch_predictor->perceptron_list[index]);
        int branch_address_digits[64];
        intToBinaryDigit(branch_address, branch_address_digits);
        int prediction = perceptronPredict(current_perceptron, branch_address_digits);
        trainPerceptron(current_perceptron, branch_address_digits, prediction, instr->taken);
        branch_predictor->perceptron_list[index] = *current_perceptron;

        // unsigned global_predictor_idx = (branch_predictor->global_history ^branch_address) &branch_predictor->global_history_mask;
        // bool global_prediction = 
        //     getPrediction(&(branch_predictor->global_counters[global_predictor_idx]));

        bool prediction_correct = prediction == instr->taken;

        // if (instr->taken)
        // {
        //     incrementCounter(&(branch_predictor->global_counters[global_predictor_idx]));
        // }
        // else
        // {
        //     decrementCounter(&(branch_predictor->global_counters[global_predictor_idx]));
        // }

        // branch_predictor->global_history = branch_predictor->global_history << 1 | instr->taken;
        return prediction_correct;
    #endif

    #ifdef TOURNAMENT
        // Step one, get local prediction.
        unsigned local_history_table_idx = getIndex(branch_address,
                                            branch_predictor->local_history_table_mask);
        
        unsigned local_predictor_idx = 
            branch_predictor->local_history_table[local_history_table_idx] & 
            branch_predictor->local_predictor_mask;

        bool local_prediction = 
            getPrediction(&(branch_predictor->local_counters[local_predictor_idx]));

        // Step two, get global prediction.
        unsigned global_predictor_idx = 
            branch_predictor->global_history & branch_predictor->global_history_mask;

        bool global_prediction = 
            getPrediction(&(branch_predictor->global_counters[global_predictor_idx]));

        // Step three, get choice prediction.
        unsigned choice_predictor_idx = 
            branch_predictor->global_history & branch_predictor->choice_history_mask;

        bool choice_prediction = 
            getPrediction(&(branch_predictor->choice_counters[choice_predictor_idx]));


        // Step four, final prediction.
        bool final_prediction;
        if (choice_prediction)
        {
            final_prediction = global_prediction;
        }
        else
        {
            final_prediction = local_prediction;
        }

        bool prediction_correct = final_prediction == instr->taken;
        // Step five, update counters
        if (local_prediction != global_prediction)
        {
            if (local_prediction == instr->taken)
            {
                // Should be more favorable towards local predictor.
                decrementCounter(&(branch_predictor->choice_counters[choice_predictor_idx]));
            }
            else if (global_prediction == instr->taken)
            {
                // Should be more favorable towards global predictor.
                incrementCounter(&(branch_predictor->choice_counters[choice_predictor_idx]));
            }
        }

        if (instr->taken)
        {
            incrementCounter(&(branch_predictor->global_counters[global_predictor_idx]));
            incrementCounter(&(branch_predictor->local_counters[local_predictor_idx]));
        }
        else
        {
            decrementCounter(&(branch_predictor->global_counters[global_predictor_idx]));
            decrementCounter(&(branch_predictor->local_counters[local_predictor_idx]));
        }

        // Step six, update global history register
        branch_predictor->global_history = branch_predictor->global_history << 1 | instr->taken;
        branch_predictor->local_history_table[local_history_table_idx] = branch_predictor->local_history_table[local_history_table_idx] << 1 | instr->taken;
        // exit(0);
        //
        return prediction_correct;
    #endif
}

inline unsigned getIndex(uint64_t branch_addr, unsigned index_mask)
{
    return (branch_addr >> instShiftAmt) & index_mask;
}

inline bool getPrediction(Sat_Counter *sat_counter)
{
    uint8_t counter = sat_counter->counter;
    unsigned counter_bits = sat_counter->counter_bits;

    // MSB determins the direction
    return (counter >> (counter_bits - 1));
}

int checkPowerofTwo(unsigned x)
{
    //checks whether a number is zero or not
    if (x == 0)
    {
        return 0;
    }

    //true till x is not equal to 1
    while( x != 1)
    {
        //checks whether a number is divisible by 2
        if(x % 2 != 0)
        {
            return 0;
        }
        x /= 2;
    }
    return 1;
}
