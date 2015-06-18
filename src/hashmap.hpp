/*** Lightness1024! ProgrammatO E.U.R.L.      ***/
/*** 08/2014                                  ***/
/*** © Vivien Oddou                           ***/


/*
//-----------------------------------------------------------------------
Ce logiciel est régi par la licence CeCILL soumise au droit français et
respectant les principes de diffusion des logiciels libres. Vous pouvez
utiliser, modifier et/ou redistribuer ce programme sous les conditions
de la licence CeCILL telle que diffusée par le CEA, le CNRS et l'INRIA
sur le site "http://www.cecill.info".

(...)

Le fait que vous puissiez accéder à cet en-tête signifie que vous avez
pris connaissance de la licence CeCILL, et que vous en avez accepté les
termes.
//-----------------------------------------------------------------------
*/

// the behavior of this map is very largely based on C++11 unordered_map


#ifndef HASHMAP_SERHUM_INCLUDEGUARD_L1024_82014
#define HASHMAP_SERHUM_INCLUDEGUARD_L1024_82014 1

#include <exception>
#include <new>
#include <tuple>
#include "pool.hpp"
#include "myutils.hpp"

namespace container
{
	template< typename MapValueType >
	struct map_buckets : boost::noncopyable
	{
		typedef MapValueType	value_type;
		
		~map_buckets()
		{
			size_t i = 0;
			size_t lim = array.size();
			for ( ; i != lim; ++i)
			{
				if (occupancy[i])
					array[i].~value_type();
			}
		}

		std::vector< bool >		occupancy;
		pool< value_type >		array;

		void resize(size_t n)
		{
			_ASSERT(array.size() == 0);  // cannot really resize. just set from 0.
			array.resize(n);
			occupancy.resize(n, false);
		}

		void swap(map_buckets& r)
		{
			occupancy.swap(r.occupancy);
			array.swap(r.array);
		}
	};

	//============================================================================
	// class hash_map<..>::iterator
	//============================================================================
	template< typename ObservedPairType, typename SourcePairType >
	class hash_map_iterator
	{
	public:
		typedef ObservedPairType pair_type;
		typedef typename pair_type::first_type	 key_t;
		typedef typename pair_type::second_type	 mapped_t;

		typedef ptrdiff_t difference_type;
		typedef pair_type value_type;
		typedef pair_type* pointer;
		typedef pair_type& reference;
		typedef std::bidirectional_iterator_tag iterator_category;

		static_assert(std::is_same< typename std::remove_const<key_t>::type,
									typename std::remove_const< typename SourcePairType::first_type >::type >::value,
					  "the both template arguments must have the same type, except qualification");

		static_assert(std::is_same< typename std::remove_const<mapped_t>::type,
									typename std::remove_const< typename SourcePairType::second_type >::type >::value,
					  "the both template arguments must have the same type, except qualification");

		hash_map_iterator();
		hash_map_iterator(map_buckets<SourcePairType>* buckets);
		hash_map_iterator(map_buckets<SourcePairType>* buckets, size_t current_offset);
		template< typename Obs2T >
		hash_map_iterator(hash_map_iterator< Obs2T, SourcePairType> const& copyfrom);

		pair_type*			operator-> () const;
		pair_type&			operator*  () const;
		hash_map_iterator&	operator++ ();  // prefix
		hash_map_iterator	operator++ (int);  // postfix
		bool				operator== (hash_map_iterator const& rhs) const;
		bool				operator!= (hash_map_iterator const& rhs) const;
		hash_map_iterator&	operator = (hash_map_iterator const& rhs);

		template< typename Ob, typename NCPT >
		friend class hash_map_iterator;
	private:
		pair_type* current;
		map_buckets<SourcePairType>* buckets_ref;
	};

#define ITER_TPL_DECL template< typename ObservedPairType, typename SourcePairType >
#define ITER_DECL hash_map_iterator<ObservedPairType, SourcePairType>

	// default construction -> just set to null.
	ITER_TPL_DECL
	ITER_DECL::hash_map_iterator()
			: current(nullptr),
			  buckets_ref(nullptr)
	{}

	// construction from a hashmap -> gives the information about the bucket we will iterate.
	ITER_TPL_DECL
	ITER_DECL::hash_map_iterator(map_buckets<SourcePairType>* buckets)
			: current(nullptr),
			  buckets_ref(buckets)
	{}

	ITER_TPL_DECL
	ITER_DECL::hash_map_iterator(map_buckets<SourcePairType>* buckets, size_t current_offset)
		: buckets_ref(buckets)
	{
		current = buckets->array.size() == 0 ? nullptr : (pair_type*)&buckets->array[0] + current_offset;
	}

	// copy construction. it should be able to copy from const_iterator (therefore template)
	ITER_TPL_DECL
	template< typename Obs2T >
	ITER_DECL::hash_map_iterator(hash_map_iterator<Obs2T, SourcePairType> const& copyfrom)
		: current((pair_type*)copyfrom.current),
		  buckets_ref(copyfrom.buckets_ref)
	{
		static_assert(!(!std::is_const<pair_type>::value
			&& std::is_const<Obs2T>::value), "cannot copy const_iterator to iterator");
	}

	ITER_TPL_DECL
	typename ITER_DECL::pair_type* ITER_DECL::operator-> () const
	{
		return current;
	}

	ITER_TPL_DECL
	typename ITER_DECL::pair_type& ITER_DECL::operator* () const
	{
		return *current;
	}

	ITER_TPL_DECL
	ITER_DECL& ITER_DECL::operator++ ()
	{
		_ASSERT(current != nullptr);  // cannot increment a non initialized iterator.
		auto baseaddr = &buckets_ref->array[0];
		auto limit = baseaddr + buckets_ref->array.size();
		++current;
		while (current < (pair_type*)limit && !buckets_ref->occupancy[current - (pair_type*)baseaddr])
			++current;
		return *this;
	}

	ITER_TPL_DECL
	ITER_DECL ITER_DECL::operator++ (int)
	{
		hash_map_iterator it = *this;
		++(*this);
		return it;
	}

	ITER_TPL_DECL
	bool ITER_DECL::operator== (hash_map_iterator const& rhs) const
	{
		return current == rhs.current && buckets_ref == rhs.buckets_ref;
	}

	ITER_TPL_DECL
	bool ITER_DECL::operator!= (hash_map_iterator const& rhs) const
	{
		return !(*this == rhs);
	}

	ITER_TPL_DECL
	ITER_DECL& ITER_DECL::operator = (hash_map_iterator const& rhs)
	{
		current = rhs.current;
		buckets_ref = rhs.buckets_ref;
		return *this;
	}

#undef ITER_TPL_DECL
#undef ITER_DECL

	namespace detail
	{
		namespace
		{
			static size_t const primes[] = {13, 29, 47, 73, 131, 257, 503, 1021,
											2039, 4093, 7919, 16333, 32749,
											47143, 65293, 96487, 150559, 199967 };
		}
	}

	static size_t next_advised_bucket_count(size_t contenance, float max_load_factor)
	{
		static size_t const cnt = _countof(detail::primes);
		size_t minbuckets = (size_t)(contenance / max_load_factor);
		auto beg = &detail::primes[0];
		auto end = beg + cnt;
		auto it = std::upper_bound(beg, end, contenance);
		if (it == end)
			return (size_t)(minbuckets * 1.1113f);
		return *it;
	}

	//============================================================================
	// class hash_map
	//============================================================================
	template< typename Key,
		      typename MappedValue,
			  typename HashFunc = std::hash<Key>,
			  typename EqualFunc = std::equal_to<Key> >
	class hash_map
	{
	public:
		// all the standard typedefs (minus allocator):
		typedef std::pair< const Key, MappedValue > value_type;
		typedef std::pair< const Key, const MappedValue > const allconst_pair;
		typedef Key				key_type;		// the first template parameter(Key)
		typedef MappedValue		mapped_type;	// the second template parameter(MappedValue)
		typedef HashFunc		hasher;			// the third template parameter(HashFunc)	defaults to : std::hash<key_type>
		typedef EqualFunc		key_equal;		// the fourth template parameter(Pred)	defaults to : std::equal_to<key_type>
		typedef value_type&		reference;
		typedef reference const& const_reference;
		typedef value_type*		pointer;
		typedef pointer const	const_pointer;
		typedef hash_map_iterator<value_type,    value_type>  iterator;
		typedef hash_map_iterator<allconst_pair, value_type>  const_iterator;
		typedef size_t			size_type;
		typedef ptrdiff_t		difference_type;

		hash_map();
		hash_map(hash_map const& copyfrom);

		hash_map&			operator = (hash_map const& assignfrom);
		void				copy(hash_map const& assignfrom);

		size_type			size() const;
		bool				empty() const;
		mapped_type&		operator [] (key_type const& key);
		mapped_type const&	operator [] (key_type const& key) const;
		mapped_type&		at(const key_type& k);
		mapped_type const&	at(const key_type& k) const;
		std::pair<bool, iterator> insert(key_type const& key, mapped_type const& mapped_value);
		std::pair<bool, iterator> insert(const_reference value);
		// emplace 0
		std::pair<bool, iterator> emplace(key_type const& key);
		iterator                  emplace_pos(size_type pos, key_type const& key);
		// emplace 1
		template< typename MappedTypeCompatType >
		std::pair<bool, iterator> emplace(key_type const& key, MappedTypeCompatType const& mappedconstruct);
		template< typename MappedTypeCompatType >
		iterator            emplace_pos(size_type pos, key_type const& key, MappedTypeCompatType const& mappedconstruct);
		iterator			erase(const_iterator position);
		size_type			erase(key_type const& k);
		bool				has_key(key_type const& tosearch) const;
		void				clear();
		void				swap(hash_map& rhs);
		float				load_factor() const;
		float				max_load_factor() const;
		void				reserve(size_type n);  //!< please indicate intended contenance (actual buckets will be n / max_load_factor)
		void 				rehash(size_type n);  //!< please indicate directly number of buckets.
		key_equal			key_eq() const;
		// iterator access:
		iterator			begin();
		const_iterator		begin() const;
		const_iterator		cbegin() const;
		iterator			end();
		const_iterator		end() const;
		const_iterator		cend() const;
		iterator			find(key_type const& k);
		const_iterator 		find(key_type const& k) const;

	private:
		enum found_status { vacant, found, notfound, full_notfound, unset };
		found_status determine_found_status(size_t at, key_type const& k) const;
		std::pair<found_status, size_t> find_placement(key_type const& k) const;
		size_t find_next_occupied(size_t from) const;

		size_type count;
		map_buckets<value_type> buckets;
		hasher hash_fn;
		key_equal eq_fn;
#ifdef _DEBUG
		value_type* _dbgbuckets;
#endif
	};


inline size_t increment_modulo(size_t value, size_t limit)
{
	++value;
	if (value >= limit)
		value = 0;
	return value;
}

#define HASHMAP_TPL_DECL	template< typename Key,\
									  typename MappedValue,\
									  typename HashFunc,\
									  typename EqualFunc >

#define HASHMAP_DECL hash_map<Key, MappedValue, HashFunc, EqualFunc>

	HASHMAP_TPL_DECL
	HASHMAP_DECL::hash_map()
		: count(0)
	{}

	HASHMAP_TPL_DECL
	HASHMAP_DECL::hash_map(hash_map const& copyfrom)
		: count(0)
	{
		copy(copyfrom);
	}

	HASHMAP_TPL_DECL
	HASHMAP_DECL& HASHMAP_DECL::operator = (hash_map const& assignfrom)
	{
		clear();
		copy(assignfrom);
		return *this;
	}

	HASHMAP_TPL_DECL
	void HASHMAP_DECL::copy(hash_map const& assignfrom)
	{
		auto it = assignfrom.cbegin();
		auto end = assignfrom.cend();
		for (; it != end; ++it)
		{
			bool did = emplace(it->first, it->second).first;
			if (did)
				++count;
		}
	}

	HASHMAP_TPL_DECL
	typename HASHMAP_DECL::found_status HASHMAP_DECL::determine_found_status(size_t at, key_type const& k) const
	{
		found_status st = notfound;
		if (buckets.occupancy.size() == 0)
			return full_notfound;
		bool occupied = buckets.occupancy[at];
		bool keyequal = occupied ? eq_fn(k, buckets.array[at].first) : false;
		if (keyequal)
			st = found;
		else if (!occupied)
			st = vacant;
		return st;
	}

	HASHMAP_TPL_DECL
	std::pair<typename HASHMAP_DECL::found_status, size_t> HASHMAP_DECL::find_placement(key_type const& k) const
	{
		size_t const limit   = buckets.array.size();
		if (limit == 0)
			return std::make_pair(full_notfound, 0);
		size_t const h       = hash_fn(k);
		size_t       pos     = h % limit;
		found_status status  = determine_found_status(pos, k);
		size_t       loopcnt = 0;
		while (status == notfound && loopcnt < limit)
		{
			// check next (linear probing):
			pos = increment_modulo(pos, limit);
			status = determine_found_status(pos, k);
			++loopcnt;
		}
		if (status == notfound && loopcnt == limit)
			status = full_notfound;
		_ASSERT(status == vacant || status == found || status == full_notfound);  // this function's invariant.
		return std::make_pair(status, pos);
	}

	HASHMAP_TPL_DECL
	size_t HASHMAP_DECL::find_next_occupied(size_t from) const
	{
		size_t lim = buckets.array.size();
		size_t nextvalid = increment_modulo(from, lim);
		size_t loop = 1;
		while (loop < lim && !buckets.occupancy[nextvalid])
		{
			nextvalid = increment_modulo(nextvalid, lim);
			++loop;
		}
		if (loop == lim)
			nextvalid = lim;  // end.
		return nextvalid;
	}

	HASHMAP_TPL_DECL
	typename HASHMAP_DECL::size_type HASHMAP_DECL::size() const
	{
		return count;
	}

	HASHMAP_TPL_DECL
	bool HASHMAP_DECL::empty() const
	{
		return size() == 0;
	}

	HASHMAP_TPL_DECL
	typename HASHMAP_DECL::mapped_type& HASHMAP_DECL::operator [] (key_type const& key)
	{
		auto stat_pos = find_placement(key);
		if (stat_pos.first != found)  // need to insert if not found. (operator [] 's responsibility)
		{
			auto res = emplace_pos(stat_pos.second, key);
			return res->second;
		}
		_ASSERT(stat_pos.first == found);
		return buckets.array[stat_pos.second].second;
	}

	HASHMAP_TPL_DECL
	typename HASHMAP_DECL::mapped_type const& HASHMAP_DECL::operator [] (key_type const& key) const
	{
		return at(key);
	}

	HASHMAP_TPL_DECL
	typename HASHMAP_DECL::mapped_type& HASHMAP_DECL::at(const key_type& k)
	{
		// call const version implementation and cast return value to non const. (factorizes code better)
		return const_cast<mapped_type&>(const_cast<hash_map const*>(this)->at(k));
	}

	HASHMAP_TPL_DECL
	typename HASHMAP_DECL::mapped_type const& HASHMAP_DECL::at(const key_type& key) const
	{
		auto stat_pos = find_placement(key);
		if (stat_pos.first != found)
		{
			throw std::out_of_range("try to access a non existing element");
		}
		_ASSERT(stat_pos.first == found);
		return buckets.array[stat_pos.second].second;
	}

	HASHMAP_TPL_DECL
	std::pair<bool, typename HASHMAP_DECL::iterator> HASHMAP_DECL::insert(key_type const& key, mapped_type const& mapped_value)
	{
		return emplace(key, mapped_value);
	}

	HASHMAP_TPL_DECL
	std::pair<bool, typename HASHMAP_DECL::iterator> HASHMAP_DECL::insert(const_reference value)
	{
		return emplace(value.first, value.second);
	}

	HASHMAP_TPL_DECL
	std::pair<bool, typename HASHMAP_DECL::iterator> HASHMAP_DECL::emplace(key_type const& key)
	{
		return emplace(key, mapped_type());
	}

	HASHMAP_TPL_DECL
	template< typename MappedTypeCompatType >
	std::pair<bool, typename HASHMAP_DECL::iterator> HASHMAP_DECL::emplace(key_type const& key, MappedTypeCompatType const& mappedconstruct)
	{
		auto stat_pos = find_placement(key);
		return std::make_pair(stat_pos.first == found, emplace_pos(stat_pos.second, key, mappedconstruct));
	}

	HASHMAP_TPL_DECL
	typename HASHMAP_DECL::iterator HASHMAP_DECL::emplace_pos(size_type pos, key_type const& key)
	{
		return emplace_pos(pos, key, mapped_type());
	}

	HASHMAP_TPL_DECL
	template< typename MappedTypeCompatType >
	typename HASHMAP_DECL::iterator HASHMAP_DECL::emplace_pos(size_type pos, key_type const& key, MappedTypeCompatType const& mappedconstruct)
	{
		found_status status = determine_found_status(pos, key);  // check the pos hint
		if (status == notfound)
		{  // it was a bad hint
			std::tie(status, pos) = find_placement(key);
		}
		if (status == found)
		{
			// overwrite value.
			buckets.array[pos].second = mappedconstruct;
			return iterator(&buckets, pos);
		}
		else
		{
			if (float(count + 1) / max_load_factor() >= buckets.array.size())  // not enough space to guarantee a correct load factor
			{
				rehash(next_advised_bucket_count(count < 30000 ? (count + 1) * 2 : count * 3 / 2, max_load_factor()));
				std::tie(status, pos) = find_placement(key);
				_ASSERT(status == vacant);
			}
			_ASSERT(!buckets.occupancy[pos]);
			new (&buckets.array[pos]) value_type(key, mappedconstruct);
			buckets.occupancy[pos] = true;
			++count;
			return iterator(&buckets, pos);
		}
	}

	HASHMAP_TPL_DECL
	typename HASHMAP_DECL::iterator HASHMAP_DECL::erase(const_iterator position)
	{
		size_t pos = (value_type*)&(*position) - &buckets.array[0];
		_ASSERT(buckets.occupancy[pos]);
		buckets.array[pos].~value_type();
		buckets.occupancy[pos] = false;
		--count;
		pos = find_next_occupied(pos);
		return iterator(&buckets, pos);
	}

	HASHMAP_TPL_DECL
	typename HASHMAP_DECL::size_type HASHMAP_DECL::erase(key_type const& key)
	{
		auto stat_pos = find_placement(key);
		if (stat_pos.first == found)
		{
			erase(const_iterator(&buckets, stat_pos.second));
			return 1;
		}
		return 0;
	}

	HASHMAP_TPL_DECL
	bool HASHMAP_DECL::has_key(key_type const& tosearch) const
	{
		auto st = find_placement(tosearch);
		return st.first == found;
	}

	HASHMAP_TPL_DECL
	void HASHMAP_DECL::clear()
	{
		map_buckets<value_type> empty;
		buckets.swap(empty);
		count = 0;
	}

	HASHMAP_TPL_DECL
	void HASHMAP_DECL::swap(hash_map& rhs)
	{
		std::swap(count, rhs.count);
		buckets.swap(rhs.buckets);
		std::swap(hash_fn, rhs.hash_fn);
		std::swap(eq_fn, rhs.eq_fn);
#ifdef _DEBUG
		_dbgbuckets = buckets.array.size() ? &buckets.array[0] : nullptr;
#endif
	}

	HASHMAP_TPL_DECL
	float HASHMAP_DECL::load_factor() const
	{
		return count / (float)buckets.array.size();
	}

	HASHMAP_TPL_DECL
	float HASHMAP_DECL::max_load_factor() const
	{
		return 0.8f;
	}

	HASHMAP_TPL_DECL
	void HASHMAP_DECL::reserve(size_type n)  // intended contenance
	{
		rehash(size_t((float)n / max_load_factor()));
	}

	HASHMAP_TPL_DECL
	void HASHMAP_DECL::rehash(size_type n)  // num of buckets
	{
		size_t minbuckets_for_currentsize = size_t((float)size() / max_load_factor());
		if (n < minbuckets_for_currentsize)
			n = minbuckets_for_currentsize;
		if (n != buckets.array.size())
		{
			map_buckets<value_type> old_buckets;
			old_buckets.swap(buckets);
			count = 0;
			// set new size:
			buckets.resize(n);
			// iterate over old buckets and re-insert everything:
			size_t it = 0;
			size_t lim = old_buckets.array.size();
			while (it != lim)
			{
				if (old_buckets.occupancy[it])
					emplace(old_buckets.array[it].first, old_buckets.array[it].second);
				++it;
			}
#ifdef _DEBUG
			_dbgbuckets = &buckets.array[0];
#endif
		}
	}

	HASHMAP_TPL_DECL
	typename HASHMAP_DECL::key_equal HASHMAP_DECL::key_eq() const
	{
		return eq_fn;
	}

	HASHMAP_TPL_DECL
	typename HASHMAP_DECL::iterator HASHMAP_DECL::begin()
	{
		return iterator(&buckets, find_next_occupied(0));
	}

	HASHMAP_TPL_DECL
	typename HASHMAP_DECL::const_iterator HASHMAP_DECL::begin() const
	{
		return cbegin();
	}

	HASHMAP_TPL_DECL
	typename HASHMAP_DECL::const_iterator HASHMAP_DECL::cbegin() const
	{
		return const_iterator(const_cast<map_buckets<value_type>*>(&buckets), find_next_occupied(0));
	}

	HASHMAP_TPL_DECL
	typename HASHMAP_DECL::iterator HASHMAP_DECL::end()
	{
		return iterator(&buckets, buckets.array.size());
	}

	HASHMAP_TPL_DECL
	typename HASHMAP_DECL::const_iterator HASHMAP_DECL::end() const
	{
		return cend();
	}

	HASHMAP_TPL_DECL
	typename HASHMAP_DECL::const_iterator HASHMAP_DECL::cend() const
	{
		return const_iterator(const_cast<map_buckets<value_type>*>(&buckets), buckets.array.size());
	}

	HASHMAP_TPL_DECL
	typename HASHMAP_DECL::iterator HASHMAP_DECL::find(key_type const& k)
	{
		auto sttpos = find_placement(k);
		return iterator(&buckets, sttpos.first == found ? sttpos.pos : buckets.array.size());
	}

	HASHMAP_TPL_DECL
	typename HASHMAP_DECL::const_iterator HASHMAP_DECL::find(key_type const& k) const
	{
		auto sttpos = find_placement(k);
		return const_iterator(&buckets, sttpos.first == found ? sttpos.pos : buckets.array.size());
	}

#undef HASHMAP_TPL_DECL
#undef HASHMAP_DECL

}

#endif
