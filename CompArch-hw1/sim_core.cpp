/* 046267 Computer Architecture - Spring 2017 - HW #1 */
/* This file should hold your implementation of the CPU pipeline core simulator */

#include "sim_api.h"
#include <assert.h>


typedef struct pipe_level {
    PipeStageState my_pipe_state; //Struct, defined in sim_api.h
    int32_t command_address;
    int32_t alu_result;
    int32_t dstVal;
    bool branch_taken;
    int32_t memory_data;
    //If you add new fields, remember to reset them in SIM_CoreReset and in advancePipe!!!!!!
}pipeLevel;

static int32_t PC;
static int32_t regFile[SIM_REGFILE_SIZE];
pipeLevel pipe[SIM_PIPELINE_DEPTH];
bool isHalt;
bool waiting_for_memory;

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
    isHalt = false;
    waiting_for_memory = false;

    for(int i=0; i<SIM_REGFILE_SIZE; i++)//Resetting registers
        regFile[i] == 0;
    for(int i=0; i<SIM_PIPELINE_DEPTH; i++){//Resetting the pipe
        pipe[i].alu_result = 0;
        pipe[i].command_address = 0;
        pipe[i].branch_taken = false;
        pipe[i].memory_data = 0;

        //Resetting the pipe state
        pipe[i].dstVal = 0;
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
    if(waiting_for_memory == true){
        return;
    }
    for(int i=SIM_PIPELINE_DEPTH-1; i>0; i--){
        pipe[i].alu_result = pipe[i-1].alu_result;
        pipe[i].branch_taken = pipe[i-1].branch_taken;
        pipe[i].command_address = pipe[i-1].command_address;
        pipe[i].memory_data = pipe[i-1].memory_data;
        //updating pipe stage state struct
        pipe[i].dstVal = pipe[i-1].dstVal;
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
    if(isHalt){
      pipe[FETCH].command_address = -1;//NOP, not read from instruction memory
    }else{
        pipe[FETCH].command_address = pc;
    }
    pipe[FETCH].branch_taken = false;
    pipe[FETCH].alu_result = 0; //ours, not necessary
    pipe[FETCH].memory_data = 0; // ours, not necessary

    //reset all printed fields - in struct pipeStageState
    pipe[FETCH].my_pipe_state.src1Val = 0;
    pipe[FETCH].my_pipe_state.src2Val = 0;
    //All fields in the command are reset below
    pipe[FETCH].my_pipe_state.cmd.opcode = CMD_NOP;
    pipe[FETCH].my_pipe_state.cmd.dst = 0;
    pipe[FETCH].my_pipe_state.cmd.src1 = 0;
    pipe[FETCH].my_pipe_state.cmd.src2 = 0;
    pipe[FETCH].my_pipe_state.cmd.isSrc2Imm = 0;

    pc += 4;
}

void decode(){
    if(!isHalt){
        SIM_MemInstRead(pipe[DECODE].command_address, &pipe[1].my_pipe_state.cmd);

        //Update src values
        int reg_src1_index = pipe[DECODE].my_pipe_state.cmd.src1;
        pipe[DECODE].my_pipe_state.src1Val = regFile[reg_src1_index];

        int reg_src2_index = pipe[DECODE].my_pipe_state.cmd.src2;
        pipe[DECODE].my_pipe_state.src2Val = regFile[reg_src2_index];

        int reg_dst_index = pipe[DECODE].my_pipe_state.cmd.dst;
        pipe[DECODE].dstVal = regFile[reg_dst_index];
    }
}

void execute(){
    int src1Val = pipe[EXECUTE].my_pipe_state.src1Val;
    int src2Val = pipe[EXECUTE].my_pipe_state.src2Val;
    int dstVal = pipe[EXECUTE].dstVal;

    switch(pipe[EXECUTE].my_pipe_state.cmd.opcode){
        case CMD_NOP:
            return;
        case CMD_ADD:
            assert(pipe[EXECUTE].my_pipe_state.cmd.isSrc2Imm == false);
            pipe[EXECUTE].alu_result = src1Val + src2Val;
            break;
        case CMD_SUB:
            assert(pipe[EXECUTE].my_pipe_state.cmd.isSrc2Imm == false);
            pipe[EXECUTE].alu_result = src1Val + src2Val;
            break;
        case CMD_ADDI:
            assert(pipe[EXECUTE].my_pipe_state.cmd.isSrc2Imm == true);
            pipe[EXECUTE].alu_result = src1Val + pipe[EXECUTE].my_pipe_state.cmd.src2;
            break;
        case CMD_SUBI:
            assert(pipe[EXECUTE].my_pipe_state.cmd.isSrc2Imm == true);
            pipe[EXECUTE].alu_result = src1Val - pipe[EXECUTE].my_pipe_state.cmd.src2;
            break;
        case CMD_LOAD:
            if(pipe[EXECUTE].my_pipe_state.cmd.isSrc2Imm == true){
                pipe[EXECUTE].alu_result = src1Val + pipe[EXECUTE].my_pipe_state.cmd.src2;
            }else{
                pipe[EXECUTE].alu_result = src1Val + src2Val;
            }
            break;
        case CMD_STORE:
            if(pipe[EXECUTE].my_pipe_state.cmd.isSrc2Imm == true){
                pipe[EXECUTE].alu_result = dstVal + pipe[EXECUTE].my_pipe_state.cmd.src2;
            }else{
                pipe[EXECUTE].alu_result = dstVal + src2Val;
            }
            break;
        case CMD_BR:
            pipe[EXECUTE].alu_result = (pc - 4) + dstVal;//-4 because we advanced PC too many times
            //PC is update at MEM stage of the pipe
            break;
        case CMD_BREQ:
            assert(pipe[2].my_pipe_state.cmd.isSrc2Imm == false);
            if(src1Val == src2Val){
                pipe[EXECUTE].branch_taken = true;
            }else{
                pipe[EXECUTE].branch_taken = false;
            }
            pipe[EXECUTE].alu_result = (pc - 4) + dstVal;//-4 because we advanced PC too many times
            break;
        case CMD_BRNEQ:
            assert(pipe[2].my_pipe_state.cmd.isSrc2Imm == false);
            if(src1Val != src2Val){
                pipe[EXECUTE].branch_taken = true;
            }else{
                pipe[EXECUTE].branch_taken = false;
            }
            pipe[EXECUTE].alu_result = (pc - 4) + dstVal;//-4 because we advanced PC too many times
            break;
        case CMD_HALT:
            isHalt = true;
            break;
        default:
            printf("Command has no legal opcode!!");
    }
}

void memory(){
    uint32_t memory_address = (uint32_t)pipe[MEMORY].alu_result;
    switch(pipe[MEMORY].my_pipe_state.cmd.opcode){
        case CMD_LOAD:
            //Memory may wait multiple cycles before giving result
            if(SIM_MemDataRead(memory_address, &pipe[MEMORY].memory_data) != 0){
                waiting_for_memory = true;
            }else{
                waiting_for_memory = false;
            }
            break;
        case CMD_STORE:
            SIM_MemDataWrite(memory_address, pipe[MEMORY].my_pipe_state.src1Val);
            break;
        default:
            //not a memory command
            break;
    }
}
void writeback(){
    switch(pipe[WRITEBACK].my_pipe_state.cmd.opcode){
        case CMD_NOP:
            break;
        case CMD_ADD:
        case CMD_SUB:
        case CMD_ADDI:
        case CMD_SUBI:
            regFile[pipe[WRITEBACK].my_pipe_state.cmd.dst] = pipe[WRITEBACK].alu_result;
            break;
        case CMD_LOAD:
            regFile[pipe[WRITEBACK].my_pipe_state.cmd.dst] = pipe[WRITEBACK].memory_data;
            break;
        case CMD_STORE:
            break;
        case CMD_BR:
        case CMD_BREQ:
        case CMD_BRNEQ:
            break;
        case CMD_HALT:
            exit(0);
            break;
    }
}

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

