/* 046267 Computer Architecture - Spring 2017 - HW #1 */
/* This file should hold your implementation of the CPU pipeline core simulator */

#include "sim_api.h"
#include <assert.h>


typedef struct pipe_level {
    PipeStageState my_pipe_state; //Struct, defined in sim_api.h
    int32_t command_address;
    int32_t alu_result;

    //If you add new fields, remember to reset them in SIM_CoreReset and in advancePipe!!!!!!
}pipeLevel;

static int32_t PC;
static int32_t regFile[SIM_REGFILE_SIZE];
pipeLevel pipe[SIM_PIPELINE_DEPTH];

/////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////Core reset/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

/*! SIM_CoreReset: Reset the processor core simulator machine to start new simulation
  Use this API to initialize the processor core simulator's data structures.
  The simulator machine must complete this call with these requirements met:
  - PC = 0  (entry point for a program is at address 0)
  - All the register file is cleared (all registers hold 0)
  - The value of IF is the instuction in address 0x0
  \returns 0 on success. <0 in case of initialization failure.
*/
int SIM_CoreReset(void) {
    PC = 0;
    for(int i=0; i<SIM_REGFILE_SIZE; i++)//Resetting registers
        regFile[i] == 0;
    for(int i=0; i<SIM_PIPELINE_DEPTH; i++){//Resetting the pipe
        pipe[i].alu_result = 0;
        pipe[i].command_address = 0;
        //Resetting the pipe state
        pipe[i].my_pipe_state.src1Val = 0;
        pipe[i].my_pipe_state.src2Val = 0;

        //Reset the command struct in the pipe state
        pipe[i].my_pipe_state.cmd.dst =0;
        pipe[i].my_pipe_state.cmd.isSrc2Imm =0;
        pipe[i].my_pipe_state.cmd.opcode = CMD_NOP;
        pipe[i].my_pipe_state.cmd.src1 =0;
        pipe[i].my_pipe_state.cmd.src2 =0;
    }
    //First command is in address 0, so already in IF in the pipe
    return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////Clock Tick/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

void advancePipe(){
    for(int i=SIM_PIPELINE_DEPTH-1; i>0; i--){
        pipe[i].alu_result = pipe[i-1].alu_result;
        pipe[i].command_address = pipe[i-1].command_address;
        //updating pipe stage state struct
        pipe[i].my_pipe_state.src1Val = pipe[i-1].my_pipe_state.src1Val;
        pipe[i].my_pipe_state.src2Val = pipe[i-1].my_pipe_state.src2Val;
        //updating command struct in pipe stage struct
        pipe[i].my_pipe_state.cmd.src1 = pipe[i-1].my_pipe_state.cmd.src1;
        pipe[i].my_pipe_state.cmd.src2 = pipe[i-1].my_pipe_state.cmd.src2;
        pipe[i].my_pipe_state.cmd.dst = pipe[i-1].my_pipe_state.cmd.dst;
        pipe[i].my_pipe_state.cmd.opcode = pipe[i-1].my_pipe_state.cmd.opcode;
        pipe[i].my_pipe_state.cmd.isSrc2Imm = pipe[i-1].my_pipe_state.cmd.isSrc2Imm;
    }
    //For now command in Fetch is garbage
}

void fetch(){
    pipe[0].command_address = pc;
    pc += 4;
}

void decode(){}

void execute(){}
void memory(){}
void writeback(){}

void hazard_detect(){}

/*! SIM_CoreClkTick: Update the core simulator's state given one clock cycle.
  This function is expected to update the core pipeline given a clock cycle event.
*/
void SIM_CoreClkTick() {
    advancePipe();

    fetch();
    decode();
    execute();
    memory();
    writeback();

    hazard_detect();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////Core get state/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

/*! SIM_CoreGetState: Return the current core (pipeline) internal state
    curState: The returned current pipeline state
    The function will return the state of the pipe at the end of a cycle
*/
void SIM_CoreGetState(SIM_coreState *curState) {
    assert(curState != NULL);
    curState->pc=pc;
    for(int i=0; i<SIM_REGFILE_SIZE; i++) {//Copying registers
        curState->regFile[i] = regFile[i];
    }
    for(int i=0; i<SIM_PIPELINE_DEPTH; i++){//Copying the pipeline
//        curState->pipeStageState[i] = pipe[i].my_pipe_state;
        curState->pipeStageState[i].cmd = pipe[i].my_pipe_state.cmd;
        curState->pipeStageState[i].src1Val = pipe[i].my_pipe_state.src1Val;
        curState->pipeStageState[i].src2Val = pipe[i].my_pipe_state.src2Val;
    }
}

