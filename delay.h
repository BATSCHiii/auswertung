#include "systemc.h"
#include "logger.h"

SC_MODULE(Delay) {
	sc_in<bool>       clk;

	sc_in<bool>       valid_cpu;
	sc_out<bool>      ready_cpu;
	sc_in<bool>       rw_cpu;
	sc_in<sc_bv<4>>   wstrb_cpu;
	sc_in<sc_bv<32>>  address_cpu;
	sc_in<sc_bv<32>>  wdata_cpu;
	sc_out<sc_bv<32>> rdata_cpu;

	sc_out<bool>      valid_mem;
	sc_in<bool>       ready_mem;
	sc_out<bool>      rw_mem;
	sc_out<sc_bv<4>>  wstrb_mem;
	sc_out<sc_bv<32>> address_mem;
	sc_out<sc_bv<32>> wdata_mem;
	sc_in<sc_bv<32>>  rdata_mem;
	int delay_cycles;
	Logger logger;

	SC_CTOR(Delay) {
		SC_THREAD(thread);	
	}

	void thread() {
		int delay_cycles = 100;
		while(1) {
			ready_cpu.write(0);
			// ONLY FOR EVALUATION! STOP AT NOP INSTRUCTION
			if(sc_uint<32>(rdata_cpu) == 0x00000013) {
				sc_stop();
			}
			wait(clk.posedge_event());
			if(valid_cpu) {
				if(!rw_cpu) {
					logger.reads++;
					logger.read_misses++;
					rw_mem.write(0);
					valid_mem.write(1);
					address_mem.write(address_cpu);
					wait(ready_mem.value_changed_event());
					valid_mem.write(0);
					rdata_cpu.write(rdata_mem.read());
				}
				else {
					logger.writes++;
					logger.write_misses++;
					wdata_mem.write(wdata_cpu);
					rw_mem.write(1);
					wstrb_mem.write(wstrb_cpu);
					valid_mem.write(1);
					address_mem.write(address_cpu);
					wait(ready_mem.value_changed_event());
					valid_mem.write(0);
				}
				for(int i = 0; i < delay_cycles; i++) {
					wait(clk.posedge_event());
				}
				ready_cpu.write(1);
				wait(clk.posedge_event());
			}	
		}
	}
};

