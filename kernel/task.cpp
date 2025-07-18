#include "task.hpp"

#include "asmfunc.h"
#include "error.hpp"
#include "timer.hpp"
#include "segment.hpp"

namespace
{
	template <class T, class U>
	void Erase(T &c, const U &value)
	{
		auto it = std::remove(c.begin(), c.end(), value);
		c.erase(it, c.end());
	}

	void TaskIdle(uint64_t task_id, int64_t data)
	{
		while (true)
		{
			__asm__("hlt");
		}
	}
}

Task::Task(uint64_t id) : id_{id}, msgs_{}
{
}

Task &Task::InitContext(TaskFunc *f, int64_t data)
{
	const size_t stack_size = kDefaultStackBytes / sizeof(stack_[0]);
	stack_.resize(stack_size);
	uint64_t stack_end = reinterpret_cast<uint64_t>(&stack_[stack_size]);

	memset(&context_, 0, sizeof(context_));
	context_.cr3 = GetCR3();
	context_.rflags = 0x202;
	context_.cs = kKernelCS;
	context_.ss = kKernelSS;
	context_.rsp = (stack_end & ~0xflu) - 8;

	context_.rip = reinterpret_cast<uint64_t>(f);
	context_.rdi = id_;
	context_.rsi = data;

	*reinterpret_cast<uint32_t *>(&context_.fxsave_area[24]) = 0x1f80;

	return *this;
}

TaskContext &Task::Context()
{
	return context_;
}

uint64_t Task::ID() const
{
	return id_;
}

Task &Task::Sleep()
{
	task_manager->Sleep(this);
	return *this;
}

Task &Task::Wakeup()
{
	task_manager->Wakeup(this);
	return *this;
}

void Task::SendMessage(const Message &msg)
{
	msgs_.push_back(msg);
	Wakeup();
}

std::optional<Message> Task::ReceiveMessage()
{
	if (msgs_.empty())
	{
		return std::nullopt;
	}

	auto m = msgs_.front();
	msgs_.pop_front();
	return m;
}

TaskManager::TaskManager()
{
	Task &task = NewTask()
					 .SetLevel(current_level_)
					 .SetRunning(true);
	running_[current_level_].push_back(&task);

	Task &idle = NewTask()
					 .InitContext(TaskIdle, 0)
					 .SetLevel(0)
					 .SetRunning(true);
	running_[0].push_back(&idle);
}

Task &TaskManager::NewTask()
{
	++latest_id_;
	return *tasks_.emplace_back(new Task{latest_id_});
}

void TaskManager::SwitchTask(bool current_sleep)
{
	auto &level_queue = running_[current_level_];
	Task *current_task = level_queue.front();
	level_queue.pop_front();
	if (!current_sleep)
	{
		level_queue.push_back(current_task);
	}
	if (level_queue.empty())
	{
		level_changed_ = true;
	}

	if (level_changed_)
	{
		level_changed_ = false;
		for (int lv = kMaxLevel; lv >= 0; --lv)
		{
			if (!running_[lv].empty())
			{
				current_level_ = lv;
				break;
			}
		}
	}

	Task *next_task = running_[current_level_].front();

	SwitchContext(&next_task->Context(), &current_task->Context());
}

void TaskManager::Sleep(Task *task)
{
	if (!task->Running())
	{
		return;
	}

	// 必要ある？？
	task->SetRunning(false);

	if (task == running_[current_level_].front())
	{
		SwitchTask(true);
		return;
	}

	Erase(running_[task->Level()], task);
}

Error TaskManager::Sleep(uint64_t id)
{
	auto it = std::find_if(tasks_.begin(), tasks_.end(), [id](const auto &t)
						   { return t->ID() == id; });

	if (it == tasks_.end())
	{
		return MAKE_ERROR(Error::kNoSuchTask);
	}

	Sleep(it->get());
	return MAKE_ERROR(Error::kSuccess);
}

void TaskManager::Wakeup(Task *task, int level)
{
	if (task->Running())
	{
		ChangeLevelRunning(task, level);
		return;
	}

	if (level < 0)
	{
		level = task->Level();
	}

	task->SetLevel(level);
	task->SetRunning(true);

	running_[level].push_back(task);
	if (level > current_level_)
	{
		// 次の SwitchTask が呼ばれるタイミングで作動？
		level_changed_ = true;
	}
	return;
}

Error TaskManager::Wakeup(uint64_t id, int level)
{
	auto it = std::find_if(tasks_.begin(), tasks_.end(), [id](const auto &t)
						   { return t->ID() == id; });

	if (it == tasks_.end())
	{
		return MAKE_ERROR(Error::kNoSuchTask);
	}

	Wakeup(it->get());
	return MAKE_ERROR(Error::kSuccess);
}

Error TaskManager::SendMessage(uint64_t id, const Message &msg)
{
	auto it = std::find_if(tasks_.begin(), tasks_.end(), [id](const auto &t)
						   { return t->ID() == id; });

	if (it == tasks_.end())
	{
		return MAKE_ERROR(Error::kNoSuchTask);
	}

	(*it)->SendMessage(msg);
	return MAKE_ERROR(Error::kSuccess);
}

Task &TaskManager::CurrentTask()
{
	return *running_[current_level_].front();
}

void TaskManager::ChangeLevelRunning(Task *task, int level)
{
	if (level < 0 || level == task->Level())
	{
		return;
	}

	// 実行されていない実行可能状態のタスクを置き換えるとき
	if (task != running_[current_level_].front())
	{
		Erase(running_[task->Level()], task);
		running_[level].push_back(task);
		task->SetLevel(level);
		if (level > current_level_)
		{
			level_changed_ = true;
		}
		return;
	}

	// 実行されているタスクを置き換えるとき
	// => 実行中のタスクが自分自身のレベルを書き換えようとしていとき
	running_[current_level_].pop_front();
	running_[level].push_front(task);
	task->SetLevel(level);
	if (level >= current_level_)
	{
		current_level_ = level;
	}
	else
	{
		// 自分のレベルを下げたら、current_level_とlevelの間になにかがいる可能性がある
		current_level_ = level;
		level_changed_ = true;
	}
}

TaskManager *task_manager;

void InitializeTask()
{
	task_manager = new TaskManager;

	__asm__("cli");
	timer_manager->AddTimer(
		Timer{timer_manager->CurrentTick() + kTaskTimerPeriod, kTaskTimerValue});
	__asm__("sti");
}
