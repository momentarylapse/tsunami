/*
 * Observable.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef SRC_LIB_PATTERN_OBSERVABLE_H_
#define SRC_LIB_PATTERN_OBSERVABLE_H_

#include "../base/base.h"
#include <functional>


namespace obs {

struct internal_node_data;
struct sink;
using Callback = std::function<void()>;

struct source {
	friend struct sink;

	source() = delete;
	source(VirtualBase* node, const string& name, int _dummy);
	template<class N>
	source(N* node, const string& name) : source(node, name, 0) {
		node->_internal_node_data.sources.add(this);
	}
	~source();
	void notify() const;
	void operator() () const { notify(); }
	void subscribe(sink& sink);
	void unsubscribe(sink& sink);
	void unsubscribe(VirtualBase* node);
	void operator>>(sink& sink);
	int count_subscribers() { return connected_sinks.num; }

private:
	void remove_sink(sink* sink);
	VirtualBase* node;
	string name;
	//mutable
	Array<sink*> connected_sinks;
};

struct sink {
	friend struct source;
	friend struct internal_node_data;

	sink() = delete;
	sink(VirtualBase* node, Callback callback, int _dummy);
	template<class N>
	sink(N* node, Callback callback) : sink(node, callback, 0) {}
	/*template<class T, class F>
	sink(T* _node, F *f) {
		node = _node;
		callback = [_node, f] { (*_node.*f)(); };
	}*/
	~sink();

private:
	void _remove(source* source);
	VirtualBase* node;
	Callback callback;
	Array<source*> connected_sources;
};

struct internal_node_data {
	friend struct source;

	~internal_node_data();
	sink& create_sink(VirtualBase* node, Callback callback);
	void cleanup_temp_sinks();
	void unsubscribe(VirtualBase* observer);

	Array<source*> sources;
	Array<sink*> temp_sinks;
};


template<class T>
class Node : public T {
	friend struct source;

public:
	template<typename... Args>
	Node(Args... args) : T(args...) {}
	~Node() {
		out_death();
	}

	sink& create_sink(Callback callback) {
		return _internal_node_data.create_sink(this, callback);
	}

	void unsubscribe(VirtualBase *observer) {
		_internal_node_data.unsubscribe(observer);
	}

	void fake_death() const {
		out_death();
	}

//private:
	internal_node_data _internal_node_data;

public:
	obs::source out_changed{this, "changed"};
	obs::source out_death{this, "death"};
};

}



#endif /* SRC_LIB_PATTERN_OBSERVABLE_H_ */
