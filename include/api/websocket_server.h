#ifndef ALLPRESS_WEBSOCKET_SERVER_H
#define ALLPRESS_WEBSOCKET_SERVER_H

#include <string>

namespace AllPress {
namespace API {

void init_websocket_server(int port);
void stop_websocket_server();
void broadcast_websocket_message(const std::string& message);

} // namespace API
} // namespace AllPress

#endif // ALLPRESS_WEBSOCKET_SERVER_H

