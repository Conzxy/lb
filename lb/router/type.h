#ifndef LB_ROUTER_TYPE_H__
#define LB_ROUTER_TYPE_H__

#include <memory>

namespace lb {

class BackendSession;

using BackendSessionSPtr = std::shared_ptr<BackendSession>;
using BackendSessionWPtr = std::weak_ptr<BackendSession>;

} // namespace lb

#endif // LB_ROUTER_TYPE_H__
