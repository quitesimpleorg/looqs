#ifndef IPC_H
#define IPC_H

enum IPCCommand
{
	GeneratePreviews = 23,
	StopGeneratePreviews,
};

enum IPCReply
{
	FinishedGeneratePreviews,
};

#endif // IPC_H
