#include "store_bridge.h"

#include <stddef.h>
#define WBY_UINT_PTR size_t
#include "wby.h"

#include <stdlib.h>

#define WBY_CON_NONE ((void*)0)
#define WBY_CON_PUSH ((void*)1)

static const struct wby_header CORS_HEADERS[] = {
  { "Access-Control-Allow-Origin",
    "*"
  },
  { "Access-Control-Allow-Methods",
    "GET, POST, PUT, PATCH, DELETE, OPTIONS"
  },
  { "Access-Control-Allow-Headers",
    "Connection, Content-Type"
  },
  { "Access-Control-Max-Age",
    "600"
  }
};

#define STATIC_ARRAY_COUNT(arr) (sizeof(arr)/sizeof(arr[0]))

static int
simple_response(
  wby_con *con
, int status)
{
  wby_response_begin(
    con, status, 0,
    CORS_HEADERS, STATIC_ARRAY_COUNT(CORS_HEADERS));
  wby_response_end(con);
  return 0;
}

static void
push(
  wby_server *wby
, const void *msg
, int sz)
{
  for (wby_size i = 0; i < wby->con_count; ++i) {
    wby_con *con = wby_conn(wby, i);
    if (con->user_data == WBY_CON_PUSH) {
      wby_frame_begin(con, WBY_WSOP_TEXT_FRAME);
      wby_write(con, msg, (wby_size)sz);
      wby_frame_end(con);
    }
  }
}

StoreBridgeBase::StoreBridgeBase()
  : _wby(new wby_server())
  , _wby_buff(nullptr)
{
  // nop
}

/*virtual*/ StoreBridgeBase::~StoreBridgeBase()
{
  stop();
  delete _wby;
  _wby = nullptr;
}

/*virtual*/ bool
StoreBridgeBase::start(
  const char *address
, unsigned short listen_port
, int max_connection)
{
  struct wby_config cfg;
  memset(&cfg, 0, sizeof(cfg));
  cfg.userdata = this;
  cfg.address = address;
  cfg.port = listen_port;
  cfg.connection_max = (unsigned int)max_connection;
  cfg.request_buffer_size = REQUEST_BUFF_SZ;
  cfg.io_buffer_size = IO_BUFF_SZ;
  cfg.dispatch     = &StoreBridgeBase::dispatch;
  cfg.ws_connect   = &StoreBridgeBase::websocket_connect;
  cfg.ws_connected = &StoreBridgeBase::websocket_connected;
  cfg.ws_frame     = &StoreBridgeBase::websocket_frame;
  cfg.ws_closed    = &StoreBridgeBase::websocket_closed;
  wby_size wby_buff_sz;
  wby_init(_wby, &cfg, &wby_buff_sz);
  _wby_buff = malloc(wby_buff_sz);
  if (_wby_buff == nullptr) goto failed;
  if (wby_start(_wby, _wby_buff) != 0) goto failed;
  return true;
failed:
  if (_wby_buff) {
    free(_wby_buff);
    _wby_buff = nullptr;
  }
  return false;
}

/*virtual*/ void
StoreBridgeBase::update(
  void)
{
  wby_update(_wby, false);
}

/*virtual*/ void
StoreBridgeBase::stop(
  void)
{
  if (!_wby_buff) return;
  wby_stop(_wby);
  free(_wby_buff);
  _wby_buff = nullptr;
  
}

/*virtual*/ int
StoreBridgeBase::handle_actions(
  struct wby_con *con)
{
  if (strcmp(con->request.method, "POST") == 0) {
    const char *content_type = wby_find_header(con, "Content-Type");
    if (!content_type || strcmp(content_type, "application/json") != 0) {
      return simple_response(con, 415);
    }
    if (con->request.content_length == 0) {
      return simple_response(con, 400);
    }
    const int csz = con->request.content_length;
    char *content = (char*)malloc(csz + 1);
    if (content == nullptr) {
      return simple_response(con, 500);
    }
    wby_read(con, content, (wby_size)csz);
    content[csz] = '\0';
    // TODO: exception handling?
    json j = json::parse(content, nullptr, false);
    dispatch_action(j);
    free(content);
    return simple_response(con, 204);
  }
  return simple_response(con, 405);
}

/*virtual*/ void
StoreBridgeBase::push_state(
  void)
{
  json j;
  state(j);
  auto s = j.dump();
  push(_wby, s.c_str(), (int)s.size());
}

/*static*/ int
StoreBridgeBase::dispatch(
  struct wby_con *con
, void *ud)
{
  if (strcmp(con->request.method, "OPTIONS") == 0) {
    return simple_response(con, 200);
  } else {
    auto self = (StoreBridgeBase*)ud;
    const char *uri = con->request.uri;
    if (strcmp(uri, "/actions") == 0) {
      return self->handle_actions(con);
    }
    return 1;
  }
  return 0;
}

/*static*/ int
StoreBridgeBase::websocket_connect(
  struct wby_con *con
, void *ud)
{
  return strcmp(con->request.uri, "/statestream");
}

/*static*/ void
StoreBridgeBase::websocket_connected(
  struct wby_con *con
, void *ud)
{
  auto self = (StoreBridgeBase*)ud;
  con->user_data = WBY_CON_PUSH;
  self->push_state();
}

/*static*/ int
StoreBridgeBase::websocket_frame(
  struct wby_con *con
, const struct wby_frame *
, void *)
{
  // TODO: ?
  return 0;
}

/*static*/ void
StoreBridgeBase::websocket_closed(
  struct wby_con *
, void *)
{
  // nop
}
