#include <iomanip> // std::setw()
#include "systemc.h"

#include "cache.h"

Cache::Cache(std::string name, CacheConfiguration config) {
	this->name = name;
	this->config = config;

	int number_sets = config.lines / config.associativity;
// Just enough to represent every set
	index_bits = ceil(log2(number_sets));

	// Always 2, 32 bit = 4 byte => log_2(4) = 2
	byte_offset_bits = 2;

	// Enough to index every word in the block
	block_offset_bits = ceil(log2((double)config.block_size));

	if(32 - index_bits - block_offset_bits - byte_offset_bits) {
		
	}

	// Use all the remaining bits as the tag
	tag_bits = 32 - index_bits - block_offset_bits - byte_offset_bits;


	std::cout << "[" << name << "] NUMBER SETS: " 
		<< number_sets << std::endl;
	std::cout << "[" << name << "] INDEX BITS: " 
		<< index_bits << std::endl;
	std::cout << "[" << name << "] TAG BITS: " 
		<< tag_bits << std::endl;
	std::cout << "[" << name << "] BLOCK OFFSET BITS: " 
		<< block_offset_bits << std::endl;

	// Initialize all sets with an instance of a IReplacement and the vector
	for(int i = 0; i < number_sets; i++) {
		switch(config.replacement_policy) {
			case FIFO: 
				sets.push_back({
					std::vector<Block>(config.associativity),
					new FirstInFirstOut(config.associativity)
				});
				break;
			case LRU: 
				sets.push_back({
					std::vector<Block>(config.associativity),
					new LeastRecentlyUsed()
				});
				break;
			case LFU: 
				sets.push_back({
					std::vector<Block>(config.associativity),
					new LeastFrequentlyUsed()
				});
				break;
			case RND: 
				sets.push_back({
					std::vector<Block>(config.associativity),
					new Random(config.associativity)
				});
				break;
			default:
				std::cerr << "No valid replacement policy found!" << std::endl;
		}
		
		for(int j = 0; j < config.associativity; j++) {
			sets[i].entries[j] = {
				false,
				false,
				0,
				std::vector<sc_uint<32>>(config.block_size)
			};
		}
	}
}


/*
 *  Returns true if the address is currently in the cache
 */
bool Cache::includes_address(sc_bv<32> address) {
	AddressInformation split_address = deconstruct_address(address);
	// Loop over every entry of the indexed set
	for(int i = 0; i < config.associativity; i++) {
		Block tmp = sets[split_address.index].entries[i];
		if((tmp.tag == split_address.tag) && tmp.valid) {
			return true;
		}
	}
	return false;
}



sc_uint<32> Cache::read_word(sc_bv<32> address) {
	AddressInformation split_address = deconstruct_address(address);
		
	Block requested;

	for(int i = 0; i < config.associativity; i++) {
		Block tmp = sets[split_address.index].entries[i];

		if(tmp.tag == split_address.tag) {
			sets[split_address.index].replacement_policy->update(i);
			requested = tmp;	
			break;
		}
	}

	// If more than 1 word is included get the requested word
	if(block_offset_bits != 0) {
		return requested.data.at(split_address.block_offset);			
	}
	// `block_offset_bits` == 0 -> only 1 word per block
	else {
		return requested.data.at(0);
	}
}


/*
 *  Return the entry at the given address, 
 *   assuming it is curently in the cache.
 *   A call to this function will be counted
 *   as an access for the replacement policy.
 */
Block Cache::read_block(sc_bv<32> address) {
	AddressInformation split_address = deconstruct_address(address);
	// Loop over every entry of the indexed set
	// ??? for(int i = 0; i < (config.lines / config.associativity); i++) {
	for(int i = 0; i < config.associativity; i++) {
		Block tmp = sets[split_address.index].entries[i];
		if(tmp.tag == split_address.tag) {
			sets[split_address.index].replacement_policy->update(i);
			return tmp;
		}
	}
	return { false, false, 0, std::vector<sc_uint<32>>(0) };
}


/*
 *  Updates the value of a single word in the cache. 
 *   The block has to be in the cache already.
 *   Assumes a call to `read_word()` prior to calling this function
 *   to update the replacement_policy, this function will NOT call 
 *   `replacement_policy->update()`
 */
void Cache::write_word(sc_bv<32> address, sc_uint<32> data, bool dirty) {
	AddressInformation split_address = deconstruct_address(address);
		
	Block requested;

	if(!includes_address(address)) {
		std::cerr << "Trying to write to a non-cached address!" << std::endl;
		return;	
	}

	for(int i = 0; i < config.associativity; i++) {
		Block tmp = sets[split_address.index].entries[i];

		if(tmp.tag == split_address.tag) {
			// If more than 1 word is included get the requested word
			if(block_offset_bits != 0) {
				sets[split_address.index].entries[i]
					.data.at(split_address.block_offset) = data;
			}
			// `block_offset_bits` == 0 -> only 1 word per block
			else {
				sets[split_address.index].entries[i].data.at(0) = data;
			}
			sets[split_address.index].entries[i].dirty = dirty;
			break;
		}
	}

}


/*
 *  Writes a full block into the cache. Returns the replaced block if it was 
 *   necessary or an all 0 block. The caller of this function is supposed to
 *   write the `ReplacedBlock` to memory if it's dirty flag is 1.
 *   This function should only be called to insert an uncached block.
 */
ReplacedBlock Cache::write_block(sc_bv<32> address,
					             std::vector<sc_uint<32>> data,
		                         bool dirty
								 ) 
{
	AddressInformation split_address = deconstruct_address(address);
	Block block = { true, dirty, split_address.tag, data };

	// Not currently cached
	if(!includes_address(address)) {
		// Replace a non-valid entry
		for(int i = 0; i < config.associativity; i++) {
			Block tmp = sets[split_address.index].entries[i];
			if(tmp.valid == false) {
				sets[split_address.index].entries[i] = block;
				sets[split_address.index].replacement_policy->insert(i);
				// No replacement happened, return all 0 block.
				return { false, 0, std::vector<sc_uint<32>>(0), false };
			}
		}
		// No non-valid block found, find a replacee
		int replacee_index = sets[split_address.index]
			.replacement_policy->get_replacee();
		std::vector<sc_uint<32>> replacee_data = 
			std::vector<sc_uint<32>>(config.block_size);
		sc_bv<32> replacee_address = 0;
		bool      dirty = false;
		Block     replacee = sets[split_address.index].entries[replacee_index];

		// If dirty flag is set, assume WriteBack and sent data to mem (return)
		if(replacee.dirty) {
			dirty = true;
			replacee_data = replacee.data;

			// Reconstruct the address from index / tag
			replacee_address.range(0, 1) = 0b00; // Byte offset is 0
			
			if(block_offset_bits != 0) {
				// Block offset is 0
				replacee_address.range(block_offset_bits + 1, 2) = 0;
			}

			if(index_bits != 0) {
				// Set the index bits (same set -> same index)
				replacee_address.range(
					index_bits + block_offset_bits + 1,
					block_offset_bits + 2
				) = split_address.index;
			}

			// Set all the remaining bits to the tag of the replacee
			replacee_address.range(
				31, 
				index_bits + block_offset_bits + 2
			) = replacee.tag;
		}
		// Replace the chosen block with the new one
		sets[split_address.index].entries[replacee_index] = block;
		sets[split_address.index].replacement_policy->insert(replacee_index);

		return { dirty, replacee_address, replacee_data, true };
	}
	// This function should only be called to insert a new block!
	/*
	// Address was cached, just need to update it
	else {
		for(int i = 0; i < config.associativity; i++) {
			Block tmp = sets[split_address.index].entries[i];
			if(tmp.tag == split_address.tag) {
				sets[split_address.index].entries[i] = block;
				// The call to `replacement_policy->update()` was done when
				//  the data was retrieved before writing with `read_word()`
			}
		}
	}
	// No replacement happened, return all 0 block.
	return { false, 0, std::vector<sc_uint<32>>(0) };*/
	return { false, 0, std::vector<sc_uint<32>>(0), false };
}


/*
 *  Resetes all cache entries to 0.
 */
void Cache::reset_entries() {
	for(int i = 0; i < config.lines / config.associativity; i++) {
		for(int j = 0; j < config.associativity; j++) {
			sets[i].entries[j].valid = 0;
			sets[i].entries[j].dirty = 0;
			sets[i].entries[j].tag = 0;
			for(int k = 0; k < config.block_size; k++) {
				sets[i].entries[j].data.at(k) = 0;
			}
		}
	}
}



/*
 *  Prints the whole cache in a readable format to stdout.
 */
void Cache::print_entries() {
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << std::setw(25) << "********** " 
		<< name << " **********" << std::endl;
	std::cout << std::setw(12) << "set";
	std::cout << std::setw(12) << "valid";
	std::cout << std::setw(12) << "dirty";
	std::cout << std::setw(12) << "tag";
	std::cout << std::setw(config.block_size * 12) << "data";
	std::cout << std::endl;

	for(int i = 0; i < config.lines / config.associativity; i++) {
		for(int j = 0; j < config.associativity; j++) {
			std::cout << std::setw(12) << i;
			std::cout << std::setw(12) << sets[i].entries[j].valid;
			std::cout << std::setw(12) << sets[i].entries[j].dirty;
			std::cout << std::setw(12) << sets[i].entries[j].tag
				.to_string(SC_HEX);
			std::cout << std::setw(12);
			for(int k = 0; k < config.block_size; k++) {
				if(k != 0) {
					std::cout << "-";
				}
				std::cout << sets[i].entries[j].data
					.at(k).to_string(SC_HEX);
			}
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}

	std::cout << std::endl;
	std::cout << std::endl;
}


/*
 *  Returns all cacheblocks, including empty and not valid ones.
 *   Used for assertions in the unit tests.
 */
std::string Cache::entries_as_string() {
	std::string s = "";
	for(int i = 0; i < config.lines / config.associativity; i++) {
		for(int j = 0; j < config.associativity; j++) {
			s.append(std::to_string(i));
			s.append("-");
			s.append(std::to_string(sets[i].entries[j].valid));	
			s.append("-");
			s.append(std::to_string(sets[i].entries[j].dirty));	
			s.append("-");
			s.append(sets[i].entries[j].tag.to_string(SC_HEX));
			for(int k = 0; k < config.block_size; k++) {
				s.append("-");
				s.append(sets[i].entries[j].data.
						at(k).to_string(SC_HEX));
			}
			s.append("\n");
		}
	}
	return s;
}



/*
 *  Splits the given address into index and tag as specified
 *   by the index-/tag-/block_offset_bits
 */
AddressInformation Cache::deconstruct_address(sc_bv<32> address) {
	sc_bv<32> index;
	sc_bv<32> tag;
	sc_bv<32> block_offset;

	if(index_bits == 0) {
		index = 0;	
	}
	else {
		index = address.range(
			index_bits + block_offset_bits + 1, block_offset_bits + 2);
	}

	if(block_offset_bits == 0) {
		block_offset = 0;
	}
	else {
		block_offset = address.range(block_offset_bits + 1, 2);
	}

	tag = address.range(31, index_bits + block_offset_bits + 2);

	return {
		sc_uint<32>(index),
		sc_uint<32>(tag),
		sc_uint<32>(block_offset),
	};
}
