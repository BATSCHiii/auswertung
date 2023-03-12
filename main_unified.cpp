#include <cstdlib>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "systemc.h"

#include "rv32_simple_core.h"
#include "cache_module.h"
#include "cache_module_thread_mem.h"
#include "bridge_module.h"

/* RAM Dual-Port, async read/write */
SC_MODULE (ram_dp_ar_aw) {
    sc_in   <sc_bv<32>>  addr_0;
    sc_in   <bool>       rw_0;
    sc_out  <sc_bv<32>>  rdata_0;
    sc_in   <sc_bv<32>>  wdata_0;
    sc_in   <sc_bv<4>>   wstrb_0;
    sc_in   <bool>       valid_0;
    sc_out  <bool>       ready_0;

    /*
    sc_in   <sc_bv<32>>  addr_1;
    sc_in   <bool>       rw_1;
    sc_out  <sc_bv<32>>  rdata_1;
    sc_in   <sc_bv<32>>  wdata_1;
    sc_in   <sc_bv<4>>   wstrb_1;
    sc_in   <bool>       valid_1;
    sc_out  <bool>       ready_1;
*/

    uint32_t *mem;
    uint32_t mem_size;

    //-----------Methods------------------------------
    void process_0() {
        //ready_0 = 1;
        if (!valid_0) {
            return;
        }
        sc_uint<32> addr = sc_uint<32>(addr_0) % mem_size;
        sc_uint<32> data = mem[addr >> 2];
        sc_uint<32> wstrb = sc_uint<32>(wstrb_0);
        sc_uint<32> wdata = sc_uint<32>(wdata_0);
        if (rw_0) {
            if (wstrb & 0b0001) data.range( 7, 0) = wdata.range( 7, 0);
            if (wstrb & 0b0010) data.range(15, 8) = wdata.range(15, 8);
            if (wstrb & 0b0100) data.range(23,16) = wdata.range(23,16);
            if (wstrb & 0b1000) data.range(31,24) = wdata.range(31,24);
            mem[addr >> 2] = data;
        } else {
            rdata_0 = data;
        }
        ready_0.write(!ready_0);
    }

    /*void process_1() {
        //ready_1 = 1;
        if (!valid_1) {
            return;
        }
        sc_uint<32> addr = sc_uint<32>(addr_1) % mem_size;
        sc_uint<32> data = mem[addr >> 2];
        sc_uint<32> wstrb = sc_uint<32>(wstrb_1);
        sc_uint<32> wdata = sc_uint<32>(wdata_1);
        if (rw_1) {
            if (wstrb & 0b0001) data.range( 7, 0) = wdata.range( 7, 0);
            if (wstrb & 0b0010) data.range(15, 8) = wdata.range(15, 8);
            if (wstrb & 0b0100) data.range(23,16) = wdata.range(23,16);
            if (wstrb & 0b1000) data.range(31,24) = wdata.range(31,24);
            mem[addr >> 2] = data;
        } else {
            rdata_1 = data;
        }
        ready_1.write(!ready_1);
    }*/

    //-----------Constructor--------------------------
    SC_HAS_PROCESS(ram_dp_ar_aw);
    ram_dp_ar_aw(sc_module_name nm, uint32_t mem_size = 1024) :
        sc_module(nm),
        mem_size(mem_size)
    {
        mem = new uint32_t[mem_size];
        for (int i = 0; i < mem_size; i++)
            mem[i] = 0;

        SC_METHOD (process_0);
        sensitive << addr_0 << valid_0 << rw_0 << wdata_0 << wstrb_0;

    //    SC_METHOD (process_1);
    //    sensitive << addr_1 << valid_1 << rw_1 << wdata_1 << wstrb_1;
    }
    
    ~ram_dp_ar_aw() { delete[] mem; }
};


SC_MODULE(monitor) {
    sc_in <bool>       clk;

    sc_in <sc_bv<32>>  i_addr;
    sc_in <sc_bv<32>>  i_rdata;
    sc_in <bool>       i_valid;
    sc_in <bool>       i_ready;

    sc_in <sc_bv<32>>  d_addr;
    sc_in <bool>       d_rw;
    sc_in <sc_bv<32>>  d_rdata;
    sc_in <sc_bv<32>>  d_wdata;
    sc_in <sc_bv<4>>   d_wstrb;
    sc_in <bool>       d_valid;
    sc_in <bool>       d_ready;

    void process() {
        std::cout << std::setw(10) << "Time";
        std::cout << std::setw(12) << "addr";
        std::cout << std::setw(12) << "rdata";
        std::cout << std::setw(6)  << "valid";
        std::cout << std::setw(6)  << "ready";
        std::cout << std::setw(6)  << " ";
        std::cout << std::setw(6)  << "rw";
        std::cout << std::setw(12) << "addr";
        std::cout << std::setw(12) << "rdata";
        std::cout << std::setw(12) << "wdata";
        std::cout << std::setw(6)  << "wstrb";
        std::cout << std::setw(6)  << "valid";
        std::cout << std::setw(6)  << "ready";
        std::cout << std::endl;
        while (true) {
            std::cout << std::setw(10) << sc_time_stamp();
            std::cout << std::setw(12) << i_addr.read().to_string(SC_HEX);
            std::cout << std::setw(12) << i_rdata.read().to_string(SC_HEX);
            std::cout << std::setw(6)  << i_valid.read();
            std::cout << std::setw(6)  << i_ready.read();
            std::cout << std::setw(6)  << " ";
            std::cout << std::setw(6)  << d_rw.read();
            std::cout << std::setw(12) << d_addr.read().to_string(SC_HEX);
            std::cout << std::setw(12) << d_rdata.read().to_string(SC_HEX);
            std::cout << std::setw(12) << d_wdata.read().to_string(SC_HEX);
            std::cout << std::setw(6)  << d_wstrb.read().to_string(SC_HEX);
            std::cout << std::setw(6)  << d_valid.read();
            std::cout << std::setw(6)  << d_ready.read();
            std::cout << std::endl;
            wait();    // wait for 1 clock cycle
        }
    }

    SC_CTOR(monitor) {
        SC_THREAD(process);
        sensitive << clk.pos();
    }
};






int sc_main(int argc, char** argv) {
    rv32_simple_core     *dut = new rv32_simple_core("Core");
    ram_dp_ar_aw  *ram = new ram_dp_ar_aw("RAM", 1024);
    monitor       *mon = new monitor("Monitor");

    CacheConfiguration i_cache_cfg {
        16,
        1,
        1,
        FIFO,
        WriteThrough,
    };

    CacheConfiguration d_cache_cfg {
        16,
        1,
        1,
        FIFO,
        WriteThrough,
    };

    CacheConfiguration u_cache_cfg {
        32,
        1,
        1,
        FIFO,
        WriteThrough,
    };

    CacheModuleThread *icache = new CacheModuleThread("Instruction_Cache", i_cache_cfg);
    CacheModuleThread *dcache = new CacheModuleThread("Data_Cache", d_cache_cfg);
    CacheModule *ucache = new CacheModule("Unified_Cache", u_cache_cfg);

    Bridge *bridge = new Bridge("BRIDGE");
    sc_time CLK_PERIOD(10, SC_NS);
    sc_clock  clock("clk", CLK_PERIOD, 0.5);
    sc_signal <bool>      reset_n;

    sc_signal <sc_bv<32>> imem_addr;
    sc_signal <bool>      _dummy_imem_rw;
    sc_signal <sc_bv<32>> imem_rdata;
    sc_signal <bool>      imem_valid;
    sc_signal <sc_bv<32>> _dummy_imem_wdata;
    sc_signal <sc_bv<4>>  _dummy_imem_wstrb;
    sc_signal <bool>      imem_ready;
    
    sc_signal <sc_bv<32>> imem_addr_cache;
    sc_signal <bool>      _dummy_imem_rw_cache;
    sc_signal <sc_bv<32>> imem_rdata_cache;
    sc_signal <bool>      imem_valid_cache;
    sc_signal <sc_bv<32>> _dummy_imem_wdata_cache;
    sc_signal <sc_bv<4>>  _dummy_imem_wstrb_cache;
    sc_signal <bool>      imem_ready_cache;
     
    sc_signal <sc_bv<32>> dmem_addr;
    sc_signal <bool>      dmem_rw;
    sc_signal <sc_bv<32>> dmem_rdata;
    sc_signal <sc_bv<32>> dmem_wdata;
    sc_signal <sc_bv<4>>  dmem_wstrb;
    sc_signal <bool>      dmem_valid;
    sc_signal <bool>      dmem_ready;
    
    sc_signal <sc_bv<32>> dmem_addr_cache;
    sc_signal <bool>      dmem_rw_cache;
    sc_signal <sc_bv<32>> dmem_rdata_cache;
    sc_signal <sc_bv<32>> dmem_wdata_cache;
    sc_signal <sc_bv<4>>  dmem_wstrb_cache;
    sc_signal <bool>      dmem_valid_cache;
    sc_signal <bool>      dmem_ready_cache;
   
    sc_signal <sc_bv<32>, SC_MANY_WRITERS> u_addr;
    sc_signal <bool, SC_MANY_WRITERS>      u_rw;
    sc_signal <sc_bv<32>, SC_MANY_WRITERS> u_rdata;
    sc_signal <sc_bv<32>, SC_MANY_WRITERS> u_wdata;
    sc_signal <sc_bv<4>, SC_MANY_WRITERS>  u_wstrb;
    sc_signal <bool, SC_MANY_WRITERS>      u_valid;
    sc_signal <bool, SC_MANY_WRITERS>      u_ready;

    sc_signal <sc_bv<32>> u_addr_cache;
    sc_signal <bool>      u_rw_cache;
    sc_signal <sc_bv<32>> u_rdata_cache;
    sc_signal <sc_bv<32>> u_wdata_cache;
    sc_signal <sc_bv<4>>  u_wstrb_cache;
    sc_signal <bool>      u_valid_cache;
    sc_signal <bool>      u_ready_cache;
 

    dut->clk(clock);
    dut->reset_n(reset_n);
    
    dut->w_imem_addr  (imem_addr);
    _dummy_imem_rw.write(0);
    dut->w_imem_rdata (imem_rdata);
    _dummy_imem_wdata.write(0);
    _dummy_imem_wstrb.write(0);
    dut->w_imem_valid (imem_valid);
    dut->w_imem_ready (imem_ready);

    dut->w_dmem_addr  (dmem_addr);
    dut->w_dmem_rw    (dmem_rw);
    dut->w_dmem_rdata (dmem_rdata);
    dut->w_dmem_wdata (dmem_wdata);
    dut->w_dmem_wstrb (dmem_wstrb);
    dut->w_dmem_valid (dmem_valid);
    dut->w_dmem_ready (dmem_ready);

    _dummy_imem_rw_cache.write(0);
    _dummy_imem_wdata_cache.write(0);
    _dummy_imem_wstrb_cache.write(0);
    
    icache->clk(clock);
    icache->valid_cpu(imem_valid);
    icache->valid_mem(imem_valid_cache);
    icache->ready_cpu(imem_ready);
    icache->ready_mem(imem_ready_cache);
    icache->rw_cpu(_dummy_imem_rw);
    icache->rw_mem(_dummy_imem_rw_cache);
    icache->wstrb_cpu(_dummy_imem_wstrb);
    icache->wstrb_mem(_dummy_imem_wstrb_cache);
    icache->address_cpu(imem_addr);
    icache->address_mem(imem_addr_cache);
    icache->wdata_cpu(_dummy_imem_wdata);
    icache->wdata_mem(_dummy_imem_wdata_cache);
    icache->rdata_cpu(imem_rdata);
    icache->rdata_mem(imem_rdata_cache);

    dcache->clk(clock);
    dcache->valid_cpu(dmem_valid);
    dcache->valid_mem(dmem_valid_cache);
    dcache->ready_cpu(dmem_ready);
    dcache->ready_mem(dmem_ready_cache);
    dcache->rw_cpu(dmem_rw);
    dcache->rw_mem(dmem_rw_cache);
    dcache->wstrb_cpu(dmem_wstrb);
    dcache->wstrb_mem(dmem_wstrb_cache);
    dcache->address_cpu(dmem_addr);
    dcache->address_mem(dmem_addr_cache);
    dcache->wdata_cpu(dmem_wdata);
    dcache->wdata_mem(dmem_wdata_cache);
    dcache->rdata_cpu(dmem_rdata);
    dcache->rdata_mem(dmem_rdata_cache);

    ucache->clk(clock);
    ucache->address_cpu(u_addr);
    ucache->rw_cpu(u_rw);
    ucache->wstrb_cpu(u_wstrb);
    ucache->rdata_cpu(u_rdata);
    ucache->ready_cpu(u_ready);
    ucache->valid_cpu(u_valid);
    ucache->wdata_cpu(u_wdata);

    ucache->address_mem(u_addr_cache);
    ucache->wdata_mem(u_wdata_cache);
    ucache->rw_mem(u_rw_cache);
    ucache->wstrb_mem(u_wstrb_cache);
    ucache->valid_mem(u_valid_cache);
    ucache->ready_mem(u_ready_cache);
    ucache->rdata_mem(u_rdata_cache);


    bridge->address_1(imem_addr_cache);
    bridge->wdata_1(_dummy_imem_wdata_cache);
    bridge->rw_1(_dummy_imem_rw_cache);
    bridge->b_en_1(_dummy_imem_wstrb_cache);
    bridge->valid_1(imem_valid_cache);
    bridge->ready_1(imem_ready_cache);
    bridge->rdata_1(imem_rdata_cache);

    bridge->address_2(dmem_addr_cache);
    bridge->wdata_2(dmem_wdata_cache);
    bridge->rw_2(dmem_rw_cache);
    bridge->b_en_2(dmem_wstrb_cache);
    bridge->valid_2(dmem_valid_cache);
    bridge->ready_2(dmem_ready_cache);
    bridge->rdata_2(dmem_rdata_cache);

    bridge->address_o(u_addr);
    bridge->wdata_o(u_wdata);
    bridge->rw_o(u_rw);
    bridge->b_en_o(u_wstrb);
    bridge->valid_o(u_valid);
    bridge->ready_o(u_ready);
    bridge->rdata_o(u_rdata);
    
    ram->addr_0  (u_addr_cache);
    ram->rw_0    (u_rw_cache);
    ram->rdata_0 (u_rdata_cache);
    ram->wdata_0 (u_wdata_cache);
    ram->wstrb_0 (u_wstrb_cache);
    ram->valid_0 (u_valid_cache);
    ram->ready_0 (u_ready_cache);


/*
    ram->addr_1  (dmem_addr_cache);
    ram->rw_1    (dmem_rw_cache);
    ram->rdata_1 (dmem_rdata_cache);
    ram->wdata_1 (dmem_wdata_cache);
    ram->wstrb_1 (dmem_wstrb_cache);
    ram->valid_1 (dmem_valid_cache);
    ram->ready_1 (dmem_ready_cache);
  */  
    mon->clk     (clock);
    mon->i_addr  (imem_addr);
    mon->i_rdata (imem_rdata);
    mon->i_valid (imem_valid);
    mon->i_ready (imem_ready);
    mon->d_addr  (dmem_addr);
    mon->d_rw    (dmem_rw);
    mon->d_rdata (dmem_rdata);
    mon->d_wdata (dmem_wdata);
    mon->d_wstrb (dmem_wstrb);
    mon->d_valid (dmem_valid);
    mon->d_ready (dmem_ready);

    ram->mem[0x04 >> 2] = 0x01100f13; // li  x30, 0x0011
    ram->mem[0x08 >> 2] = 0x0f002f83; // lw  x31, 0xF0(x0)
    ram->mem[0x0C >> 2] = 0x01ff0fb3; // add x31, x30, x31
    ram->mem[0x10 >> 2] = 0x01ff0c63; // beq x30, x31, end
    ram->mem[0x14 >> 2] = 0x00000013; // nop
    ram->mem[0x18 >> 2] = 0x00000013; // nop
    ram->mem[0x1C >> 2] = 0x02800067; // jalr x0, x0, 0x28
    ram->mem[0x20 >> 2] = 0xfff00f93; // li  x31, -1
    ram->mem[0x24 >> 2] = 0xfff00f93; // li  x31, -1
    ram->mem[0x28 >> 2] = 0x0000006f; // end: jal x0, end
    ram->mem[0x2C >> 2] = 0xfff00f93; // li  x31, -1
    ram->mem[0x30 >> 2] = 0x00000013; // li  x31, -1
    ram->mem[0xF0 >> 2] = 0xdeadbeef;

    reset_n = 1;
    sc_start(10, SC_NS);
    reset_n = 0;
    sc_start(10, SC_NS);
    reset_n = 1;
    sc_start(310, SC_NS);

    icache->cache.print_entries();
    icache->logger.print();
    dcache->cache.print_entries();
    dcache->logger.print();
    ucache->cache.print_entries();
    ucache->logger.print();

    delete dut;
    delete ram;
    delete mon;
    delete icache;
    delete dcache;

    exit(EXIT_SUCCESS);
}
