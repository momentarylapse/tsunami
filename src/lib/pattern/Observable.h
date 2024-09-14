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
#include <type_traits>


namespace obs {

struct internal_node_data;
struct base_sink;
struct base_source;

struct base_source {
	friend struct base_sink;
	base_source(VirtualBase* _node, const string& _name, int);
	~base_source();

	void unsubscribe(base_sink& sink);
	void unsubscribe(VirtualBase* node);

	int count_subscribers() const { return connected_sinks.num; }

protected:
	void _subscribe(base_sink& sink);
	void remove_sink(base_sink* sink);
	void _notify() const;
	VirtualBase* node;
	string name;
	//mutable
	Array<base_sink*> connected_sinks;
};

struct base_sink {
	friend struct base_source;
	friend struct internal_node_data;
	~base_sink();
protected:
	void remove_source(base_source* source);
	VirtualBase* node;
	Array<base_source*> connected_sources;
};





template<class... T> struct xsink;

template<class... T>
struct xsource : base_source {
	friend struct xsink<T...>;

	xsource() = delete;
	template<class N>
	xsource(N* node, const string& name) : base_source(node, name, 0) {
		node->_internal_node_data.sources.add(this);
	}

	void notify(T... t) const {
		_notify();
		for (const base_sink* s: connected_sinks) {
			//if constexpr (NODE_DEBUG_LEVEL >= 2)
			//	msg_write(format("send  %s  ---%s--->>  %s", get_obs_name(node), name, get_obs_name(s->node)));
			reinterpret_cast<const xsink<T...>*>(s)->callback(t...);
		}
	}
	void operator() (T... t) const { notify(t...); }
	void subscribe(xsink<T...>& s) {
		_subscribe(s);
	}
	void operator>>(xsink<T...>& s) {
		_subscribe(s);
	}
};

template<class... T>
struct xsink : base_sink {
	friend struct xsource<T...>;
	friend struct base_source;
	friend struct internal_node_data;

	using Callback = std::function<void(T...)>;

	xsink() = delete;
	template<class N, class F>
	xsink(N* _node, F f) {
		node = _node;
		if constexpr (std::is_member_function_pointer<F>::value)
			callback = [_node, f] (T... t) { (*_node.*f)(t...); };
		else
			callback = f;
	}

private:
	//void init(VirtualBase* node, Callback callback);
	Callback callback;
};

struct internal_node_data {
	friend struct base_source;

	~internal_node_data();
	//template<class... T>
	//xsink<T...>& create_sink(VirtualBase* node, std::function<void(T...)> callback);
	void cleanup_temp_sinks();
	void unsubscribe(VirtualBase* observer);

	Array<base_source*> sources;
	Array<base_sink*> temp_sinks;
};

using sink = xsink<>;
using source = xsource<>;

template<class Base>
class Node : public Base {
	friend struct base_source;

public:
	template<typename... Args>
	explicit Node(Args... args) : Base(args...) {}
	~Node() {
		out_death();
	}

	template<class... T, class F>
	xsink<T...>& create_data_sink(F f) {
		_internal_node_data.cleanup_temp_sinks();
		_internal_node_data.temp_sinks.add(new xsink<T...>(this, f));
		return *reinterpret_cast<xsink<T...>*>(_internal_node_data.temp_sinks.back());
		//return _internal_node_data.create_sink(this, callback);
	}
	sink& create_sink(std::function<void()> callback) {
		return create_data_sink<>(callback);
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
