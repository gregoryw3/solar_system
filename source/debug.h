#undef Assert
#define Assert HardAssert
#define HardAssert(b) if(!(b)) { _HardAssertFailure(__LINE__, __FILE__, #b); }
#define SoftAssert(b) if(!(b)) { _SoftAssertFailure(__LINE__, __FILE__, #b); }

static void
_HardAssertFailure(int line, char *file, char *condition)
{
    platform->OutputError("Fatal Assertion Failure", "Condition %s failed (%s:%i). Trying to crash...",
                          condition, file, line);
    *(int *)0 = 0;
}

static void
_SoftAssertFailure(int line, char *file, char *condition)
{
    platform->OutputError("Non-Fatal Assertion Failure", "Condition %s failed (%s:%i). Continuing...",
                          condition, file, line);
}

#define Log(...) fprintf(stdout, __VA_ARGS__); fprintf(stdout, "\n");