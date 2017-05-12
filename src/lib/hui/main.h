/*
 * hui_main.h
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#ifndef HUI_MAIN_H_
#define HUI_MAIN_H_

namespace hui
{

// execution
void Init(const string &program, bool load_res, const string &def_lang);
void _MakeUsable_();
int Run();
void PushMainLevel();
void PopMainLevel();

void SetIdleFunction(const Callback &idle_function);

int RunLater(float time, const Callback &function);

int RunRepeated(float time, const Callback &function);

void CancelRunner(int i);

void DoSingleMainLoop();
void End();

};

#endif /* HUI_MAIN_H_ */
