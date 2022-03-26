// This example shows how to use a function object as a task, which may also manage
// an internal state as part of its runtime.

#include "../TaskManager.h"
#include <iostream>
#include <string>

using std::cout;
using std::string;

struct GreetTask
{
	GreetTask(string name) : name(name), greeted(false) {}
	string name;
	bool greeted;

	bool operator()()
	{
		bool alreadyGreeted = greeted;
		
		cout << (greeted ? "Goodbye, " : "Hello, ") << name << "!\n";
		greeted = true;

		return alreadyGreeted;
	}
};

int main()
{
	TaskManager<> tasks;
	tasks.AddTask<GreetTask>("Bob");
	tasks.Run();
	tasks.Run();

	return 0;
}
