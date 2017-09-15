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

class Observable : public VirtualBase
{
public:
	Observable();
	virtual ~Observable();

	typedef std::function<void()> Callback;
	typedef std::function<void(Observable*)> CallbackP;

	static const string MESSAGE_CHANGE;
	static const string MESSAGE_DELETE;
	static const string MESSAGE_ANY;

	//void subscribe(Observer *observer, const string &message = MESSAGE_ANY);
	void unsubscribe(VirtualBase *observer);
	void subscribe2(VirtualBase *observer, const Callback &callback, const string &message = MESSAGE_ANY);
	void subscribe3(VirtualBase *observer, const CallbackP &callback, const string &message = MESSAGE_ANY);
	void subscribe_kaba(hui::EventHandler* handler, hui::kaba_member_callback *function);

	void notifyBegin();
	void notify(const string &message = MESSAGE_CHANGE);
	void notifyEnd();
private:
	void notifyEnqueue(const string &message);
	void notifySend();

protected:
	void _observable_destruct_();

private:

	// observers
	struct Subscription
	{
		Subscription();
		Subscription(VirtualBase *o, const string *message, const Callback &callback, const CallbackP &callback_p);
		VirtualBase* observer;
		const string *message;
		Callback callback;
		CallbackP callback_p;
	};
	Array<Subscription> observable_subscriptions;

	struct Notification : Subscription
	{
		Notification();
		Notification(VirtualBase *o, const string *message, const Callback &callback, const CallbackP &callback_p);
	};

	// current notifies
	Array<const string*> observable_message_queue;
	int observable_notify_level;

	const string* observable_cur_message;

public:
	const string& cur_message() const;

private:
	static const bool DEBUG_MESSAGES;
};

#define subscribe_old(OBJECT, CLASS) subscribe3(OBJECT, std::bind(&CLASS::onUpdate, OBJECT, std::placeholders::_1))
#define subscribe_old2(OBJECT, CLASS, MESSAGE) subscribe3(OBJECT, std::bind(&CLASS::onUpdate, OBJECT, std::placeholders::_1), MESSAGE)

#endif /* OBSERVABLE_H_ */
