#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <time.h>
#include <iostream>
#include <iomanip>
#include <sys/socket.h>
#include <sys/types.h>
#include <cctype>
//#include "game1.h"

using namespace std;

#define ONLINE 0x00000001                              	//online
#define BEBLOCKED 0x00000002                 		  	//blocked
#define QUIET 0x00000004                           		//quite mode
#define PLAYER 0x00000008                        		//player
#define OBSERVER 0x00000010                     		//observer
#define ACTIVE 0x00000020                          		//playing game
#define WINNER 0x00000040                            	//winner

struct User
{
	string name;
	string pwd;
	string info;
	int sockfd;   //present socket belonging to the user
	int state;
	int winN;
	int loseN;
    double rating;
    int mygameID;
    string match_errno;
	vector<int> observing_games;      //game_id;
	vector<string> block;             //block user names;
    vector<struct Match_info> matches;
	vector<string> invitations;       //invitation sent from other users, <match name x 600 300>
	vector<struct Mymail>  mails;	
	User():name(""), pwd(""), info("<none>"),sockfd(-1), state(0), winN(0),loseN(0),\
	mygameID(0),match_errno(""){} 
	
};

struct Mymail
{
	string ltime;
	int n;   /*number of unread*/
	string ifread;    /*default unread*/
	string sender;
	string receiver;
	string header;
	string message;
    
	Mymail():ltime("time"), n(0), ifread("N"), sender(""), receiver(""), header(""), message(""){}
    
    void clear()
    {
        ltime = "time";
        n = 0;
        ifread = "N";
        sender = "";
        receiver = "";
        header = "";
        message = "";
    }

};

struct Match_info //create when recieve the msg first time
{
    string partner;
    string invitation_recv; 
    string invitation_send;

    Match_info(string s1, string m0, string m1):partner(""),invitation_recv(""),invitation_send("")
    {
        partner = s1; // the person that need to match
        invitation_recv = m0;
        invitation_send = m1;
    }
};


