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
    //If you add new fields, remember to add them to flushPipeStage
}pipeLevel;

static int32_t PC;
static int32_t regFile[SIM_REGFILE_SIZE];
pipeLevel pipe[SIM_PIPELINE_DEPTH];
bool isHalt;
bool waiting_for_memory;
bool hazard_exe_stage_nop;

/////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////Helper functions///////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

//Flushes the pipe's command at stage i
//
//A flushed command is of type CMD_NOP, has all values set to 0
//The command address is -1, as it was not read from anywhere (for debugging)
void flushPipeStage(int stage){
    assert(stage>=FETCH && stage <=WRITEBACK);//legal stage parameter

    //Set to 0 all the fields we have added to the pipe stages
    pipe[stage].alu_result = 0;
    pipe[stage].command_address = -1;//For debugging - this is a flush not a nope we read from instruction memory
    pipe[stage].branch_taken = false;
    pipe[stage].memory_data = 0;
    pipe[stage].dstVal = 0;

    //Resetting the pipe state stage as was defined in sim_api.h
    pipe[stage].dstVal = 0;
    pipe[stage].my_pipe_state.src1Val = 0;
    pipe[stage].my_pipe_state.src2Val = 0;

    //Reset the command struct in the pipe state stage
    pipe[stage].my_pipe_state.cmd.dst =0;
    pipe[stage].my_pipe_state.cmd.isSrc2Imm =0;
    pipe[stage].my_pipe_state.cmd.opcode = CMD_NOP;
    pipe[stage].my_pipe_state.cmd.src1 =0;
    pipe[stage].my_pipe_state.cmd.src2 =0;
}

//Advances the pipe's command at stage i to the pipe at stage i+1
//If advancing multiple stages, MUST advance stages from higher i values to lower i values
void advancePipeStage(int i){
    assert(i>=FETCH && i<WRITEBACK);//Only the first four stages can be advanced

    //Updating the fields we have added on top of the given struct
    pipe[i+1].alu_result = pipe[i].alu_result;
    pipe[i+1].branch_taken = pipe[i].branch_taken;
    pipe[i+1].command_address = pipe[i].command_address;
    pipe[i+1].memory_data = pipe[i].memory_data;

    //updating pipe stage state struct
    pipe[i+1].dstVal = pipe[i].dstVal;
    pipe[i+1].my_pipe_state.src1Val = pipe[i].my_pipe_state.src1Val;
    pipe[i+1].my_pipe_state.src2Val = pipe[i].my_pipe_state.src2Val;

    //updating command struct in pipe stage struct
    pipe[i+1].my_pipe_state.cmd.src1 = pipe[i].my_pipe_state.cmd.src1;
    pipe[i+1].my_pipe_state.cmd.src2 = pipe[i].my_pipe_state.cmd.src2;
    pipe[i+1].my_pipe_state.cmd.dst = pipe[i].my_pipe_state.cmd.dst;
    pipe[i+1].my_pipe_state.cmd.opcode = pipe[i].my_pipe_state.cmd.opcode;
    pipe[i+1].my_pipe_state.cmd.isSrc2Imm = pipe[i].my_pipe_state.cmd.isSrc2Imm;
}

//Gets a pipe stage index
//Returns -1 if the command in that pipe stage doesn't write to the register stage in WRITEBACK stage
//Otherwise, returns the dst register index
int getDstRegOfPipeStage(int i){
    assert(i>=FETCH && i<=WRITEBACK);
    switch(pipe[i].my_pipe_state.cmd.opcode){
        case CMD_NOP:
        case CMD_HALT:
        case CMD_BR:
        case CMD_BRNEQ:
        case CMD_BREQ:
        case CMD_STORE:
            return -1;
    }
    return pipe[i].my_pipe_state.cmd.dst;
}

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
    hazard_exe_stage_nop = false;


    for(int i=0; i<SIM_REGFILE_SIZE; i++)//Resetting registers
        regFile[i] == 0;

    for(int i=0; i<SIM_PIPELINE_DEPTH; i++)//Resetting the pipe
        flushPipeStage(i);

    //Current state is similar to fetch.
    int curr_st = FETCH;

    pipe[curr_st].command_address = PC;//first command is at address 0
    SIM_MemInstRead(pipe[curr_st].command_address, &pipe[curr_st].my_pipe_state.cmd);

    //Update src values
    int reg_src1_index = pipe[curr_st].my_pipe_state.cmd.src1;
    pipe[curr_st].my_pipe_state.src1Val = regFile[reg_src1_index];

    pipe[curr_st].my_pipe_state.src2Val = 0; //src2Val is set in decode, after we determine if it is an immediate or not

    int reg_dst_index = pipe[curr_st].my_pipe_state.cmd.dst;
    pipe[curr_st].dstVal = regFile[reg_dst_index];

    return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////Clock Tick/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

//Advances the pipe regularly if there are no hazards. No nopes are added.
//Works also in isHalted situation, if there are no hazards
void advancePipeNoHazards(){
    PC += 4;
    for(int i=MEMORY; i>=FETCH; i--)
        advancePipeStage(i);

    //For now command in Fetch is garbage, fetch is going to read it (or add nope if we are halted)
}

//Flush the pipe in case we are taking a branch.
//After this command, advancePipeNoHazards will be called and move everything one stage forward
void flushPipeBranchTaken(){
    assert(pipe[MEMORY].branch_taken == true);
    flushPipeStage(FETCH);
    flushPipeStage(DECODE);
    flushPipeStage(EXECUTE);
    printf("we are about to flush for a taken branch. We have command address as %d and dstVal as %d\n", pipe[MEMORY].command_address, pipe[MEMORY].dstVal);
    PC = pipe[MEMORY].command_address+pipe[MEMORY].dstVal; //update PC address
}

//Called to solve data hazards
//Advances stages MEM and EXE, puts a NOP in EXE and doesn't advance IF or ID
void advanceExeStageNop(){

    printf("Advance EXE stage NOP invoked\n");
    //Only EXE and MEM stages advance to MEM and WB stages
    advancePipeStage(MEMORY);
    advancePipeStage(EXECUTE);

    //Add NOP to EXE
    flushPipeStage(EXECUTE);

    //set down the flag
    hazard_exe_stage_nop = false;
}

//This function puts new commands and/or nops in different stages of the pipe
void advancePipe() {
    if (waiting_for_memory == true) {
        //All pipe stages besides WRITEBACK stay the same
        flushPipeStage(WRITEBACK);
        return;
    }
    if(pipe[MEMORY].branch_taken){      //If branch is taken, flush pipe BEFORE advancing the pipe
        flushPipeBranchTaken();
        advancePipeNoHazards();

        assert(waiting_for_memory == false);//We jump from MEM stage, so we cannot have a LOAD command in the me stage
        hazard_exe_stage_nop = false;       //No need to flush for hazards if we took the branch
    }else if(hazard_exe_stage_nop == true){ //Data Hazard
        advanceExeStageNop();
    }else{
        advancePipeNoHazards();
    }
}

void fetch(){
    int curr_st = FETCH;

    if(waiting_for_memory || hazard_exe_stage_nop){     //In this case do NOT fetch a new command
        return;
    }

    flushPipeStage(curr_st); //Set all fields to zero and all values to default. Cmd is set to NOP

    if(!isHalt){ //Read command from memory
        pipe[curr_st].command_address = PC;
        SIM_MemInstRead(pipe[curr_st].command_address, &pipe[curr_st].my_pipe_state.cmd);

        //registers are only read in decode stage
        pipe[curr_st].my_pipe_state.src1Val = 0;
        pipe[curr_st].my_pipe_state.src2Val = 0;
        pipe[curr_st].dstVal = 0;
    }
}

void decode(){
    int curr_st = DECODE;

    int reg_src1_index = pipe[curr_st].my_pipe_state.cmd.src1;
    pipe[curr_st].my_pipe_state.src1Val = regFile[reg_src1_index];

    // Check if src2 is an immediate value.
    if(pipe[curr_st].my_pipe_state.cmd.isSrc2Imm == false){
        int reg_src2_index = pipe[curr_st].my_pipe_state.cmd.src2;
        pipe[curr_st].my_pipe_state.src2Val = regFile[reg_src2_index];
    } else{
        pipe[curr_st].my_pipe_state.src2Val = pipe[curr_st].my_pipe_state.cmd.src2;
    }

    int reg_dst_index = pipe[curr_st].my_pipe_state.cmd.dst;
    pipe[curr_st].dstVal = regFile[reg_dst_index];

    //If the command is BR, the branch is always taken
    if(pipe[curr_st].my_pipe_state.cmd.opcode == CMD_BR)
        pipe[curr_st].branch_taken = true;
}

void forwardData(){
    if(!forwarding) return;
    int exe_opcode = pipe[EXECUTE].my_pipe_state.cmd.opcode;    //Opcode of the command in EXE

    //commands that do not read data from registers
    if(exe_opcode == CMD_BR || exe_opcode == CMD_NOP || exe_opcode == CMD_HALT) return;

    //Forwarding from MEMORY to EXECUTE
    int mem_cmd_dst = getDstRegOfPipeStage(MEMORY);
    if(mem_cmd_dst != -1 && pipe[MEMORY].my_pipe_state.cmd.opcode != CMD_LOAD){ //Forward to the command before
        //NOTE: LOAD cannot be solved by forwarding as the data isn't ready yet. We must use nop
        //Therefore we are certain the result is in alu_result and not memory_address

        if(pipe[EXECUTE].my_pipe_state.cmd.src1 == mem_cmd_dst){
            printf("Data forwarded from MEM to EXE\n");
            pipe[EXECUTE].my_pipe_state.src1Val = pipe[MEMORY].alu_result;
            return; //Data cannot be forwarded twice
        }else if(!pipe[EXECUTE].my_pipe_state.cmd.isSrc2Imm && pipe[EXECUTE].my_pipe_state.cmd.src2 == mem_cmd_dst){
            printf("Data forwarded from MEM to EXE\n");
            pipe[EXECUTE].my_pipe_state.src2Val = pipe[MEMORY].alu_result;
            return; //Data cannot be forwarded twice
        }
    }

    //Forwarding from WRITEBACK to EXECUTE
    int wb_cmd_dst = getDstRegOfPipeStage(WRITEBACK);
    if(wb_cmd_dst != -1){           //If the signal regWrite = 1

        //Find the data to forward can be either in alu_result or in memory_data
        int data_to_forward = -1;
        switch(pipe[WRITEBACK].my_pipe_state.cmd.opcode){
            case CMD_ADD:
            case CMD_SUB:
            case CMD_ADDI:
            case CMD_SUBI:
                data_to_forward = pipe[WRITEBACK].alu_result;
                break;
            case CMD_LOAD:
                data_to_forward = pipe[WRITEBACK].memory_data;
                break;
            default:
                assert(0!=0);//If the command is not one of the above 5 we shouldn't have gotten here
                return;
        }

        if(pipe[EXECUTE].my_pipe_state.cmd.src1 == wb_cmd_dst){
            printf("Data forwarded from WB to EXE\n");
            pipe[EXECUTE].my_pipe_state.src1Val = data_to_forward;
        }else if(!pipe[EXECUTE].my_pipe_state.cmd.isSrc2Imm && pipe[EXECUTE].my_pipe_state.cmd.src2 == wb_cmd_dst){
            printf("Data forwarded from WB to EXE\n");
            pipe[EXECUTE].my_pipe_state.src2Val = data_to_forward;
        }
    }
}

void execute(){
    forwardData();//Will only work if forwarding flag was given

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
            pipe[EXECUTE].alu_result = src1Val - src2Val;
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
            pipe[EXECUTE].alu_result = (PC - 4) + dstVal;//-4 because we advanced PC too many times
            //PC is update at MEM stage of the pipe
            break;
        case CMD_BREQ:
            assert(pipe[EXECUTE].my_pipe_state.cmd.isSrc2Imm == false);
            if(src1Val == src2Val){
                pipe[EXECUTE].branch_taken = true;
            }else{
                pipe[EXECUTE].branch_taken = false;
            }
            pipe[EXECUTE].alu_result = (PC - 4) + dstVal;//-4 because we advanced PC too many times
            break;
        case CMD_BRNEQ:
            assert(pipe[EXECUTE].my_pipe_state.cmd.isSrc2Imm == false);
            if(src1Val != src2Val){
                pipe[EXECUTE].branch_taken = true;
            }else{
                pipe[EXECUTE].branch_taken = false;
            }
            pipe[EXECUTE].alu_result = (PC - 4) + dstVal;//-4 because we advanced PC too many times
            break;
        case CMD_HALT:
            if(!pipe[MEMORY].branch_taken){ //If we are about to branch over this halt, do not stop the program
                isHalt = true;
            }
            break;
        default:
            printf("Command has no legal opcode!!");
    }
}

void memory(){
    int curr_st = MEMORY;
    uint32_t memory_address = (uint32_t)pipe[MEMORY].alu_result;
    switch(pipe[curr_st].my_pipe_state.cmd.opcode){
        case CMD_LOAD:
            assert(memory_address%4 == 0);//legal memory address
            //Memory may wait multiple cycles before giving result
            if(SIM_MemDataRead(memory_address, &pipe[curr_st].memory_data) != 0){
                waiting_for_memory = true;
            }else{
                waiting_for_memory = false;
            }
            break;
        case CMD_STORE:
            assert(memory_address%4 == 0);//legal memory address
            SIM_MemDataWrite(memory_address, pipe[curr_st].my_pipe_state.src1Val);
            break;
        case CMD_BR:
        case CMD_BREQ:
        case CMD_BRNEQ:
            //Jumping is done only after clock tick! Not now. See flushPipeBranchTaken()
            break;
        default:
            //not a memory command
            break;
    }
}

void splitRegfile(){
    assert(split_regfile || forwarding);

    int dst_reg_value = -1;         //The register value we need to forward, can be either in alu_result or memory_data
    switch(pipe[WRITEBACK].my_pipe_state.cmd.opcode){
        case CMD_LOAD:
            dst_reg_value = pipe[WRITEBACK].memory_data;
            break;
        case CMD_ADD:
        case CMD_SUB:
        case CMD_ADDI:
        case CMD_SUBI:
            dst_reg_value = pipe[WRITEBACK].alu_result;
            break;
        default:
            //If the command doesn't write to any registers in the writeback stage
            return;
    }

    //The actual forwarding
    int dst_reg_index = pipe[WRITEBACK].my_pipe_state.cmd.dst;
    if(dst_reg_index == pipe[DECODE].my_pipe_state.cmd.src1){
        pipe[DECODE].my_pipe_state.src1Val = dst_reg_value;
    }else if (!pipe[DECODE].my_pipe_state.cmd.isSrc2Imm && dst_reg_index == pipe[DECODE].my_pipe_state.cmd.src2){
        pipe[DECODE].my_pipe_state.src2Val = dst_reg_value;
    }
}

void writeback(){
    int dstRegIndex = pipe[WRITEBACK].my_pipe_state.cmd.dst;

    switch(pipe[WRITEBACK].my_pipe_state.cmd.opcode){
        case CMD_ADD:
        case CMD_SUB:
        case CMD_ADDI:
        case CMD_SUBI:
            if(dstRegIndex != 0){       //r0 must always contain 0
                regFile[pipe[WRITEBACK].my_pipe_state.cmd.dst] = pipe[WRITEBACK].alu_result;
            }
            break;
        case CMD_LOAD:
            if(dstRegIndex != 0){       //r0 must always contain 0
                regFile[pipe[WRITEBACK].my_pipe_state.cmd.dst] = pipe[WRITEBACK].memory_data;
            }
            break;
        case CMD_HALT:
            break;
        default:
            break;
    }

    if(split_regfile || forwarding){
        splitRegfile();
    }
}


//Finds data hazard from pipe_stage to the command in DECODE
//@param pipe_stage - EXECUTE, MEMORY or WRITEBACK
//The function checks whether the command in the given pipe stage will write to one of the registers the command in
//DECODE stage is trying to read now.
void detectDataHazardFromPipeStage(int pipe_stage){
    int destination = getDstRegOfPipeStage(pipe_stage);
    if(destination == -1) return;           //If this command does not write back to the register file
    if(pipe[DECODE].my_pipe_state.cmd.src1 == destination){
        printf("data hazard from WB to ID detected!\n");
        hazard_exe_stage_nop = true;
    }else if (pipe[DECODE].my_pipe_state.cmd.src2 == destination){ //Impossible if split-regfile
        printf("data hazard from WB to ID detected!\n");
        hazard_exe_stage_nop = true;
    }
}

void hazard_detect(){
    //Set off all hazard flags
    hazard_exe_stage_nop = false;

    //If one function is writing to the registers at the same cycle another function is reading them
    int dst_reg_index = pipe[WRITEBACK].my_pipe_state.cmd.dst;
    if(dst_reg_index == pipe[DECODE].my_pipe_state.cmd.src1){
        pipe[DECODE].my_pipe_state.src1Val = regFile[dst_reg_index];
    }else if (dst_reg_index == pipe[DECODE].my_pipe_state.cmd.src2){
        pipe[DECODE].my_pipe_state.src2Val = regFile[dst_reg_index];
    }

    //Find data hazards
    if(!forwarding || pipe[EXECUTE].my_pipe_state.cmd.opcode == CMD_LOAD){  //LOAD will still cause a hazard here
        detectDataHazardFromPipeStage(EXECUTE);
    }
    if(!forwarding){
        detectDataHazardFromPipeStage(MEMORY);
    }
    if(!split_regfile && !forwarding){      //Data hazard from Writeback impossible with split regfile
        detectDataHazardFromPipeStage(WRITEBACK);
    }
}

/*! SIM_CoreClkTick: Update the core simulator's state given one clock cycle.
  This function is expected to update the core pipeline given a clock cycle event.
*/
void SIM_CoreClkTick() {
    hazard_detect();
    advancePipe();

    fetch();
    decode();
    execute();
    memory();
    writeback();
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
    curState->pc=PC;
    for(int i=0; i<SIM_REGFILE_SIZE; i++) {//Copying registers
        curState->regFile[i] = regFile[i];
    }
    for(int i=0; i<SIM_PIPELINE_DEPTH; i++){//Copying the pipeline
        curState->pipeStageState[i].cmd = pipe[i].my_pipe_state.cmd;
        curState->pipeStageState[i].src1Val = pipe[i].my_pipe_state.src1Val;
        curState->pipeStageState[i].src2Val = pipe[i].my_pipe_state.src2Val;
    }
}

