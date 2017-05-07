#include <vector>
#include <string>
#include <cstring>
#include <sstream>
#include <pthread.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <time.h>
#include "user.h"
#include "game.h"

using namespace std;

#define GUEST 0
#define USER 1
#define LOGIN 2
//---XIE---
#define MAIL 3

#define MAXONLINE 10
#define MAXBUFLEN 8192

int highestsock = 0;
int SERVERPORT = 5000;

fd_set master;
fd_set readfds;

vector<struct User> users;
vector<struct Game> games;

int listener = -1; // listen the connection request from client

//----EDITED!------
const char* authorized_info = "                       -=-= AUTHORIZED USERS ONLY =-=-\n\
You are attempting to log into online tic-tac-toe Server.\n\
Please be advised by continuing that you agree to the terms of the\n\
Computer Access and Usage Policy of online tic-tac-toe Server.\n\n\n";

const char* cmd_list = "Commands supported:\n\
  who                     # List all online users\n\
  stats [name]            # Display user information\n\
  game                    # list all current games\n\
  observe <game_num>      # Observe a game\n\
  unobserve               # Unobserve a game\n\
  match <name> <b|w> [t]  # Try to start a game\n\
  <A|B|C><1|2|3>          # Make a move in a game\n\
  resign                  # Resign a game\n\
  refresh                 # Refresh a game\n\
  shout <msg>             # shout <msg> to every one online\n\
  tell <name> <msg>       # tell user <name> message\n\
  kibitz <msg>            # Comment on a game when observing\n\
  ' <msg>                 # Comment on a game\n\
  quiet                   # Quiet mode, no broadcast messages\n\
  nonquiet                # Non-quiet mode\n\
  block <id>              # No more communication from <id>\n\
  unblock <id>            # Allow communication from <id>\n\
  listmail                # List the header of the mails\n\
  readmail <msg_num>      # Read the particular mail\n\
  deletemail <msg_num>    # Delete the particular mail\n\
  mail <id> <title>       # Send id a mail\n\
  info <msg>              # change your information to <msg>\n\
  passwd <new>            # change password\n\
  exit                    # quit the system\n\
  quit                    # quit the system\n\
  help                    # print this message\n\
  ?                       # print this message\n";                              

vector<string> cmd_process(string cmd,int level,vector<struct User>::iterator uit)    //只检查键盘输入格式 +  state
{
	vector<string> order;
	string inst,para,str;
	stringstream ss;
    cmd = cmd + " " + "#" + " ";
	ss<<cmd;
	ss>>inst;
    if(level==GUEST)
    {
		if(inst.compare("quit")==0 || inst.compare("exit")==0)
        {
            order.push_back(inst);
        }
        //----EDITED!------
        else if(inst.compare("register")==0)
        {
            int i;
            order.push_back(inst);
            cmd = cmd.substr(0,cmd.size()-3);
            i = cmd.find("register");
            cmd = cmd.erase(0,i+8);
            
            ss.clear();  // reset stringstream
            ss.str("");
            
            para = "";
            ss<<cmd;
            ss>>para;
            if(!para.empty()) // has paras
            {
                order.push_back(para); // get username
                
                i = cmd.find(para);
                cmd = cmd.erase(0,i+para.size());
                
                ss.clear();  // reset stringstream
                ss.str("");
                para = "";
                ss<<cmd;
                ss>>para;
                if(!para.empty())
                    order.push_back(para); // get password               
            }
                    
            // ss>>para;
            // order.push_back(para);
            // ss>>para;
            // order.push_back(para);
        }       
    }
    else if(level==LOGIN)
    {
        uit->state |= ONLINE;
	    if(inst.compare("quit") == 0 || inst.compare("exit") == 0)
            order.push_back(inst);
        
		else if(inst.compare("who") == 0)
		    order.push_back(inst);
		
		else if(inst.compare("stats") == 0)  //stats name,如果没有参数， stats自己本人
		{
			order.push_back(inst);
			ss<<cmd;
			ss>>para;
			order.push_back(para);
			ss<<cmd;
			ss>>para;
			if(para.compare("#") == 0)
				order.push_back(uit->name);
			else
			    order.push_back(para);
		}
		
		else if(inst.compare("game") == 0)
		    order.push_back(inst);
		
		else if(inst.compare("observe") == 0)
		{
			uit->state |= OBSERVER;
			order.push_back(inst);
			ss<<cmd;
			ss>>para;
			if(para.compare("#") != 0)
				order.push_back(para);
		}
		
		else if(inst.compare("unobserve") == 0)
		{
			order.push_back(inst);
		    if((uit->observing_games).size() == 0)
				uit->state ^= OBSERVER;
		}
		else if(inst.compare("match") == 0)
		{	
			vector<struct User>::size_type j;
			vector<struct Match_info>::size_type i;
			vector<string>::size_type p;
			string nam;
			string s = "";
			char buf[200];
			string msg = "match ";
			order.push_back(inst);   //"match"
			ss>>para;  
	        if(para.compare("#") != 0)    //有名字
			{
				for(j = 0; j < users.size(); j++)
				{
					if((users[j].name).compare(para)==0)
						break;
				}
	           
				if(j != users.size())    //名字验证确实为users成员
				{
					nam = para;   
				    order.push_back(para);
                    
					msg = msg + para + " ";
                    
					ss>>para;  // b/w
				    if(para.compare("b") != 0)
					{
                         for(i = 0; i < (uit->matches).size(); i++)
                         {
                             //printf("\n\n%s\n\n",((uit -> matches)[i].partner).c_str());
                             if(((uit->matches)[i].partner).compare(nam) == 0)
                                 break;
                         }
                         if(i!=(uit->matches).size())
                         {
                             string color;
                             stringstream ss2;
                             ss2 << (uit->matches)[i].invitation_recv;
                             for(int u = 0; u < 3; u++)
                                 ss2>>color;
                             if(color.compare("b")==0)
                                 color = "w";
                             else
                                 color = "b";
                             order.push_back(color);
                             msg = msg + color + " ";                             
                         }
                         
                         else
                         {
                            order.push_back("w");
					        msg = msg + "w" +" "; 
                         }    
					}
					else if(para.compare("b") == 0)
					{
						order.push_back("b");
						msg = msg + "b" + " ";
					}
                    //-------------------------------
					if(para.compare("#") == 0)
					{				    
						order.push_back("600");
                        order.push_back("300");
                        msg = msg + "600 300";						 
					}
                    else
					{                         
                         ss>>para;
                         if(para.compare("#") == 0 || para.find_first_not_of("1234567890") != string::npos)  //空或者格式不对
						 {
							 order.push_back("600");
							 order.push_back("300");
							 msg = msg + "600 300";
						 }
                         else
						 {
							 order.push_back(para);
							// ss<<cmd;
							 //ss>>para;
							 /*if(para.compare("#") == 0 || para.find_first_not_of("1234567890") != string::npos)  //空或者格式不对
						    {
								 order.push_back("600");
								 msg = msg + "600" + " ";
						    }
							else
							{
								order.push_back(para);
							    msg = msg + para + " ";
							}*/
                            msg = msg + para + " ";
							ss>>para;
							
							if(para.compare("#") == 0 || para.find_first_not_of("1234567890") != string::npos)  //空或者格式不对
						    {
								 order.push_back("300");
								 msg = msg + "300";
						    }
							else
							{
								order.push_back(para);
							    msg = msg + para;
							}
						 }						 
					}
		             if(!(users[j].state & ONLINE))
					 {
						 sprintf(buf, "User %s is not online.\n",nam.c_str());
						 uit->match_errno = string(buf);
					 }
					 
                     if(uit->state & PLAYER)
					 {
						 printf("6666666-----%s\n",msg.c_str());
						 sprintf(buf, "Please finish your present game first.\n");
						 uit->match_errno = string(buf);
					 }
                     
                     if((users[j].name).compare(uit->name) == 0)
                     {
                         sprintf(buf, "You can't match with yourself.\n");
                         uit->match_errno = string(buf);
                     }
						 
					 for(p=0; p < (users[j].block).size(); p++)
					 {
						 if((uit->name).compare((users[j].block)[p]) == 0)
						 {
							 sprintf(buf,"You are blocked by User %s.\n", nam.c_str());
							 uit->match_errno = string(buf);
						 }
					 }

                     for(i = 0; i < (uit->matches).size(); i++)
					 {
						 printf("\n\n%s\n\n",((uit -> matches)[i].partner).c_str());
						 if(((uit->matches)[i].partner).compare(nam) == 0)
							 break;
					 }
                     
					 if(i != (uit->matches).size())   //match 生成过
					 {
						(uit->matches)[i].invitation_send = msg;
                         for(i = 0; i < (users[j].matches).size(); i++)
						 {
							 if(((users[j].matches)[i].partner).compare(uit->name) == 0)
								 break;
						 }
                         (users[j].matches)[i].invitation_recv = msg;						 
					 }      
					 else
					 {
						 struct Match_info newmatch1(nam,s,msg);
						 printf("%s\n\n\n\n", ((newmatch1.invitation_send).c_str()));
						 (uit->matches).push_back(newmatch1);
						 struct Match_info newmatch2(uit->name,msg,s);
						 (users[j].matches).push_back(newmatch2);
					 }						 
				}
				else
				{
                    nam = para;
					sprintf(buf, "%s is not a legal user.\n", nam.c_str());
					uit->match_errno = string(buf);
				}					
		    }
            else
             	uit->match_errno = "User  is not online.\n";			
		    printf("==================end=====================\n");
		}
	     
        else if(inst.compare("a1") == 0 || inst.compare("a2") == 0 || inst.compare("a3") == 0
       || inst.compare("b1") == 0 || inst.compare("b2") == 0 || inst.compare("b3") == 0
        || inst.compare("c1") == 0 || inst.compare("c2") == 0 || inst.compare("c3") == 0
        || inst.compare("A1") == 0 || inst.compare("A2") == 0 || inst.compare("A3") == 0
        || inst.compare("B1") == 0 || inst.compare("B2") == 0 || inst.compare("B3") == 0
        || inst.compare("C1") == 0 || inst.compare("C2") == 0 || inst.compare("C3") == 0)
        {
			order.push_back("move");
			order.push_back(inst);
		}	
		
		else if((inst.compare("resign")) == 0)
			order.push_back(inst);
		
		else if((inst.compare("refresh")) == 0)
			order.push_back(inst);
		//----EDITED!------
		else if((inst.compare("shout")) == 0)
		{
            order.push_back(inst);
            cmd = cmd.substr(0,cmd.size()-3);
            if(cmd.size()==5 || cmd.size()==6) // the command is <shout> or <shout >
                order.push_back("");
            else
            {
                string msg(cmd,6,cmd.size());
                cout<<msg<<endl;
                order.push_back(msg);                
            }
		}
		
		else if((inst.compare("tell")) == 0)
		{
            //----EDITED!------
            int i;
			order.push_back(inst);   //tell
            cmd = cmd.substr(0,cmd.size()-3);
            i = cmd.find("tell");
            cmd = cmd.erase(0,i+4);
            
            ss.clear(); // clear the stringstream
            ss.str("");
            
            para = "";            
            ss<<cmd; // reset the stringstream
			ss>>para;
            if(!para.empty()) // has paras
            {
                order.push_back(para); // get name
                
                i = cmd.find(para);
                cmd = cmd.erase(0,i+para.size());
                
                if(cmd.size()>1)
                {
                    string msg = cmd.substr(1);
                    order.push_back(msg);
                }               
            }
		}
		
		else if((inst.compare("kibitz")) == 0 || (inst.compare("'")) == 0)
		{
			 order.push_back(inst);
			 ss<<cmd;
			 ss>>para;
			 if(para.compare("#") != 0)
				 order.push_back(para);
		}
		
		else if((inst.compare("quiet")) == 0 || (inst.compare("nonquiet")) == 0)
			order.push_back(inst);
		
		else if((inst.compare("block")) == 0 || (inst.compare("unblock")) == 0)
		{
			order.push_back(inst);
			ss<<cmd;
			ss>>para;
			if(para.compare("#") != 0)
				order.push_back(para);
		}
		
		else if((inst.compare("listmail")) == 0)
			order.push_back(inst);
		//---XIE---
	    else if((inst.compare("readmail")) == 0)
		{
            int i;
			order.push_back(inst);
            cmd = cmd.substr(0,cmd.size()-3);
            i = cmd.find("readmail");
            cmd = cmd.erase(0,i+8);
            
            ss.clear(); // clear the stringstream
            ss.str("");
            
            para = "";            
            ss<<cmd; // reset the stringstream
			ss>>para;
            if(!para.empty()) // has paras
            {
                order.push_back(para);
            }				
		}
        //---XIE---
        else if((inst.compare("deletemail")) == 0)
        {
            int i;
			order.push_back(inst);
            cmd = cmd.substr(0,cmd.size()-3);
            i = cmd.find("deletemail");
            cmd = cmd.erase(0,i+10);
            
            ss.clear(); // clear the stringstream
            ss.str("");
            
            para = "";            
            ss<<cmd; // reset the stringstream
			ss>>para;
            if(!para.empty()) // has paras
            {
                order.push_back(para);
            }	            
        }
		//----EDITED!------
	    else if((inst.compare("mail")) == 0)
		{
            int i;
			order.push_back(inst);   //tell
            cmd = cmd.substr(0,cmd.size()-3);
            i = cmd.find("mail");
            cmd = cmd.erase(0,i+4);
            
            ss.clear(); // clear the stringstream
            ss.str("");
            
            para = "";            
            ss<<cmd; // reset the stringstream
			ss>>para;
            if(!para.empty()) // has paras
            {
                order.push_back(para); // get name
                
                i = cmd.find(para);
                cmd = cmd.erase(0,i+para.size());
                
                if(cmd.size()>1)
                {
                    string msg = cmd.substr(1);
                    order.push_back(msg); // get title
                }               
            }
		}
		//----EDITED!------
		else if((inst.compare("info")) == 0)
		{
            order.push_back(inst);
            cmd = cmd.substr(0,cmd.size()-3);
            int i  = cmd.find("info");
            string msg = cmd.substr(i+4);           
            if(msg.size()>1)
            {
               msg = msg.substr(1);
               order.push_back(msg);                
            } 
		}
        //----EDITED!------
        else if((inst.compare("passwd")) == 0)
        {
            order.push_back(inst);
            cmd = cmd.substr(0,cmd.size()-3);
            if(cmd.size()>7)
            {
                string msg(cmd,7,cmd.size());
                order.push_back(msg);                
            }          
        }
		
		else if((inst.compare("exit")) == 0 || (inst.compare("quit")) == 0 || (inst.compare("help")) == 0 || (inst.compare("?")) == 0)
			order.push_back(inst);
    }
	return order;
}

void *handler(void *arg)
{
	pthread_detach(pthread_self()); // the main thread won't be blocked even if the child doesn't finish
    char sendbuf[MAXBUFLEN];
    char recvbuf[MAXBUFLEN] = {0};
    string cmd;
    int n;
	int sockfd = *(int *)arg;  // get the socket of the user
	vector<string> order; // store the user command
	vector<struct User>::iterator uit; // the iterator for vector users
	User player;
    //---XIE---
    Mymail mail;
    string mailmsg = "";
    vector<struct User>::iterator recvit;
    
	int countrecv = 0; // count how many times the server recv from client
	int countguest = 0; // count lines as a guest
    int countuser = -1;
	int level = USER;

	// inform client connected
	memset(sendbuf,'\0',MAXBUFLEN);
	sprintf(sendbuf,authorized_info);
	send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);
	memset(sendbuf,'\0',MAXBUFLEN);
	sprintf(sendbuf,"username (guest):");
	send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);

	while(1)
	{
		n = recv(sockfd,recvbuf,MAXBUFLEN,0);
		if(n<0)
		{
			printf("Fail to read.\n");
            close(sockfd);
			pthread_exit(NULL);

		}
		else if(n==0)
		{
			printf("User quit.\n");  // ? STILL HAS QUESTION
            close(sockfd);
			pthread_exit(NULL);
		}
		else
		{
			countrecv++;
            cmd = string(recvbuf);
            cmd = cmd.substr(0,n-2); // get the command and remove the newline
			cout<<cmd.size()<<endl;
			printf("get:%s\n",cmd.c_str());
			if(countrecv==1 && cmd.size()==0)
				level = GUEST;
			//------------------------------Guest-----------------------------------
			if(level==GUEST)
			{
				if(cmd.size()==0) // get cmd: newline
				{
                    //----EDITED!------
					if(countrecv==1) // should show Commands
					{
                        memset(sendbuf,'\0',MAXBUFLEN);
                        sprintf(sendbuf,cmd_list);
                        send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);

                        memset(sendbuf,'\0',MAXBUFLEN);
						sprintf(sendbuf,"You log in as a guest. The only command you can use is\n'register username password'\n\n");
						send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);
                        
						memset(sendbuf,'\0',MAXBUFLEN);						
						sprintf(sendbuf,"<guest: %d>",countguest);
						send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);
					}
					else
					{
						sprintf(sendbuf,"You are not supposed to do this.\nYou can only use 'register username password' as a guest.\n");
						send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);
						memset(sendbuf,'\0',MAXBUFLEN);
                        
                        memset(sendbuf,'\0',MAXBUFLEN);
                        sprintf(sendbuf,"<guest: %d>",++countguest);
                        send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);                        
					}
				}
				else // get cmd: register (have to check cmd)
				{
                    order = cmd_process(cmd,GUEST,uit);	//check the command's format
                    if(!order.empty())
                    {
                        //----EDITED!------
                        if(order[0].compare("register")==0)  // new user register
                        {
                            bool registered = false;
                            if(order.size()<3)
                            {
                                memset(sendbuf,'\0',MAXBUFLEN);
                                sprintf(sendbuf,"Parameters are insufficient! Please give out username and password.\n");
                                send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);                                
                            }
                            else
                            {
                                for(uit=users.begin();uit!=users.end();uit++)
                                {
                                    if(order[1].compare(uit->name)==0)
                                    {
                                        registered = true;
                                        break;
                                    }
                                }
                                if(registered) // the name has already be registered
                                {
                                    memset(sendbuf,'\0',MAXBUFLEN);
                                    sprintf(sendbuf,"%s has already been registered! Please change a username.\n",order[1].c_str());
                                    send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);                                 
                                }
                                else
                                {
                                    player.name = order[1];
                                    player.pwd = order[2];
                                    //---XIE---
                                    player.sockfd = -1; 
                                    player.mails.clear();
                                    cout<<"playermail:"<<player.mails.size()<<endl;
                                    users.push_back(player);
                                    //----------------------inform user register succeed-----------------------
                                    memset(sendbuf,'\0',MAXBUFLEN);
                                    sprintf(sendbuf,"User %s registered!\n",order[1].c_str());
                                    send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);
                                    printf("User %s registered!\n",order[1].c_str());                               
                                }                                
                            }
                            memset(sendbuf,'\0',MAXBUFLEN);
                            sprintf(sendbuf,"<guest: %d>",++countguest);
                            send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);                            
                        }
                        //----EDITED!------
                        else if(order[0].compare("quit")==0 || order[0].compare("exit")==0)
                        {
                            printf("Guest quit.\n"); 
                            memset(sendbuf,'\0',MAXBUFLEN);
                            sprintf(sendbuf,"Thank you for using Online Tic-tac-toe Server.\nSee you next time.\n\n");
                            send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);                            
                            close(sockfd);
                            pthread_exit(NULL);
                        }
                    }
                    //----EDITED!------
                    else
                    {
						memset(sendbuf,'\0',MAXBUFLEN);
                        sprintf(sendbuf,"You are not supposed to do this.\nYou can only use 'register username password' as a guest.\n");
						send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);
                        printf("Invalid command!\n");
                        memset(sendbuf,'\0',MAXBUFLEN);
                        sprintf(sendbuf,"<guest: %d>",++countguest);
                        send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);                        
                    }
                    order.clear();
				}
			}
			//-------------------------------User----------------------------------
			else if(level==USER)
			{
				//-----------------------Login---------------------
				if(countrecv==1)
				{
					bool isauser = false;
					for(uit = users.begin();uit!=users.end();uit++)
					{
						// printf("user:%s\n",uit->name.c_str());
						// printf("judge:%d\n",uit->name.compare(cmd));
						if(uit->name.compare(cmd)==0)
						{
							isauser = true;
							break;
						}
					}
					if(isauser==false)  // can't find the user
					{
						memset(sendbuf,'\0',MAXBUFLEN);
						sprintf(sendbuf,"The user doesn't exit!\nThank you for using Online Tic-tac-toe Server.\nSee you next time.\n");
						send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);
						close(sockfd);
						pthread_exit(NULL);
					}
					else // found the user 
					{
						memset(sendbuf,'\0',MAXBUFLEN);
						sprintf(sendbuf,"password:");
						send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);
					}
				}
                else if(countrecv==2) // check the password
                {
                    cout<<"pwd:"<<uit->pwd<<"get:"<<cmd<<endl;
                    if(uit->pwd.compare(cmd)==0) //password match
                    {
                        if(uit->state & ONLINE) // if the user is already login somewhere
                        {                      
                            int pastsockfd = uit->sockfd; //record the past sockfd
                            cout<<"pastsockfd:"<<pastsockfd<<endl;
                            uit->state ^= ONLINE;
                            memset(sendbuf,'\0',MAXBUFLEN);
                            sprintf(sendbuf,"You are not in the room.\nYou login from another place.\nThank you for using Online Tic-tac-toe Server.\nSee you next time.\n\n");
                            send(pastsockfd,(void*)(sendbuf),sizeof(sendbuf),0);
                            close(pastsockfd);
                        }
                        uit->sockfd = sockfd;
                        cout<<"currentsockfd:"<<sockfd<<endl;
                        uit->state |= ONLINE;
                        level = LOGIN;
                        memset(sendbuf,'\0',MAXBUFLEN);
                        sprintf(sendbuf,"%s now login!\n",uit->name.c_str());
                        send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);
                        //----EDITED!------
                        memset(sendbuf,'\0',MAXBUFLEN);
                        sprintf(sendbuf,cmd_list);
                        send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);                        
                        //-----UNFINISH: show unread mail-------------
                        //---XIE---
                        vector<struct Mymail>::iterator mit;
                        int unread = 0;
                        for(mit=uit->mails.begin();mit!=uit->mails.end();mit++)
                        {
                            if((*mit).ifread.compare("N")==0)
                                unread++;
                        }
                        if(unread!=0)
                        {
                            memset(sendbuf,'\0',MAXBUFLEN);
                            sprintf(sendbuf,"You have %d unread message.\n",unread);
                            send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);                            
                        }
                        memset(sendbuf,'\0',MAXBUFLEN);
                        sprintf(sendbuf,"<%s: %d>",uit->name.c_str(),++countuser);
                        send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);
                        
                    }
                    else //password doesn't match
                    {
						memset(sendbuf,'\0',MAXBUFLEN);
						sprintf(sendbuf,"Login failed!\nThank you for using Online Tic-tac-toe Server.\nSee you next time.\n");
						send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);
						close(sockfd);
						pthread_exit(NULL);                        
                    }
                }
			}
            else if(level==LOGIN)
            {
                if(cmd.size()==0) //newline
                {
                    memset(sendbuf,'\0',MAXBUFLEN);
                    sprintf(sendbuf,"<%s: %d>",uit->name.c_str(),++countuser);
                    send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);                    
                }
                else
                {
                    order = cmd_process(cmd,LOGIN,uit); //check the command and format
                    if(order.empty())  // the command is invalid
                    {
						memset(sendbuf,'\0',MAXBUFLEN);
                        sprintf(sendbuf,"Invalid command!\n");
						send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);
                        printf("Invalid command!\n");                         
                    }
                    else
                    {
                        if(order[0].compare("who") == 0)
						{
							int count;
							string online_users = "\0";
							for(vector<struct User>::size_type i = 0; i < users.size(); i++)
								if(users[i].state & ONLINE)
								{
									count++;
									online_users = online_users + users[i].name + " ";
							    }
							online_users += "\n"; 	
							sprintf(sendbuf,"Total %d user(s) online:\n", count);
							send(sockfd, (void *)(sendbuf), sizeof(sendbuf), 0);
							send(sockfd, (void *)(online_users.c_str()),online_users.size(), 0);
						}
						
					    else if((order[0].compare("game")) == 0)
						{
							vector<struct Game>::size_type i,j;
							i = games.size();
							sprintf(sendbuf,"Total %ld game(s):\n",i);
							send(sockfd,(void *)(sendbuf), sizeof(sendbuf), 0);
							for(j = 0; j < i; j++)
							{
								sprintf(sendbuf, "Game %ld(%ld): %s .vs. %s, %d moves\n", j+1, j+1, (games[j].player1).c_str(), (games[j].player2).c_str(), games[j].moves);
								send(sockfd, sendbuf, sizeof(sendbuf), 0);
							}
						}
						
						else if(order[0].compare("observe") == 0)
                        {
			                if(order[1].size() == 0)
								cerr << "Usage: observe <game_num>" << endl;
						    else if(atoi(order[1].c_str()) > (int)(games.size()) - 1 || atoi(order[1].c_str()) < 0)
								cerr << "game_num overflows." << endl;
							else
							{
							    (uit->observing_games).push_back(atoi(order[1].c_str()));
							    (games[atoi(order[1].c_str())].observer).push_back(sockfd);
								uit->state |= OBSERVER;
								sprintf(sendbuf,"You are now observing the game.\n");
								send(sockfd, sendbuf, sizeof(sendbuf), 0);
							}         									
						}
						
					    else if(order[0].compare("unobserve") == 0)
						{	
							if((uit->observing_games).size() == 0)
								cerr << "You are not observing anything." << endl;
							else
							{
							   int gamid;
							   gamid = (uit->observing_games)[(uit->observing_games).size() - 1];
							   uit->observing_games.pop_back();
							   if((uit->observing_games).size() == 0)
								   uit->state ^= OBSERVER;
							   sprintf(sendbuf, "Unobserving game %d",gamid);
							   send(sockfd, sendbuf, sizeof(sendbuf), 0);
						    }
						}
						
						else if(order[0].compare("stats") == 0)
						{   	
							string block_list = "";
							vector <User>:: iterator l_uit;
							vector <struct User>::iterator p;
							if(order[1].size() == 0)
								l_uit = uit;
							else
							{								
								for(p = users.begin(); p != users.end(); p++)
								{
									if(p->name.compare(order[1]) == 0)
										break;
								}
								if(p == users.end())
								{
									sprintf(sendbuf,"User %s does not exist.\n", (order[1]).c_str());
									send(sockfd, sendbuf, sizeof(sendbuf), 0);
								}
				
								else
								l_uit = p;
							}
							if(p != users.end())
							{
								sprintf(sendbuf, "\nUser: %s\n", (l_uit->name).c_str());
								send(sockfd, sendbuf, sizeof(sendbuf), 0);
								sprintf(sendbuf,"Info: %s\n", (l_uit->info).c_str());
								send(sockfd, sendbuf, sizeof(sendbuf), 0);
								sprintf(sendbuf, "Rating: %f\n", l_uit->rating);
								send(sockfd, sendbuf, sizeof(sendbuf), 0);
								sprintf(sendbuf,"Wins: %d, Loses: %d\n", l_uit->winN, l_uit->loseN);
								send(sockfd, sendbuf, sizeof(sendbuf), 0);
								if(l_uit->state & QUIET)
								    sprintf(sendbuf,"Quiet: YES\n");
							    else
									sprintf(sendbuf,"Quiet: NO\n");
								send(sockfd, sendbuf, sizeof(sendbuf), 0);
								for(vector<string>::size_type t = 0; t < (uit->block).size(); t++)
									block_list = block_list + (uit->block)[t];
								sprintf(sendbuf,"Blocked users: %s\n\n", (block_list).c_str());
								send(sockfd, sendbuf, sizeof(sendbuf), 0);
								if(l_uit->state & ONLINE)
									sprintf(sendbuf,"%s is online.\n", (l_uit->name).c_str());
								else
									sprintf(sendbuf,"%s is off-line.\n",(l_uit->name).c_str());
								send(sockfd, sendbuf, sizeof(sendbuf), 0);
							}  	   
						}
						else if(order[0].compare("match") == 0)
						{
							vector<User>::size_type i;
							vector<struct Match_info>:: size_type j;
							char output[500];
							output[0]='\0';
                            string player1;
                            string player2;
                            int time_flag = 0;
                            time_t time;
							
							if((uit->match_errno).compare("") != 0)
							{
                              					 printf("====================747===================\n");												 
							  send(sockfd, (uit->match_errno).c_str(), (uit->match_errno).size(), 0);	
							}
							else 
							{	
								for(j = 0; j < ((uit->matches).size()); j++)  //找到当前match 对象
								{
									if(((uit->matches)[j].partner).compare(order[1]) == 0)
									 break;
								}
								
							    for(i = 0; i < users.size(); i++)
								{
									if((users[i].name).compare(order[1])==0)
										break;
								}
													 printf("=============757==========================\n");
													 printf("%s\n\n\n\n",((uit->matches)[j].invitation_recv).c_str());
								if(((uit->matches)[j].invitation_recv).compare("") != 0)
								{
                                    printf("===========760============================\n");
                                    printf("%s\n",((uit->matches)[j].invitation_recv).c_str());
                                    printf("%s\n",((uit->matches)[j].invitation_send).c_str());
									sprintf(output,"%s wants <%s>, %s wants <%s>\n", (uit->name).c_str(),((uit->matches)[j].invitation_send).c_str(),((uit->matches)[j].partner).c_str(),((uit->matches)[j].invitation_recv).c_str());
									stringstream s1((uit->matches)[j].invitation_recv);
									stringstream s2((uit->matches)[j].invitation_send);
									string word1,word2;
									// while(s1 >> word1 && s2 >> word2)
									// {
										// cout << word1 << word2<< endl;
										// if(word1.compare("match") == 0 && (word2.compare("match")) == 0)
											// continue;
										// if((word1.compare(uit->name) == 0) && (word2.compare(users[j].name) == 0))
											// continue;
										// if(((word1.compare("b") == 0)&&(word2.compare("w") == 0)) || 
                                        // ((word1.compare("w") == 0)&&(word2.compare("b") == 0)))
											// continue;
										// if(word1.compare(word2) == 0 && (word1.find_first_not_of("0123456789")) == string::npos)
											// continue;
										// else
											// break;
									// }
                                    for(int kk=0; kk < 5; kk++)
                                    {
                                        s1>>word1;
                                        s2>>word2;
										cout << word1 << word2<< endl;
										if(word1.compare("match") == 0 && (word2.compare("match")) == 0)
											continue;
										if((word1.compare(uit->name) == 0) && (word2.compare(users[j].name) == 0))
											continue;
										if(((word1.compare("b") == 0)&&(word2.compare("w") == 0)) || 
                                        ((word1.compare("w") == 0)&&(word2.compare("b") == 0)))
                                        {
                                            if(word1.compare("b") == 0)
                                            {
                                                player1 = uit->name;
                                                uit->state |= ACTIVE;
                                                player2 = users[i].name;
                                                printf("player1 = %s---------players = %s---", player1.c_str(), player2.c_str());
                                            }

                                            else if(word2.compare("b") == 0)
                                            {
                                                player1 = users[i].name;
                                                users[i].state |= ACTIVE;
                                                player2 = uit->name;
                                                printf("player1 = %s---------players = %s---", player1.c_str(), player2.c_str());
                                            }                                            

                                            continue;
                                        }
											
										if(word1.compare(word2) == 0 && (word1.find_first_not_of("0123456789")) == string::npos)
                                        {
                                            if(time_flag ==0)
                                            {
                                                time = atoi(word1.c_str());
                                                time_flag = 1;
                                            }
                                            continue;
                                        }

										else
											break;                                        
                                    }
									if((s1>>word1 || s2 >>word2)&&(word1.compare("#")!=0)&&(word2.compare("#"))!=0)
									{
									   send(sockfd, output,sizeof(output),0);
									   send(users[i].sockfd, output,sizeof(output), 0);
									}
								    else
									{
										fflush(stdout);
										printf("------------------------MATCH!-------------------------------\n");
                                        struct Game newgame(player1, player2, time);
                                        games.push_back(newgame);
                                        uit->mygameID = games.size()-1;
                                        users[i].mygameID = games.size()-1;
                                        (games[games.size()-1]).gameid = games.size()-1;
                                        (games[games.size()-1]).printf_map();
                                        send(sockfd, (games[games.size()-1]).printmap, sizeof((games[games.size()-1]).printmap), 0);
                                        send(users[i].sockfd, (games[games.size()-1]).printmap, sizeof((games[games.size()-1]).printmap), 0);
                                        (games[games.size()-1]).start = clock()/1000;                                        
									} 
								}
							}
								
						}
                        else if(order[0].compare("move") == 0)
                        {
                            int i,j;
                            vector<struct User>::size_type k;
                            vector<int>::size_type l;
                            string::size_type index;
                            (games[uit->mygameID]).end = clock();
                            for(k = 0; k < users.size(); k++) //找到match 对象的users号
                            {
                                if((uit->mygameID == users[k].mygameID) && ((uit->name).compare(users[k].name) != 0))
                                break;
                            }

                            if(!(uit->state & ACTIVE))
                            {
                                memset(sendbuf,'\0', sizeof(sendbuf));
                                sprintf(sendbuf,"It is not your turn.\n");
                                send(sockfd,(void *)sendbuf, sizeof(sendbuf), 0);
                            }

                            else if(!(users[k].state & ONLINE))
                            {
                                memset(sendbuf,'\0', sizeof(sendbuf));
                                sprintf(sendbuf,"Your opponent just logged off.\n");
                                send(sockfd,(void *)sendbuf, sizeof(sendbuf), 0);
                            }
                            
                            else
                            {
                                printf("%c----%c",order[1][0],order[1][1]);
                                if(order[1][0] == 'A' || order[1][0] == 'a')
                                i = 1;
                                else if(order[1][0] == 'B' || order[1][0] == 'b')
                                i = 2;
                                else if(order[1][0] == 'C' || order[1][0] == 'c')
                                i = 3;
                                else
                                {
                                    memset(sendbuf, '\0', sizeof(sendbuf));
                                    sprintf(sendbuf,"Command not supported.\n");
                                    send(sockfd,(void *)sendbuf, sizeof(sendbuf), 0);
                                }

                               if(order[1][1] != '1' && order[1][1] != '2' && order[1][1] != '3')
                                {
                                    memset(sendbuf, '\0', sizeof(sendbuf));
                                    sprintf(sendbuf,"Command not supported.\n");
                                    send(sockfd,(void *)sendbuf, sizeof(sendbuf), 0);
                                }
                                else
                                {
                                    j = (int)order[1][1]-(int)('0');
                                }
                                    
                                
                                cout<<"-----------------"<<(games[uit->mygameID]).map[i][j] <<"-----------"<<i<<j<<endl;
                                if((games[uit->mygameID]).map[i][j] == '.')
                                {
                                    if(uit->name.compare(games[games.size()-1].player1) == 0) //是黑棋
                                    {
                                        (games[uit->mygameID]).map[i][j] = '#';
                                        (games[uit->mygameID]).time1 = (games[uit->mygameID]).time1 - ((games[uit->mygameID]).end - (games[uit->mygameID]).start);
                                    }
                                    else if(uit->name.compare(games[games.size()-1].player2) == 0) //是白棋
                                    {
                                        (games[uit->mygameID]).map[i][j] = 'O';
                                        (games[uit->mygameID]).time2 = (games[uit->mygameID]).time2 - ((games[uit->mygameID]).end - (games[uit->mygameID]).start);
                                    }	

                                    (games[uit->mygameID]).printf_map();
                                    (games[uit->mygameID]).moves++;
                                    for(l = 0; l < (games[uit->mygameID].observer).size(); l++)
                                        send((games[uit->mygameID].observer)[l], (games[uit->mygameID]).printmap, sizeof((games[uit->mygameID]).printmap), 0);

                                    send(sockfd, (games[games.size()-1]).printmap, sizeof((games[games.size()-1]).printmap), 0);
                                    send(users[k].sockfd, (games[games.size()-1]).printmap, sizeof((games[games.size()-1]).printmap), 0);

                                    if(int result = games[uit->mygameID].end_check() != 0) //someone won
                                    {
                                        memset(sendbuf, '\0', sizeof(sendbuf));
                                        if(result == 1)//O赢,W
                                        sprintf(sendbuf,"%s won the game\n",((games[uit->mygameID]).player2).c_str());
                                        else if(result == 2) //# 赢，B
                                        sprintf(sendbuf, "%s won the game.\n", ((games[uit->mygameID]).player1).c_str());
                                        else
                                        cerr << "something wrong about your result computation." << endl;

                                        uit->winN++;
                                        users[k].loseN++;
                                        if(uit->loseN != 0)
                                        uit->rating = double(uit->winN)/double(uit->loseN);
                                        users[k].rating = double(users[k].winN)/double(users[k].loseN);	
                                        send(sockfd, sendbuf, sizeof(sendbuf), 0);
                                        send(users[k].sockfd, sendbuf, sizeof(sendbuf), 0);
                                    }

                                    else
                                    {
                                        uit->state ^= ACTIVE;
                                        users[k].state |= ACTIVE;
                                        (games[uit->mygameID]).start = clock()/1000;	
                                    }                                   
                                }
                                else
                                {
                                    memset(sendbuf,'\0',sizeof(sendbuf));
                                    sprintf(sendbuf,"This location has been occupied.\n");
                                    send(sockfd, sendbuf, sizeof(sendbuf), 0);
                                }
                            }	
                        }    

                       //----EDITED!------
                        else if(order[0].compare("quit")==0||order[0].compare("exit")==0)
                        {
                            uit->state ^= ONLINE;
                            uit->sockfd = -1;
                            uit->mygameID = 0;
                            uit->observing_games.clear();
                            printf("User %s quit.\n",uit->name.c_str());
                            memset(sendbuf,'\0',MAXBUFLEN);
                            sprintf(sendbuf,"You are not in the room.\nThank you for using Online Tic-tac-toe Server.\nSee you next time.\n\n");
                            send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);                            
                            close(sockfd);
                            pthread_exit(NULL);                            
                        }
						                       
                        else if(order[0].compare("shout") == 0)
                        {
                            string msg = "";
                            vector<struct User>::iterator it;
                            vector<string>::iterator blockit;
                            int sendfd;
                            bool blocked = false;
                            if(order.size()> 1)
                                msg = order[1];
                            for(it=users.begin();it!=users.end();it++)
                            {
                                if(it->state & ONLINE && !(it->state & QUIET))
                                {
                                    for(blockit=it->block.begin();blockit!=it->block.end();blockit++)
                                    {
                                        if(uit->name.compare(*blockit)==0)
                                        {
                                            blocked = true;
                                            break;
                                        }
                                    }
                                    if(blocked == false)
                                    {
                                        sendfd = it->sockfd;
                                        memset(sendbuf,'\0',MAXBUFLEN);
                                        sprintf(sendbuf,"!shout! *%s*: %s\n",uit->name.c_str(),msg.c_str());
                                        send(sendfd,(void*)(sendbuf),sizeof(sendbuf),0);                                           
                                    }                                   
                                }
                                blocked = false;
                            }                                                         
                        }                                                 
                        else if(order[0].compare("tell") == 0)
                        {
                            if(order.size()==1) // no name
                            {
                                memset(sendbuf,'\0',MAXBUFLEN);
                                sprintf(sendbuf,"Please give out the user you want to tell.\n");
                                send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);                               
                            }
                            else
                            {
                                bool exist = false;
                                bool blocked = false;
                                vector<struct User>::iterator it;
                                vector<string>::iterator blockit;
                                int sendfd;
                                for(it=users.begin();it!=users.end();it++)
                                {
                                    if(order[1].compare(it->name)==0)
                                    {
                                        sendfd = it->sockfd;
                                        exist = true;
                                        break;
                                    }
                                }
                                if(exist==false) // the name doesn't exist
                                {
                                    memset(sendbuf,'\0',MAXBUFLEN);
                                    sprintf(sendbuf,"User %s doesn't exist.\n",order[1].c_str());
                                    send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);                                    
                                }
                                else
                                {
                                    if(it->state & ONLINE)
                                    {
                                        for(blockit=it->block.begin();blockit!=it->block.end();blockit++)
                                        {
                                            if(uit->name.compare(*blockit)==0)
                                            {
                                                blocked = true;
                                                break;
                                            }
                                        }
                                        if(blocked==false) // user is not blocked by the name
                                        {
                                            string msg = "";
                                            if(order.size()>2)
                                                msg = order[2];
                                            memset(sendbuf,'\0',MAXBUFLEN);
                                            sprintf(sendbuf,"%s: %s\n",uit->name.c_str(),msg.c_str());
                                            send(sendfd,(void*)(sendbuf),sizeof(sendbuf),0);                                            
                                        }
                                        else
                                        {
                                            memset(sendbuf,'\0',MAXBUFLEN);
                                            sprintf(sendbuf,"You cannot talk to %s. You are blocked.\n",order[1].c_str());
                                            send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);                                            
                                        }
                                    }
                                    else
                                    {
                                        memset(sendbuf,'\0',MAXBUFLEN);
                                        sprintf(sendbuf,"User %s is not online.\n",order[1].c_str());
                                        send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);                                       
                                    }
                                }
                            }                                                           
                        }
                        else if(order[0].compare("block") == 0)
                        {
                            bool exist = false;
                            bool isuser = false;
                            vector<struct User>::iterator it;
                            vector<string>::iterator blockit;
                            string blockname = uit->name;
                            if(order.size()>1)
                                blockname =  order[1];
                            //check whether the blockname is in the block list already
                            for(blockit=uit->block.begin();blockit!=uit->block.end();blockit++)
                            {
                                if(blockname.compare(*blockit)==0)
                                {
                                    exist = true;
                                    memset(sendbuf,'\0',MAXBUFLEN);
                                    sprintf(sendbuf,"user %s has already been blocked!\n",blockname.c_str());
                                    send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);                                     
                                    break;
                                }
                            }
                            if(exist == false)  // the blockname hasn't been blocked or blockname is not a user
                            {
                                //check if the blockname is a user
                                for(it=users.begin();it!=users.end();it++)
                                {
                                    if(it->name.compare(blockname)==0)
                                    {
                                        isuser = true;
                                        uit->block.push_back(blockname);
                                        memset(sendbuf,'\0',MAXBUFLEN);
                                        sprintf(sendbuf,"User %s blocked!\n",blockname.c_str());
                                        send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);                                          
                                        break;
                                    }
                                }
                                if(isuser== false)
                                {
                                    memset(sendbuf,'\0',MAXBUFLEN);
                                    sprintf(sendbuf,"user %s doesn't exist!\n",order[1].c_str());
                                    send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);                                    
                                }                                 
                            }
                        }
                        else if(order[0].compare("unblock") == 0)
                        {
                            bool exist = false;
                            vector<string>::iterator it;
                            string deletename = uit->name;
                            if(order.size()>1)
                                deletename = order[1];
                            for(it=uit->block.begin();it!=uit->block.end();it++)
                            {
                                if(deletename.compare(*it)==0)
                                {
                                    uit->block.erase(it);  // delete the unblocked user from the block list
                                    exist = true;
                                    memset(sendbuf,'\0',MAXBUFLEN);
                                    sprintf(sendbuf,"User %s unblocked!\n",deletename.c_str());
                                    send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);                                     
                                    break;
                                }
                            }
                            if(exist==false)
                            {
                                memset(sendbuf,'\0',MAXBUFLEN);
                                sprintf(sendbuf,"Fail to unblock user %s.\n",deletename.c_str());
                                send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);                                    
                            }                            
                        } 
                        else if(order[0].compare("quiet") == 0)
                        {
                            uit->state |= QUIET;
                            memset(sendbuf,'\0',MAXBUFLEN);
                            sprintf(sendbuf,"Enter quiet mode.\n");
                            send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);                            
                        }
                        else if(order[0].compare("nonquiet") == 0)
                        {
                            uit->state ^= QUIET;
                            memset(sendbuf,'\0',MAXBUFLEN);
                            sprintf(sendbuf,"Enter nonquiet mode.\n");
                            send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);                            
                        }
                        else if(order[0].compare("info") == 0)
                        {
                            if(order.size()==1)
                                uit->info = "<none>";                         
                            else
                                uit->info = order[1];    
                            memset(sendbuf,'\0',MAXBUFLEN);
                            sprintf(sendbuf,"Info changed!\n");
                            send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);
                            cout<<"Info:"<<uit->info<<endl;
                        }
                        else if(order[0].compare("passwd") == 0)
                        {
                            if(order.size()>1)
                            {
                                uit->pwd = order[1];
                                memset(sendbuf,'\0',MAXBUFLEN);
                                sprintf(sendbuf,"User %s's password changed successfully!\n",uit->name.c_str());
                                send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);                                 
                            }                                
                            else //doesn't give the new password
                            {
                                memset(sendbuf,'\0',MAXBUFLEN);
                                sprintf(sendbuf,"Please give the new password!\n");
                                send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);                                 
                            }
                        }
                        else if(order[0].compare("help")==0||order[0].compare("?")==0)
                        {
                        memset(sendbuf,'\0',MAXBUFLEN);
                        sprintf(sendbuf,cmd_list);
                        send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);                            
                        }
                        else if(order[0].compare("mail")==0)
                        {
                            if(order.size()==1) // no name
                            {
                                memset(sendbuf,'\0',MAXBUFLEN);
                                sprintf(sendbuf,"Please give out the user you want to mail.\n");
                                send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);                               
                            }
                            else
                            {
                                bool exist = false;
                                bool blocked = false;
                                vector<string>::iterator blockit;
                                for(recvit=users.begin();recvit!=users.end();recvit++)
                                {
                                    if(order[1].compare(recvit->name)==0)
                                    {
                                        exist = true;
                                        break;
                                    }
                                }
                                if(exist==false) // the name doesn't exist
                                {
                                    memset(sendbuf,'\0',MAXBUFLEN);
                                    sprintf(sendbuf,"User %s doesn't exist.\n",order[1].c_str());
                                    send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);                                    
                                }
                                else
                                {
                                    for(blockit=recvit->block.begin();blockit!=recvit->block.end();blockit++)
                                    {
                                        if(uit->name.compare(*blockit)==0)
                                        {
                                            blocked = true;
                                            break;
                                        }
                                    }
                                    if(blocked==false) // user is not blocked by the name
                                    {
                                        mail.clear();
                                        string title = "";
                                        if(order.size()>2)
                                            title = order[2];
                                        mail.sender = uit->name;
                                        mail.receiver = recvit->name;
                                        mail.header = title;
                                        
                                        memset(sendbuf,'\0',MAXBUFLEN);
                                        sprintf(sendbuf,"Please input mail body, finishing with '.' at the beginning of a line.\n\n");
                                        send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);   
                                        
                                        level = MAIL;
                                    }
                                    else
                                    {
                                        memset(sendbuf,'\0',MAXBUFLEN);
                                        sprintf(sendbuf,"You cannot send mail to %s. You are blocked.\n",order[1].c_str());
                                        send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);                                            
                                    }                                    
                                }
                            }
                            
                        }
                        else if(order[0].compare("listmail")==0)
                        {
                            memset(sendbuf,'\0',MAXBUFLEN);
                            sprintf(sendbuf,"Your messages:\n");
                            send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0); 
                            vector<struct Mymail>::iterator mit;
                            string msg;
                            for(mit=uit->mails.begin();mit!=uit->mails.end();mit++)
                            {
                                msg = ""; 
                                stringstream ss;
                                string seq;
                                ss<<(*mit).n;
                                ss>>seq;
                                msg += "  "+seq+"  "+(*mit).ifread+"  "+(*mit).sender;  
                                for(unsigned int i=0;i<10-(*mit).sender.size();i++)
                                    msg += " ";
                                msg += "\""+(*mit).header+"\"";
                                for(unsigned int i=0;i<20-(*mit).header.size()-2;i++)
                                    msg += " ";
                                msg += (*mit).ltime;
                                memset(sendbuf,'\0',MAXBUFLEN);
                                sprintf(sendbuf,"%s",msg.c_str());
                                send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);                               
                            }
                        }
                        else if(order[0].compare("readmail")==0)
                        {
                            unsigned int msgN;
                            bool valid = true;
                            if(order.size()==1) // doesn't give out the message number
                            {
                                msgN = 0;
                                if(uit->mails.size()==0)
                                    valid = false;
                            }
                            else
                            {
                                std::size_t found = order[1].find_first_not_of("0123456789");
                                if(found!=std::string::npos)
                                    valid = false;
                                else
                                {
                                    string str = order[1];
                                    if(order[1].size()>1 && str.substr(0,1).compare("0")==0)
                                        valid = false;
                                    else
                                    {
                                        msgN = atoi(order[1].c_str());
                                        if(uit->mails.empty())
                                        {
                                            valid = false;
                                        }                                        
                                        else if(msgN > ((uit->mails).size()-1))
                                        {                                            
                                            valid = false;
                                        }
                                    }
                                }
                            }
                            if(valid == false)
                            {
                                memset(sendbuf,'\0',MAXBUFLEN);
                                sprintf(sendbuf,"Message number invalid.\n");
                                send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);                                
                            }
                            else
                            {
                                uit->mails[msgN].ifread = " "; // set the message state to already read                               
                                memset(sendbuf,'\0',MAXBUFLEN);
                                sprintf(sendbuf,"From: %s\n",uit->mails[msgN].sender.c_str());
                                send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);  
                                
                                memset(sendbuf,'\0',MAXBUFLEN);
                                sprintf(sendbuf,"Title: %s\n",uit->mails[msgN].header.c_str());
                                send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);    

                                memset(sendbuf,'\0',MAXBUFLEN);
                                sprintf(sendbuf,"Time: %s\n",uit->mails[msgN].ltime.c_str());
                                send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);

                                memset(sendbuf,'\0',MAXBUFLEN);
                                sprintf(sendbuf,"\n%s",uit->mails[msgN].message.c_str());
                                send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);                                
                            }
                            
                        }
                        else if(order[0].compare("deletemail")==0)
                        {
                            unsigned int msgN;
                            bool valid = true;
                            bool empty = false;
                            if(order.size()==1) // doesn't give out the message number
                            {
                                msgN = 0;
                                
                                memset(sendbuf,'\0',MAXBUFLEN);
                                sprintf(sendbuf,"Please give out the mail NO.\n");
                                send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0); 
                            }
                            else // give out the message number
                            {
                                std::size_t found = order[1].find_first_not_of("0123456789");
                                if(found!=std::string::npos)
                                    valid = false;
                                else
                                {
                                    string str = order[1];
                                    if(order[1].size()>1 && str.substr(0,1).compare("0")==0)
                                        valid = false;
                                    else
                                    {
                                        msgN = atoi(order[1].c_str());
                                        if(uit->mails.empty())
                                        {
                                            valid = false;
                                            empty = true;
                                        }                                        
                                        else if(msgN > ((uit->mails).size()-1))
                                        {                                            
                                            valid = false;
                                        }
                                    }
                                }
                                if(valid == false)
                                {
                                    if(empty==true)
                                    {
                                        memset(sendbuf,'\0',MAXBUFLEN);
                                        sprintf(sendbuf,"No message.\n");
                                        send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);                                        
                                        
                                    }
                                    else
                                    {
                                        memset(sendbuf,'\0',MAXBUFLEN);
                                        sprintf(sendbuf,"Message number invalid.\n");
                                        send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);                                        
                                    }                               
                                }
                                else // message number is valid, now delete it
                                {
                                    vector<struct Mymail>::iterator mit;
                                    for(mit=uit->mails.begin();mit!=uit->mails.end();mit++)
                                    {
                                        unsigned int a = (*mit).n;
                                        if(msgN == a)
                                            uit->mails.erase(mit);
                                        break;                                           
                                    }
                                    int i = -1;
                                    for(mit=uit->mails.begin();mit!=uit->mails.end();mit++)
                                    {
                                        (*mit).n = ++i;
                                    }
                                    memset(sendbuf,'\0',MAXBUFLEN);
                                    sprintf(sendbuf,"Message deleted.\n");
                                    send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);                                 
                                }                                
                            }                           
                        }
                    }
                     //---XIE---
                    if(order.empty() || (order.size()>0 && order[0].compare("mail")!=0))
                    {
                        memset(sendbuf,'\0',MAXBUFLEN);
                        sprintf(sendbuf,"<%s: %d>",uit->name.c_str(),++countuser);
                        send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);                        
                    }
                        // memset(sendbuf,'\0',MAXBUFLEN);
                        // sprintf(sendbuf,"<%s: %d>",uit->name.c_str(),++countuser);
                        // send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);                     
                }
                order.clear();
            }
            else if(level==MAIL)
            {
                // memset(sendbuf,'\0',MAXBUFLEN);
                // sprintf(sendbuf,"Please input mail body, finishing with '.' at the beginning of a line.\n\n");
                // send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);   
                
                int sendfd = recvit->sockfd;
                if(cmd[0]!='.')
                {
                    cmd += "\n";
                    mailmsg += cmd;
                }
                else
                {
                    mail.message = mailmsg;
                    mail.n = recvit->mails.size();
                    
                    time_t sendt;
                    time(&sendt);
                    char* sendtime = ctime(&sendt);
                    string mailsendt = string(sendtime);
                    mail.ltime = mailsendt;
                    
                    recvit->mails.push_back(mail);
                    
                    memset(sendbuf,'\0',MAXBUFLEN);
                    sprintf(sendbuf,"Message sent.\n");
                    send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);

                    if(recvit->state & ONLINE)
                    {
                        memset(sendbuf,'\0',MAXBUFLEN);
                        sprintf(sendbuf,"A new message just received.\n");
                        send(sendfd,(void*)(sendbuf),sizeof(sendbuf),0);                        
                    }
                                        
                    memset(sendbuf,'\0',MAXBUFLEN);
                    sprintf(sendbuf,"<%s: %d>",uit->name.c_str(),++countuser);
                    send(sockfd,(void*)(sendbuf),sizeof(sendbuf),0);
                    mailmsg = "";
                    level = LOGIN;                             
                }               
            }
        }
	}

}

void read_from_socket()
{
	struct sockaddr_in client_addr;
	int newfd;
	socklen_t addrlen;
	pthread_t tid;
	char cmd[MAXBUFLEN];

	if(FD_ISSET(listener,&readfds))   // if receive connection request
	{
		addrlen = sizeof(client_addr);
		if((newfd = accept(listener,(struct sockaddr*)&client_addr,&addrlen))==-1)
		{
			printf("Fail to accept a new connection.\n");
		}
		else
		{
			// FD_SET(newfd,&master);
			// if(newfd>highestsock)
				// highestsock = newfd;

			if(pthread_create(&tid,NULL,&handler,&newfd)!=0)
				printf("Fail to create new a thread.\n");
		}
	}
	if(FD_ISSET(fileno(stdin),&readfds)) // if receive quit from stdin
	{
		fgets(cmd,sizeof(cmd),stdin);
		if(cmd[strlen(cmd)-1]=='\n')
			cmd[strlen(cmd)-1] = '\0';
		string comm = string(cmd);
		if(comm.compare("quit")==0)
		{
			cout<<"Server quit."<<endl;
			exit(0);
		}
	}
}

void server_run()
{
	struct sockaddr_in server_addr;
	//socklen_t addrlen;

	FD_ZERO(&master);
	FD_ZERO(&readfds);

	//create listener for the connection request
	if((listener = socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		perror("Fail to create listener socket.\n");
		exit(-1);
	}

	// set listener address to be reused immediately after release
	int a=1;
	if(setsockopt(listener,SOL_SOCKET,SO_REUSEADDR,&a,sizeof(int))==-1)
	{
		perror("Fail to set socket option.\n");
		exit(-1);
	}

	//bind to the port
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(SERVERPORT);
	memset(&(server_addr.sin_zero),'\0',8);
	if(bind(listener,(struct sockaddr *)&server_addr,sizeof(server_addr))==-1)
	{
		perror("Fail to bind to listener.\n");
		exit(-1);
	}

	if(listen(listener,MAXONLINE)<0)
	{
		perror("Fail to listen.\n");
		exit(-1);
	}

	FD_SET(listener,&master);
	if(listener>highestsock)
	{
		highestsock = listener;
	}

	FD_SET(fileno(stdin),&master);
	if(fileno(stdin)>highestsock)
	{
		highestsock = fileno(stdin);
	}

	while(1)  // keep listen the connection request
	{
		readfds = master;
		if(select(highestsock+1,&readfds,NULL,NULL,NULL) == -1)
		{
			perror("Fail to select.\n");
			exit(-1);
		}
		read_from_socket();
	}
	close(listener);
}

int main(int argc, char **argv)
{
	if(argc<2)
	{
		perror("Insufficient parameter.\n");
		return 1;
	}
	else
	{
		SERVERPORT = atoi(argv[1]);  // get the server port
	}

	server_run();
	return 0;
}
