#include <cstdlib>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "systemc.h"

#include "rv32_simple_core.h"
#include "cache_module.h"
#include "delay.h"


/* RAM Dual-Port, async read/write */
SC_MODULE (ram_dp_ar_aw) {
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

    void process_1() {
		std::cout << "TRIGGERED AT " << sc_time_stamp() << std::endl;
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
    }

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

        SC_METHOD (process_1);
        sensitive << addr_1 << valid_1 << rw_1 << wdata_1 << wstrb_1;
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

	Delay *delay1 = new Delay("Delay_1");
    Delay *delay2 = new Delay("Delay_2");


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
    
	delay1->clk(clock);
	delay1->valid_cpu(imem_valid);
	delay1->valid_mem(imem_valid_cache);
	delay1->ready_cpu(imem_ready);
	delay1->ready_mem(imem_ready_cache);
	delay1->rw_cpu(_dummy_imem_rw);
	delay1->rw_mem(_dummy_imem_rw_cache);
	delay1->wstrb_cpu(_dummy_imem_wstrb);
	delay1->wstrb_mem(_dummy_imem_wstrb_cache);
	delay1->address_cpu(imem_addr);
	delay1->address_mem(imem_addr_cache);
	delay1->wdata_cpu(_dummy_imem_wdata);
	delay1->wdata_mem(_dummy_imem_wdata_cache);
	delay1->rdata_cpu(imem_rdata);
	delay1->rdata_mem(imem_rdata_cache);

	delay2->clk(clock);
	delay2->valid_cpu(dmem_valid);
	delay2->valid_mem(dmem_valid_cache);
	delay2->ready_cpu(dmem_ready);
	delay2->ready_mem(dmem_ready_cache);
	delay2->rw_cpu(dmem_rw);
	delay2->rw_mem(dmem_rw_cache);
	delay2->wstrb_cpu(dmem_wstrb);
	delay2->wstrb_mem(dmem_wstrb_cache);
	delay2->address_cpu(dmem_addr);
	delay2->address_mem(dmem_addr_cache);
	delay2->wdata_cpu(dmem_wdata);
	delay2->wdata_mem(dmem_wdata_cache);
	delay2->rdata_cpu(dmem_rdata);
	delay2->rdata_mem(dmem_rdata_cache);

    ram->addr_0  (imem_addr_cache);
    ram->rw_0    (_dummy_imem_rw_cache);
    ram->rdata_0 (imem_rdata_cache);
    ram->wdata_0 (_dummy_imem_wdata_cache);
    ram->wstrb_0 (_dummy_imem_wstrb_cache);
    ram->valid_0 (imem_valid_cache);
    ram->ready_0 (imem_ready_cache);

    ram->addr_1  (dmem_addr_cache);
    ram->rw_1    (dmem_rw_cache);
    ram->rdata_1 (dmem_rdata_cache);
    ram->wdata_1 (dmem_wdata_cache);
    ram->wstrb_1 (dmem_wstrb_cache);
    ram->valid_1 (dmem_valid_cache);
    ram->ready_1 (dmem_ready_cache);
    
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

/*
        li  x30, 0xDEADBEEF
        sw  x30, 0xF0(x0)
        li  x29, 0xDEADBEE0
label:  lw  x31, 0xF0(x0)
        addi x29, x29, 0x1
        addi x29, x29, 0x1
        addi x29, x29, 0x1
        bne  x31, x29, label
        nop

*/

    ram->mem[0x00 >> 2] = 0xDEADCF37; 
    ram->mem[0x04 >> 2] = 0xEEFF0F13; 
    ram->mem[0x08 >> 2] = 0x0FE02823; 
    ram->mem[0x0C >> 2] = 0xDEADCEB7; 
    ram->mem[0x10 >> 2] = 0xEE0E8E93; 
    ram->mem[0x14 >> 2] = 0x0F002F83; 
    ram->mem[0x18 >> 2] = 0x001E8E93; 
    ram->mem[0x1C >> 2] = 0x001E8E93; 
    ram->mem[0x20 >> 2] = 0x001E8E93;
    ram->mem[0x24 >> 2] = 0xFFdF98E3;
    ram->mem[0x28 >> 2] = 0x00000013;

	reset_n = 1;
    sc_start(3000, SC_NS);
	
    delay1->logger.print();
    delay2->logger.print();


    delete dut;
    delete ram;
    delete mon;
	delete delay1;
    delete delay2;


    exit(EXIT_SUCCESS);
}
