// This file is part of the loginx project
//
// Copyright (c) 2013 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "ui.h"
#include "pw.h"
#include <pwd.h>
#include <signal.h>
#include <time.h>

//{{{ Signal handling --------------------------------------------------

#define S(s) (1u<<(s))
enum {
    sigset_Quit	= S(SIGINT)|S(SIGQUIT)|S(SIGTERM)|S(SIGPWR)|
		S(SIGILL)|S(SIGABRT)|S(SIGBUS)|S(SIGFPE)|
		S(SIGSYS)|S(SIGSEGV)|S(SIGALRM)|S(SIGXCPU),
    qc_ShellSignalQuitOffset = 128
};

static void OnSignal (int s)
{
    static bool s_DoubleSignal = false;
    if (!s_DoubleSignal) {
	s_DoubleSignal = true;
	psignal (s, "[S] Error");
	exit (s+qc_ShellSignalQuitOffset);
    }
    _exit (s+qc_ShellSignalQuitOffset);
}

static void InstallCleanupHandlers (void)
{
    for (unsigned i = 0; i < NSIG; ++i)
	if (S(i) & sigset_Quit)
	    signal (i, OnSignal);
}
#undef S

//}}}-------------------------------------------------------------------
//{{{ Utility functions

void* xmalloc (size_t n)
{
    void* p = calloc (1, n);
    if (!p) {
	puts ("Error: out of memory");
	exit (EXIT_FAILURE);
    }
    return (p);
}

void xfree (void* p)
{
    if (p)
	free (p);
}

#if 0
//}}}-------------------------------------------------------------------
//{{{ Shell

static void LaunchShell (const char* username)
{
    struct passwd* pwn = getpwnam (username);
    fprintf (stderr, "The user [%s,%hu,%hu] has been authenticated and `logged in'\n", username, pwn->pw_uid, pwn->pw_gid);

    setuid (pwn->pw_uid);
    setgid (pwn->pw_gid);
    setsid();
    setpgrp();

    /* this is always a really bad thing for security! */
    system("/bin/sh");
}

//}}}-------------------------------------------------------------------
#endif

int main (void)
{
    InstallCleanupHandlers();
    acclist_t al = ReadAccounts();
    char password [MAX_PW_LEN];
    unsigned ali = 1;
    ReadLastlog();
    LoginBox (al, &ali, password);
#if 0
    PamOpen();
    const char* username = PamLogin();
    LaunchShell (username);
    PamLogout();
    PamClose();
#endif
    time_t ltime = al[ali]->ltime;
    printf ("Logging in user '%s', password '%s', lastlog %s", al[ali]->name, password, ctime(&ltime));
    return (0);
}
