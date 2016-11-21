#include "stdwx.h"
#include "tasks_handler.h"

std::string tasks_handler::addTask()
{
	std::stringstream id;
	id << "id_" << nextTaskId;

	tasks[id.str()] = std::make_shared<my_task>(id.str(), utility::datetime::utc_now());

	nextTaskId++;
	return id.str();
}

void tasks_handler::handle(web::http::http_request& request)
{
	using namespace web;

	const auto tasksPrefix = utility::string_t(U("/tasks/"));
	const auto path = request.absolute_uri().path();
	auto taskId = path.substr(tasksPrefix.length());

	if (request.method() == web::http::methods::GET)
	{
		// /tasks/ -> return overview of all tasks
		if (taskId.length() == 0)
		{
			json::value root;
			std::vector<json::value> tasks;

			for (auto kv : this->tasks)
			{
				auto task = kv.second;
				auto t = json::value::object();
				t[L"id"] = json::value::string(utility::conversions::to_string_t(task->get_id()));
				t[L"startedAt"] = json::value::string(task->get_startedAt().to_string());
				t[L"finished"] = json::value::boolean(task->is_finished());

				tasks.push_back(t);
			}

			root[L"tasks"] = json::value::array(tasks);
			request.reply(web::http::status_codes::OK, root);
		}
		else
		{
			auto taskIt = this->tasks.find(utility::conversions::to_utf8string(taskId));
			if (taskIt != this->tasks.end())
			{
				auto& task = taskIt->second;
				auto t = json::value::object();
				t[L"id"] = json::value::string(utility::conversions::to_string_t(task->get_id()));
				t[L"startedAt"] = json::value::string(task->get_startedAt().to_string());
				t[L"finished"] = json::value::boolean(task->is_finished());
				t[L"output"] = json::value::string(utility::conversions::to_string_t(task->get_output()));

				request.reply(web::http::status_codes::OK, t);
			}
			else
			{
				request.reply(web::http::status_codes::NotFound);
			}
		}
	}
	else if (request.method() == web::http::methods::POST)
	{
		if (taskId.length() == 0)
		{
			auto id = this->addTask();
			request.reply(web::http::status_codes::OK, json::value::string(utility::conversions::to_string_t(id)));
		}
		else
		{
			request.reply(web::http::status_codes::NotFound);
		}
	}
	else if (request.method() == web::http::methods::DEL)
	{
		if (taskId.length() > 0)
		{
			auto taskIt = this->tasks.find(utility::conversions::to_utf8string(taskId));
			if (taskIt != this->tasks.end())
			{
				taskIt->second->stop();
				this->tasks.erase(taskIt);
				request.reply(web::http::status_codes::OK);
			}
			else
			{
				request.reply(web::http::status_codes::NotFound);
			}
		}
	}
}