#ifndef SRC_LIB_BASE_XPARAM_H_
#define SRC_LIB_BASE_XPARAM_H_

#include "macros.h"

namespace base {

	// automatic call-by-value/reference type
	// default: call-by-reference
	template<class T>
	struct xparam {
		using t = const T&;
	};
	// explicit call-by-value
	template<>
	struct xparam<int> {
		using t = int;
	};
	template<>
	struct xparam<int64> {
		using t = int64;
	};
	template<>
	struct xparam<int8> {
		using t = int8;
	};
	template<>
	struct xparam<uint8> {
		using t = uint8;
	};
	template<>
	struct xparam<float> {
		using t = float;
	};
	template<>
	struct xparam<double> {
		using t = double;
	};
	template<>
	struct xparam<bool> {
		using t = bool;
	};
	template<>
	struct xparam<char> {
		using t = char;
	};
	template<class T>
	struct xparam<T*> {
		using t = T*;
	};
	template<>
	struct xparam<void> {
		using t = void;
	};
}

#endif //XPARAM_H
