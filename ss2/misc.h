
#ifndef FRELEASE
#define FORMATSYSTEMERROR(s) format_system_error(s)
#else
#define FORMATSYSTEMERROR(s) "release"
#endif

char* format_system_error(DWORD);
void set_globals(void);