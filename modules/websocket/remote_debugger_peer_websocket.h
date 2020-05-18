/*************************************************************************/
/*  script_debugger_websocket.h                                          */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2019 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2019 Godot Engine contributors (cf. AUTHORS.md)    */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef SCRIPT_DEBUGGER_WEBSOCKET_H
#define SCRIPT_DEBUGGER_WEBSOCKET_H

#ifdef JAVASCRIPT_ENABLED
#include "modules/websocket/emws_client.h"
#else
#include "modules/websocket/wsl_client.h"
#endif
#include "core/debugger/remote_debugger_peer.h"

class RemoteDebuggerPeerWebSocket : public RemoteDebuggerPeer {

	Ref<WebSocketClient> ws_client;
	Ref<WebSocketPeer> ws_peer;
	List<Array> in_queue;
	List<Array> out_queue;

	int max_queued_messages;

public:
	static RemoteDebuggerPeer *create(const String &p_uri);

	Error connect_to_host(const String &p_uri);
	bool is_peer_connected();
	int get_max_message_size() const;
	bool has_message();
	Error put_message(const Array &p_arr);
	Array get_message();
	void close();
	void poll();
	bool can_block() const;

	RemoteDebuggerPeerWebSocket(Ref<WebSocketPeer> p_peer = Ref<WebSocketPeer>());
};

#endif // SCRIPT_DEBUGGER_WEBSOCKET_H
