#pragma once

#include <functional>
#include <thread>
#include <mutex>

namespace TSPlugin {

	class RunLoop {

		std::thread workerThread;

		std::queue<std::function<void()>> taskQueue;
		std::mutex queueMutex;
		std::condition_variable condition;
		volatile bool stop = false;
	public:
		enum DeferredInit_t { DeferredInit };

		RunLoop() {
			Start();
		}

		RunLoop(DeferredInit_t) {
			// nem csinál semmit
		}

		~RunLoop() {
			StopAndWait();
		}


		void Start() {
			workerThread = std::thread([this] { Loop(); });
		}

		void Stop() {
			stop = true;
		}

		void StopAndWait() {
			Stop();

			condition.notify_one();
			if (workerThread.joinable()) {
				workerThread.join();
			}
		}

		void Add(std::function<void()>&& fn) {
			std::unique_lock<std::mutex> lock(queueMutex);
			taskQueue.push(fn);
			lock.unlock();
			condition.notify_one();
		}

		void operator<<(std::function<void()>&& fn) {
			Add(std::forward<std::function<void()>>(fn));
		}


	private:

		void Loop() {
			while (!stop) {
				ExecuteTaskInQueue();
				SleepUntilNotified();
			}

		}

		void SleepUntilNotified() {
			std::unique_lock<std::mutex> lock(queueMutex);
			if (taskQueue.empty()) {
				condition.wait(lock);
			}
		}

		void ExecuteTaskInQueue() {

			std::unique_lock<std::mutex> lock(queueMutex);
			if (!taskQueue.empty()) {
				auto task = std::move(taskQueue.front());
				taskQueue.pop();
				lock.unlock();
				task();
			}

		}

	};


} // namespace TSPlugin
