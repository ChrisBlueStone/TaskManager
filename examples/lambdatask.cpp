// This example demonstrates how to easily add a task via an inline lambda expression.
// The lambda function may capture and modify any variables during its runtime.

#include "../TaskManager.h"
#include <iostream>

using std::cout;

int main()
{
	TaskManager<bool, WhenTrue, int> tasks;
	unsigned int i = 3;

	// Create a task inline that captures i and decrements it each time Run is called until it equals zero.
	tasks.AddTask(
		[&i](int j)
		{
			cout << "i: " << i + j << '\n';
			return --i == 0;
		});

	for (int i = 10; tasks.CountPendingTasks() > 0; i += 10)
	{
		cout << "Running tasks with " << i << '\n';
		tasks.Run(i);
	}

	return 0;
}
