#pragma once

#include <string>

#include <map>
#include <future>

#include "request_handler.h"

class my_task
{
	std::string id;
	utility::datetime startedAt;

	std::mutex output_mutex;
	std::stringstream output;
	bool finished = false;
	bool forcedStop = false;

	std::future<int> operation;

	int run()
	{
		for (int i = 0; i < 100 && !this->forcedStop; i++)
		{
			{
				std::lock_guard<std::mutex> g(this->output_mutex);
				output << "task " << this->id << " at " << i << std::endl;
			}

			::_sleep(1000);
		}

		this->finished = true;

		return 0;
	}
public:
	my_task(std::string id, utility::datetime startedAt)
		: id(id), startedAt(startedAt), operation(std::async(std::launch::async, [&]() { return this->run(); }))
	{
	}


	bool is_finished() const
	{
		return this->finished;
	}

	void stop()
	{
		this->forcedStop = true;
		// wait for operation to finish
		this->operation.get();
	}

	std::string get_output()
	{
		std::lock_guard<std::mutex> g(this->output_mutex);
		return output.str();
	}

	std::string get_id() const { return this->id; }
	utility::datetime get_startedAt() const { return this->startedAt; }
};

class tasks_handler : public request_handler
{
	std::map<std::string, std::shared_ptr<my_task>> tasks;
	int nextTaskId = 1;

public:
	tasks_handler()
		: request_handler("/tasks/")
	{
	}

	std::string addTask();

	virtual void handle(web::http::http_request& request);
};