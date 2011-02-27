#include "bame.h"

#include <curses.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAX_ROWS 15
#define LINES_FOR_LEVEL_UP 10
#define INIT_DELAY_MS 1500
#define LEVEL_DELAY_DECREASE_MS 100

unsigned int score;
unsigned int rows[MAX_ROWS];
unsigned int row_index;
unsigned short cur_line;
unsigned int delay;
unsigned int lines_in_level;
unsigned int level;
char go_on;
pthread_t thread;
pthread_mutex_t mutex;

int
main(int argc, char **arvc) {

	// init ncurses screen
	init_curses();

	// install signal handler
	signal(SIGINT, sig_handler);
	signal(SIGTERM, sig_handler);

	// init random number generator
	srand((unsigned) time(NULL));	

	// init mutex for save access to global variables
	if(pthread_mutex_init(&mutex, NULL)) {
		printf("can't create mutex\n");
		pthread_cancel(thread);
		endwin();
		return 1;
	}

	int i;
	for(i = 7; i >= 0; i--) {
		mvprintw(MAX_ROWS + 5, (7 - i) * 7 + 3 , "% 3u", 1<<i);
	}
	mvprintw(3, 1, "-----------------------------------------");

	char c;
	while(1) {

		reset_game();

		// start input thread (listening for keystrokes)
		if(pthread_create(&thread, NULL, input_thread, NULL)) {
			printf("can't create thread");
			endwin();
			return 1;
		}

		while(go_on) {
			usleep(delay);
			add_row();
		}

		pthread_cancel(thread);

		mvprintw(2, 1, "press space for new game, or 'q' to exit");
		while(c = getch()) {
			if(c == ' ') {
				break;
			}
			if(c == 'q') {
				goto out;
			}
		}
		
	}
out:

	endwin();
	return 0;
}

void
init_curses() {
	initscr();
	curs_set(0);
	noecho();	// don't echo keystrokes
	cbreak();	// raw keyboard input
	start_color();
	init_pair(1, COLOR_WHITE, COLOR_BLACK);
	init_pair(2, COLOR_WHITE, COLOR_RED);
	attron(COLOR_PAIR(1));
	mvprintw(0,0,"..:: B-A-M-E ::..");
}

void
reset_game() {
	int i;
	score = 0;
	update_score();
	for(i = 0; i < MAX_ROWS; i++) {
		rows[i] = 0;
	}
	row_index = 0;
	cur_line = 0;
	print_current();
	print_rows();
	go_on = 1;
	mvprintw(2,0,"                                           ");
	delay = INIT_DELAY_MS * 1000;
	lines_in_level = 0;
	level = 0;
}

static void
sig_handler(int s) {
	endwin();
	printf("blame exiting...cya\n");
	exit(EXIT_SUCCESS);
}

void*
input_thread(void *args) {
	char c;

	while(1) {
		c = getch();
		switch(c) {
			case 'a':
				cur_line ^= 1 << 7;
				break;
			case 's':
				cur_line ^= 1 << 6;
				break;
			case 'd':
				cur_line ^= 1 << 5;
				break;
			case 'f':
				cur_line ^= 1 << 4;
				break;
			case 'j':
				cur_line ^= 1 << 3;
				break;
			case 'k':
				cur_line ^= 1 << 2;
				break;
			case 'l':
				cur_line ^= 1 << 1;
				break;
			case ';':
				cur_line ^= 1 << 0;
				break;
			case 'q':
				go_on = 0;
				break;
		}
		check_row();
	}
}

void
update_score() {
	mvprintw(1, 0, "score: % 10u   level:  % 2u", score, level);
}

void
add_row() {

	int i = MAX_ROWS;
	int index = row_index;
	while(i) {
		
		if(!rows[index]) {
			rows[index] = (rand() % 255) + 1;
			break;
		}

		i--;
		index = (index + 1) % MAX_ROWS;
	}

	if(!i) {
		go_on = i;
		return;
	}

	lines_in_level++;
	if(lines_in_level == LINES_FOR_LEVEL_UP) {
		delay -= LEVEL_DELAY_DECREASE_MS * 1000;
		lines_in_level = 0;
		level++;
	}

	print_rows();
}

void
check_row() {

	if(rows[row_index] && (rows[row_index] == cur_line)) {
		score += 10;
		update_score();
		cur_line = 0;
		delete_row();
	} 
	print_current();
}

void
print_current() {

	int i;
	int line = MAX_ROWS + 4;
	move(line, 1);

	printw(" | ");

	for(i = 7; i >= 0; i--) {
		if(cur_line & (1 << i)) {
			attron(COLOR_PAIR(2));
			printw("__");
			attron(COLOR_PAIR(1));
			printw("  |  ");
		} else {
			printw("__  |  ");
		}
	}

	refresh();
}

void
delete_row() {
	rows[row_index] = 0;
	row_index = (row_index + 1) % MAX_ROWS;
	print_rows();
}

void
print_rows() {

	int i;
	int index = row_index;
	int line = MAX_ROWS + 3;
	int col = 5;

	move(line, col);

	for(i = 0; i < MAX_ROWS; i++) {
		if(!rows[index]) {
			printw("      ");
		} else {
			printw("% 6u", rows[index]);
		}

		index = (index + 1) % MAX_ROWS;
		move(--line, col);
	}

	refresh();
}
