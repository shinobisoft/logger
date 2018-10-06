
#include <sys/stat.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "logger.h"

// UTILITY MACROS
#ifndef MALLOC
    #define MALLOC (char*)malloc
#else
    #undef MALLOC
    #define MALLOC (char*)malloc
#endif // MALLOC
#ifndef REALLOC
    #define REALLOC (char*)realloc
#else
    #undef REALLOC
    #define REALLOC (char*)realloc
#endif // REALLOC

////////////////////////////////////////////////////////////////////////////////
// Private variables used within this module.
////////////////////////////////////////////////////////////////////////////////
static int      __LOG_BUFFER_SIZE__     = 1024;
static char*    __LOG_FILENAME__        = NULL;
static char*    __LOG_BUFFER__          = NULL;
static int      __LOG_INITIALIZED__     = 0;
static int      __LOG_BUFFER_ALLOCED__  = 0;

/*#define __DBUG_PREFIX 0
#define __EROR_PREFIX 1
#define __INFO_PREFIX 2
#define __WARN_PREFIX 3 */
static char** __PREFIXES = NULL;
static char* __DEFAULT_PREFIXES[] = {
    "[DEBUG]",
    "[ERROR]",
    "[INFO]",
    "[WARNING]"
};

// Default time format for log entries
// Set to '0' for 12 hr time with am/pm
static int TIME_24HR = 1;

////////////////////////////////////////////////////////////////////////////////
// The formats below are private formats. By this I mean they are NOT
// proper for passing to functions that format date and time data to
// human readable strings. Time and date information is formatted
// internally, with these formats, in this module.
////////////////////////////////////////////////////////////////////////////////
static char* FMT_TIME = "%02i:%02i:%02i";
static char* FMT_DATE = "%02i/%02i/%04i";

////////////////////////////////////////////////////////////////////////////////
// Private function prototypes
////////////////////////////////////////////////////////////////////////////////
int     APPEND  ( const char* s );
int     WRITE   ( const char* s );

// Set all bytes in __LOG_BUFFER__ to NULL bytes ( \0 ).
void empty() {
    if( ! __LOG_INITIALIZED__ || !__LOG_BUFFER_ALLOCED__ )
        return;

    for( int i = 0; i < __LOG_BUFFER_SIZE__; i++ )
        __LOG_BUFFER__[ i ] = '\0';
}


////////////////////////////////////////////////////////////////////////////////
// Closes the logger. This function frees the buffer maintained by this module.
////////////////////////////////////////////////////////////////////////////////
void log_close() {
    if( __LOG_BUFFER_ALLOCED__ && __LOG_BUFFER__ != NULL )
        free( __LOG_BUFFER__ );
    __LOG_BUFFER_ALLOCED__ = 0;
    __LOG_BUFFER__ = NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Creates a logger and initializes an internal buffer used by this module.
// Remember to call log_close() to release memory claimed.
////////////////////////////////////////////////////////////////////////////////
int log_init( const char* filename, const char** prefixes, const char* initial_text ) {

    if( ( filename == NULL || *filename == '\0' ) && __LOG_FILENAME__ == NULL )
        return 0;

    if( !__LOG_INITIALIZED__ && !__LOG_BUFFER_ALLOCED__ ) {
        __LOG_BUFFER__ = MALLOC( __LOG_BUFFER_SIZE__ );

        memset( __LOG_BUFFER__, 0, __LOG_BUFFER_SIZE__ );
        if( __LOG_BUFFER__ != NULL )
            __LOG_BUFFER_ALLOCED__ = 1;
    }

    if( !__LOG_INITIALIZED__  && filename != NULL ) {
        __LOG_FILENAME__ = (char*)filename;
    }

    if( initial_text == NULL ) {
        sprintf( __LOG_BUFFER__, "# Log started: %s %s\n", LOGDATE(), LOGTIME() );
    } else {
        strcpy( __LOG_BUFFER__, initial_text );
    }

     __LOG_INITIALIZED__ = WRITE( __LOG_BUFFER__ );

    if( __LOG_INITIALIZED__ == strlen( __LOG_BUFFER__ ) ) {
        __LOG_INITIALIZED__ = 1;

        if( prefixes != NULL ) {
            __PREFIXES = (char**)prefixes;
        } else {
            __PREFIXES = __DEFAULT_PREFIXES;
        }
    }
    return __LOG_INITIALIZED__;
}

////////////////////////////////////////////////////////////////////////////////
// Writes "[ current_time ] 'text'" to the log file
////////////////////////////////////////////////////////////////////////////////
int log_( const char* text ) {

    if( !__LOG_INITIALIZED__ || text == NULL || *text == '\0' )
        return -1;

    char* fmt = "[%s] %s\n";
    sprintf( __LOG_BUFFER__, fmt, LOGTIME(), text );
    return APPEND( __LOG_BUFFER__ );
}

////////////////////////////////////////////////////////////////////////////////
// Writes "[ current_time ][DEBUG_PREFIX] 'text'" to the log file
////////////////////////////////////////////////////////////////////////////////
int log_d( const char* text ) {
    char* fmt = "[%s]%s %s\n";

    if( !__LOG_INITIALIZED__ || text == NULL || *text == '\0' )
        return -1;

    sprintf( __LOG_BUFFER__, fmt, LOGTIME(), __PREFIXES[ DEBUG ], text );
    return APPEND( __LOG_BUFFER__ );
}

////////////////////////////////////////////////////////////////////////////////
// Writes "[ current_time ][ERROR_PREFIX] 'text'" to the log file
////////////////////////////////////////////////////////////////////////////////
int log_e( const char* text ) {
    char* fmt = "[%s]%s %s\n";

    if( !__LOG_INITIALIZED__ || text == NULL || *text == '\0' )
        return -1;

    sprintf( __LOG_BUFFER__, fmt, LOGTIME(), __PREFIXES[ ERROR ], text );
    return APPEND( __LOG_BUFFER__ );
}

////////////////////////////////////////////////////////////////////////////////
// Writes "[ current_time ][INFO_PREFIX] 'text'" to the log file
////////////////////////////////////////////////////////////////////////////////
int log_i( const char* text ) {
    char* fmt = "[%s]%s %s\n";

    if( !__LOG_INITIALIZED__ || text == NULL || *text == '\0' )
        return -1;

    sprintf( __LOG_BUFFER__, fmt, LOGTIME(), __PREFIXES[ INFORMATION ], text );
    return APPEND( __LOG_BUFFER__ );
}

////////////////////////////////////////////////////////////////////////////////
// Writes "[ current_time ][WARN_PREFIX] 'text'" to the log file
////////////////////////////////////////////////////////////////////////////////
int log_w( const char* text ) {
    char* fmt = "[%s]%s %s\n";

    if( !__LOG_INITIALIZED__ || text == NULL || *text == '\0' )
        return -1;

    sprintf( __LOG_BUFFER__, fmt, LOGTIME(), __PREFIXES[ WARNING ], text );
    return APPEND( __LOG_BUFFER__ );
}

////////////////////////////////////////////////////////////////////////////////
// Writes "[ current_time ][__PREFIXES[ level ]] <formatted 'fmt' text>"
// to the log. This function ensures that a new-line character is output to
// the log file. If level == DEBUG, [__PREFIXES[ level ]] is omitted from
// the output.
////////////////////////////////////////////////////////////////////////////////
int log_format( LOGLEVEL level, const char* fmt, ... ) {

    if( level >= DEBUG && level <= WARNING ) {
        // Format output according to 'level'
        char* FMT = "[%s]%s ";
        sprintf( __LOG_BUFFER__, FMT, LOGTIME(), __PREFIXES[ level ] );
        LOGP( __LOG_BUFFER__ );

        va_list args;
        va_start( args, fmt );
        vsprintf( __LOG_BUFFER__, fmt, args );
        va_end( args );

        if( __LOG_BUFFER__[ strlen( __LOG_BUFFER__ )-1 ] != '\n' )
            __LOG_BUFFER__[ strlen( __LOG_BUFFER__ ) ] = '\n';

        return APPEND( __LOG_BUFFER__ );

    } else if( level == NONE ) {

        // No formatting other than prepending the current time
        char* FMT = "[%s] ";
        sprintf( __LOG_BUFFER__, FMT, LOGTIME() );
        LOGP( __LOG_BUFFER__ );

        va_list args;
        va_start( args, fmt );
        vsprintf( __LOG_BUFFER__, fmt, args );
        va_end( args );

        if( __LOG_BUFFER__[ strlen( __LOG_BUFFER__ )-1 ] != '\n' )
            __LOG_BUFFER__[ strlen( __LOG_BUFFER__ ) ] = '\n';

        return APPEND( __LOG_BUFFER__ );

    } else if( level == PRINT ) {

        // Outputs formatted string directly to the log file without
        // a time stamp. Ths also does NOT ensure
        // that a new line is appended to the output. This allows
        // us to "build" log entries one word or several words at
        // a time, with formatting.
        va_list args;
        va_start( args, fmt );
        vsprintf( __LOG_BUFFER__, fmt, args );
        va_end( args );

        return APPEND( __LOG_BUFFER__ );
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Writes 'text' to the log file. An EOL character is NOT written to the log
// file with this function unless it's included at the end of 'text'.
////////////////////////////////////////////////////////////////////////////////
int log_print( const char* text ) {

    if( !__LOG_INITIALIZED__ || text == NULL || *text == '\0' )
        return -1;
    return APPEND( text );
}

////////////////////////////////////////////////////////////////////////////////
// Get the byte size of the internal buffer used by this module.
////////////////////////////////////////////////////////////////////////////////
int log_get_buffer_size() { return __LOG_BUFFER_SIZE__; }

////////////////////////////////////////////////////////////////////////////////
// Set the byte size of the internal buffer used by this module. This will
// also cause the module to realloc the internal buffer but only if the buffer
// has already been allocated... IE; calling log_init()
////////////////////////////////////////////////////////////////////////////////
int log_set_buffer_size( int buf_size ) {
    int old = __LOG_BUFFER_SIZE__;
    __LOG_BUFFER_SIZE__ = ( buf_size > 1024 ) ? buf_size : 1024;
    if( __LOG_BUFFER_ALLOCED__ ) {
        __LOG_BUFFER__ = REALLOC( __LOG_BUFFER__, __LOG_BUFFER_SIZE__ );
    }
    return old;
}

////////////////////////////////////////////////////////////////////////////////
// Gets the prefixes used by log_d(), log_e(), log_i(), and log_w().
// See 'static char* __DEFAULT_PREFIXES[]' above...
////////////////////////////////////////////////////////////////////////////////
const char** log_get_prefixes() {
    return (const char**)__PREFIXES;
}

////////////////////////////////////////////////////////////////////////////////
// Sets the prefixes used by log_d(), log_e(), log_i(), and log_w().
////////////////////////////////////////////////////////////////////////////////
void   log_set_prefixes( const char** prefixes ) {
    if( prefixes != NULL ) {
        __PREFIXES = (char**)prefixes;
    } else {
        __PREFIXES = __DEFAULT_PREFIXES;
    }
}

////////////////////////////////////////////////////////////////////////////////
// Toggle the logs time format between 12 and 24 hour formats.
////////////////////////////////////////////////////////////////////////////////
void log_set_use_24hr_time( int use24hr ) {
    if( !use24hr )
        TIME_24HR = 0;
    else
        TIME_24HR = 1;
}

////////////////////////////////////////////////////////////////////////////////
// Get the current time as a string
////////////////////////////////////////////////////////////////////////////////
char* log_time() {
    time_t now;
    time( &now );

    struct tm* tm = localtime( &now );
    char* res = (char*)malloc( 32 );
    if( TIME_24HR )
        sprintf( res, FMT_TIME, tm->tm_hour, tm->tm_min, tm->tm_sec );

    else {
        int hrs = tm->tm_hour;
        char *ampm = " AM";
        if( hrs > 12 ) {
            hrs -= 12;
            ampm = " PM";
        }
        sprintf( res, FMT_TIME, hrs, tm->tm_min, tm->tm_sec );
        strcat( res, ampm );
    }
    return res;
}

////////////////////////////////////////////////////////////////////////////////
// Get the current date as a string
////////////////////////////////////////////////////////////////////////////////
char* log_date() {
    time_t now;
    time( &now );

    struct tm* tm = localtime( &now );
    char* res = (char*)malloc( 32 );
    sprintf( res, FMT_DATE, tm->tm_mon, tm->tm_mday, tm->tm_year+1900 );
    return res;
}

////////////////////////////////////////////////////////////////////////////////
// Append data to the log file
////////////////////////////////////////////////////////////////////////////////
int APPEND( const char* s ) {
    FILE* fp = fopen( __LOG_FILENAME__, "a" );
    if( fp != NULL ) {
        int w = fwrite( s, 1, strlen( s ), fp );
        fclose( fp );
        empty();
        return w;
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Create and write initial data to the log file. This will overwrite any
// existing log files of the same filename...
////////////////////////////////////////////////////////////////////////////////
int WRITE( const char* s ) {
    FILE* fp = fopen( __LOG_FILENAME__, "w" );
    if( fp != NULL ) {
        int w = fwrite( s, 1, strlen( s ), fp );
        fclose( fp );
        empty();
        return w;
    }
    return 0;
}


