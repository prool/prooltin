DO_COMMAND(do_prool);
DO_COMMAND(do_proolwatchdogtimer);
DO_COMMAND(do_totalon);
DO_COMMAND(do_totaloff);
DO_COMMAND(do_addword);
DO_COMMAND(do_delword);
DO_COMMAND(do_listdic);
DO_COMMAND(do_writedic);
DO_COMMAND(do_mouseon);
DO_COMMAND(do_mouseoff);
DO_COMMAND(do_proolcoder_switch);

char *prooltran(char *si);
void prool_log(char *message);
void prool_ident(void);
char *ptime(void);
void uptime_(void);
void prooltranslate_init(int greeting);
int prool_loop(void);

void enable_mouse(void);
void disable_mouse(void);

void koi_to_utf8(char *str_i, char *str_o);
void utf8_to_koi(char *str_i, char *str_o);

extern int total_log;
extern int tron;
extern int coder;
extern int total_log;
extern long int start_time;
extern long int startwatchtime;
extern int prool_loop_counter;
extern long int watchdog;
