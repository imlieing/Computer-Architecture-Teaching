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

const unsigned perceptronTableSize = 2048;
const unsigned globalHistoryBits = 12;
// Taken from page 5 of supplement one in project 2


int sign(int prediction)
{
    if (prediction > 0)
    {
        return 1;
    }
    else if (prediction < 0)
    {
        return -1;
    }
   
}
int perceptronPredict(Perceptron* perceptron, unsigned global_history_register)
{   
    int i = 0;
    signed input = 1;
    size_t n = sizeof(perceptron->weights) / sizeof(int);
    int summation = 0;

    for (i = 1; i < n; i++)
    {
        unsigned address_bit = (global_history_register >> i) & 1;
        if (address_bit == 0){
            input = -1;
        }
        else{
            input = 1;
        }
        summation = summation + perceptron->weights[i] * input;    
    }
    summation = summation + perceptron->weights[0] * 1;
    return summation;
}

// Doesnt work
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
    perceptron->threshold = ceil(1.93 * globalHistoryBits + 14);
    int i = 0;
    for (i = 0; i < globalHistoryBits; i++){
        perceptron->weights[i] = 0;
    }
}

void trainPerceptron(Perceptron* perceptron, unsigned global_history_address, signed t)
{
    signed input = 1;
    unsigned address_bit = 0;
    int i = 0;

    perceptron->weights[0] = perceptron->weights[0] + t; //Learns the bias of the branch
    for (int i=1; i < globalHistoryBits; i++) {
        address_bit = (global_history_address >>i) & 1;
        if (address_bit == 0) {
            input = -1; 
        }
        else {
            input = 1;
        }
        perceptron->weights[i] = perceptron->weights[i] + t*input;
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
        branch_predictor->perceptron_list_size = perceptronTableSize;
        assert(checkPowerofTwo(branch_predictor->perceptron_list_size));
        branch_predictor->perceptron_mask = branch_predictor->perceptron_list_size - 1;
        branch_predictor->perceptron_list = 
            (Perceptron *)malloc(branch_predictor->perceptron_list_size * sizeof(Perceptron));
        int i = 0;
        for (i = 0; i < branch_predictor->perceptron_list_size; i++)
        {
            initPerceptron(&(branch_predictor->perceptron_list[i]));
        }

        branch_predictor->global_history_mask = (1 << globalPredictorSize) - 1;
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

        unsigned index = getIndex(branch_address, 
            branch_predictor->perceptron_list_size - 1);
        Perceptron* current_perceptron = &(branch_predictor->perceptron_list[index]);

        int output = perceptronPredict(current_perceptron, branch_predictor->global_history);

        branch_predictor->perceptron_list[index] = *current_perceptron;
        int prediction;
        if (output < 0)
            {
                prediction = 0;
            }
            else
            {
                prediction = 1;
            }
        bool prediction_correct = prediction == instr->taken;
        signed t; //t is actual outcome of branch, t is 1 when taken and -1 when not taken
        if (instr->taken) {
             t = 1;
        } 
        else { 
            t = -1;
        }

        if ((instr->taken ^ prediction) || (abs(output) <= current_perceptron->threshold)) {
            trainPerceptron(current_perceptron, branch_predictor->global_history, t);
        }
        

        branch_predictor->global_history = branch_predictor->global_history << 1 | instr->taken;
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
