Sample README.txt

 Group Member:
    Tianji Li
    Siyang Sun

Contribution:
MS1:
    Tianji wrote both the connection and the receiver. 
    Siyang wrote the sender. 

Eventually your report about how you implemented thread synchronization
in the server should go here

Report:
Critical Sections:
1. map of rooms on a server (Server m_rooms)
2. list of clients in a room (Room memebrs)
3. messages queue of each client (MessageQueue m_messages)

Determination:
The critical sections are determined by whether it will be accessed by multiple 
thread at the same time(if there might be a race condition).

The RoomMap m_rooms might be accessing by multiple thread at the same time if there
are multiple user trying to join in the room simultaneously.

The Userser members might be accessing by multiple thread at the same time if there
are multiple users joining a room and/or leaving a room simultaneously.

The deque m_messages might be accessing by multiple thread at the same time if there
are multiple users trying to broadcasting and/or receiving messages simultaneously. 

Choose synchronization primitives:
we choose mutex because this ensures that if one thread is executing the code, other 
threads cannot executing the same part at the same time. And for the server implementation
we need to prevent race conditions of any kinds. mutex matches the best with our needs.

Meet synchronization requirements:
By using Guard, we lock then unlock the shared data (Server, Room, and MessageQueue) for operations on them. 
This ensures that no matter what operations different users are trying to do "at the same time"
the operations will always execute one by one in order, which prevent race conditions from happening.



