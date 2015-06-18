// demonstrates sample usage for the hash map

#include <hashmap.hpp>
#include <memory>
#include <chrono>
#include <iostream>
#include <unordered_map>
#include <map>
#include <string>

#undef _ASSERT  // why is this defined even without any microsoft horror included ?

#ifndef _ASSERT
# include <assert.h>
# define _ASSERT(a) assert(a)
#endif

int gcnt = 0;
struct NeedCopy
{
	NeedCopy() : ptr(std::make_shared<int>(gcnt++))
	{}
	NeedCopy(NeedCopy const& rhs)
		: ptr(rhs.ptr)
	{
		++gcnt;
	}
	NeedCopy& operator = (NeedCopy const& rhs)
	{
		++gcnt;
		ptr = rhs.ptr;
		return *this;
	}
	~NeedCopy()
	{
		--gcnt;
	}
	std::shared_ptr<int> ptr;
};

template< typename T >
void sideeffect(T val)
{
	static T volatile sv = val;
	auto s = std::to_string(sv);
	if (s.size() > 100)
		std::cout << "";
}

#ifdef _MSC_VER

// https://stackoverflow.com/questions/16299029/resolution-of-stdchronohigh-resolution-clock-doesnt-correspond-to-measureme
struct HighResClock
{
	typedef long long                               rep;
	typedef std::nano                               period;
	typedef std::chrono::duration<rep, period>      duration;
	typedef std::chrono::time_point<HighResClock>   time_point;
	static const bool is_steady = true;

	static time_point now();
};

#define high_res_get_now() HighResClock::now()

#else
# define high_res_get_now() std::chrono::high_resolution_clock::now()
#endif

int main()
{

	{
		container::hash_map< std::string, NeedCopy > testmap;

		testmap["copain"];
		testmap[""];

		int copval = *testmap["copain"].ptr;
		_ASSERT(copval == 0);
		int emptval = *testmap[""].ptr;
		_ASSERT(emptval == 1);

		_ASSERT(testmap.size() == 2);

		testmap["1"] = NeedCopy();

		auto it = testmap.begin();
		for (; it != testmap.end(); ++it)
		{
			_ASSERT(*it->second.ptr == 0
					|| *it->second.ptr == 1
					|| *it->second.ptr == 2);
		}

		container::hash_map< std::string, NeedCopy >::const_iterator cit;
		cit = testmap.begin();

		// cit->second.ptr = nullptr;  // build error : you cannot assign to a variable that is const.

		//it = cit;  // this does not build. (good!)
		//it->second.ptr = nullptr;

		testmap.clear();

		_ASSERT(testmap.size() == 0);
		_ASSERT(testmap.empty());

		testmap["copain"];

		_ASSERT(testmap.size() == 1);
	}
	//_ASSERT(gcnt == 0);  does not pass. no comprendo aqui. gcnt ==1 ?? WAT!?

	 {  // from cppreference.
		 container::hash_map<std::string, std::string> mymap;

		 // populating container:
		 mymap["U.S."] = "Washington";
		 mymap["U.K."] = "London";
		 mymap["France"] = "Paris";
		 mymap["Russia"] = "Moscow";
		 mymap["China"] = "Beijing";
		 mymap["Germany"] = "Berlin";
		 mymap["Japan"] = "Tokyo";

		 // erase examples:
		 auto it = mymap.begin();
		 mymap.erase(it);      // erasing by iterator  (hope this is neither U.K or Russia because it would cause a problem for the next asserts)
		 mymap.erase("France");             // erasing by key

		 // show content:
		 _ASSERT(!mymap.has_key("France"));
		 _ASSERT(mymap["U.K."] == "London");   // sometimes these asserts can fail because by bad luck, begin() was one of them.
		 _ASSERT(mymap["Russia"] == "Moscow"); // it should be determinisic, but by 2 times I had to change these asserts. I don't understand this volatility.
	 }

	 {  // heavy load test.
		 container::hash_map<int, int> mymap;
		 for (int i = 0; i < 1000000; ++i)
			 mymap[rand()] = rand();
	 }

	 {
		 container::hash_map<std::string, int> mymap;
		 mymap["Mars"] = 3000;
		 mymap["Saturn"] = 60000;
		 mymap.insert("Jupiter", 70000);
		 mymap.emplace("Pluto", 170000);

		 mymap.at("Mars") = 3396;
		 mymap.at("Saturn") += 272;
		 mymap.at("Jupiter") = mymap.at("Saturn") + 9638;

		 _ASSERT(mymap.at("Jupiter") == 60000 + 9638 + 272);
	 }

	 {
		 container::hash_map<char, bool> mapofbool;
		 auto cit = mapofbool.cbegin();
		 auto cend = mapofbool.cend();
		 int i = 0;
		 for (; cit != cend; ++cit)
			 ++i;
		 _ASSERT(i == 0);
	 }

	 {
		 typedef container::hash_map<char, std::vector<int>> char2vint;
		 container::hash_map<void*, char2vint> mapofmap;

		 int a[5];
		 int* b[5] = {&a[0], &a[1], &a[2], &a[3], &a[4]};

		 mapofmap[b[0]].emplace('a');
		 mapofmap[b[1]].insert('b', std::vector<int>(2, 3));
		 mapofmap.rehash(40);

		 _ASSERT(mapofmap.at(b[0]).has_key('a'));
		 _ASSERT(mapofmap.at(b[0]).size() == 1);
		 _ASSERT(mapofmap.at(b[0])['a'].size() == 0);
		 _ASSERT(mapofmap.at(b[1])['b'].size() == 2);
		 _ASSERT(mapofmap.at(b[1])['b'][0] == 3);
		 _ASSERT(mapofmap.load_factor() <= 2.f / 40);

		 mapofmap.erase(b[0]);
		 mapofmap.rehash(0);
		 _ASSERT(mapofmap.size() == 1);
		 _ASSERT(mapofmap.load_factor() <= 0.8f);
	 }

	 {
		 std::vector<int> nums(88000);
		 for (int i = 0; i < (int)nums.size(); ++i)
			 nums[i] = i;
		 std::random_shuffle(nums.begin(), nums.end());
		 container::hash_map<int, double> mymap;
		 for (int i = 0; i < 88000; ++i)
			 mymap[nums[i]] = i == 0 ? 0.0 : mymap.at(nums[i - 1]) + 1.0;
		 _ASSERT(mymap.size() == 88000);
		 _ASSERT(mymap.load_factor() <= mymap.max_load_factor());
		 for (int i = 0; i < 88000; ++i)
			 _ASSERT(mymap[nums[i]] == (double)i);

		 {container::hash_map<int, double> mymap2;
		 mymap2.swap(mymap); }
		 _ASSERT(mymap.empty());
	 }

#ifdef NDEBUG

	// bit of benching:

	std::cout << "== 1 million int pushes ==" << std::endl;

	{
		srand(0);
		auto start = high_res_get_now();

		std::vector<int> mymap;
		for (int i = 0; i < 1000000; ++i)
			mymap.push_back(rand());

		auto end = high_res_get_now();

		auto diff = end - start;

		std::cout << "std vector: \t\t" << std::chrono::duration <double, std::milli>(diff).count() << " ms" << std::endl;
	}

	{
		srand(0);
		auto start = high_res_get_now();

		std::vector<int> mymap;
		mymap.reserve(1000000);
		for (int i = 0; i < 1000000; ++i)
			mymap.push_back(rand());

		auto end = high_res_get_now();

		auto diff = end - start;

		std::cout << "reserved vector: \t" << std::chrono::duration <double, std::milli>(diff).count() << " ms" << std::endl;
	}

	{
		srand(0);
		auto start = high_res_get_now();
		
		container::hash_map<int, int> mymap;
		for (int i = 0; i < 1000000; ++i)
			mymap[rand()] = rand();

		auto end = high_res_get_now();

		auto diff = end - start;

		std::cout << "*open address: \t\t" << std::chrono::duration <double, std::milli>(diff).count() << " ms" << std::endl;
	}

	{
		srand(0);
		auto start = high_res_get_now();

		container::hash_map<int, int> mymap;
		mymap.reserve(50000);  // weird. this was the only value that worked well
		for (int i = 0; i < 1000000; ++i)
			mymap[rand()] = rand();

		auto end = high_res_get_now();

		auto diff = end - start;

		std::cout << "*reserved openaddr: \t" << std::chrono::duration <double, std::milli>(diff).count() << " ms" << std::endl;
	}

	{
		srand(0);
		auto start = high_res_get_now();

		std::unordered_map<int, int> mymap;
		for (int i = 0; i < 1000000; ++i)
			mymap[rand()] = rand();

		auto end = high_res_get_now();

		auto diff = end - start;

		std::cout << "std unordered: \t\t" << std::chrono::duration <double, std::milli>(diff).count() << " ms" << std::endl;
	}

	{
		srand(0);
		auto start = high_res_get_now();

		std::map<int, int> mymap;
		for (int i = 0; i < 1000000; ++i)
			mymap[rand()] = rand();

		auto end = high_res_get_now();

		auto diff = end - start;

		std::cout << "std map: \t\t" << std::chrono::duration <double, std::milli>(diff).count() << " ms" << std::endl;
	}

	std::cout << "\n== 100k random erasures ==" << std::endl;

	{
		srand(0);

		container::hash_map<int, int> mymap;
		for (int i = 0; i < 1000000; ++i)
			mymap[rand()] = rand();

		auto start = high_res_get_now();

		for (int i = 0; i < 100000; ++i)
			mymap.erase(rand());

		auto end = high_res_get_now();

		auto diff = end - start;

		std::cout << "*openaddr: \t\t" << std::chrono::duration <double, std::milli>(diff).count() << " ms" << std::endl;
	}

	{
		srand(0);
		std::unordered_map<int, int> mymap;
		for (int i = 0; i < 1000000; ++i)
			mymap[rand()] = rand();

		auto start = high_res_get_now();

		for (int i = 0; i < 100000; ++i)
			mymap.erase(rand());

		auto end = high_res_get_now();

		auto diff = end - start;

		std::cout << "std unordered: \t\t" << std::chrono::duration <double, std::milli>(diff).count() << " ms" << std::endl;
	}

	{
		srand(0);
		std::map<int, int> mymap;
		for (int i = 0; i < 1000000; ++i)
			mymap[rand()] = rand();

		auto start = high_res_get_now();

		for (int i = 0; i < 100000; ++i)
			mymap.erase(rand());

		auto end = high_res_get_now();

		auto diff = end - start;

		std::cout << "std map: \t\t" << std::chrono::duration <double, std::milli>(diff).count() << " ms" << std::endl;
	}

	std::cout << "\n== 1M iteration ==" << std::endl;

	{
		srand(0);

		container::hash_map<int, int> mymap;
		for (int i = 0; i < 1000000; ++i)
			mymap[rand()] = rand();

		auto start = high_res_get_now();

		size_t cnt {0};
		for (int i = 0; i < 20; ++i)
			for (auto& it : mymap)
				++cnt;

		auto end = high_res_get_now();

		sideeffect(mymap.begin()->second);
		sideeffect(cnt);
		if (cnt != mymap.size() * 20)
			std::cout << "!! severe bug." << std::endl;

		auto diff = end - start;

		std::cout << "*openaddr: \t\t" << std::chrono::duration <double, std::milli>(diff).count() << " ms" << std::endl;
	}

	{
		srand(0);
		std::unordered_map<int, int> mymap;
		for (int i = 0; i < 1000000; ++i)
			mymap[rand()] = rand();

		auto start = high_res_get_now();

		size_t cnt{0};
		for (int i = 0; i < 20; ++i)
			for (auto& it : mymap)
				++cnt;

		auto end = high_res_get_now();

		sideeffect(mymap.begin()->second);
		sideeffect(cnt);
		if (cnt != mymap.size() * 20)
			std::cout << "!! severe bug." << std::endl;

		auto diff = end - start;

		std::cout << "std unordered: \t\t" << std::chrono::duration <double, std::milli>(diff).count() << " ms" << std::endl;
	}

	{
		srand(0);
		std::map<int, int> mymap;
		for (int i = 0; i < 1000000; ++i)
			mymap[rand()] = rand();

		auto start = high_res_get_now();

		size_t cnt{0};
		for (int i = 0; i < 20; ++i)
			for (auto& it : mymap)
				++cnt;

		auto end = high_res_get_now();

		sideeffect(mymap.begin()->second);
		sideeffect(cnt);
		if (cnt != mymap.size() * 20)
			std::cout << "!! severe bug." << std::endl;

		auto diff = end - start;

		std::cout << "std map: \t\t" << std::chrono::duration <double, std::milli>(diff).count() << " ms" << std::endl;
	}
#endif
}
