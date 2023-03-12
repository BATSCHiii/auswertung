#include <iostream>
#include <map>
#include <time.h>
#include <stdlib.h>

class IReplacement {
	public:
		virtual ~IReplacement() {};
		virtual void insert(int) = 0;
		virtual void update(int) = 0;
		virtual int get_replacee() = 0;
};

class FirstInFirstOut: public IReplacement {
	public:
		FirstInFirstOut(int size) {
			this->size = size;
		}

		virtual void insert(int id) {
		}

		virtual void update(int id) {
		}

		virtual int get_replacee() {
			int replacee = counter;
			counter = (counter + 1) % size;
			return replacee;
		}

	private:
		int size;
		int counter = 0;
};

class Random: public IReplacement {
	public:
		Random(int size) {
			this->size = size;
			srand(time(nullptr)); // current time as seed
		}

		virtual void insert(int id) {
		}

		virtual void update(int id) {
		}

		virtual int get_replacee() {
			int replacee = rand() % size;
			return replacee;
		}

	private:
		int size;
		int counter = 0;
};


class LeastRecentlyUsed: public IReplacement {
	public:
		LeastRecentlyUsed() {
		}

		virtual void insert(int id) {
			last_access.insert(std::pair<int, int>(id, clk));
			clk++;
		}

		virtual void update(int id) {
			auto it = last_access.find(id);
			if(it != last_access.end()) {
				it->second = clk;
			}
			clk++;
		}

		virtual int get_replacee() {
			int candidate = 0;
			int candidate_access = clk;
			
			for(auto it = last_access.begin();
					it != last_access.end(); it++) {
				if(it->second < candidate_access) {
					candidate_access = it->second;
					candidate = it->first;
				}
			}

			last_access.erase(candidate);
			
			return candidate;
		}

	private:
		int clk = 0;
		std::map<int, int> last_access;
};

class LeastFrequentlyUsed: public IReplacement {
	public:
		LeastFrequentlyUsed() {
		}

		virtual void insert(int id) {
			access_count.insert(std::pair<int, int>(id, 0));
		}

		virtual void update(int id) {
			auto it = access_count.find(id);
			if(it != access_count.end()) {
				it->second += 1;
			}
		}

		virtual int get_replacee() {
			int candidate = 0;
			int candidate_count = 9999;
			
			for(auto it = access_count.begin();
					it != access_count.end(); it++) {
				if(it->second < candidate_count) {
					candidate_count = it->second;
					candidate = it->first;
				}
			}

			access_count.erase(candidate);
			
			return candidate;
		}

	private:
		std::map<int, int> access_count;
};
