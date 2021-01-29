#include<stdio.h>
#include<curses.h>
#define KEY_UP 0403
#define KEY_DOWN 0402
#define KEY_LEFT 0404
#define KEY_RIGHT 0405
int main(){
printf("uw\n");
//if you don't want to display the character you press on the screen, use c = getch();.
//else use c = getchar();
char c=getchar();
///then simply compare c with those macros.
if(c == KEY_UP)
printf("uparrow\n");

if(c == KEY_DOWN)
printf("down arrow\n");
return 0;
}
