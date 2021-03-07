/*
 * Observable.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef OBSERVABLE_H_
#define OBSERVABLE_H_

#include "../lib/base/base.h"
#include <functional>

class ObservableData {
public:
	ObservableData();
	~ObservableData();
	
	void fake_death();

	void notify_begin();
	void notify(const string &message);
	void notify_end();
	void notify_enqueue(const string &message);
	void notify_send();

	typedef std::function<void()> Callback;
	typedef std::function<void(VirtualBase*)> CallbackP;

	// observers
	class Subscription {
	public:
		Subscription();
		Subscription(VirtualBase *o, const string *message, const Callback &callback, const CallbackP &callback_p);
		VirtualBase* observer;
		const string *message;
		Callback callback;
		CallbackP callback_p;
	};
	Array<Subscription> subscriptions;
	void subscribe(VirtualBase *me, VirtualBase *observer, const Callback &callback, const CallbackP &callback_p, const string &message);
	void unsubscribe(VirtualBase *observer);

	class Notification : public Subscription {
	public:
		Notification();
		Notification(VirtualBase *o, const string *message, const Callback &callback, const CallbackP &callback_p);
	};

	// current notifies
	Array<const string*> message_queue;
	int notify_level;

	const string* cur_message;
	VirtualBase *me;
};

template<class T>
class Observable : public T {
public:
	static const string MESSAGE_CHANGE;
	static const string MESSAGE_DELETE;
	static const string MESSAGE_ANY;

	//void subscribe(Observer *observer, const string &message = MESSAGE_ANY);
	void unsubscribe(VirtualBase *observer) {
		observable_data.unsubscribe(observer);
	}
	void subscribe(VirtualBase *observer, const ObservableData::Callback &callback, const string &message = MESSAGE_ANY) {
		observable_data.subscribe(this, observer, callback, nullptr, message);
	}
	void subscribe3(VirtualBase *observer, const ObservableData::CallbackP &callback_p, const string &message = MESSAGE_ANY) {
		observable_data.subscribe(this, observer, nullptr, callback_p, message);
	}

	void notify_begin() const {
		observable_data.notify_begin();
	}
	void notify(const string &message = MESSAGE_CHANGE) const {
		observable_data.notify(message);
	}
	void notify_end() const {
		observable_data.notify_end();
	}
	const string& cur_message() const {
		return *observable_data.cur_message;
	}
	void fake_death() const {
		observable_data.fake_death();
	}
private:

	mutable ObservableData observable_data;
};


template<class T>
const string Observable<T>::MESSAGE_CHANGE = "Change";
template<class T>
const string Observable<T>::MESSAGE_DELETE = "Delete";
template<class T>
const string Observable<T>::MESSAGE_ANY = "";

#endif /* OBSERVABLE_H_ */
