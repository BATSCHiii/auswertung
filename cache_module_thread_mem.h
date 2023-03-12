#include "cache.h"
#include "logger.h"

class CacheModuleThread: sc_module {
public:
	// The clock of this cache, used to sync with the cpu
	sc_in<bool>       clk;

	sc_in<bool>       valid_cpu;
	sc_out<bool>      ready_cpu;
	sc_in<bool>       rw_cpu;
	sc_in<sc_bv<4>>   wstrb_cpu;
	sc_in<sc_bv<32>>  address_cpu;
	sc_in<sc_bv<32>>  wdata_cpu;
	sc_out<sc_bv<32>> rdata_cpu;

	sc_out<bool>       valid_mem;
	sc_in<bool>       ready_mem;
	sc_out<bool>      rw_mem;
	sc_out<sc_bv<4>>  wstrb_mem;
	sc_out<sc_bv<32>> address_mem;
	sc_out<sc_bv<32>> wdata_mem;
	sc_in<sc_bv<32>>  rdata_mem;

	// The actual cache, this module only handles the access to it
	Cache             cache;
	Logger            logger;

	// Needed for explicit constructor
	SC_HAS_PROCESS(CacheModuleThread);

	CacheModuleThread(sc_module_name n, CacheConfiguration config):
		sc_module(n),
		cache(name(), config)
	{
		logger.name = name();
		// Register the main loop as systemc thread
		SC_THREAD(handle_request);
	}

private:
	void handle_request() {
		while(1) {
			ready_cpu.write(0);
			// Wait for the next positive edge
			wait(clk.posedge_event());
			// Check if the cpu signals are valid
			if(valid_cpu) {
				// Store the signals 
				sc_bv<32> address = address_cpu.read();
				bool      rw = rw_cpu.read();
				sc_bv<4>  wstrb = wstrb_cpu.read();
				sc_bv<32> wdata = wdata_cpu.read();

				// Check if the address is currently cached
				bool cached = cache.includes_address(address);
				// Cache miss
				if(!cached) {
					if(!rw) {
						// Read miss
						logger.read_misses++;
					}
					else {
						// Write miss
						logger.write_misses++;
					}					

					// Get the corresponding block of data from memory
					std::vector<sc_uint<32>> data = 
						read_block_from_mem(address);	

					// If the address data is not cached first 
					// Write the whole block to the cache
					ReplacedBlock replaced = cache.write_block(
						address,
						data,
						false
					);
					// Only set if a replacement happened
					if(replaced.replaced) {
						logger.replacements++;
					}
					// If the address data is not cached first 
					// A return of a dirty block implies the cache had to
					//  replace a block and it has to be written to mem.

					if(replaced.replaced && replaced.dirty) {

						write_block_to_mem(replaced.address, replaced.data);

					}

					// Cache miss penalty 
					for(int i = 0; i < cache.config.miss_penalty; i++) {

						wait(clk.posedge_event());
					}
				}
				else {

					if(!rw) {
						logger.read_hits++;
					}
					else {
						logger.write_hits++;
					}
					// Cache hit penalty
					for(int i = 0; i < cache.config.hit_penalty; i++) {
						wait(clk.posedge_event());
					}
				}
				// From here the addresses block is guaranteed to be in 
				// the cache so we get the requested word of the cached block 

				sc_uint<32> requested = cache.read_word(address);

				// rw == 0 -> read operation
				if(!rw) {
					logger.reads++;
					// Write the requested word to the rdata signal
					rdata_cpu.write(requested);
				}
				// rw == 1 -> write operation
				else {
					logger.writes++;
					sc_uint<32> wstrb_uint = sc_uint<32>(wstrb);
					sc_uint<32> wdata_uint = sc_uint<32>(wdata);
					// Modify the data word according to write-strobes
					if(wstrb_uint & 0b0001) {
						requested.range( 7,  0) = wdata_uint.range( 7,  0);
					}
					if(wstrb_uint & 0b0010) {
						requested.range(15,  8) = wdata_uint.range(15,  8);
					}
					if(wstrb_uint & 0b0100) {
						requested.range(23, 16) = wdata_uint.range(23, 16);
					}
					if(wstrb_uint & 0b1000) {
						requested.range(31, 24) = wdata_uint.range(31, 24);
					}
					
					// Handle writing policy 
					bool dirty = false;
					// If it's `WriteBack` set dirty flag
					if(cache.config.write_policy == WriteBack) {
						dirty = true;
					}
					// It it's `WriteThrough` leave dirty flag == false
					//  and directly update the memory
					else if(cache.config.write_policy == WriteThrough) {
						// Need to wait a delta cycle, since a
						//  read and a write in the same cycle break the 
						//  bridge module. 
						wait(SC_ZERO_TIME);
						write_word_to_mem(address, requested);
					}
					// Write the modified data word to the cache.
					//  This operation cannot replace a different block
					//  since we already stored the block of this address
					//  earlier.
					cache.write_word(address, requested, dirty);
				}
				ready_cpu.write(1);
				wait(clk.posedge_event());
			}
		}
	}


	std::vector<sc_uint<32>> read_block_from_mem(sc_bv<32> address) {
		std::vector<sc_uint<32>> data =
			std::vector<sc_uint<32>>(cache.config.block_size);

		AddressInformation split_address = cache.deconstruct_address(address);
		for(int i = 0; i < cache.config.block_size; i++) {
			sc_bv<32> addr;
			addr.range(1, 0) = 0b00;
			if(cache.block_offset_bits != 0) {
				addr.range(cache.block_offset_bits + 1, 2) = i;
			}
			if(cache.index_bits != 0) {
				addr.range(cache.index_bits + cache.block_offset_bits + 1,
					       cache.block_offset_bits + 2) = split_address.index;
			}
			addr.range(31, cache.index_bits + cache.block_offset_bits + 2) 
				= split_address.tag;

			rw_mem.write(0);
			address_mem.write(addr);
			valid_mem.write(1);
			wait(ready_mem.posedge_event());
			valid_mem.write(0); 
			//wait(ready_mem.value_changed_event());
			wait(SC_ZERO_TIME);
			// if !ready -> wait(ready_changed)

			data[i] = sc_uint<32>(rdata_mem);
		}
		return data; 
	}


	void write_word_to_mem(sc_bv<32> address, sc_uint<32> data) {
		wdata_mem.write(data);
		rw_mem.write(1);
		wstrb_mem.write(0b1111);
		address_mem.write(address);
		valid_mem.write(1);
		wait(ready_mem.posedge_event());
		//wait(ready_mem.value_changed_event());
		valid_mem.write(0);
		//wait(SC_ZERO_TIME);
	}


	void write_block_to_mem(sc_bv<32> address, std::vector<sc_uint<32>> data) {
		sc_bv<32> addr = address;
		for(int i = 0; i < cache.config.block_size; i++) {
			addr.range(1, 0) = 0b00;
			if(cache.block_offset_bits) {
				addr.range(cache.block_offset_bits + 1, 2) = i;
			}
			write_word_to_mem(addr, data.at(i));
		}
	}
};
