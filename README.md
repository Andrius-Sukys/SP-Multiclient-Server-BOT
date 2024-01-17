Socket Programming for Linux. The Program acts like a Chatroom for Multiple Clients.
Bot called BOTAS is able to provide Statistical Data concerning the Chat to every Client individually.

BOTAS' commands include:
"@BOTAS #" – Returns the number of words that a client has sent;
"@BOTAS #MIN" – Returns the shortest word that a client has sent as well as its length (if there are multiple equal in length, the oldest one gets picked);
"@BOTAS #MIN" – Returns the longest word that a client has sent as well as its length (if there are multiple equal in length, the oldest one gets picked).

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

To COMPILE the Program:
1. Go to the directory where the Project Files are located.
2. Compile server.c file by typing "gcc server.c -o server.exe" in your Terminal.
3. Compile botas.c file by typing "gcc botas.c -o botas.exe" in your Terminal.

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

To RUN the Program:
1. The first active instance has to be the Server. Launch server.exe by typing "./server.exe port1" (port1 > 1024!) in your Terminal.
   Example: ./server.exe 2000
2. If data tracking (that is provided by BOTAS) is needed, it needs a separate active instance. Launch botas.exe by typing "./botas.exe port2" (port2 = port1!).
   Example: ./botas.exe 2000
   In order to provide additional security (as clients can name themselves BOTAS), the User will be prompted to enter a password. Enter "notahacker" and press "Enter".
3. Any client may join the chatroom via telnet – telnet ::0 port OR telnet 127.0.0.1 port (works with both IPv4 and IPv6).
   When connected, the client can send any message and it will be visible both to them and other participants in the chatroom. BOTAS' commands are active only from the moment the bot is started and joins the chatroom.

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

In order to visualize BOTAS' capabilities, this is a logged file that depicts a possible conversation with two participants – V1 and telnet6:
```
PRANESIMASV1: 1
PRANESIMASV1: 2
PRANESIMASV1: 3
PRANESIMASV1: 4
PRANESIMASV1: 5
PRANESIMASV1: 6
PRANESIMASV1: 7
PRANESIMASV1: 8
PRANESIMASV1: 9
PRANESIMASV1: 10
PRANESIMASV1: @BOTAS #
PRANESIMASBOTAS: V1 has sent 10 words.
PRANESIMASV1: @BOTAS #MIN
PRANESIMASBOTAS: V1's shortest word is [1], length: 1 letters.
PRANESIMASV1: @BOTAS #MAX
PRANESIMASBOTAS: V1's longest word is [10], length: 2 letters.
PRANESIMAStelnet6: 10
PRANESIMAStelnet6: 20
PRANESIMAStelnet6: 30
PRANESIMAStelnet6: 40
PRANESIMAStelnet6: 50
PRANESIMAStelnet6: 60
PRANESIMAStelnet6: 70
PRANESIMAStelnet6: 80
PRANESIMAStelnet6: 90
PRANESIMAStelnet6: 100
PRANESIMAStelnet6: @BOTAS #
PRANESIMASBOTAS: telnet6 has sent 10 words.
PRANESIMAStelnet6: @BOTAS #MIN
PRANESIMASBOTAS: telnet6's shortest word is [10], length: 2 letters.
PRANESIMAStelnet6: @BOTAS #MAX
PRANESIMASBOTAS: telnet6's longest word is [100], length: 3 letters.
```
