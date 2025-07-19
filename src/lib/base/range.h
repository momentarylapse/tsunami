#if !defined(LIB_BASE_RANGE_H__INCLUDED_)
#define LIB_BASE_RANGE_H__INCLUDED_

#include "xparam.h"

namespace base {

	// assume T fits in register
	// ...otherwise add xparam<>...!

	template<class T>
	struct range {
		T start, end;

		T size() const {
			return end - start;
		}
		bool inside(T x) const {
			return start <= x and x < end;
		}
		bool covers(const range& other) const {
			return start <= other.start and end >= other.end;
		}
		range canonical() const {
			return {min(start, end), max(start, end)};
		}

		bool operator==(const range& other) const {
			return start == other.start and end == other.end;
		}
		bool operator!=(const range& other) const {
			return start != other.start or end != other.end;
		}
		range operator||(const range& r) const {
			return {min(start, r.start), max(r.end, r.end)};
		}
		range operator&&(const range& r) const {
			return {max(start, r.start), min(r.end, r.end)};
		}
		range operator+(T shift) const {
			return {start + shift, end + shift};
		}
		range operator-(T shift) const {
			return {start - shift, end - shift};
		}
	};
}

#endif
