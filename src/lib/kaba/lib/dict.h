#include "../../base/base.h"
#include "../../base/map.h"

namespace kaba {
	class Class;
	class SyntaxTree;
	
	void lib_make_dict(Class *t, SyntaxTree *ps);


	template<class T>
	class XDict : public base::map<string, T> {
	public:
		void __init__() {
			new(this) base::map<string, T>();
		}
	};


}
