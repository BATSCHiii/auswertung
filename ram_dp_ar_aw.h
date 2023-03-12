#ifndef __RAM_DP_AR_AW_H__
#define __RAM_DP_AR_AW_H__

#include "systemc.h"

/* RAM Dual-Port, async read/write */
SC_MODULE (ram_dp_ar_aw) {
    sc_in   <bool>       clk;

    sc_in   <sc_bv<32>>  addr_0;
    sc_in   <bool>       rw_0;
    sc_out  <sc_bv<32>>  rdata_0;
    sc_in   <sc_bv<32>>  wdata_0;
    sc_in   <sc_bv<4>>   wstrb_0;
    sc_in   <bool>       valid_0;
    sc_out  <bool>       ready_0;

    sc_in   <sc_bv<32>>  addr_1;
    sc_in   <bool>       rw_1;
    sc_out  <sc_bv<32>>  rdata_1;
    sc_in   <sc_bv<32>>  wdata_1;
    sc_in   <sc_bv<4>>   wstrb_1;
    sc_in   <bool>       valid_1;
    sc_out  <bool>       ready_1;

    uint32_t *mem;
    uint32_t mem_size;

    //-----------Methods------------------------------
    void process_0();
    void process_1();

    void thread_0();
    void thread_1();

    //-----------Constructor--------------------------
    SC_HAS_PROCESS(ram_dp_ar_aw);
    ram_dp_ar_aw(sc_module_name nm, uint32_t mem_size = 1024) :
        sc_module(nm),
        mem_size(mem_size)
    {
        #ifdef RAM_USE_THREAD
            SC_CTHREAD(thread_0, clk);
            SC_CTHREAD(thread_1, clk);
        #else
            SC_METHOD (process_0);
            sensitive << addr_0 << valid_0 << rw_0 << wdata_0 << wstrb_0;
            SC_METHOD (process_1);
            sensitive << addr_1 << valid_1 << rw_1 << wdata_1 << wstrb_1;
        #endif
    }

    void init_mem(uint32_t mem_size) {
        if (mem != nullptr) delete[] mem;
        this->mem_size = mem_size;
        mem = new uint32_t[mem_size/4];
        for (int i = 0; i < mem_size/4; i++)
            mem[i] = 0;
    }

    ~ram_dp_ar_aw() { delete[] mem; }
};



void ram_dp_ar_aw::process_0() {
    if (!valid_0) {
        return;
    }
    sc_uint<32> addr = sc_uint<32>(addr_0) % mem_size;
    sc_uint<32> data = mem[addr >> 2];
    if (rw_0) {
        if (sc_uint<32>(wstrb_0) & 0b0001) data.range( 7, 0) = sc_uint<32>(wdata_0).range( 7, 0);
        if (sc_uint<32>(wstrb_0) & 0b0010) data.range(15, 8) = sc_uint<32>(wdata_0).range(15, 8);
        if (sc_uint<32>(wstrb_0) & 0b0100) data.range(23,16) = sc_uint<32>(wdata_0).range(23,16);
        if (sc_uint<32>(wstrb_0) & 0b1000) data.range(31,24) = sc_uint<32>(wdata_0).range(31,24);
        mem[addr >> 2] = data;
    } else {
        rdata_0 = data;
    }
    ready_0.write(!ready_0);
}

void ram_dp_ar_aw::process_1() {
    if (!valid_1) {
        return;
    }
    sc_uint<32> addr = sc_uint<32>(addr_1) % mem_size;
    sc_uint<32> data = mem[addr >> 2];
    if (rw_1) {
        if (sc_uint<32>(wstrb_1) & 0b0001) data.range( 7, 0) = sc_uint<32>(wdata_1).range( 7, 0);
        if (sc_uint<32>(wstrb_1) & 0b0010) data.range(15, 8) = sc_uint<32>(wdata_1).range(15, 8);
        if (sc_uint<32>(wstrb_1) & 0b0100) data.range(23,16) = sc_uint<32>(wdata_1).range(23,16);
        if (sc_uint<32>(wstrb_1) & 0b1000) data.range(31,24) = sc_uint<32>(wdata_1).range(31,24);
        mem[addr >> 2] = data;
    } else {
        rdata_1 = data;
    }
    ready_1.write(!ready_1);
}

void ram_dp_ar_aw::thread_0() {
    ready_0 = 0;
    while (true) {
        wait();
        if (!valid_0) continue;

        sc_uint<32> addr = sc_uint<32>(addr_0) % mem_size;
        sc_uint<32> data = mem[addr >> 2];
        if (rw_0) {
            if (sc_uint<32>(wstrb_0) & 0b0001) data.range( 7, 0) = sc_uint<32>(wdata_0).range( 7, 0);
            if (sc_uint<32>(wstrb_0) & 0b0010) data.range(15, 8) = sc_uint<32>(wdata_0).range(15, 8);
            if (sc_uint<32>(wstrb_0) & 0b0100) data.range(23,16) = sc_uint<32>(wdata_0).range(23,16);
            if (sc_uint<32>(wstrb_0) & 0b1000) data.range(31,24) = sc_uint<32>(wdata_0).range(31,24);
            mem[addr >> 2] = data;
        } else {
            rdata_0 = data;
        }
        
        ready_0 = 1;

        // wait for cpu to de-assert signal
        while (valid_0) wait();
        ready_0 = 0;
    }
}

void ram_dp_ar_aw::thread_1() {
    ready_1 = 0;
    while (true) {
        wait();
        if (!valid_1) continue;

        sc_uint<32> addr = sc_uint<32>(addr_1) % mem_size;
        sc_uint<32> data = mem[addr >> 2];
        if (rw_1) {
            if (sc_uint<32>(wstrb_1) & 0b0001) data.range( 7, 0) = sc_uint<32>(wdata_1).range( 7, 0);
            if (sc_uint<32>(wstrb_1) & 0b0010) data.range(15, 8) = sc_uint<32>(wdata_1).range(15, 8);
            if (sc_uint<32>(wstrb_1) & 0b0100) data.range(23,16) = sc_uint<32>(wdata_1).range(23,16);
            if (sc_uint<32>(wstrb_1) & 0b1000) data.range(31,24) = sc_uint<32>(wdata_1).range(31,24);
            mem[addr >> 2] = data;
        } else {
            rdata_1 = data;
        }

        ready_1 = 1;

        // wait for cpu to de-assert signal
        while (valid_1) wait();
        ready_1 = 0;
    }
}


#endif
