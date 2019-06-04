/*
 * Observable.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef OBSERVABLE_H_
#define OBSERVABLE_H_

#include "../lib/base/base.h"
#include "../lib/hui/hui.h"
#include <functional>


//extern const string MESSAGE_CHANGE;
//extern const string MESSAGE_DELETE;
//extern const string MESSAGE_ANY;

class ObservableData
{
public:
	ObservableData();
	~ObservableData();

	void notify_begin();
	void notify(const string &message);
	void notify_end();
	void notify_enqueue(const string &message);
	void notify_send();

	typedef std::function<void()> Callback;
	typedef std::function<void(VirtualBase*)> CallbackP;

	// observers
	class Subscription
	{
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

	class Notification : public Subscription
	{
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
class Observable : public T
{
public:
	static const string MESSAGE_CHANGE;
	static const string MESSAGE_DELETE;
	static const string MESSAGE_ANY;

	//static constexpr string &MESSAGE_CHANGE = ObservableData::MESSAGE_CHANGE;
	//static const string &MESSAGE_DELETE = ObservableData::MESSAGE_DELETE;
	//static const string &MESSAGE_ANY = ObservableData::MESSAGE_ANY;

	//void subscribe(Observer *observer, const string &message = MESSAGE_ANY);
	void unsubscribe(VirtualBase *observer)
	{
		observable_data.unsubscribe(observer);
	}
	void subscribe(VirtualBase *observer, const ObservableData::Callback &callback, const string &message = MESSAGE_ANY)
	{
		observable_data.subscribe(this, observer, callback, nullptr, message);
	}
	void subscribe3(VirtualBase *observer, const ObservableData::CallbackP &callback_p, const string &message = MESSAGE_ANY)
	{
		observable_data.subscribe(this, observer, nullptr, callback_p, message);
	}
	void subscribe_kaba(hui::EventHandler* handler, hui::kaba_member_callback *function, const string &message)
	{
		subscribe(handler, std::bind(function, handler), message);
	}

	void notify_begin() const
	{
		observable_data.notify_begin();
	}
	void notify(const string &message = MESSAGE_CHANGE) const
	{
		observable_data.notify(message);
	}
	void notify_end() const
	{
		observable_data.notify_end();
	}
	const string& cur_message() const
	{
		return *observable_data.cur_message;
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

//#define subscribe_old(OBJECT, CLASS) subscribe3(OBJECT, std::bind(&CLASS::onUpdate, OBJECT, std::placeholders::_1))
//#define subscribe_old2(OBJECT, CLASS, MESSAGE) subscribe3(OBJECT, std::bind(&CLASS::onUpdate, OBJECT, std::placeholders::_1), MESSAGE)

#endif /* OBSERVABLE_H_ */
