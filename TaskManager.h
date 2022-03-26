// TaskManager library
// 2022 Christopher Salcido
// https://github.com/ChrisBlueStone/TaskManager
#ifndef TASKS_H
#define TASKS_H

#include <algorithm>
#include <functional>
#include <list>

/// \brief Default function object used for testing when a task has completed.
struct WhenTrue { bool operator()(bool result) { return result; } };

/// \brief \a TaskManager is used to manage and run arbitrary functions in the order that they are added.
/// \tparam ...RunArgs The argument types required when calling the \a Run method. The arguments provided
/// to \a Run will be passed to each task when it is ran.
template <typename Result = bool, class IsComplete = WhenTrue, typename... RunArgs>
class TaskManager
{
	/// \brief Wrapper class for a task to manage.
	class Task {
		friend class TaskManager<Result, IsComplete, RunArgs...>;
		/// \brief Constructs a \a Task object.
		/// \param func A function that the Task calls when it's run. The function should return \a true if it completes, otherwise \a false.
		Task(const std::function<Result(RunArgs...)>& func, bool active, bool expired) : func(func), active(active), expired(expired) {}
		std::function<Result(RunArgs...)> func;

		/// \brief Signifies if a task is active. A task is only inactive if it has been added while \a Run
		/// is in the middle of execution and will not be executed until the next time \a Run is called.
		bool active = false;
		/// \brief Signifies if a task is expired. A task is expired if it has been killed but is
		/// still in memory due to \a Run still being executed
		bool expired = false;
	};

	public:
		/// \brief Executes each task that has been added in order unless the task was added during execution
		/// of this method or the task has expired via the \a KillTask method.
		/// \param ...args The arguments to pass to each task as they are executed.
		void Run(RunArgs... args) {
			if (!running) {
				running = true;

				for (auto && i=tasks.begin(); i != tasks.end(); i++) {
					if (i->expired || (i->active && IsComplete()(i->func(args...))))
						tasks.erase(i--);
					else
						i->active = true;
				}

				running = false;
			}
		}

		/// \brief Adds a task to the \a TaskManager.
		/// \tparam T The class type to use as the task.
		/// \param task A reference to the function or object to use as the task.
		/// \return Returns a pointer to the constructed \a T object that's being managed by the \a TaskManager.
		/// \details Adds a \a Task object to manage that will call \a task each time the \a Run is called on the \a TaskManager object
		/// as long as the task hasn't completed or has expired via \a KillTask.
		/// If \a Run is currently executing when this method is called, the task will not execute until the next time \a Run
		/// is called.
		///
		/// Example usage:
		/// ```cpp
		/// TaskManager<bool, WhenTrue, int> tasks;
		/// unsigned int i=3;
		/// // Create a task inline that captures i and decrements it each time Run is called until it equals zero.
		/// tasks.AddTask([&i](int j){ cout << "i: " << i << " arg: " << j << '\n'; return --i == 0; });
		/// tasks.Run(10);
		/// tasks.Run(20);
		/// // The task function will be destroyed the next time Run() is called.
		/// tasks.Run(30);
		/// ```
		template <class T>
		T* AddTask(const T& task) {
			tasks.emplace_back(Task{task, !running, false});
			return tasks.back().func.template target<T>();
		}

		/// \brief Constructs a new object of type \a T to be used and managed as a task.
		/// \tparam T The class type to use as the task.
		/// \tparam ...Args The argument types to use in the constructor of \a T.
		/// \param ...args The arguments to pass to the constructor of \a T when constructing the task.
		/// \details Example usage:
		/// ```cpp
		/// struct GreetTask {
		/// 	GreetTask(string name) : name(name), greeted(false) {}
		/// 	string name;
		/// 	bool greeted;
		/// 
		/// 	bool operator()() {
		/// 		cout << (greeted ? "Goodbye, " : "Hello, ") << name << "!\n";
		/// 		greeted = true;
		/// 		return greeted;
		/// 	}
		/// };
		/// 
		/// TaskManager<> tasks;
		/// tasks.AddTask<GreetTask>("Bob");
		/// tasks.Run();
		/// tasks.Run();
		/// ```
		template <class T, typename... Args>
		T* AddTask(Args&&... args) {
			tasks.emplace_back(Task{T(args...), !running, false});
			return tasks.back().func.template target<T>();
		}

		/// \brief Kills a task by marking it as expired. If the \a Run method is currently executing when
		/// this method is called, then the task will not execute if it hasn't already and will remain in
		/// memory until \a Run completes.
		/// \tparam T The class type of the task to kill.
		/// \param t A pointer to the task to kill.
		/// \details Example usage:
		/// ```cpp
		/// TaskManager<bool, WhenTrue, const char*> tasks;
		/// auto echoTask = tasks.AddTask([](const char* input){ cout << input << endl; return false; });
		/// tasks.Run("Hello?");
		/// tasks.Run("Echo!");
		/// tasks.KillTask(echoTask); // Destroy the task manually
		/// tasks.Run("Quack.");
		/// ```
		template <class T>
		void KillTask(T* t) {
			for (auto && task : tasks)
			{
				if (task.func.template target<T>() == t)
				{
					task.expired = true;
					break;
				}
			}
		}

		/// \brief Returns true if a specific task is being managed and has not been destroyed.
		/// \param t A pointer to the task to find.
		template <class T>
		bool HasTask(T* t) const {
			return std::any_of(
				tasks.cbegin(),
				tasks.cend(),
				[t](auto && task){ return task.func.template target<T>() == t; });
		}

		/// \brief Returns true if a specific task is being not being managed
		/// by the TaskManager instance or if it has expired.
		/// \param t A pointer to the task to find.
		template <class T>
		bool IsTaskExpired(T* t) const {
			for (auto && task : tasks)
				if (task.func.template target<T>() == t)
					return task.expired;
			return true;
		}

		/// \brief Returns if any tasks are being managed, regardless of active or expiration status.
		bool HasTasks() const { return !tasks.empty(); }
		/// \brief Returns if any tasks are being managed and haven't expired via the \a KillTask method.
		bool HasPendingTasks() const { return std::any_of(tasks.cbegin(), tasks.cend(), [](auto && t){ return !t.expired; }); }
		/// \brief Returns the number of tasks that aren't expired
		auto CountPendingTasks() const { return std::count_if(tasks.cbegin(), tasks.cend(), [](auto && t) { return !t.expired; }); }
	private:
		bool running = false;
		std::list<Task> tasks;
};

#endif // TASKS_H
