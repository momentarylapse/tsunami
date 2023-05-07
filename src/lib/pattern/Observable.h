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
	struct Source;
}

class ObservableData {
	friend struct obs::Source;
public:
	ObservableData();
	~ObservableData();
	
	void fake_death();

	void notify(const string &message);

	using Callback = std::function<void()>;

	// observers
	struct Subscription {
		Subscription();
		Subscription(VirtualBase *o, const string *message, const Callback &callback);
		VirtualBase* observer;
		const string *message;
		Callback callback;
	};
	Array<Subscription> subscriptions;
	void subscribe(VirtualBase *me, VirtualBase *observer, const Callback &callback, const string &message);
	void unsubscribe(VirtualBase *observer);

	struct Notification : public Subscription {
		Notification();
		Notification(VirtualBase *o, const string *message, const Callback &callback);
	};

	VirtualBase *me;
};


template<class T>
class LegacyObservable : public T {
public:
	template<typename... Args>
	LegacyObservable(Args... args) : T(args...) {}

	static const string MESSAGE_CHANGE;
	static const string MESSAGE_DELETE;
	static const string MESSAGE_ANY;

	void unsubscribe(VirtualBase *observer) {
		observable_data.unsubscribe(observer);
	}
	void subscribe(VirtualBase *observer, const ObservableData::Callback &callback, const string &message) {
		observable_data.subscribe(this, observer, callback, message);
	}

	void notify(const string &message = MESSAGE_CHANGE) const {
		observable_data.notify(message);
	}
	void fake_death() const {
		observable_data.fake_death();
	}

protected:
	mutable ObservableData observable_data;
};


template<class T>
const string LegacyObservable<T>::MESSAGE_CHANGE = "changed";
template<class T>
const string LegacyObservable<T>::MESSAGE_DELETE = "death";
template<class T>
const string LegacyObservable<T>::MESSAGE_ANY = "";


namespace obs {
struct Sink;
using Callback = std::function<void()>;

struct Source {
	friend class Sink;

	Source() = delete;
	Source(VirtualBase* node, const string& name, ObservableData* obs_data);
	template<class T>
	Source(T* node, const string& name) : Source(node, name, &node->observable_data) {
		node->_sources.add(this);
	}
	~Source();
	void notify() const;
	void subscribe(Sink& sink);
	void unsubscribe(Sink& sink);
	void unsubscribe(VirtualBase* node);
	void operator>>(Sink& sink);

private:
	void _remove(Sink* sink);
	ObservableData* legacy_observable_data;
	VirtualBase* node;
	string name;
	//mutable
	Array<Sink*> sinks;
};

struct Sink {
	friend class Source;

	Sink() = delete;
	Sink(VirtualBase* node, Callback callback);
	~Sink();

private:
	void _remove(Source* source);
	VirtualBase* node;
	Callback callback;
	Array<Source*> sources;
};


template<class T>
class Node : public LegacyObservable<T> {
	friend class Source;

public:
	template<typename... Args>
	Node(Args... args) : LegacyObservable<T>(args...) {}
	~Node() {
		for (auto s: _private_sinks)
			delete s;
	}

private:
	Array<Source*> _sources;
	Array<Sink*> _private_sinks;

public:
	obs::Source out_changed{this, "changed"};
	obs::Source out_death{this, "death"};

	Sink& create_sink(Callback callback) {
		_private_sinks.add(new Sink(this, callback));
		return *_private_sinks.back();
	}

	void unsubscribe(VirtualBase *observer) {
		LegacyObservable<T>::unsubscribe(observer);
		for (auto s: _sources)
			s->unsubscribe(observer);
	}

	void fake_death() const {
		out_death.notify();
		this->observable_data.fake_death();
	}
};

}



#endif /* SRC_LIB_PATTERN_OBSERVABLE_H_ */
