// This example shows how to store a reference to a task and manually remove it
// from the TaskManager instance that generated it.

#include "../TaskManager.h"
#include <iostream>

using std::cout;
using std::endl;

int main()
{
	TaskManager<bool, WhenTrue, const char *> tasks;
	auto echoTask = tasks.AddTask(
		[](const char *input)
		{
			cout << input << endl;
			return false; // Never complete
		});

	tasks.Run("Hello?");
	tasks.Run("Echo!");
	tasks.KillTask(echoTask); // End the task manually
	tasks.Run("Quack.");

	return 0;
}
