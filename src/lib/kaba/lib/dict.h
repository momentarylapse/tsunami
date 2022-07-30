#include "../../base/base.h"
#include "../../base/map.h"

namespace kaba {
	class Class;
	class SyntaxTree;
	
	void kaba_make_dict(Class *t, SyntaxTree *ps);


	template<class T>
	class XDict : public Map<string, T> {
	public:
		void __init__() {
			new(this) Map<string, T>();
		}
	};


}
