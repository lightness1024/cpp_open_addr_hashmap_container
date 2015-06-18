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

#ifndef POOL_SERHUM_INCLUDEGUARD_L1024_82014
#define POOL_SERHUM_INCLUDEGUARD_L1024_82014 1

#include <algorithm>
#include <boost/noncopyable.hpp>

namespace container
{
	namespace detail
	{
		struct uninitialized_space : boost::noncopyable
		{
			uninitialized_space()
				: zone(nullptr),
				  slots(0)
			{}
			~uninitialized_space()
			{
				clear();
			}

			char* zone;
			size_t slots;

			template< typename T >
			void alloc_space(size_t size)
			{
				_ASSERT(zone == nullptr);
				zone = new char[sizeof(T) * size];
				slots = size;
			}

			void clear()
			{
				if (zone != nullptr)
					delete[] zone;
				zone = nullptr;
				slots = 0;
			}

			void swap(uninitialized_space& rhs)
			{
				std::swap(zone, rhs.zone);
				std::swap(slots, rhs.slots);
			}
		};
	}

	template< typename T >
	class pool
	{
	public:

		T& at(size_t position)
		{
			_ASSERT(position < memory.slots);
			return *reinterpret_cast<T*>(&memory.zone[sizeof(T)* position]);
		}

		T const& at(size_t position) const
		{
			_ASSERT(position < memory.slots);
			return *reinterpret_cast<T*>(&memory.zone[sizeof(T)* position]);
		}

		T& operator[](size_t position)
		{
			return at(position);
		}

		T const& operator[](size_t position) const
		{
			return at(position);
		}

		size_t size() const { return memory.slots; }
		
		void resize(size_t n) { memory.alloc_space<T>(n); }

		void swap(pool& r)
		{
			memory.swap(r.memory);
		}

	private:
		detail::uninitialized_space memory;
	};
}

#endif // #define POOL_SERHUM_INCLUDEGUARD_L1024_82014 1
