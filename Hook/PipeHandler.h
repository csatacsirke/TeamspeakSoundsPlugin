#pragma once

#include <concurrent_queue.h>


class PipeHandler
{
	std::mutex mutex;

	volatile bool stop;
	concurrency::concurrent_queue<KBDLLHOOKSTRUCT> queue;
	std::function<void(PipeHandler&)> OnNewEntry;
public:
	PipeHandler(void);
	// TODO a destruktor csin�lhat vicces dolgokat... 
	// majd egyszer �rd meg rendesen... 
	//update: lehet hogy megold�dott
	~PipeHandler(void);
	
	bool ListenPipe(CString pipeName);
	void Stop();

	void SetOnNewEntryListener(std::function<void(PipeHandler&)> callback);

	bool TryPop(_Out_ KBDLLHOOKSTRUCT& hookStruct);
	void Push(const KBDLLHOOKSTRUCT& hookStruct);
private:
	BOOL RunPipe(CString pipeName);
};

