/*
 * Callback.h
 *
 *  Created on: 15.05.2017
 *      Author: michi
 */

#ifndef SRC_LIB_HUI_CALLBACK_H_
#define SRC_LIB_HUI_CALLBACK_H_

#include <functional>

class Painter;

namespace hui
{

class EventHandler;

typedef void kaba_callback();
typedef void kaba_member_callback(EventHandler *h);
typedef void kaba_member_callback_p(EventHandler *h, void *p);

typedef std::function<void()> Callback;
typedef std::function<void(Painter*)> CallbackP;



void SetIdleFunction(const Callback &idle_function);
int RunLater(float time, const Callback &function);
int RunRepeated(float time, const Callback &function);
void CancelRunner(int i);


}



#endif /* SRC_LIB_HUI_CALLBACK_H_ */
