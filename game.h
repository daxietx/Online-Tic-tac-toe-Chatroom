#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <time.h>
#include <iostream>
#include <cctype>


using namespace std;
struct Game
{
	int gameid;
	int moves;
	string player1;  // black                  
	string player2;
	char printmap[500];
	vector <int> observer;          //  sockfd;
	time_t start;
	time_t end;
	time_t time1,time2;
	char map[4][4];
    void printf_map();
    int end_check();	
	Game(string p1, string p2, time_t t): gameid(0), moves(0), start(0), end(0)
	{
		int i,j;
		player1 = p1;
		player2 = p2;
		time1 = t;
		time2 = t;
		observer.clear();
		//map 初始化
		for(i = 0;i < 4; i++)
		{
			
			if(i == 0)
			{
				map[i][0] = ' ';
				map[i][1] = '1';
				map[i][2] = '2';
				map[i][3] = '3';
			}
			else
			{
				for (j = 0; j < 4; j++)
			   {
					if(j == 0)
					{
						if(i == 1)
							map[i][j] = 'A';
						else if(i == 2)
							map[i][j] = 'B';
						else
							map[i][j] = 'C';
					}
					else
						map[i][j] = '.';
			   }
			}
		}
	}
};

void Game::printf_map()
{
	memset(printmap,'\0',sizeof(printmap));
	sprintf(printmap,"\nBlack:%-15s          White:%-15s\n Time:%-7ld seconds           Time:%-7ld seconds\n\n\
%-3c%-3c%-3c%-3c\n%-3c%-3c%-3c%-3c\n%-3c%-3c%-3c%-3c\n%-3c%-3c%-3c%-3c\n",player1.c_str(), player2.c_str(), time1, time2, map[0][0],\
map[0][1],map[0][2],map[0][3],map[1][0],map[1][1],map[1][2],map[1][3],map[2][0],map[2][1],map[2][2],map[2][3],map[3][0],map[3][1],map[3][2],map[3][3]);
}

int Game::end_check()
{
	int i,j;
	for(i = 1; i < 4; i++)
	{
         if(map[i][1] == 'O' && map[i][2] == 'O' && map[i][3] == 'O')
            return 1;
         else if(map[i][1] == '#' && map[i][2] == '#' && map[i][3] == '#')
            return 2;			 
	}
	
	for(j = 1; j < 4; j++)
	{
		 if(map[1][j] == 'O' && map[2][j] == 'O' && map[3][j] == 'O')
			 return 1;
		 else if(map[1][j] == '#' && map[2][j] == '#' && map[3][j] == '#')
			 return 2;
	}
	
	if((map[1][1] == 'O' && map[2][2] == 'O' && map[3][3] == 'O') || (map[1][3] == 'O' && map[2][2] == 'O' && map[3][1] == 'O'))
		return 1;
	else if((map[1][1] == '#' && map[2][2] == '#' && map[3][3] == '#') || (map[1][3] == '#' && map[2][2] == '#' && map[3][1] == '#'))
		return 2;
    
    for(i = 0; i < 4; i++)
    {
        for(j = 0; j < 4; j++)
        {
            if(map[i][j]=='O' && map[i][j] == '#')
                continue;
            else
                break;
        }
        if( j != 4)
            break;
    }
    
    if(i == 4)
        return 3;
    

    return 0;	
}