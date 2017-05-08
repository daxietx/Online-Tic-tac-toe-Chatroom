# Online-Tic-tac-toe-Chatroom
The tic-tac-toe game is played on a 3 * 3 board with two types of stones, black and white. Each player takes turn to play (place a stone anywhere in the board that is still available), with the black always making the first move. The game is won by having three consecutive stones horizontally, vertically, or diagonally on the board. 

The following commands are supported by  the tic-tac-toe server:  

[...] optional field, <......> required field  


| Command | Discription |  
| :------:| :---------: |  
| who | list all users online |
| stats [name] | display information about `[name]` |  
| match `<name>` `<b/w>` `[t]` | try to play with `[name]` as b/w in a t-sec game |
| resign | resign from a game |  
| game | list all current games |  
| observe `<num>` | observe game `<num>` |  
| unobserve | unobserve a game |  
| refresh | redisplay an in-play or observed game |  
| kibitz `<msg>` | comment on an observed game |  
| ' `<msg>` | another form of kibitz |  
| tell `<name>` `<msg>` | tell user `<name>` message |
| shout `<msg>` | shout `<msg>` to every one online |  
| quiet | quiet mode, no broadcast messages (short, kibitz) |  
| nonquiet | non-quiet mode |  
| block `<id>` | not any form of communication from `<id>` |  
| unblock `<id>` | allow communication from `<id>` |  
| mail `<id>` `<title>` | send id a mail |  
| listmail | list the header of the mails |  
| readmail `<msg_num>` | read the particular mail |  
| deletemail `<msg_num>` | delete the particular mail |  
| info `<msg>` | change your information to `<msg>` |  
| passwd `<new>` | change password |  
| exit | quit the system |  
| quit | quit the system |  
| ? | print this message |  
| register `<name>` `<pwd>` | register a new user |  
  
After a game starts, the player types in the coordinate to place each stone. In this game, the
coordinate is represented as XY, where X can be 'A', 'B', 'C' and Y can be 1, 2, 3.  

To run the server and client  

run server:  
```./server <portnumber>  ```  
run client:  
```./client <servername> <portnumber>  ```
