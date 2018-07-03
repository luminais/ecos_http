/*dyndns.h for dyndns*/
#ifndef __DYNDNS_H__
#define __DYNDNS_H__

#define DYNHOST "members.dyndns.org"
#define DYNPORT 80

#define YESORNO(x) ((x)? "YES":"NO")
#define ONOROFF(x) ((x)? ((x-1)? "OFF":"ON"):"NOCHG")

static const char *dd_system[] = { 
	"dyndns",
	"statdns",
	"custom",
	NULL
};

static struct dyndns_return_msgs {
        const char *code;
        const char *message;
        const int  error;
} dyndns_msgs[] = {
        { "badauth",    "Bad authorization (username or password).",            1 },
        { "badsys",     "The system parameter given was not valid.",            1 },
        { "badagent",   "The useragent your client sent has been blocked "
          "at the access level.",                                               1
        },
        { "good",       "Update good and successful, IP updated.",              0 },
        { "nochg",      "No changes, update considered abusive.",               0 },
        { "notfqdn",    "A Fully-Qualified Domain Name was not provided.",      1 },
        { "nohost",     "The hostname specified does not exist.",               1 },
        { "!donator",   "The offline setting was set, when the user is "
          "not a donator.",                                                     1
        },
        { "!yours",     "The hostname specified exists, but not under "
          "the username currently being used.",                                 1
        },
        { "!active",    "The hostname specified is in a Custom DNS "
          "domain which has not yet been activated.",                           1
        },
        { "abuse",      "The hostname specified is blocked for abuse",          1 },
        { "notfqdn",    "No hosts are given.",                                  1 },
        { "numhost",    "Too many or too few hosts found.",                     1 },
        { "dnserr",     "DNS error encountered.",                               1 },
        { NULL,         NULL,                                                   0 }
};
#endif
