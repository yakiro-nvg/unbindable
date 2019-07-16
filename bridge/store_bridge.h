#pragma once

#include <functional>

#include <json.hpp>
using json = nlohmann::json;

class StoreBridgeBase
{
private:
  enum { REQUEST_BUFF_SZ = 2048 };
  enum { IO_BUFF_SZ = 8192 };
  
public:
  StoreBridgeBase();
  virtual ~StoreBridgeBase();
  
  virtual bool start(
    const char *address
  , unsigned short listen_port
  , int max_connections
  );
  
  virtual void
  update(
    void
  );
  
  virtual void
  stop(
    void
  );
  
protected:
  virtual void
  state(
    json &j
  ) const = 0;
  
  virtual void
  dispatch_action(
    const json &j
  ) = 0;
  
protected:
  virtual int
  handle_actions(
    struct wby_con *con
  );
  
  virtual void
  push_state(
    void
  );
  
  static int
  dispatch(
    struct wby_con *con
  , void *ud
  );
  
  static int
  websocket_connect(
    struct wby_con *con
  , void *ud
  );
  
  static void
  websocket_connected(
    struct wby_con *con
  , void *ud
  );
  
  static int
  websocket_frame(
    struct wby_con *con
  , const struct wby_frame *frame
  , void *ud
  );
  
  static void
  websocket_closed(
    struct wby_con *con
  , void *ud
  );
  
  struct wby_server *_wby;
  void *_wby_buff;
};

template <typename TStore>
class StoreBridge : public StoreBridgeBase
{
  typedef typename TStore::Action TAction;
  typedef typename TStore::State TState;
  typedef typename TStore::OptionalState TOptionalState;
  
public:
  StoreBridge(TStore &store)
    : _store(store)
  {
    // nop
  }
  
  virtual bool start(
    const char *address
  , int listen_port
  , int max_connections)
  {
    if (StoreBridgeBase::start(address, listen_port, max_connections)) {
      _unsubscribe = _store.subscribe(
        [this](const TOptionalState &) { push_state(); });
      return true;
    } else {
      return false;
    }
  }
  
  virtual void
  stop(
    void)
  {
    if (_unsubscribe) {
      _unsubscribe();
      _unsubscribe = nullptr;
    }
    StoreBridgeBase::stop();
  }
  
protected:
  virtual void
  state(
    json &j) const
  {
    j = _store;
  }
  
  virtual void
  dispatch_action(
    const json &j)
  {
    TAction a = j;
    _store.dispatch(a);
  }
  
private:
  TStore &_store;
  std::function<void ()> _unsubscribe;
};
