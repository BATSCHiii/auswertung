#include "systemc.h"

/*
 *  This Module can be used to combine two separate caches into a single one.
 */
SC_MODULE(Bridge) {
	// Signals of the first cache.
	sc_in<bool> valid_1;
	sc_out<bool> ready_1;
	sc_in<sc_bv<32>> address_1;
	sc_out<sc_bv<32>> rdata_1;
	sc_in<sc_bv<32>> wdata_1;
	sc_in<bool> rw_1;
	sc_in<sc_bv<4>> b_en_1;

	// Signals of the second cache.
	sc_in<bool> valid_2;
	sc_out<bool> ready_2;
	sc_in<sc_bv<32>> address_2;
	sc_out<sc_bv<32>> rdata_2;
	sc_in<sc_bv<32>> wdata_2;
	sc_in<bool> rw_2;
	sc_in<sc_bv<4>> b_en_2;

	// Output signals to be connected to the third, combined cache.
	sc_out<bool> valid_o;
	sc_in<bool> ready_o;
	sc_out<sc_bv<32>> address_o;
	sc_in<sc_bv<32>> rdata_o;
	sc_out<sc_bv<32>> wdata_o;
	sc_out<bool> rw_o;
	sc_out<sc_bv<4>> b_en_o;

	// A classical mutex used to handle access to the output signals
	sc_mutex mutex;

	SC_CTOR(Bridge) {
		SC_THREAD(cache_1);
		SC_THREAD(cache_2);
	}

	void cache_1() {
		ready_1.write(0);
		while(1) {
			// Wait for caches signals to be stable
			wait(valid_1.posedge_event());
			// Try to lock the mutex, -1 -> mutex is already in use
			if(mutex.trylock() == -1) {
				// This will suspend the process until the mutex is freed 
				//  and lock it.
				std::cout << "WAITING 1" << std::endl;
				mutex.lock();
			}
			std::cout << "responding ot add " << hex <<address_1  << std::endl;
			// Set the output signals to the input from the first cache
			address_o.write(address_1);
			valid_o.write(1); 
			wdata_o.write(wdata_1);
			rw_o.write(rw_1);
			b_en_o.write(b_en_1);

			// Wait for the unified cache to set the ready signal
			wait(ready_o.posedge_event());
			
			// Unset the valid signal
			valid_o.write(0);

			// Set the ready signal of the first cache
			ready_1.write(1);
			// Pass the data from the unified cache to the first cache
			rdata_1.write(rdata_o);

			// Need to wait a delta cycle so the signals are updated before
			//  we can unlock the mutex and unset the ready signal.
			wait(SC_ZERO_TIME);
			std::cout << "responding ot add " << hex <<address_1  << "DONE "<< std::endl;

			ready_1.write(0);
			
			mutex.unlock();
		}
	}


	void cache_2() {
		ready_2.write(0);
		while(1) {
			// Wait for caches signals to be stable
			wait(valid_2.posedge_event());
			// Try to lock the mutex, -1 -> mutex is already in use
			if(mutex.trylock() == -1) {
				// This will suspend the process until the mutex is freed 
				//  and lock it.
								std::cout << "WAITING 2" << std::endl;

				mutex.lock();
			}
			// Set the output signals to the input from the second cache
			address_o.write(address_2);
			valid_o.write(1); 
			wdata_o.write(wdata_2);
			rw_o.write(rw_2);
			b_en_o.write(b_en_2);

			// Wait for the unified cache to set the ready signal
			wait(ready_o.posedge_event());
			
			// Unset the valid signal
			valid_o.write(0);

			// Set the ready signal of the second cache
			ready_2.write(1);
			// Pass the data from the unified cache to the second cache
			rdata_2.write(rdata_o);

			// Need to wait a delta cycle so the signals are updated before
			//  we can unlock the mutex and unset the ready signal.
			wait(SC_ZERO_TIME);
			ready_2.write(0);
			
			mutex.unlock();
		}
	}
};
