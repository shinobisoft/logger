#ifndef _LOGGER_H_
#define _LOGGER_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


typedef enum LOGLEVEL {
    DEBUG       = 0,    // Same as log_d OR log_format( LOGLEVEL DEBUG, char* fmt, ... )
    ERROR       = 1,    // Same as log_e OR log_format( LOGLEVEL ERROR, char* fmt, ... )
    INFORMATION = 2,    // Same as log_i OR log_format( LOGLEVEL INFORMATION, char* fmt, ... )
    WARNING     = 3,    // Same as log_w OR log_format( LOGLEVEL WARNING, char* fmt, ... )
    PRINT       = 4,    // Same as log_p OR log_format( LOGLEVEL PRINT, char* fmt, ... )
    NONE        = 99,   // Same as log_ OR log_format( LOGLEVEL NONE, char* fmt, ... )
} LOGLEVEL;


void    log_close   ( void );
int     log_init    ( const char* filename, const char** prefixes, const char* initial_text );
int     log_        ( const char* text );
int     log_d       ( const char* text ); // DEBUG
int     log_e       ( const char* text ); // ERROR
int     log_i       ( const char* text ); // INFO
int     log_w       ( const char* text ); // WARNING
int     log_format  ( LOGLEVEL level, const char* fmt, ... );
int     log_print   ( const char* text );

int     log_get_buffer_size( void );
int     log_set_buffer_size( int buf_size );

const char**  log_get_prefixes( void );
void    log_set_prefixes( const char** prefixes );

int     log_get_use_24hr_time( void );
void    log_set_use_24hr_time( int use24 );

char*   log_time   ( void );
char*   log_date   ( void );

/*
    Convenience macros for easier function access.
*/
#define LOGINIT log_init
#define LOGEXIT log_close
#define LOG     log_
#define LOGD    log_d
#define LOGE    log_e
#define LOGI    log_i
#define LOGW    log_w
#define LOGF    log_format
#define LOGP    log_print

#define LOGTIME log_time
#define LOGDATE log_date


#ifdef __cplusplus
}
#endif // __cplusplus


#endif // _LOGGER_H_
