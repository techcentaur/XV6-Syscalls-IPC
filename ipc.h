// IPC definitions

#define MSG_SIZE 8

// message definition
typedef struct message{
	int m_id;
	int pid_sender;
	int pid_receiver;
	char *actual_message;
	// char actual_message[MSG_SIZE];
} message;

// queue definition
typedef struct queue{
	int q_id;
	int size;
	struct message *m_first;
	struct message *m_last;
} queue;


// message definition
// typedef struct message{
// 	int m_id;
// 	int pid_sender;
// 	int pid_receiver;
// 	char *actual_message;
// 	// char actual_message[MSG_SIZE];
// } message;

// // queue definition
// typedef struct queue{
// 	int q_id;
// 	int size;
// 	struct message *m_first;
// 	struct message *m_last;
// } queue;
