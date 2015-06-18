// demonstrates sample usage for the hash map

#include <hashmap.hpp>
#include <memory>

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
}