/* dnsmasq is Copyright (c) 2000-2013 Simon Kelley

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991, or
   (at your option) version 3 dated 29 June, 2007.
 
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
     
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* Declare static char *compiler_opts  in config.h */
#define DNSMASQ_COMPILE_OPTS

#include "dnsmasq.h"

struct daemon *daemon;

static volatile pid_t pid = 0;
static volatile int pipewrite;

#ifdef CONFIG_RTL_819X
cyg_uint8  rtl_dnsmasq_stack[RTL_DNSMASQ_THREAD_STACKSIZE];
cyg_handle_t  rtl_dnsmasq_thread;
cyg_thread  rtl_dnsmasq_thread_object;

static char rtl_dnsmasq_running=0;    // Boolean (running loop)
static char rtl_dnsmasq_started=0;
static cyg_uint8 rtl_dnsmasq_quitting=0;

#endif

static int set_dns_listeners(time_t now, fd_set *set, int *maxfdp);
static void check_dns_listeners(fd_set *set, time_t now);

static void free_dnsmasq_res()
{
	struct listener *tmp_lsr=NULL;
	struct serverfd *tmp_sfd=NULL; 
	struct resolvc *tmp_rlc=NULL;
	struct mx_srv_record *tmp_mxnames=NULL;
	struct host_record *tmp_hrecd=NULL;
	struct cname *tmp_cname=NULL;
	struct interface_name *tmp_ifname=NULL;
	struct cond_domain *tmp_cdmn=NULL;
	struct iname *tmp_iname=NULL;
	struct bogus_addr *tmp_baddr=NULL;
	struct server *tmp_serv=NULL;

	if(daemon)
	{
		if(daemon->packet)
		{
			free(daemon->packet);
			daemon->packet=NULL;
		}

		if(daemon->namebuff)
		{
			free(daemon->namebuff);
			daemon->namebuff=NULL;
		}
		
		if(daemon->addrbuff)
		{
			free(daemon->addrbuff);
			daemon->addrbuff=NULL;
		}

		for( ; daemon->listeners; daemon->listeners=tmp_lsr)
		{
			tmp_lsr=daemon->listeners->next;
			if(daemon->listeners->fd>0)
			{
				close(daemon->listeners->fd);
				daemon->listeners->fd=-1;
			}
			if(daemon->listeners->tcpfd>0)
			{
				close(daemon->listeners->tcpfd);
				daemon->listeners->tcpfd=-1;
			}
			free(daemon->listeners);
			daemon->listeners=NULL;
		}	
		
		for( ; daemon->sfds; daemon->sfds=tmp_sfd)
		{
			tmp_sfd=daemon->sfds->next;
			if(daemon->sfds->fd>0)
			{
				close(daemon->sfds->fd);
				daemon->sfds->fd=-1;
			}
			free(daemon->sfds);
			daemon->sfds=NULL;
		}
/*
		for( ; daemon->resolv_files; daemon->resolv_files=tmp_rlc)
		{
			tmp_rlc=daemon->resolv_files->next;
			if(daemon->resolv_files->name)
			{
				free(daemon->resolv_files->name);
				daemon->resolv_files->name=NULL;
			}
			free(daemon->resolv_files);
			daemon->resolv_files=NULL;
		}
*/
		for( ; daemon->mxnames; daemon->mxnames=tmp_mxnames)
		{
			tmp_mxnames=daemon->mxnames->next;
			if(daemon->mxnames->name)
			{
				free(daemon->mxnames->name);
				daemon->mxnames->name=NULL;
			}
			if(daemon->mxnames->target)
			{
				free(daemon->mxnames->target);
				daemon->mxnames->target=NULL;
			}
			free(daemon->mxnames);
			daemon->mxnames=NULL;		
		}

		for( ; daemon->host_records; daemon->host_records=tmp_hrecd)
		{
			tmp_hrecd=daemon->host_records->next;
			struct name_list *tmp_nl=NULL;
			for( ; daemon->host_records->names; daemon->host_records->names=tmp_nl)
			{
				tmp_nl=daemon->host_records->names->next;
				if(daemon->host_records->names->name)
				{
					free(daemon->host_records->names->name);
					daemon->host_records->names->name=NULL;
				}
				free(daemon->host_records->names);
				daemon->host_records->names=NULL;
			}
			free(daemon->host_records);
			daemon->host_records=NULL;
		}

		for( ; daemon->cnames; daemon->cnames=tmp_cname)
		{
			tmp_cname=daemon->cnames->next;
			if(daemon->cnames->alias)
			{
				free(daemon->cnames->alias);
				daemon->cnames->alias=NULL;
			}
			if(daemon->cnames->target)
			{
				free(daemon->cnames->target);
				daemon->cnames->target=NULL;
			}
			free(daemon->cnames);
			daemon->cnames=NULL;
		}

		for( ; daemon->int_names; daemon->int_names=tmp_ifname)
		{
			tmp_ifname=daemon->int_names->next;
			if(daemon->int_names->name)
			{
				free(daemon->int_names->name);
				daemon->int_names->name=NULL;
			}
			if(daemon->int_names->intr)
			{
				free(daemon->int_names->intr);
				daemon->int_names->intr=NULL;
			}
			free(daemon->int_names);
			daemon->int_names=NULL;
		}

		if(daemon->domain_suffix)
		{
			free(daemon->domain_suffix);
			daemon->domain_suffix=NULL;
		}

		for( ; daemon->cond_domain; daemon->cond_domain=tmp_cdmn)
		{
			tmp_cdmn=daemon->cond_domain->next;
			if(daemon->cond_domain->domain)
			{
				free(daemon->cond_domain->domain);	
				daemon->cond_domain->domain=NULL;
			}
			free(daemon->cond_domain);
			daemon->cond_domain=NULL;
		}

		for( ; daemon->if_names; daemon->if_names=tmp_iname)
		{
			tmp_iname=daemon->if_names->next;
			if(daemon->if_names->name)
			{
				free(daemon->if_names->name);
				daemon->if_names->name=NULL;
			}
			free(daemon->if_names);
			daemon->if_names=NULL;
		}
		
		for( ; daemon->if_addrs; daemon->if_addrs=tmp_iname)
		{
			tmp_iname=daemon->if_addrs->next;				
			free(daemon->if_addrs);
			daemon->if_addrs=NULL;
		}
		
		for( ; daemon->if_except; daemon->if_except=tmp_iname)
		{
			tmp_iname=daemon->if_except->next;
			if(daemon->if_except->name)
			{
				free(daemon->if_except->name);
				daemon->if_except->name=NULL;
			}
			free(daemon->if_except);
			daemon->if_except=NULL;
		}

		for( ; daemon->bogus_addr; daemon->bogus_addr=tmp_baddr)
		{
			tmp_baddr=daemon->bogus_addr->next;				
			free(daemon->bogus_addr);
			daemon->bogus_addr=NULL;			
		}

		for( ; daemon->servers; daemon->servers=tmp_serv)
		{
			tmp_serv=daemon->servers->next;
			if(daemon->servers->domain)
			{
				free(daemon->servers->domain);	
				daemon->servers->domain=NULL;
			}
			free(daemon->servers);
			daemon->servers=NULL;
		}	
		free(daemon);
		daemon=NULL;
	}
}

int start_rtl_dnsmasq_thread()
{
  time_t now;
  struct iname *if_tmp;  

  rtl_dnsmasq_running=1;  

  long i;
  
  read_opts(compile_opts);
    
  if (daemon->edns_pktsz < PACKETSZ)
    daemon->edns_pktsz = PACKETSZ;
  daemon->packet_buff_sz = daemon->edns_pktsz > DNSMASQ_PACKETSZ ? 
    daemon->edns_pktsz : DNSMASQ_PACKETSZ;
  daemon->packet = safe_malloc(daemon->packet_buff_sz);

  daemon->addrbuff = safe_malloc(ADDRSTRLEN);
  
  now = dnsmasq_time();

  /* Create a serial at startup if not configured. */

  if (!enumerate_interfaces())
    die(_("failed to find list of interfaces: %s"), NULL, EC_MISC);  
 
    create_wildcard_listeners(); 
  
  if (daemon->port != 0)
    cache_init();     
  
  if (daemon->port != 0)
  	  pre_allocate_sfds(); 

  
  int maxfd = -1;
  struct timeval t;
  fd_set rset, wset, eset;
	  
  while (1)
    {     	
    	if(rtl_dnsmasq_quitting)
    	{
    		//diag_printf("%s:%d####calling free_dnsmasq_res()!\n",__FUNCTION__,__LINE__);
		free_dnsmasq_res();
		rtl_dnsmasq_quitting=0;
		return 0;
	}
		
      maxfd = -1;	
      FD_ZERO(&rset);
      FD_ZERO(&wset);
      FD_ZERO(&eset);

	set_dns_listeners(now, &rset, &maxfd);
	t.tv_sec=1;
	t.tv_usec=0;
	
      if (select(maxfd+1, &rset, &wset, &eset, &t) <= 0)
	{
	  /* otherwise undefined after error */
	  FD_ZERO(&rset); FD_ZERO(&wset); FD_ZERO(&eset);
	}
	
      now = dnsmasq_time();
      
      /* Check the interfaces to see if any have exited DAD state
	 and if so, bind the address. */
      if (is_dad_listeners())
	{
	  diag_printf("%s:%d\n",__FUNCTION__,__LINE__);
	  enumerate_interfaces();
	  /* NB, is_dad_listeners() == 1 --> we're binding interfaces */
	  create_bound_listeners(0);
	}

      /* Check for changes to resolv files once per second max. */
      /* Don't go silent for long periods if the clock goes backwards. */
      if (daemon->last_resolv == 0 || 
	  difftime(now, daemon->last_resolv) > 1.0 || 
	  difftime(now, daemon->last_resolv) < -1.0)
	{
	  /* poll_resolv doesn't need to reload first time through, since 
	     that's queued anyway. */

	  poll_resolv(0, daemon->last_resolv != 0, now); 	  
	  daemon->last_resolv = now;
	}
      
      check_dns_listeners(&rset, now);      
    }
}

void poll_resolv(int force, int do_reload, time_t now)
{
  struct resolvc *res, *latest;
  struct stat statbuf;
  time_t last_change = 0;
  /* There may be more than one possible file. 
     Go through and find the one which changed _last_.
     Warn of any which can't be read. */

  if (daemon->port == 0 || option_bool(OPT_NO_POLL))
    return;
  
  for (latest = NULL, res = daemon->resolv_files; res; res = res->next)
    if (stat(res->name, &statbuf) == -1)
      {
	if (force)
	  {
	    res->mtime = 0; 
	    continue;
	  }

	if (!res->logged)
	  my_syslog(LOG_WARNING, _("failed to access %s: %s"), res->name, strerror(errno));
	res->logged = 1;
	
	if (res->mtime != 0)
	  { 
	    /* existing file evaporated, force selection of the latest
	       file even if its mtime hasn't changed since we last looked */
	    poll_resolv(1, do_reload, now);
	    return;
	  }
      }
    else
      {
	res->logged = 0;
	if (force || (statbuf.st_mtime != res->mtime))
          {
            res->mtime = statbuf.st_mtime;
	    if (difftime(statbuf.st_mtime, last_change) > 0.0)
	      {
		last_change = statbuf.st_mtime;
		latest = res;
	      }
	  }
      }
  
  if (latest)
    {
      static int warned = 0;
      if (reload_servers(latest->name))
	{
	  my_syslog(LOG_INFO, _("reading %s"), latest->name);
	  warned = 0;
	  check_servers();
	  if (option_bool(OPT_RELOAD) && do_reload)
	    clear_cache_and_reload(now);
	}
      else 
	{
	  latest->mtime = 0;
	  if (!warned)
	    {
	      my_syslog(LOG_WARNING, _("no servers found in %s, will retry"), latest->name);
	      warned = 1;
	    }
	}
    }
}       

void clear_cache_and_reload(time_t now)
{
  if (daemon->port != 0)
    cache_reload();  
}

static int set_dns_listeners(time_t now, fd_set *set, int *maxfdp)
{
  struct serverfd *serverfdp;
  struct listener *listener;
  int wait = 0, i;  
  
  /* will we be able to get memory? */
  if (daemon->port != 0)
    get_new_frec(now, &wait);
  
  for (serverfdp = daemon->sfds; serverfdp; serverfdp = serverfdp->next)
    {
      FD_SET(serverfdp->fd, set);
      bump_maxfd(serverfdp->fd, maxfdp);
    }

  if (daemon->port != 0 && !daemon->osport)
    for (i = 0; i < RANDOM_SOCKS; i++)
      if (daemon->randomsocks[i].refcount != 0)
	{
	  FD_SET(daemon->randomsocks[i].fd, set);
	  bump_maxfd(daemon->randomsocks[i].fd, maxfdp);
	}
  
  for (listener = daemon->listeners; listener; listener = listener->next)
    {
      /* only listen for queries if we have resources */
      if (listener->fd != -1 && wait == 0)
	{
	  FD_SET(listener->fd, set);
	  bump_maxfd(listener->fd, maxfdp);
	}

      /* death of a child goes through the select loop, so
	 we don't need to explicitly arrange to wake up here */
      if  (listener->tcpfd != -1)
	for (i = 0; i < MAX_PROCS; i++)
	  if (daemon->tcp_pids[i] == 0)
	    {
	      FD_SET(listener->tcpfd, set);
	      bump_maxfd(listener->tcpfd, maxfdp);
	      break;
	    }
    }
  
  return wait;
}

static void check_dns_listeners(fd_set *set, time_t now)
{
  struct serverfd *serverfdp;
  struct listener *listener;
  int i;

  for (serverfdp = daemon->sfds; serverfdp; serverfdp = serverfdp->next)
    if (FD_ISSET(serverfdp->fd, set))
    {    		
      		//diag_printf("%s:%d serverfdp->fd=%d\n",__FUNCTION__,__LINE__,serverfdp->fd);
     		reply_query(serverfdp->fd, serverfdp->source_addr.sa.sa_family, now);
    }
  
  if (daemon->port != 0 && !daemon->osport)
    for (i = 0; i < RANDOM_SOCKS; i++)
      if (daemon->randomsocks[i].refcount != 0 && 
	  FD_ISSET(daemon->randomsocks[i].fd, set))
	reply_query(daemon->randomsocks[i].fd, daemon->randomsocks[i].family, now);
  
  for (listener = daemon->listeners; listener; listener = listener->next)
    {
      if (listener->fd != -1 && FD_ISSET(listener->fd, set))
      {
      		//diag_printf("%s:%d listener->fd=%d\n",__FUNCTION__,__LINE__,listener->fd);
		receive_query(listener, now); 
      	}     

      if (listener->tcpfd != -1 && FD_ISSET(listener->tcpfd, set))
	{
	  int confd, client_ok = 1;
	  struct irec *iface = NULL;
	  pid_t p;
	  union mysockaddr tcp_addr;
	  socklen_t tcp_len = sizeof(union mysockaddr);

	  while ((confd = accept(listener->tcpfd, NULL, NULL)) == -1 && errno == EINTR);
	  
	  if (confd == -1)
	    continue;

	  if (getsockname(confd, (struct sockaddr *)&tcp_addr, &tcp_len) == -1)
	    {
	      close(confd);
	      continue;
	    }

	   if (option_bool(OPT_NOWILD))
	    iface = listener->iface; /* May be NULL */
	   else 
	     {
	       int if_index;
	       char intr_name[IF_NAMESIZE];
 
	       /* In full wildcard mode, need to refresh interface list.
		  This happens automagically in CLEVERBIND */
	       if (!option_bool(OPT_CLEVERBIND))
		 enumerate_interfaces();
	       
	       /* if we can find the arrival interface, check it's one that's allowed */
	       if ((if_index = tcp_interface(confd, tcp_addr.sa.sa_family)) != 0 &&
		   indextoname(listener->tcpfd, if_index, intr_name))
		 {
		   struct all_addr addr;
		   addr.addr.addr4 = tcp_addr.in.sin_addr;
#ifdef HAVE_IPV6
		   if (tcp_addr.sa.sa_family == AF_INET6)
		     addr.addr.addr6 = tcp_addr.in6.sin6_addr;
#endif
		   
		   for (iface = daemon->interfaces; iface; iface = iface->next)
		     if (iface->index == if_index)
		       break;
		   
		   if (!iface && !loopback_exception(listener->tcpfd, tcp_addr.sa.sa_family, &addr, intr_name))
		     client_ok = 0;
		 }
	       
	       if (option_bool(OPT_CLEVERBIND))
		 iface = listener->iface; /* May be NULL */
	       else
		 {
		    /* Check for allowed interfaces when binding the wildcard address:
		       we do this by looking for an interface with the same address as 
		       the local address of the TCP connection, then looking to see if that's
		       an allowed interface. As a side effect, we get the netmask of the
		      interface too, for localisation. */
		   
		   for (iface = daemon->interfaces; iface; iface = iface->next)
		     if (sockaddr_isequal(&iface->addr, &tcp_addr))
		       break;
		   
		   if (!iface)
		     client_ok = 0;
		 }
	     }
	  	  
	  if (!client_ok)
	    {
	      shutdown(confd, SHUT_RDWR);
	      close(confd);
	    }
#ifndef NO_FORK
	  else if (!option_bool(OPT_DEBUG) && (p = fork()) != 0)
	    {
	      if (p != -1)
		{
		  int i;
		  for (i = 0; i < MAX_PROCS; i++)
		    if (daemon->tcp_pids[i] == 0)
		      {
			daemon->tcp_pids[i] = p;
			break;
		      }
		}
	      close(confd);
	    }
#endif
	  else
	    {
	      unsigned char *buff;
	      struct server *s; 
	      int flags;
	      struct in_addr netmask;
	      int auth_dns;

	      if (iface)
		{
		  netmask = iface->netmask;
		  auth_dns = iface->dns_auth;
		}
	      else
		{
		  netmask.s_addr = 0;
		  auth_dns = 0;
		}

#ifndef NO_FORK
	      /* Arrange for SIGALARM after CHILD_LIFETIME seconds to
		 terminate the process. */
	      if (!option_bool(OPT_DEBUG))
		alarm(CHILD_LIFETIME);
#endif

	      /* start with no upstream connections. */
	      for (s = daemon->servers; s; s = s->next)
		 s->tcpfd = -1; 
	      
	      /* The connected socket inherits non-blocking
		 attribute from the listening socket. 
		 Reset that here. */
	      if ((flags = fcntl(confd, F_GETFL, 0)) != -1)
		fcntl(confd, F_SETFL, flags & ~O_NONBLOCK);
	      
	      buff = tcp_request(confd, now, &tcp_addr, netmask, auth_dns);
	       
	      shutdown(confd, SHUT_RDWR);
	      close(confd);
	      
	      if (buff)
		free(buff);
	      
	      for (s = daemon->servers; s; s = s->next)
		if (s->tcpfd != -1)
		  {
		    shutdown(s->tcpfd, SHUT_RDWR);
		    close(s->tcpfd);
		  }
#ifndef NO_FORK		   
	      if (!option_bool(OPT_DEBUG))
		{
		  flush_log();
		  _exit(0);
		}
#endif
	    }
	}
    }
}

#ifdef HAVE_SYSTEM_REINIT
void kill_dnsmasq()
{
	if(rtl_dnsmasq_started && rtl_dnsmasq_running)
	{
		//diag_printf("%s:%d####\n",__FUNCTION__,__LINE__);
		rtl_dnsmasq_quitting = 1;		
		while(rtl_dnsmasq_quitting)
			cyg_thread_delay(5);
		
		//diag_printf("%s:%d####before killing dnsmasq!\n",__FUNCTION__,__LINE__);
		cyg_thread_kill(rtl_dnsmasq_thread);
		cyg_thread_delete(rtl_dnsmasq_thread);
		rtl_dnsmasq_started =0;
		rtl_dnsmasq_running=0;
		//diag_printf("%s:%d####after killing dnsmasq!\n",__FUNCTION__,__LINE__);
	}
}
#endif

__externC int rtl_dnsmasq_startup()
{
	diag_printf("enter rtl_dnsmasq_startup!\n");
	if (rtl_dnsmasq_started==1)
	{
		diag_printf("rtl_dnsmasq has already been startup\n");
		return(-1);
	}	
	if (rtl_dnsmasq_running==0)
	{
		cyg_thread_create(RTL_DNSMASQ_THREAD_PRIORITY,
		start_rtl_dnsmasq_thread,
		0,
		"rtl dnsmasq",
		rtl_dnsmasq_stack,
		sizeof(rtl_dnsmasq_stack),
		&rtl_dnsmasq_thread,
		&rtl_dnsmasq_thread_object);
		
		diag_printf("Starting RTL DNSMASQ thread\n");
		cyg_thread_resume(rtl_dnsmasq_thread);
		rtl_dnsmasq_started=1;
		return(0);
	}
	else
	{
		diag_printf("RTL DNSMASQ is already running\n");
		return(-1);
	}
}
 
