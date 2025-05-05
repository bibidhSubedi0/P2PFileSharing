# 🚀 P2P File Sharing and Messaging System (C++ / Boost.Asio)

This project is a **peer-to-peer file sharing and messaging application** written in C++ using **Boost.Asio** for asynchronous networking. It allows multiple peers to connect with each other directly, exchange text messages, and share files in a decentralized manner without relying on a central server.

---

## ✨ Features

* 🔌 **Peer Discovery & Direct Connections**
  Connect directly to peers using their IP address and port.

* 📡 **Message Sending**
  Send direct text messages to connected peers using a `send` command.

* 📣 **Broadcast Messaging**
  Use `echo` to send a message to all connected peers simultaneously.

* 📁 **(Upcoming)** File transfer capabilities between peers.

* 🧵 **Thread-safe Communication**
  All shared structures like the peer map are protected with mutexes to ensure thread safety.

---

## 📦 Dependencies

* [Boost.Asio](https://think-async.com/) (header-only or linked)
* C++17 or later

Make sure Boost is correctly installed on your system.

---

## 🧠 Project Structure

p2p-sharing/<br>
├── include/<br>
│   ├── PeerClient.h        # Client-side networking logic<br>
│   ├── PeerServer.h        # Server to accept incoming peer connections<br>
│   └── utils.h             # Shared utilities (e.g., logging, string ops)<br>
│<br>
├── resources/              # Assets or placeholder for shared files<br>
│<br>
├── src/<br>
│   ├── main.cpp            # Entry point, handles user input & commands<br>
│   ├── PeerClient.cpp      # Implementation of client logic<br>
│   ├── PeerServer.cpp      # Server-side handling of new peer connections<br>
│   └── utils.cpp           # Utility functions used across the project<br>

---

## 🛠️ Commands

### Connect to a peer

```sh
connect username
```

Example:

```sh
connect bob
```

### Send a message to a specific peer

```sh
send <username> <message>
```
Example:

```sh
send peer4 Hello, Peer 4!
```

### List of all the avilable peers in the network

```sh
list
```


### Echo a message to all connected peers

```sh
echo <message>
```

Example:

```sh
echo Hello everyone!
```

---

## 🧪 Example Interaction

Clients
![c1](https://github.com/user-attachments/assets/5a1683d9-57dc-4587-bdef-c09656060031)
![c2](https://github.com/user-attachments/assets/93b3de2b-bcad-4161-8ffa-342643c1772d)

Server Logs
![c3](https://github.com/user-attachments/assets/1ab1b0f8-6a45-436b-89ab-803d4ad10e54)



---

## 🧩 How Peer Identification Works

Each peer is uniquely identified by a string of the form:

```
<peer_name>@<ip>:<port>
```

When connecting, peers must announce their identity, and the application stores this key in a thread-safe map for future messaging.

---

## 📚 Future Improvements

* 🔐 Add peer authentication
* 📁 File sharing (send/receive files)
* 📡 NAT traversal / peer discovery service
* 🖥️ GUI interface using Qt or ImGui

---


Start one instance for each peer using different ports.

---

## 👨‍💻 Author

Developed by [Bibidh Subedi](https://github.com/bibidhsubedi0)
Feel free to open issues or contribute!

---
