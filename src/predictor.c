//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "Victor Arango-Quiroga";
const char *studentID   = "A59017776";
const char *email       = "varangoquiroga@ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//
// gshare variables
unsigned* pattern_history_table;
unsigned global_history_table;
unsigned pc_masked;
unsigned ghistory_masked;
unsigned ghistory_mask;
unsigned pht_idx;
unsigned pred;
int size;

// Tournament
unsigned* local_BHT;
unsigned* local_PHT;

// train

unsigned train_pred;

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//
void
init_PHT(){
    // Initialize Pattern History Table (PHT) 
    // allocate size      
    size = 0x1<<ghistoryBits; // 2^ghistoryBits
    pattern_history_table = (unsigned*) malloc(sizeof(unsigned)*size);
    // initialize all entries with weak_not_taken state
    for(int i=0; i<size; i++){
        pattern_history_table[i] = WN; //WN from predictor.h
    }  
}
void
init_ghistory_mask(){
    ghistory_mask = (0 << sizeof(unsigned) * 8 -1)  |  (0x1 << ghistoryBits) - 0x1 ; 
}
void 
init_BHT(){
    size = 0x1<<pcIndexBits; // 2^number of bits in PC index
    local_BHT = (unsigned*) malloc(sizeof(unsigned)*size);
    for(int i=0;i<size;i++){
      local_BHT[i] = WN; 
    }  
}
void
init_local_PHT(){
    size = 0x1<<lhistoryBits; // 2^number of bits in local history
    local_PHT = (unsigned*) malloc(sizeof(unsigned)*size);
    for(int i=0;i<size;i++){
      local_PHT[i] = WN; 
    }    
}
// Initialize the predictor
//
void
init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //
  switch (bpType) {
    // case STATIC:
    case GSHARE:

        init_PHT();
        init_ghistory_mask();
        // Initialize Pattern History Table (PHT) 
        // allocate size        
        // size = 0x1<<ghistoryBits; // 2^ghistoryBits
        // pattern_history_table = (unsigned*) malloc(sizeof(unsigned)*size);
        // // initialize all entries with weak_not_taken state
        // for(int i=0; i<size; i++){
        //     pattern_history_table[i] = WN; //WN from predictor.h
        // }
        // // ---
        // // setup mask
        // // ghistory_mask = (0x1 << ghistoryBits) - 0x1 ; 
        // ghistory_mask = (0 << sizeof(unsigned) * 8 -1)  |  (0x1 << ghistoryBits) - 0x1 ; 
        break;
    case TOURNAMENT:
        // GLOBAL PREDICTOR
        init_PHT(); // --> pattern_history_table
        init_ghistory_mask(); // --> ghistory_mask

        // LOCAL PREDICTOR  
        init_BHT(); // branch_history_table --> local_BHT
        init_local_PHT(); // local Pattern History Table --> local_PHT

        // CHOOSER
        init_chooser(); 

    case CUSTOM:
    default:
      break;
  }
  
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{
  //
  //TODO: Implement prediction scheme
  //

  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
        // The Gshare predictor is characterized by XORing the global history register with the lower bits (same length as the global history) of 
        // the branch's address. This XORed value is then used to index into a 1D BHT of 2-bit predictors.

        // 1. Make mask of 1s for ghistoryBits bits
        // ghistoryMask = (0x1 << ghistoryBits) - 0x1 ; // 2^n-1 where n is number of bits used for Global History <- moved to init

        // 2. XOR PC and global history registry + keep out bits out of the mask
        pc_masked = pc &  ghistory_mask;
        ghistory_masked = global_history_table & ghistory_mask;
        pht_idx = pc_masked ^ ghistory_masked; // XOR
        pred = pattern_history_table[pht_idx];
        if (pred >= 2) {
          return TAKEN;
        }else {
          return NOTTAKEN; 
        }

    case TOURNAMENT:
      // Alpha 21264 with 2-bit predictor

      // The 'ghistoryBits' will be used to size the global and choice predictors

      // 'lhistoryBits' and 'pcIndexBits' will be used to size the local predictor
    case CUSTOM:
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void
train_predictor(uint32_t pc, uint8_t outcome)
{
  //
  //TODO: Implement Predictor training
  //

  train_pred = make_prediction(pc);

  switch (bpType) {
    case STATIC:
      break;
    case GSHARE:
        pc_masked = pc &  ghistory_mask;
        ghistory_masked = global_history_table & ghistory_mask;
        pht_idx = pc_masked ^ ghistory_masked; 
        unsigned curr_state = pattern_history_table[pht_idx] ;

        if (outcome == TAKEN){
          // update PHT accordingly 
           if (curr_state == WT || curr_state == WN ||  curr_state == SN){
              pattern_history_table[pht_idx]++;  
              }
        } 
        else{
           if (curr_state == WT || curr_state == WN ||  curr_state == ST){
              pattern_history_table[pht_idx]--;              
            }
        }
        // Update GHT: shift left 1 bit and insert appropriate bit to the right
        global_history_table = global_history_table << 1 | outcome; 
            case TOURNAMENT:
    case CUSTOM:
    default:
      break;
  }  
}
