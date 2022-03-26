# TaskManager
A simple C++ template header library for cooperative multitasking of functions, lambda functions, and function objects.

TaskManager is used to manage and run arbitrary functions in the order that they are added, and destroy them each as they are completed.

This is a very handy utility class for writing cooperative systems, such as an animation system where each animation is a function object that update objects according to a delta time, or other such systems as audio systems, physics systems, etc.

- [Usage](#usage)
- [Class Definition](#class-definition)
- [Examples](#examples)

## Usage:
1. Create TaskManager objects to hold a collection of tasks.
	```
	TaskManager<> tasks;
	TaskManager<float> floatTasks;
	```

2. Add tasks to it via [`AddTask`](#addtask), passing to it any lambda, function, or function object that you want to be called each time `Run()` is called on the TaskManager object. The function should return its completion state. By default, returning `true` signifies that the task is complete and should be removed from the TaskManager instance and destroyed, and returning `false` signifies that the task is not yet complete.
	```
	MyTask* myTask = tasks.AddTask<MyTask>();
	floatTasks.AddTask<MyFloatTask>(initialValue);
	floatTasks.AddTask([](const float f){ cout << "Tasks were run with " << f << endl; });
	```

3. Remove manually tasks via [`KillTask`](#killtask), passing to it a function reference returned by `AddTask`.
	```
	tasks.KillTask(myTask);
	```

4. Call [`Run()`](#run) to execute each of the tasks that have been added to the TaskManager object. Any arguments passed to `Run()` will also be passed to the tasks associated with that TaskManager instance.
	```
	tasks.Run();
	floatTasks.Run(4.2f);
	```

## Class Definition
```cpp
template <
	typename Result = bool,
	class IsComplete = WhenTrue,
	typename... RunArgs
> class TaskManager;
```
The `TaskManager` class is a variadic template where the first two template arguments define the type that tasks are expected to return and a function object to test if the task is complete or not. Any remaining template parameters are used as the types expected when calling the `Run` method.

### Template Parameters
`Result` - The return type of task functions.

`IsComplete` - Function object type used to test if a task has expired.

`RunArgs...` - The types of the parameters required by `Run` and associated tasks.

### Methods
- [AddTask](#addtask)
- [KillTask](#killtask)
- [Run](#run)
- [HasTask](#hastask)
- [IsTaskExpired](#istaskexpired)
- [HasTasks](#hastasks)
- [HasPendingTasks](#haspendingtasks)
- [CountPendingTasks](#countpendingtasks)

#### AddTask
1. `template <class T, typename... Args> T* AddTask(Args&&... args)`
2. `template <class T> T* AddTask(const T& task)`

Adds a new task to the TaskManager instance and returns a reference to the function, lamda, or function object that was added. The reference is valid only until the task ends or is killed via the `KillTask` method.

`1.` constructs the task in-place, passing `args` to the constructor.

`2.` accepts a reference to a task and creates a copy of it within TaskManager. A reference to the copy is returned.

#### KillTask
`template <class T> void KillTask(T* task)`

Accepts a pointer to a function/lambda/function-object and marks the associated task as expired if it exists within the TaskManager instance. The task is not destroyed immediately in case it is currently running, but will it will not run again and will be deleted from TaskManager the next time `Run` is called or until the TaskManager instance is destroyed.

#### Run
`void Run(RunArgs... args)`

Loops through all tasks, calling and passing `args` to each non-expired task, and deleting any previously expired or currently expiring tasks.

#### HasTask
`template <class T> bool HasTask(T* task) const`

Returns true if `task` is currently being managed by the TaskManager instance. This includes if a task has expired but has not yet been removed from the manager.

#### IsTaskExpired
`template <class T> bool IsTaskExpired(T* task) const`

Returns true if `task` is not being managed by the TaskManager instance or if it is being managed by the TaskManager instance but has already expired. 

#### HasTasks
`bool HasTasks() const`

Returns true if TaskManager has any tasks, expired or not.

#### HasPendingTasks
`bool HasPendingTasks() const`

Returns true if TaskManager has any non-expired tasks.

#### CountPendingTasks
`auto CountPendingTasks() const`

Returns the number of non-expired tasks.

## Examples
Below are some example snippets, but the full examples can be found in [/examples](/examples)

- [Using a lambda as a task](#using-a-lambda-as-a-task)
- [Using a function object as a task](#using-a-function-object-as-a-task)
- [Manually ending a task](#creating-storing-and-manually-ending-a-task)
- [Custom completion test](#using-a-custom-task-completion-test)

### Using a lambda as a task:
```cpp
TaskManager<bool, WhenTrue, int> tasks;
unsigned int i=3;
// Create a task inline that captures i and decrements it each time Run is called until it equals zero.
tasks.AddTask([&i](int j){ cout << "i: " << i + j << '\n'; return --i == 0; });
tasks.Run(90);
tasks.Run(80);
// The task function will be destroyed the next time Run() is called.
tasks.Run(70);
```
Output
```
i: 93
i: 82
i: 71
```

---
### Using a function object as a task:
```cpp
struct GreetTask {
	GreetTask(string name) : name(name), greeted(false) {}
	string name;
	bool greeted;

	bool operator()() {
		bool alreadyGreeted = greeted;
		
		cout << (greeted ? "Goodbye, " : "Hello, ") << name << "!\n";
		greeted = true;

		return alreadyGreeted;
	}
};

TaskManager<> tasks;
tasks.AddTask<GreetTask>("Bob");
tasks.Run();
tasks.Run();
```
Output:
```
Hello, Bob!
Goodbye, Bob!
```

---
### Creating, storing, and manually ending a task:
```cpp
TaskManager<bool, WhenTrue, const char*> tasks;
auto echoTask = tasks.AddTask([](const char* input){ cout << input << endl; return false; })
tasks.Run("Hello?");
tasks.Run("Echo!");
tasks.KillTask(echoTask); // Destroy the task manually
tasks.Run("Quack.");
```
Output:
```
Hello?
Echo!
```

---
### Using a custom task completion test:
```cpp
struct WhenNegative { bool operator()(float time) { return time < 0.0f; } };

struct TimerTask {
	TimerTask(float timeRemaining) : timeRemaining(timeRemaining) {}
	float timeRemaining;
	float operator()(float dt) {
		timeRemaining -= dt;
		if (timeRemaining > 0.0f)
			cout << "Time remaining: " << timeRemaining << '\n';
		else
			cout << "Time expired.\n";
		return timeRemaining;
	}
};

TaskManager<float, WhenNegative, float> tasks;
tasks.AddTask<TimerTask>(10.0f);
while (tasks.HasPendingTasks())
	tasks.Run((10 + rand() % 15) / 10.0f);
```
Possible output:
```
Time remaining: 7.9
Time remaining: 6.7
Time remaining: 5.3
Time remaining: 3.3
Time remaining: 0.9
Time expired.
```
