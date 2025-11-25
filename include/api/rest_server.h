#ifndef ALLPRESS_REST_SERVER_H
#define ALLPRESS_REST_SERVER_H

#include <string>
#include <functional>
#include <memory>
#include "../core/printer_manager.h"
#include "../core/job_queue.h"

namespace AllPress {
namespace API {

class RESTServer {
public:
    RESTServer(int port, PrinterManager* printer_mgr, JobQueue* job_queue);
    ~RESTServer();
    
    void start();
    void stop();
    bool is_running() const;
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace API
} // namespace AllPress

#endif // ALLPRESS_REST_SERVER_H
