/*
ESP8266 web server - platform-dependent routines, nonos version
*/

#include <esp8266.h>
#include "httpd.h"
#include "platform.h"
#include "httpd-platform.h"

#ifndef FREERTOS

//Listening connection data
static struct espconn httpdConn;
static struct espconn httpdSSLConn;
static esp_tcp httpdTcp;
static esp_tcp httpdSSLTcp;

//Set/clear global httpd lock.
//Not needed on nonoos.
void ICACHE_FLASH_ATTR httpdPlatLock() {
}
void ICACHE_FLASH_ATTR httpdPlatUnlock() {
}


static void ICACHE_FLASH_ATTR platReconCb(void *arg, sint8 err) {
	//From ESP8266 SDK
	//If still no response, considers it as TCP connection broke, goes into espconn_reconnect_callback.

	ConnTypePtr conn=arg;
	//Just call disconnect to clean up pool and close connection.
	httpdDisconCb(conn, (char*)conn->proto.tcp->remote_ip, conn->proto.tcp->remote_port);
}

static void ICACHE_FLASH_ATTR platDisconCb(void *arg) {
	ConnTypePtr conn=arg;
	httpdDisconCb(conn, (char*)conn->proto.tcp->remote_ip, conn->proto.tcp->remote_port);
}

static void ICACHE_FLASH_ATTR platRecvCb(void *arg, char *data, unsigned short len) {
	ConnTypePtr conn=arg;
	httpdRecvCb(conn, (char*)conn->proto.tcp->remote_ip, conn->proto.tcp->remote_port, data, len);
}

static void ICACHE_FLASH_ATTR platSentCb(void *arg) {
	ConnTypePtr conn=arg;
	httpdSentCb(conn, (char*)conn->proto.tcp->remote_ip, conn->proto.tcp->remote_port);
}

static void ICACHE_FLASH_ATTR platConnCb(void *arg) {
	ConnTypePtr conn=arg;
	if (httpdConnectCb(conn, (char*)conn->proto.tcp->remote_ip, conn->proto.tcp->remote_port)) {
		espconn_regist_recvcb(conn, platRecvCb);
		espconn_regist_reconcb(conn, platReconCb);
		espconn_regist_disconcb(conn, platDisconCb);
		espconn_regist_sentcb(conn, platSentCb);
	} else {
		espconn_disconnect(conn);
	}
}

static void ICACHE_FLASH_ATTR platReconSSLCb(void *arg, sint8 err) {
    //From ESP8266 SDK
    //If still no response, considers it as TCP connection broke, goes into espconn_reconnect_callback.
    
    ConnTypePtr conn=arg;
    //Just call disconnect to clean up pool and close connection.
    httpdDisconCb(conn, (char*)conn->proto.tcp->remote_ip, conn->proto.tcp->remote_port);
}

static void ICACHE_FLASH_ATTR platDisconSSLCb(void *arg) {
    ConnTypePtr conn=arg;
    httpdDisconCb(conn, (char*)conn->proto.tcp->remote_ip, conn->proto.tcp->remote_port);
}

static void ICACHE_FLASH_ATTR platRecvSSLCb(void *arg, char *data, unsigned short len) {
    ConnTypePtr conn=arg;
    httpdRecvCb(conn, (char*)conn->proto.tcp->remote_ip, conn->proto.tcp->remote_port, data, len);
}

static void ICACHE_FLASH_ATTR platSentSSLCb(void *arg) {
    ConnTypePtr conn=arg;
    httpdSentCb(conn, (char*)conn->proto.tcp->remote_ip, conn->proto.tcp->remote_port);
}

static void ICACHE_FLASH_ATTR platConnSSLCb(void *arg) {
    ConnTypePtr conn=arg;
    if (httpdConnectCb(conn, (char*)conn->proto.tcp->remote_ip, conn->proto.tcp->remote_port)) {
        espconn_regist_recvcb(conn, platRecvSSLCb);
        espconn_regist_reconcb(conn, platReconSSLCb);
        espconn_regist_disconcb(conn, platDisconSSLCb);
        espconn_regist_sentcb(conn, platSentSSLCb);
    } else {
        espconn_secure_disconnect(conn);
    }
}


int ICACHE_FLASH_ATTR httpdPlatSendData(ConnTypePtr conn, char *buff, int len) {
	int r;
    if (conn == &httpdSSLConn) {
        r=espconn_secure_send(conn, (uint8_t*)buff, len);
    } else {
        r=espconn_sent(conn, (uint8_t*)buff, len);
    }
    return (r>=0);
}

void ICACHE_FLASH_ATTR httpdPlatDisconnect(ConnTypePtr conn) {
    if (conn == &httpdSSLConn) {
        espconn_secure_disconnect(conn);
    } else {
        espconn_disconnect(conn);
    }
}

void ICACHE_FLASH_ATTR httpdPlatDisableTimeout(ConnTypePtr conn) {
	//Can't disable timeout; set to 2 hours instead.
	espconn_regist_time(conn, 7199, 1);
}

//Initialize listening socket, do general initialization
void ICACHE_FLASH_ATTR httpdPlatInit(int port, int maxConnCt) {
	httpdConn.type=ESPCONN_TCP;
	httpdConn.state=ESPCONN_NONE;
	httpdTcp.local_port=port;
	httpdConn.proto.tcp=&httpdTcp;
	espconn_regist_connectcb(&httpdConn, platConnCb);
    espconn_accept(&httpdConn);
    espconn_tcp_set_max_con_allow(&httpdConn, maxConnCt);
    
}
//Initialize listening socket, do general initialization
void ICACHE_FLASH_ATTR httpdPlatSSLInit(int port, int maxConnCt) {
    httpdSSLConn.type=ESPCONN_TCP;
    httpdSSLConn.state=ESPCONN_NONE;
    httpdSSLTcp.local_port=port;
    httpdSSLConn.proto.tcp=&httpdSSLTcp;
    espconn_regist_connectcb(&httpdSSLConn, platConnCb);
    //espconn_secure_set_default_certificate(default_certificate, default_certificate_len);
    //espconn_secure_set_default_private_key(default_private_key, default_private_key_len);
    espconn_secure_accept(&httpdSSLConn);
    espconn_tcp_set_max_con_allow(&httpdSSLConn, maxConnCt);
    
}


#endif