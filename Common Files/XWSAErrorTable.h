#ifndef XWSAERRORTABLE_H
#define XWSAERRORTABLE_H


struct WSA_ERROR_TABLE
{
	int nErrorCode;
	const TCHAR * pszErrorString;
	const TCHAR * pszShortDescription;
	const TCHAR * pszLongDescription;
};

static WSA_ERROR_TABLE error_table[] =
{
	WSAEINTR,
	_T("WSAEINTR"),
	_T("Interrupted function call"),
	_T("A blocking operation was interrupted by a call to WSACancelBlockingCall"),

	WSAEACCES,
	_T("WSAEACCES"),
	_T("Permission denied"),
	_T("An attempt was made to access a socket in a way forbidden by its access permissions. An example is using a broadcast address for sendto without broadcast permission being set using setsockopt (SO_BROADCAST). \r\n\r\nAnother possible reason for the WSAEACCES error is that when the bind function is called (on Windows NT 4 SP4 or later), another application, service, or kernel mode driver is bound to the same address with exclusive access. Such exclusive access is a new feature of Windows NT 4 SP4 and later, and is implemented by using the SO_EXCLUSIVEADDRUSE option"),

	WSAEFAULT,
	_T("WSAEFAULT"),
	_T("Bad address"),
	_T("The system detected an invalid pointer address in attempting to use a pointer argument of a call. This error occurs if an application passes an invalid pointer value, or if the length of the buffer is too small. For instance, if the length of an argument, which is a sockaddr structure, is smaller than the sizeof(sockaddr)"),

	WSAEINVAL,
	_T("WSAEINVAL"),
	_T("Invalid argument"),
	_T("Some invalid argument was supplied (for example, specifying an invalid level to the setsockopt function). In some instances, it also refers to the current state of the socket - for instance, calling accept on a socket that is not listening"),

	WSAEMFILE,
	_T("WSAEMFILE"),
	_T("Too many open files"),
	_T("Too many open sockets. Each implementation may have a maximum number of socket handles available, either globally, per process, or per thread"),

	WSAEWOULDBLOCK,
	_T("WSAEWOULDBLOCK"),
	_T("Resource temporarily unavailable"),
	_T("This error is returned from operations on nonblocking sockets that cannot be completed immediately, for example recv when no data is queued to be read from the socket. It is a nonfatal error, and the operation should be retried later. It is normal for WSAEWOULDBLOCK to be reported as the result from calling connect on a nonblocking SOCK_STREAM socket, since some time must elapse for the connection to be established"),

	WSAEINPROGRESS,
	_T("WSAEINPROGRESS"),
	_T("Operation now in progress"),
	_T("A blocking operation is currently executing. Windows Sockets only allows a single blocking operation per- task or thread to be outstanding, and if any other function call is made (whether or not it references that or any other socket) the function fails with the WSAEINPROGRESS error"),

	WSAEALREADY,
	_T("WSAEALREADY"),
	_T("Operation already in progress"),
	_T("An operation was attempted on a nonblocking socket with an operation already in progress - that is, calling connect a second time on a nonblocking socket that is already connecting, or canceling an asynchronous request (WSAAsyncGetXbyY) that has already been canceled or completed"),

	WSAENOTSOCK,
	_T("WSAENOTSOCK"),
	_T("Socket operation on nonsocket"),
	_T("An operation was attempted on something that is not a socket. Either the socket handle parameter did not reference a valid socket, or for select, a member of an fd_set was not valid"),

	WSAEDESTADDRREQ,
	_T("WSAEDESTADDRREQ"),
	_T("Destination address required"),
	_T("A required address was omitted from an operation on a socket. For example, this error is returned if sendto is called with the remote address of ADDR_ANY"),

	WSAEMSGSIZE,
	_T("WSAEMSGSIZE"),
	_T("Message too long"),
	_T("A message sent on a datagram socket was larger than the internal message buffer or some other network limit, or the buffer used to receive a datagram was smaller than the datagram itself"),

	WSAEPROTOTYPE,
	_T("WSAEPROTOTYPE"),
	_T("Protocol wrong type for socket"),
	_T("A protocol was specified in the socket function call that does not support the semantics of the socket type requested. For example, the ARPA Internet UDP protocol cannot be specified with a socket type of SOCK_STREAM"),

	WSAENOPROTOOPT,
	_T("WSAENOPROTOOPT"),
	_T("Bad protocol option"),
	_T("An unknown, invalid or unsupported option or level was specified in a getsockopt or setsockopt call"),

	WSAEPROTONOSUPPORT,
	_T("WSAEPROTONOSUPPORT"),
	_T("Protocol not supported"),
	_T("The requested protocol has not been configured into the system, or no implementation for it exists. For example, a socket call requests a SOCK_DGRAM socket, but specifies a stream protocol"),

	WSAESOCKTNOSUPPORT,
	_T("WSAESOCKTNOSUPPORT"),
	_T("Socket type not supported"),
	_T("The support for the specified socket type does not exist in this address family. For example, the optional type SOCK_RAW might be selected in a socket call, and the implementation does not support SOCK_RAW sockets at all"),

	WSAEOPNOTSUPP,
	_T("WSAEOPNOTSUPP"),
	_T("Operation not supported"),
	_T("The attempted operation is not supported for the type of object referenced. Usually this occurs when a socket descriptor to a socket that cannot support this operation is trying to accept a connection on a datagram socket"),

	WSAEPFNOSUPPORT,
	_T("WSAEPFNOSUPPORT"),
	_T("Protocol family not supported"),
	_T("The protocol family has not been configured into the system or no implementation for it exists. This message has a slightly different meaning from WSAEAFNOSUPPORT. However, it is interchangeable in most cases, and all Windows Sockets functions that return one of these messages also specify WSAEAFNOSUPPORT"),

	WSAEAFNOSUPPORT,
	_T("WSAEAFNOSUPPORT"),
	_T("Address family not supported by protocol family"),
	_T("An address incompatible with the requested protocol was used. All sockets are created with an associated address family (that is, AF_INET for Internet Protocols) and a generic protocol type (that is, SOCK_STREAM). This error is returned if an incorrect protocol is explicitly requested in the socket call, or if an address of the wrong family is used for a socket, for example, in sendto"),

	WSAEADDRINUSE,
	_T("WSAEADDRINUSE"),
	_T("Address already in use"),
	_T("Typically, only one usage of each socket address (protocol/IP address/port) is permitted. This error occurs if an application attempts to bind a socket to an IP address/port that has already been used for an existing socket, or a socket that was not closed properly, or one that is still in the process of closing. For server applications that need to bind multiple sockets to the same port number, consider using setsockopt (SO_REUSEADDR). Client applications usually need not call bind at all - connect chooses an unused port automatically. When bind is called with a wildcard address (involving ADDR_ANY), a WSAEADDRINUSE error could be delayed until the specific address is committed. This could happen with a call to another function later, including connect, listen, WSAConnect, or WSAJoinLeaf"),

	WSAEADDRNOTAVAIL,
	_T("WSAEADDRNOTAVAIL"),
	_T("Cannot assign requested address"),
	_T("The requested address is not valid in its context. This normally results from an attempt to bind to an address that is not valid for the local computer. This can also result from connect, sendto, WSAConnect, WSAJoinLeaf, or WSASendTo when the remote address or port is not valid for a remote computer (for example, address or port 0)"),

	WSAENETDOWN,
	_T("WSAENETDOWN"),
	_T("Network is down"),
	_T("A socket operation encountered a dead network. This could indicate a serious failure of the network system (that is, the protocol stack that the Windows Sockets DLL runs over), the network interface, or the local network itself"),

	WSAENETUNREACH,
	_T("WSAENETUNREACH"),
	_T("Network is unreachable"),
	_T("A socket operation was attempted to an unreachable network. This usually means the local software knows no route to reach the remote host"),

	WSAENETRESET,
	_T("WSAENETRESET"),
	_T("Network dropped connection on reset"),
	_T("The connection has been broken due to keep-alive activity detecting a failure while the operation was in progress. It can also be returned by setsockopt if an attempt is made to set SO_KEEPALIVE on a connection that has already failed"),

	WSAECONNABORTED,
	_T("WSAECONNABORTED"),
	_T("Software caused connection abort"),
	_T("An established connection was aborted by the software in your host computer, possibly due to a data transmission time-out or protocol error"),

	WSAECONNRESET,
	_T("WSAECONNRESET"),
	_T("Connection reset by peer"),
	_T("An existing connection was forcibly closed by the remote host. This normally results if the peer application on the remote host is suddenly stopped, the host is rebooted, the host or remote network interface is disabled, or the remote host uses a hard close (see setsockopt for more information on the SO_LINGER option on the remote socket). This error may also result if a connection was broken due to keep-alive activity detecting a failure while one or more operations are in progress. Operations that were in progress fail with WSAENETRESET. Subsequent operations fail with WSAECONNRESET"),

	WSAENOBUFS,
	_T("WSAENOBUFS"),
	_T("No buffer space available"),
	_T("An operation on a socket could not be performed because the system lacked sufficient buffer space or because a queue was full"),

	WSAEISCONN,
	_T("WSAEISCONN"),
	_T("Socket is already connected"),
	_T("A connect request was made on an already-connected socket. Some implementations also return this error if sendto is called on a connected SOCK_DGRAM socket (for SOCK_STREAM sockets, the to parameter in sendto is ignored) although other implementations treat this as a legal occurrence"),

	WSAENOTCONN,
	_T("WSAENOTCONN"),
	_T("Socket is not connected"),
	_T("A request to send or receive data was disallowed because the socket is not connected and (when sending on a datagram socket using sendto) no address was supplied. Any other type of operation might also return this error - for example, setsockopt setting SO_KEEPALIVE if the connection has been reset"),

	WSAESHUTDOWN,
	_T("WSAESHUTDOWN"),
	_T("Cannot send after socket shutdown"),
	_T("A request to send or receive data was disallowed because the socket had already been shut down in that direction with a previous shutdown call. By calling shutdown a partial close of a socket is requested, which is a signal that sending or receiving, or both have been discontinued"),

	WSAETIMEDOUT,
	_T("WSAETIMEDOUT"),
	_T("Connection timed out"),
	_T("A connection attempt failed because the connected party did not properly respond after a period of time, or the established connection failed because the connected host has failed to respond"),

	WSAECONNREFUSED,
	_T("WSAECONNREFUSED"),
	_T("Connection refused"),
	_T("No connection could be made because the target computer actively refused it. This usually results from trying to connect to a service that is inactive on the foreign host - that is, one with no server application running"),

	WSAEHOSTDOWN,
	_T("WSAEHOSTDOWN"),
	_T("Host is down"),
	_T("A socket operation failed because the destination host is down. A socket operation encountered a dead host. Networking activity on the local host has not been initiated. These conditions are more likely to be indicated by the error WSAETIMEDOUT"),

	WSAEHOSTUNREACH,
	_T("WSAEHOSTUNREACH"),
	_T("No route to host"),
	_T("A socket operation was attempted to an unreachable host. See WSAENETUNREACH"),

	WSAEPROCLIM,
	_T("WSAEPROCLIM"),
	_T("Too many processes"),
	_T("A Windows Sockets implementation may have a limit on the number of applications that can use it simultaneously. WSAStartup may fail with this error if the limit has been reached"),

	WSASYSNOTREADY,
	_T("WSASYSNOTREADY"),
	_T("Network subsystem is unavailable"),
	_T("This error is returned by WSAStartup if the Windows Sockets implementation cannot function at this time because the underlying system it uses to provide network services is currently unavailable. Users should check: \r\n1. That the appropriate Windows Sockets DLL file is in the current path; \r\n2. That they are not trying to use more than one Windows Sockets implementation simultaneously. If there is more than one Winsock DLL on your system, be sure the first one in the path is appropriate for the network subsystem currently loaded; \r\n3. The Windows Sockets implementation documentation to be sure all necessary components are currently installed and configured correctly"),

	WSAVERNOTSUPPORTED,
	_T("WSAVERNOTSUPPORTED"),
	_T("Winsock.dll version out of range"),
	_T("The current Windows Sockets implementation does not support the Windows Sockets specification version requested by the application. Check that no old Windows Sockets DLL files are being accessed"),

	WSANOTINITIALISED,
	_T("WSANOTINITIALISED"),
	_T("Successful WSAStartup not yet performed"),
	_T("Either the application has not called WSAStartup or WSAStartup failed. The application may be accessing a socket that the current active task does not own (that is, trying to share a socket between tasks), or WSACleanup has been called too many times"),

	WSAEDISCON,
	_T("WSAEDISCON"),
	_T("Graceful shutdown in progress"),
	_T("Returned by WSARecv and WSARecvFrom to indicate that the remote party has initiated a graceful shutdown sequence"),

	WSATYPE_NOT_FOUND,
	_T("WSATYPE_NOT_FOUND"),
	_T("Class type not found"),
	_T("The specified class was not found"),

	WSAHOST_NOT_FOUND,
	_T("WSAHOST_NOT_FOUND"),
	_T("Host not found"),
	_T("No such host is known. The name is not an official host name or alias, or it cannot be found in the database(s) being queried. This error may also be returned for protocol and service queries, and means that the specified name could not be found in the relevant database"),

	WSATRY_AGAIN,
	_T("WSATRY_AGAIN"),
	_T("Nonauthoritative host not found"),
	_T("This is usually a temporary error during host name resolution and means that the local server did not receive a response from an authoritative server. A retry at some time later may be successful"),

	WSANO_RECOVERY,
	_T("WSANO_RECOVERY"),
	_T("This is a nonrecoverable error"),
	_T("This indicates that some sort of nonrecoverable error occurred during a database lookup. This may be because the database files (for example, BSD-compatible HOSTS, SERVICES, or PROTOCOLS files) could not be found, or a DNS request was returned by the server with a severe error"),

	WSANO_DATA,
	_T("WSANO_DATA"),
	_T("Valid name, no data record of requested type"),
	_T("The requested name is valid and was found in the database, but it does not have the correct associated data being resolved for. The usual example for this is a host name-to-address translation attempt (using gethostbyname or WSAAsyncGetHostByName) which uses the DNS (Domain Name Server). An MX record is returned but no A record - indicating the host itself exists, but is not directly reachable"),

	WSA_INVALID_HANDLE,
	_T("WSA_INVALID_HANDLE"),
	_T("Specified event object handle is invalid"),
	_T("An application attempts to use an event object, but the specified handle is not valid"),

	WSA_INVALID_PARAMETER,
	_T("WSA_INVALID_PARAMETER"),
	_T("One or more parameters are invalid"),
	_T("An application used a Windows Sockets function which directly maps to a Windows  function. The Windows function is indicating a problem with one or more parameters"),

	WSA_IO_INCOMPLETE,
	_T("WSA_IO_INCOMPLETE"),
	_T("Overlapped I/O event object not in signaled state"),
	_T("The application has tried to determine the status of an overlapped operation which is not yet completed. Applications that use WSAGetOverlappedResult (with the fWait flag set to FALSE) in a polling mode to determine when an overlapped operation has completed, get this error code until the operation is complete"),

	WSA_IO_PENDING,
	_T("WSA_IO_PENDING"),
	_T("Overlapped operations will complete later"),
	_T("The application has initiated an overlapped operation that cannot be completed immediately. A completion indication will be given later when the operation has been completed"),

	WSA_NOT_ENOUGH_MEMORY,
	_T("WSA_NOT_ENOUGH_MEMORY"),
	_T("Insufficient memory available"),
	_T("An application used a Windows Sockets function that directly maps to a Windows function. The Windows function is indicating a lack of required memory resources"),

	WSA_OPERATION_ABORTED,
	_T("WSA_OPERATION_ABORTED"),
	_T("Overlapped operation aborted"),
	_T("An overlapped operation was canceled due to the closure of the socket, or the execution of the SIO_FLUSH command in WSAIoctl"),

	WSAEINVALIDPROCTABLE,
	_T("WSAEINVALIDPROCTABLE"),
	_T("Invalid procedure table from service provider"),
	_T("A service provider returned a bogus procedure table to Ws2_32.dll. (This is usually caused by one or more of the function pointers being null.)"),

	WSAEINVALIDPROVIDER,
	_T("WSAEINVALIDPROVIDER"),
	_T("Invalid service provider version number"),
	_T("A service provider returned a version number other than 2.0"),

	WSAEPROVIDERFAILEDINIT,
	_T("WSAEPROVIDERFAILEDINIT"),
	_T("Unable to initialize a service provider"),
	_T("Either a service provider's DLL could not be loaded (LoadLibrary failed) or the provider's WSPStartup NSPStartup function failed"),

	WSASYSCALLFAILURE,
	_T("WSASYSCALLFAILURE"),
	_T("System call failure"),
	_T("Generic error code, returned under various conditions.Returned when a system call that should never fail does fail. For example, if a call to WaitForMultipleEvents fails or one of the registry functions fails trying to manipulate the protocol/namespace catalogs. Returned when a provider does not return SUCCESS and does not provide an extended error code. Can indicate a service provider implementation error"),
};

#define NUMERRORTABLEENTRIES (sizeof(error_table)/sizeof(error_table[0]))

#endif // XWSAERRORTABLE_H
