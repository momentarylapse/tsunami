#if !defined(LIB_BASE_SPAN_H__INCLUDED_)
#define LIB_BASE_SPAN_H__INCLUDED_

#include "macros.h"

namespace base {

template<class T>
struct span {
	T* data;
	int num;

	int size_bytes() const {
		return (int)sizeof(T) * num;
	}
};

using binary_span = span<const uint8>;

template<class T>
binary_span binary_ref(const T& t) {
	return {(const uint8*)&t, (int)sizeof(T)};
};
}

#endif
