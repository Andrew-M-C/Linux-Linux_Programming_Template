




#include "AMCLibevTools.h"
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
/* socket */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>		/* AF_UNIX */
#include <netinet/in.h>		/* AF_INET */
#include <arpa/inet.h>

/******** internal functions ********/
#define __INTERNAL_FUNCTIONS
#ifdef __INTERNAL_FUNCTIONS

static inline void *_ev_watcher_alloc(size_t structSize)
{
	void *ret = malloc(structSize);
	if (ret) {
		memset(ret, 0, structSize);
	}
	return ret;
}




#endif



/******** public functions ********/
#define __PUBLIC_FUNCTIONS
#ifdef __PUBLIC_FUNCTIONS

struct AMC_ev_io *AMCEvIo_new()
{
	return (struct AMC_ev_io*)_ev_watcher_alloc(sizeof(struct AMC_ev_io));
}

struct AMC_ev_timer *AMCEvTimer_new()
{
	return (struct AMC_ev_timer*)_ev_watcher_alloc(sizeof(struct AMC_ev_timer));
}
struct AMC_ev_periodic *AMCEvPeriodic_new()
{
	return (struct AMC_ev_periodic*)_ev_watcher_alloc(sizeof(struct AMC_ev_periodic));
}

struct AMC_ev_signal *AMCEvSignal_new()
{
	return (struct AMC_ev_signal*)_ev_watcher_alloc(sizeof(struct AMC_ev_signal));
}

struct AMC_ev_child *AMCEvChild_new()
{
	return (struct AMC_ev_child*)_ev_watcher_alloc(sizeof(struct AMC_ev_child));
}

struct AMC_ev_stat *AMCEvStat_new()
{
	return (struct AMC_ev_stat*)_ev_watcher_alloc(sizeof(struct AMC_ev_stat));
}

struct AMC_ev_idle *AMCEvIdle_new()
{
	return (struct AMC_ev_idle*)_ev_watcher_alloc(sizeof(struct AMC_ev_idle));
}

struct AMC_ev_prepare *AMCEvPrepare_new()
{
	return (struct AMC_ev_prepare*)_ev_watcher_alloc(sizeof(struct AMC_ev_prepare));
}

struct AMC_ev_check *AMCEvCheck_new()
{
	return (struct AMC_ev_check*)_ev_watcher_alloc(sizeof(struct AMC_ev_check));
}

struct AMC_ev_fork *AMCEvFork_new()
{
	return (struct AMC_ev_fork*)_ev_watcher_alloc(sizeof(struct AMC_ev_fork));
}

struct AMC_ev_cleanup *AMCEvCleanup_new()
{
	return (struct AMC_ev_cleanup*)_ev_watcher_alloc(sizeof(struct AMC_ev_cleanup));
}

struct AMC_ev_embed *AMCEvEmbed_new()
{
	return (struct AMC_ev_embed*)_ev_watcher_alloc(sizeof(struct AMC_ev_embed));
}

struct AMC_ev_async *AMCEvAsync_new()
{
	return (struct AMC_ev_async*)_ev_watcher_alloc(sizeof(struct AMC_ev_async));
}



void AMCEvWatcher_free(void *watcher)
{
	int fd;

	if (NULL == watcher)
	{
		return;
	}

	fd = ((struct ev_io*)watcher)->fd;
	if (fd >= 2)
	{
		close(fd);
	}
	free(watcher);
	return;
}



struct AMC_ev_io *AMCEvIo_TcpListenNew(int port, int listenBacklog, void *arg)
{
	struct sockaddr_in address;
	struct AMC_ev_io *ret = AMCEvIo_new();
	int callStat = 0;
	int fd, errnoCopy, flag;

	if (ret)
	{
		ret->arg = arg;
	
		// socket
		fd = socket(AF_INET, SOCK_STREAM, 0);
		if (fd)
		{
			callStat = fcntl(fd, F_GETFL, 0);
			flag = fcntl(fd, F_SETFL, callStat | O_NONBLOCK);
			//printf("Set noblock\n");

			callStat = fcntl(fd, F_GETFD, 0);
			flag = fcntl(fd, F_SETFD, flag | FD_CLOEXEC);
			//printf("Set cloexec\n");
		}

		// bind
		if (fd)
		{
			bzero(&address, sizeof(address));
			address.sin_port = htons((uint16_t)port);
			address.sin_addr.s_addr = htonl(INADDR_ANY);
			address.sin_family = AF_INET;

			callStat = bind(fd, (struct sockaddr *)&address, sizeof(address));
			if (callStat < 0)
			{
				errnoCopy = errno;
				fprintf(stderr, "Cannot bind port %d: %s\n", port, strerror(errnoCopy));
				close(fd);
				fd = 0;
			}
			else
			{
				//printf("Binded port: %d\n", port);
			}
		}

		// listen
		if (fd)
		{
			callStat = listen(fd, listenBacklog);

			if (0 != callStat)
			{
				errnoCopy = errno;
				fprintf(stderr, "Cannot bind port: %s\n", strerror(errnoCopy));
				close(fd);
				fd = 0;
			}
			else
			{
				//printf("Listen OK\n");
			}
		}
	}

	if (ret && (0 == fd))
	{
		free(ret);
		fprintf(stderr, "Failed to create and init TCP listen socket.\n");
		ret = NULL;
	}
	else
	{
		ret->watcher.fd = fd;
		//printf("Return ev_io\n");
	}

	return ret;
}

#endif


