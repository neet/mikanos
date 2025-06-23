#include <array>
#include <cstddef>
#include <vector>
#include <cstdint>
#include <deque>
#include <optional>

#include "message.hpp"
#include "error.hpp"

struct TaskContext
{
	uint64_t cr3, rip, rflags, reserved1;
	uint64_t cs, ss, fs, gs;
	uint64_t rax, rbx, rcx, rdx, rdi, rsi, rsp, rbp;
	uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
	std::array<uint8_t, 512> fxsave_area;
} __attribute__((packed));

using TaskFunc = void(uint64_t, int64_t);

static const unsigned int kDefaultLevel = 1;

class Task
{
public:
	static const size_t kDefaultStackBytes = 4096;
	Task(uint64_t id);

	uint64_t ID() const;
	Task &Sleep();
	Task &Wakeup();

	Task &InitContext(TaskFunc *f, int64_t data);
	TaskContext &Context();

	void SendMessage(const Message &msg);
	std::optional<Message> ReceiveMessage();

	unsigned int Level() { return level_; }
	bool Running() { return running_; }

private:
	uint64_t id_;
	std::vector<uint64_t> stack_;
	alignas(16) TaskContext context_;
	std::deque<Message> msgs_;
	unsigned int level_{kDefaultLevel};
	bool running_{false};

	Task &SetLevel(int level)
	{
		level_ = level;
		return *this;
	}

	Task &SetRunning(bool running)
	{
		running_ = running;
		return *this;
	}

	friend class TaskManager;
};

class TaskManager
{
public:
	static const int kMaxLevel = 3;

	TaskManager();
	Task &NewTask();
	// current_sleep = 実行中のタスクをスリープさせてから次へ移る
	void SwitchTask(bool current_sleep = false);

	void Sleep(Task *task);
	Error Sleep(uint64_t id);
	void Wakeup(Task *task, int level = -1);
	Error Wakeup(uint64_t id, int level = -1);
	Error SendMessage(uint64_t id, const Message &msg);
	Task &CurrentTask();

private:
	// 全ての状態のタスクが入っている
	std::vector<std::unique_ptr<Task>> tasks_{};
	uint64_t latest_id_{0};
	// 実行可能状態のタスクが入っている
	std::array<std::deque<Task *>, kMaxLevel + 1> running_{};
	int current_level_{kMaxLevel};
	bool level_changed_{false};

	void ChangeLevelRunning(Task *task, int level);
};

extern TaskManager *task_manager;

void InitializeTask();
