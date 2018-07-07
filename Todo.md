# Todo

Below is the current todo list for the project. The legend is as follows:

----------------------------


- [x] Hello World
- [x] DPServer Class
- [ ] Server class
- [ ] Client class
- [ ] INET Library

	- -[x] Socket
	- -[x] ServiceAddress
	- -[x] IPConnection
	- -[ ] TCPConnection
	- -[ ] UDPConnection
	- -[ ] MasterTCPConnection
		- -[x] Accept
		- -[x] Connect
		- -[ ] Before we go forward. We need to consider our design. We
			shouldn't simply create one thread for every connection. That
			will bog us down. So take time to consider if we can simply do
			everything in one thread… We'll need to make a branch for this
			so we can revert changes
			- -[x] Create a branch called connection-redesign
			- -[ ] Consider if we should make more than one thread, if so, how
				many for server and clients, and what will their purpose be?
				- -[ ] Design a single thread connection
				- [ ] removeConnection - For removing previously accepted
				  connections
					- [ ] Interface
					- [ ] Implementation
					- [ ] Test
				- -[x] Remove connection maps, and make vectors again…
				- -[x] IPConnection::operator int()
					- -[x] Interface
					- -[x] Implementation
					- -[x] Test
				- -[x] getLargestSocket
					- -[x] Interface
					- -[x] Implementation
				- -[x] checkAllConnectionsForData Function
					- -[x] Interface
					- -[x] Implementation
				- [ ] getNumConnections - We need this so that we can create
				  a successful test case for listenForIncominConnections. The
				  Server side should test to see if the number of connections
				  is greater than 0
				  	- [ ] Interface
					- [ ] Implementation
					- [ ] Test
				- -[ ] listenForIncominConnections Function
					- -[x] Interface
					- -[x] Implementation
					- -[ ] Test
				- Layout
					- Basic steps
						- Listen for a knock at the door or for incoming letters
						- If It's a knock at the door
							- Open the door when some one is there
							- Decide whether to let them in
							- Once they're in add them to the list of connections
						- If it's a letter
							- Process the message

	  - -[ ] Send
	  - -[ ] Recv
	  - -[ ] sendFrom
	  - -[ ] recvFrom
