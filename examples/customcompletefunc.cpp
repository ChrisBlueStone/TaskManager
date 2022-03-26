// This example demonstrates how to define and use a custom IsComplete function
// to determine when tasks should be destroyed.
///
// In this example, the default IsComplete function is replaced with WhenNegative
// that judge a task as being complete when it returns a negative float value.

#include "../TaskManager.h"
#include <iostream>

using std::cout;

struct WhenNegative { bool operator()(float time) { return time < 0.0f; } };

struct TimerTask
{
	TimerTask(float timeRemaining) : timeRemaining(timeRemaining) {}
	float timeRemaining;
	float operator()(float dt)
	{
		timeRemaining -= dt;
		if (timeRemaining > 0.0f)
			cout << "Time remaining: " << timeRemaining << '\n';
		else
			cout << "Time expired.\n";
		return timeRemaining;
	}
};

int main()
{
	TaskManager<float, WhenNegative, float> tasks;
	tasks.AddTask<TimerTask>(10.0f);

	while (tasks.HasPendingTasks())
		tasks.Run((10 + rand() % 15) / 10.0f);

	return 0;
}
