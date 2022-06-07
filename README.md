# Network Programming

## Communication program with TCP between the server and the client
1) Write a communication program between the server and the client.
(Caution: Write so that at least 5 clients are connected.)
2) If the communication between the server and the client is successful, you can check the information of the client through the following command.

#### Command
- /help: Prints a list of commands.
- /hello: When the server process receives the “hello” message, it sends the server's initial connection time to all clients, and prints its own server's initial connection time on the client screen.
- /whoami: Displays information such as the current client's name, access time, and access IP address.
- /getname: Prints the current client's name.
- /getip: Prints the current client's IP address.
- /showclients: Prints a list of names of clients connected to the server.
- /timepassed: Prints how much time has passed since the client connected to the server.
- /exit: Exits the connection of the client with the server.

## File transfer program with FTP between Server and client
1) File transfer The server is running on the server and plays a role in saving and sending the content delivered by the client as a file.
2) File transfer The client selects a file to send to the server and sends it. Executes the function of receiving and saving the selected file from the server.
- The client uploads and downloads files to and from the entire directory.
- The directory where the file sent to the server is saved is where the server process is executed. Downloadable files make it possible to target the entire directory. Files that can be downloaded here are made possible by considering the permission.

#### Command
- GET: Downloads file from server's file.
- PUT: Uploads file to server.
- CDIR: Scans directory of the client.
- SDIR: Scans directory of the server.
- EXIT: Exits the connection of the client with the server.
