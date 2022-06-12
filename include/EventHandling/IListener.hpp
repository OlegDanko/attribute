#pragma once

struct IEventListener {
  virtual ~IEventListener() = default;
  virtual void serve_events() = 0;
};
