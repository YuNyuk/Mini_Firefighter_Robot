#ifndef __AI_H__
#define __AI_H__

int onAI(int *);
int offAI(int *);
int workAI(int *);
void rmText();

bool findPos(int *angle_x, int *angle_y);

void handler(int signo);
int takePic(const char *filename);

#endif 		// __AI_H_
