#ifndef __CACHE_HEADER__
#define __CACHE_HEADER__

#include "systemc.h"

#include "replacement.h"

/*
 *  All possible replacement policy options
 */
enum ReplacementPolicy {
	FIFO,
	LRU,
	LFU,
	RND,
};


/*
 *	All possible write policy options
 */
enum WritePolicy {
	WriteThrough,
	WriteBack,
};


/*
 *  Representing a single cache block / line.
 *   data vector size will be set to the block size specified in the cache  
 */
struct Block {
	bool					 valid;
	bool                     dirty;
	sc_uint<32>              tag;
	std::vector<sc_uint<32>> data;
};


/*
 *  Representing a set in the cache, how many entries this set has will
 *   be specified in the cache. Includes an instance of a replacement policy
 *   implementing the IReplaceme interface.
 */
struct Set {
	std::vector<Block>  entries;
	IReplacement       *replacement_policy;
};


/*
 *  This struct represents all the configurable options of the cache
 */
struct CacheConfiguration {
	sc_uint<32>	      lines;
	sc_uint<32>  	  associativity;
	sc_uint<32>	      block_size; // in words
	ReplacementPolicy replacement_policy;
	WritePolicy       write_policy;
	sc_uint<8>        miss_penalty; // in clock-cycles
	sc_uint<8>        hit_penalty; // in clock-cycle
};


/*
 *  This struct is used to describe a replaced Block,
 *   used to write dirty blocks to memory after recplacing
 */
struct ReplacedBlock {
	bool                     dirty;
	sc_bv<32>                address;
	std::vector<sc_uint<32>> data;
	bool                     replaced;
};


/*
 *  The cache only cares about the index / tag combination of an address
 */
struct AddressInformation {
	sc_uint<32> index;
	sc_uint<32> tag;
	sc_uint<32> block_offset;
};



class Cache {
public:
	// Holding all the configurable options of this specific cache
	CacheConfiguration config;

	// Specifying how many bits are used for the index
	sc_uint<8> index_bits;

	// Specifying how many bits are used for the tag
	sc_uint<8> tag_bits;
	
	// Specifying how many bits are used as byte-offset
	sc_uint<8> byte_offset_bits;
	
	// Specifying how many bits are used as block-offset
	sc_uint<8> block_offset_bits;

	// The name of the cache instance
	std::string name;

	Cache(std::string name, CacheConfiguration config);

	bool includes_address(sc_bv<32> address);

	sc_uint<32> read_word(sc_bv<32> address);

	Block read_block(sc_bv<32> address);

	void write_word(sc_bv<32> address, sc_uint<32> data, bool dirty);

	ReplacedBlock write_block(sc_bv<32> addres,
					    std::vector<sc_uint<32>> data,
					    bool dirty
	 	                );

	void reset_entries();

	void print_entries();
	
	std::string entries_as_string();

	AddressInformation deconstruct_address(sc_bv<32> address);

private:
	// This vector holds all the data in the cache
	std::vector<Set> sets; 
};

#endif
