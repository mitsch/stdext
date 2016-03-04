/// @file dictionary.hpp
/// @author Michael Koch
/// @copyright CC BY 3.0


#ifndef __STDEXT_DICTIONARY_HPP__
#define __STDEXT_DICTIONARY_HPP__

#include <stdext/callable.hpp>
#include <stdext/allocator.hpp>
#include <stdext/optional.hpp>

namespace stdext
{

	template <typename K, typename V, typename H, typename A, typename M, std::size_t L>
		requires Callable<H, std::size_t, K> and
		         Allocator<A> and
						 Callable<M, bool, const K&, const K&>
	class basic_dictionary
	{
	
		private:

			K * keys = nullptr;
			V * values = nullptr;
			std::size_t * indices = nullptr;
			std::size_t used = 0;
			std::size_t max = 0;
			H hasher;
			M matcher;
			A allocator;


			
			constexpr std::size_t linear_lookup (K key) const
			{
				assert(used <= L);
				assert(used == 0 or (keys != nullptr and values != nullptr));
				assert(used <= max);

				auto index = std::size_t(0);
				while (index < used and not matcher(key, keys[index]))
					++index;
				return index;
			}

			constexpr std::tuple<std::size_t, std::size_t> hash_lookup (K key) const
			{
				assert(L == 0 or used > L);
				assert(used == 0 or (keys != nullptr and values != nullptr and indices != nullptr));
				assert(used <= max);

				const auto startingMetaIndex = hasher(key) % max;
				auto metaIndex = startingMetaIndex;

				while (metaIndex < max and indices[metaIndex] != max and not matcher(key, keys[metaIndex]))
					++metaIndex;

				if (metaIndex == max)
				{
					metaIndex = 0;
					while (metaIndex < startingMetaIndex and indices[metaIndex] != max and not matcher(key, keys[metaIndex]))
						++metaIndex;

					if (metaIndex == startingMetaIndex)
						return std::make_tuple(max, max);
				}
			
				const auto index = indices[metaIndex];
				return std::make_tuple(index, metaIndex);
			}

			template <BoundedSequence<std::tuple<K, V>> S>
			static constexpr std::tuple<K*, V*, std::size_t*, std::size_t, std::size_t> construct (S sequence, A & allocator, H & hasher)
			{
				const auto count = length(sequence);

				if (count <= L and count > 0)
				{
					auto keyAllocation = allocator.allocate(sizeof(K) * count);
					auto valueAllocation = allocator.allocate(sizeof(V) * count);

					if (keyAllocation.length() >= (count * sizeof(K)) or valueAllocation.length() >= (count * sizeof(V)))
					{
						K * keys = static_cast<K*>(keyAllocation.data());
						V * values = static_cast<V*>(valueAllocation.data());
						const auto maxKeys = keyAllocation.length() / sizeof(K);
						const auto maxValues = valueAllocation.length() / sizeof(V);
						const auto max = maxKeys > maxValues ? maxValues : maxKeys;

						fold([&](auto index, auto pair)
						{
							assert(index < count);
							new (keys + index) K(std::move(std::get<0>(pair)));
							new (values + index) V(std::move(std::get<1>(pair)));
							return index + 1;
						}, std::size_t(0), std::move(sequence));

						return std::make_tuple(keys, values, nullptr, count, max);
					}
					else
					{
						allocator.deallocate(keyAllocation);
						allocator.deallocate(valueAllocation);
						throw bad_alloc();
					}
				}
			}


		public:


			constexpr optional<V&> get (K key)
			{
				if (used <= L)
				{
					const auto index = linear_lookup(std::move(key));
					return index == used ? optional<V&>() : optional<V&>(values[index]);
				}
				else
				{
					const auto indices = hash_lookup(std::move(key));
					const auto index = std::get<0>(indices);
					return index == max ? optional<V&>() : optional<V&>(values[index]);
				}
			}

			constexpr optional<const V&> get (K key) const
			{
				if (used <= L)
				{
					const auto index = linear_lookup(std::move(key));
					return index == used ? optional<const V&>() : optional<const V&>(values[index]);
				}
				else
				{
					const auto indices = hash_lookup(std::move(key));
					const auto index = std::get<0>(indices);
					return index == max ? optional<const V&>() : optional<const V&>(values[index]);
				}
			}

			template <Callable<T, T> C>
			constexpr void transform (C transformer, K key)
			{
				if (used <= L)
				{
					const auto index = linear_lookup(std::move(key));
					if (index < used) values[index] = transformer(std::move(values[index]));
				}
				else
				{
					const auto indices = hash_lookup(std::move(key));
					const auto index = std::get<0>(indices);
					if (index != max) values[index] = transformer(std::move(values[index]));
				}
			}

			template <Callable<V, const K&, V> C, BoundedSequence<K> S>
			constexpr void transform (C transformer, S keys)
			{}



			constexpr void set (K key, V value);
			
			template <BoundedSequence<std::tuple<K, V>> S>
			constexpr void set (S pairs);

			template <Callable<V, V, V> C>
			constexpr void set (C merger, K key, V value);

			template <Callable<V, const K&, V, V> C, BoundedSequence<std::tuple<K, V>> S>
			constexpr void set (C merger, S pairs);

	
	};


}

#endif

