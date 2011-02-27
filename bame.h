#ifndef BAME_H
#define BAME_H

void init_curses();
void* input_thread(void *arg);
void update_score();
void reset_game();
void add_row();
void check_row();
void delete_row();
void print_current();
void print_rows();
static void sig_handler(int s);

#endif // BAME_H
