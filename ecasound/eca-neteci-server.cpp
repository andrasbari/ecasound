// ------------------------------------------------------------------------
// eca-neteci-server.c: NetECI server implementation.
// Copyright (C) 2002 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
// ------------------------------------------------------------------------

#include <cstring>        /* memcpy() */
#include <iostream>
#include <string>

#include <fcntl.h>        /* POSIX: fcntl() */
#include <pthread.h>      /* POSIX: pthread_* */
#include <unistd.h>       /* POSIX: fcntl() */
#include <arpa/inet.h>    /* BSD: inet_ntoa() */
#include <netinet/in.h>   /* BSD: inet_ntoa() */
#include <sys/poll.h>     /* POSIX: poll() */
#include <sys/socket.h>   /* BSD: getpeername() */

#include <kvu_dbc.h>
#include <kvu_fd_io.h>
#include <kvu_numtostr.h>
#include <kvu_utils.h>

#include <eca-control.h>
#include <eca-logger.h>

#include "ecasound.h"
#include "eca-neteci-server.h"

/** 
 * Options
 */
// #define NETECI_DEBUG_ENABLED

/**
 * Macro definitions
 */

#ifdef NETECI_DEBUG_ENABLED
#define NETECI_DEBUG(x) x
#else
#define NETECI_DEBUG(x) ((void) 0)
#endif

/** 
 * Import namespaces
 */

using namespace std;

ECA_NETECI_SERVER::ECA_NETECI_SERVER(struct ecasound_state* state)
  : state_repp(state),
    srvfd_rep(-1),
    server_listening_rep(false),
    unix_sockets_rep(false)
{
}

ECA_NETECI_SERVER::~ECA_NETECI_SERVER(void)
{
  if (server_listening_rep == true) {
    close_server_socket();
  }
}

/**
 * Launches the server thread.
 *
 * @param arg pointer to a ECA_NETECI_SERVER object
 */
void* ECA_NETECI_SERVER::launch_server_thread(void* arg)
{
  ECA_LOG_MSG(ECA_LOGGER::user_objects, "(eca-neteci-server) Server thread started");

  ECA_NETECI_SERVER* self = 
    reinterpret_cast<ECA_NETECI_SERVER*>(arg);
  self->run();
  return(0);
}

void ECA_NETECI_SERVER::run(void)
{
  create_server_socket();
  open_server_socket();
  if (server_listening_rep == true) {
    listen_for_events();
  }
  else {
    ECA_LOG_MSG(ECA_LOGGER::info, 
		"(eca-neteci-server) Unable to start NetECI server. Please check that no other program is using the 2868 TCP port used by NetECI.");
  }
  close_server_socket();

  ECA_LOG_MSG(ECA_LOGGER::user_objects, 
	      "(eca-neteci-server) server thread exiting");
}

/**
 * Creates a server socket with 'socket()'. Depending on 
 * object configuration either UNIX or IP socket is 
 * created.
 */
void ECA_NETECI_SERVER::create_server_socket(void)
{
  DBC_REQUIRE(server_listening_rep != true);
  DBC_REQUIRE(srvfd_rep <= 0);

  if (unix_sockets_rep == true) {
    srvfd_rep = socket(AF_UNIX, SOCK_STREAM, 0);
    if (srvfd_rep >= 0) {
      /* create a temporary filename for the socket in a secure way */
      socketpath_rep = "/tmp/neteci_server_1";
      addr_un_rep.sun_family = AF_UNIX;
      memcpy(addr_un_rep.sun_path, socketpath_rep.c_str(), socketpath_rep.size() + 1);
      addr_repp = reinterpret_cast<struct sockaddr*>(&addr_un_rep);
    }
  }
  else {
    srvfd_rep = socket(PF_INET, SOCK_STREAM, 0);
    if (srvfd_rep >= 0) {
      addr_in_rep.sin_family = AF_INET;
      addr_in_rep.sin_port = htons(2868);
      addr_in_rep.sin_addr.s_addr = INADDR_ANY;
      
      addr_repp = reinterpret_cast<struct sockaddr*>(&addr_in_rep);
    }
  }
}

/**
 * Opens the server socket for listening. If succesful,
 * 'server_listening_rep' will be true after the call.
 */
void ECA_NETECI_SERVER::open_server_socket(void)
{
  DBC_REQUIRE(server_listening_rep != true);
  DBC_REQUIRE(srvfd_rep > 0);

  int val = 1;
  int ret = setsockopt(srvfd_rep, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
  if (ret < 0) 
    std::cerr << "(eca-neteci-server) setsockopt() failed." << endl;
  
  // int res = bind(srvfd_rep, (struct sockaddr*)addr_repp, sizeof(*addr_repp));
  
  int res = 0;
  if (unix_sockets_rep == true) 
    res = bind(srvfd_rep, (struct sockaddr*)&addr_un_rep, sizeof(addr_un_rep));
  else
    res = bind(srvfd_rep, (struct sockaddr*)&addr_in_rep, sizeof(addr_in_rep));
  
  if (res == 0) {
    res = listen(srvfd_rep, 5);
    if (res == 0) {
      int res = fcntl(srvfd_rep, F_SETFL, O_NONBLOCK);
      if (res == -1) 
	std::cerr << "(eca-neteci-server) fcntl() failed." << endl;
      
      NETECI_DEBUG(std::cout << "(eca-neteci-server) server socket created." << endl);
      server_listening_rep = true;
    }
    else 
      std::cerr << "(eca-neteci-server) listen() failed." << endl;
  }
  else {
    if (unix_sockets_rep == true) {
      unlink(socketpath_rep.c_str());
    }
    socketpath_rep.resize(0);
    std::cerr << "(eca-neteci-server) bind() failed." << endl;
  }
  
  DBC_ENSURE((unix_sockets_rep == true) && 
	     ((server_listening_rep == true && socketpath_rep.size() > 0 ||
	       server_listening_rep != true && socketpath_rep.size() == 0)) ||
	     (unix_sockets_rep != true));
}

/**
 * Closes the server socket.
 */
void ECA_NETECI_SERVER::close_server_socket(void)
{
  DBC_REQUIRE(srvfd_rep > 0);
  DBC_REQUIRE(server_listening_rep == true);

  NETECI_DEBUG(cout << "(eca-neteci-server) closing socket " << kvu_numtostr(srvfd_rep) << "." << endl);
  close(srvfd_rep);
  srvfd_rep = -1;
  server_listening_rep = false;

  DBC_ENSURE(srvfd_rep == -1);
  DBC_ENSURE(server_listening_rep != true);
}

/**
 * Listens for and accepts incoming connections.
 */
void ECA_NETECI_SERVER::listen_for_events(void)
{
  /* 
   * - loop until we get an exit request from network or from
   *   ecasound_state
   */
  
  /* - enter poll
   * - if new connections, accept them and add the new client to
   *   client list
   * - if incoming bytes, grab ecasound_state lock, send command,
   *   store retval, release lock, send the reply to client
   * - return to poll
   */
  while(state_repp->exit_request == 0) {
    // NETECI_DEBUG(cout << "(eca-neteci-server) checking for events" << endl);
    check_for_events(5000);
  }

  if (state_repp->exit_request != 0) {
    NETECI_DEBUG(cout << "(eca-neteci-server) exit_request received" << endl);
  }
}

/**
 * Checks for new connections and messages from 
 * clients.
 * 
 * @param timeout upper-limit in ms for how long 
 *        function waits for events; if -1, 
 *        call will return immediately
 *        (ie. is non-blocking)
 */
void ECA_NETECI_SERVER::check_for_events(int timeout)
{
  int nfds = clients_rep.size() + 1;
  struct pollfd* ufds = new struct pollfd [nfds];

  ufds[0].fd = srvfd_rep;
  ufds[0].events = POLLIN;
  ufds[0].revents = 0;
  
  std::list<struct ecasound_neteci_server_client*>::iterator p = clients_rep.begin();
  for(int n = 1; n < nfds; n++) {
    ufds[n].fd = (*p)->fd;
    ufds[n].events = POLLIN;
    ufds[n].revents = 0;
    ++p;
  }
  DBC_CHECK(nfds == 1 || p == clients_rep.end());

  int ret = poll(ufds, nfds, timeout);
  if (ret > 0) {
    if (ufds[0].revents & POLLIN) {
      /* 1. new incoming connection */
      handle_connection(srvfd_rep);
    }
    p = clients_rep.begin();
    for(int n = 1; n < nfds; n++) {
      if (ufds[n].revents & POLLIN) {
	/* 2. client has sent a message */
	handle_client_messages(*p);
      }
      else if (ufds[n].revents == POLLERR ||
	       ufds[n].revents == POLLHUP ||
	       ufds[n].revents == POLLNVAL) {
	/* 3. error, remove client */
	remove_client(*p);
	p = clients_rep.end();
      }
      if (p != clients_rep.end()) ++p;
    }
  }

  delete[] ufds;
}

void ECA_NETECI_SERVER::handle_connection(int fd)
{
  socklen_t bytes = 0;
  string peername;
  int connfd = 0;

  
  if (unix_sockets_rep == true) {
    bytes = static_cast<socklen_t>(sizeof(addr_un_rep));
    connfd = accept(fd, reinterpret_cast<struct sockaddr*>(&addr_un_rep), &bytes);
    peername = "UNIX:" + socketpath_rep;
  }
  else {
    bytes = static_cast<socklen_t>(sizeof(addr_in_rep));
    connfd = accept(fd, reinterpret_cast<struct sockaddr*>(&addr_in_rep), &bytes);

    if (connfd > 0) {
      struct sockaddr_in peeraddr;
      socklen_t peernamelen;
      // struct in_addr peerip;
      int res = getpeername(connfd, 
			    reinterpret_cast<struct sockaddr*>(&peeraddr), 
			    reinterpret_cast<socklen_t*>(&peernamelen));
      DBC_CHECK(res == 0);
      peername = "TCP/IP:" + string(inet_ntoa(peeraddr.sin_addr));
      // peerip.s_addr = peeraddr.s_addr;
    }
  }

  ECA_LOG_MSG(ECA_LOGGER::info,
	      "(eca-neteci-server) New connection from " + 
	      peername + ".");


  if (connfd >= 0) {
    NETECI_DEBUG(cout << "(eca-neteci-server) incoming connection accepted" << endl);
    struct ecasound_neteci_server_client* client = new struct ecasound_neteci_server_client; /* add a new client */
    client->fd = connfd; 
    client->buffer = new char (32); 
    client->buffer_length = 32;
    client->buffer_current_ptr = 0;
    client->peername = peername;
    clients_rep.push_back(client);
  }
}

/**
 * Handle incoming messages for client 'client'.
 */
void ECA_NETECI_SERVER::handle_client_messages(struct ecasound_neteci_server_client* client)
{
  char* buf[128];
  int connfd = client->fd;

  NETECI_DEBUG(cout << "(eca-neteci-server) handle_client_messages for fd " 
	       << connfd << endl);

  ssize_t c = kvu_fd_read(connfd, buf, 128, 5000);
  if (c > 0) {
    parse_raw_incoming_data(reinterpret_cast<char*>(buf), c, client);
    while(parsed_cmd_queue_rep.size() > 0) {
      handle_eci_command(parsed_cmd_queue_rep.front(), client);
      parsed_cmd_queue_rep.pop_front();
    }
    /* ... */
  }
  else {
    /* read() <= 0 */
    NETECI_DEBUG(cout << "(eca-neteci-server) read error, removing client-fd " << connfd << "." << endl);
    remove_client(client);
  }
}

void ECA_NETECI_SERVER::parse_raw_incoming_data(const char* buffer,
						ssize_t bytes,
						struct ecasound_neteci_server_client* client)
{
  DBC_REQUIRE(buffer != 0);
  DBC_REQUIRE(bytes >= 0);
  DBC_REQUIRE(client != 0);
  DBC_DECLARE(int old_client_ptr = client->buffer_current_ptr);
  DBC_DECLARE(unsigned int old_cmd_queue_size = parsed_cmd_queue_rep.size());

  NETECI_DEBUG(cout << "(eca-neteci-server) parse incoming data; "
	       << bytes << " bytes. Buffer length is " 
	       << client->buffer_length << endl);
  
  for(int n = 0; n < bytes; n++) {
    DBC_CHECK(client->buffer_current_ptr < client->buffer_length + 1);
    if (client->buffer_current_ptr == client->buffer_length) {
      cerr << "(eca-neteci-server) client buffer overflow, flushing." << endl;
      client->buffer_current_ptr = 0;
    }
    else {
      NETECI_DEBUG(cout << "(eca-neteci-server) copying '" << buffer[n] << "'\n");
      client->buffer[client->buffer_current_ptr] = buffer[n];
      if (client->buffer_current_ptr > 0 &&
	  client->buffer[client->buffer_current_ptr] == '\n' &&
	  client->buffer[client->buffer_current_ptr - 1] == '\r') {

	string cmd (client->buffer, client->buffer_current_ptr);
	NETECI_DEBUG(cout << "(eca-neteci-server) storing command " << cmd << endl);
	parsed_cmd_queue_rep.push_back(cmd);

	NETECI_DEBUG(cout << "(eca-neteci-server) copying " 
		     << client->buffer_length - client->buffer_current_ptr - 1
		     << " bytes from " << client->buffer_current_ptr + 1 
		     << " to the beginning of the buffer."
		     << " Index is " << client->buffer_current_ptr << endl);

	DBC_CHECK(client->buffer_current_ptr < client->buffer_length);

#if 0
	/* must not use memcpy() as the 
	   affected areas may overlap! */
	for(int o = 0, p = index + 1; 
	    p < client->buffer_length; o++, p++) {
	  client->buffer[o] = client->buffer[p];
	}
#endif
	client->buffer_current_ptr = 0;
      }
      else {
	// NETECI_DEBUG(cout << "(eca-neteci-server) crlf not found, index=" << index << ", n=" << n << "cur_ptr=" << client->buffer_current_ptr << ".\n");
	client->buffer_current_ptr++;
      }
    }
  }

  DBC_ENSURE(client->buffer_current_ptr > old_client_ptr ||
	     parsed_cmd_queue_rep.size() > old_cmd_queue_size);
}

void ECA_NETECI_SERVER::handle_eci_command(const string& cmd, struct ecasound_neteci_server_client* client)
{
  string retstr;
  ECA_CONTROL* ctrl = state_repp->control;

  NETECI_DEBUG(cout << "(eca-neteci-server) handle eci command: " << cmd << endl);

  int res = pthread_mutex_lock(state_repp->lock);
  DBC_CHECK(res == 0);

  DBC_CHECK(ctrl != 0);
  ctrl->command(cmd);
  
  /* 1. status code */
  if (ctrl->last_error().size() > 0) {
    retstr += "400";
  }
  else {
    retstr += "100";
  }
  retstr += " ";

  /* 2. return value type */
  retstr += ctrl->last_type();
  retstr += " ";

  /* 3. create return value */
  string retvalue;
  switch(ctrl->last_type()[0])
    {
    case 'f':
      retvalue = kvu_numtostr(ctrl->last_float(),9);
      break;
    case 'i': /* "li" */
      retvalue = kvu_numtostr(ctrl->last_integer());
      break;
    case 'l':
      retvalue = kvu_numtostr(ctrl->last_long_integer());
      break;
    case 's':
      retvalue = ctrl->last_string();
      break;
    case 'S':
      retvalue = kvu_vector_to_string(ctrl->last_string_list(), ",");
      break;
    default:
      DBC_CHECK(false);
    }

  /* 4. return size */
  retstr += kvu_numtostr(retvalue.size());
  retstr += " ";

  /* 5. add return value */
  retstr += "\r\n\r\n";
  retstr += retvalue;
  
  res = pthread_mutex_unlock(state_repp->lock);
  DBC_CHECK(res == 0);

  int bytes_to_send = retstr.size();
  while(bytes_to_send > 0) {
    int ret = kvu_fd_write(client->fd, retstr.c_str(), retstr.size(), 5000);
    if (ret < 0) {
      cerr << "(eca-neteci-server) error in kvu_fd_write(), removing client.\n";
      remove_client(client);
      break;
    }
    else {
      bytes_to_send -= ret;
    }
  }
}

/**
 * Removes 'client' from list of clients.
 *
 * Note! Internally, the 'fd' field of the deleted client 
 * is marked to be -1.
 *
 * @see clean_removed_clients()
 */
void ECA_NETECI_SERVER::remove_client(struct ecasound_neteci_server_client* client)
{
  DBC_DECLARE(size_t oldsize = clients_rep.size());

  NETECI_DEBUG(std::cout << "(eca-neteci-server) removing client." << std::endl);

  if (client->fd > 0) {
    ECA_LOG_MSG(ECA_LOGGER::info, 
		"(eca-neteci-server) Closing connection " +
		client->peername + ".");
    close(client->fd);
    client->fd = -1;
  }

  clean_removed_clients();
  
  DBC_ENSURE(clients_rep.size() == oldsize - 1);
}

/**
 * Cleans the list of clients from removed objects.
 *
 * @see remove_client()
 */
void ECA_NETECI_SERVER::clean_removed_clients(void)
{
  DBC_DECLARE(size_t oldsize = clients_rep.size());
  DBC_DECLARE(size_t counter = 0);

  NETECI_DEBUG(std::cout << "(eca-neteci-server) cleaning removed clients." << std::endl);

  std::list<struct ecasound_neteci_server_client*>::iterator p = clients_rep.begin();
  while(p != clients_rep.end()) {
    if ((*p)->fd == -1) {
      delete (*p)->buffer;
      delete *p;
      clients_rep.erase(p);
      p = clients_rep.begin();
      DBC_DECLARE(++counter);
    }
    else {
      ++p;
    }
  }

  DBC_ENSURE(clients_rep.size() == oldsize - counter);
}