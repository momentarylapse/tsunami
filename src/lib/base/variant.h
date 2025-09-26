/*
 * variant.h
 *
 *  Created on: 8 May 2022
 *      Author: michi
 */

#ifndef SRC_LIB_BASE_VARIANT_H_
#define SRC_LIB_BASE_VARIANT_H_

#include <type_traits>
#include "base.h"

namespace base {

template<typename T, typename... Ts>
constexpr size_t _max_type_size() {
	if constexpr (sizeof...(Ts) == 0) {
		return sizeof(T);
	} else {
		const size_t s1 = sizeof(T);
		const size_t s2 = _max_type_size<Ts...>();
		return (s1 > s2) ? s1 : s2;
	}
}


template<typename T, typename... Ts>
class variant {
public:
	variant() {
		_index = -1;
		memset(data, 0, sizeof(data));
	}

	template<typename TT>
	void operator=(TT&& t) {
		_make<TT>();
		*(TT*)&data[0] = t;
	}

	template<typename TT>
	TT get() const {
		if (is<TT>())
			throw Exception("wrong type");
		return *(TT *) &data[0];
	}

	int index() const {
		return _index;
	}

	void clear() {
		if (_index < 0)
			return;
		//_clear_if<>()
		//variant<Ts...>::template _clear_if();
		_index = -1;
	}

	template<typename TT>
	bool is() const {
		return _index == to_type_id<TT>();
	}

private:
	int _index = -1;
	alignas(T) alignas(Ts...) char data[_max_type_size<T, Ts...>()];

	template<typename TT>
	void _clear_if() {
		if (is<TT>())
			(*(TT *) &data[0])->~TT();
	}

	template<typename TT>
	void _make() {
		if (is<TT>())
			return;
		clear();
		new(&data[0]) TT();
		_index = to_type_id<TT>();
	}

public:
	template<typename TT>
	static constexpr int to_type_id(int offset = 0) {
		if constexpr (std::is_same<TT, T>::value) {
			return offset;
		} else if (sizeof...(Ts) == 0) {
			return -1;
		} else {
			return variant<Ts...>::template to_type_id<TT>(offset + 1);
		}
	}
};

}


#endif /* SRC_LIB_BASE_VARIANT_H_ */
