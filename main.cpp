#include <iostream>
#include <unordered_map>
#include <sstream>
#include "json11.hpp"
#include <dcm/interprocess/listener_factory.hpp>
#include <dcm/socket/dcm_socket_server.hpp>
using namespace std;

std::string http_headers(){
    return  "HTTP/1.1 200 OK\n"
            "Server: DCM/0.1\n"
            "Content-Type: text/event-stream\n"
            "Expires: Fri, 01 Jan 1990 00:00:00 GMT\n"
            "Cache-Control: no-cache, no-store, max-age=0, must-revalidate\n"
            "Pragma: no-cache\n"
            "Access-Control-Allow-Origin: *\n"
            "Access-Control-Expose-Headers: *\n"
            "Access-Control-Allow-Credentials: true\n"
            "Connection: close\n\n";
}

std::string event_string(std::unordered_map<std::string, std::string> &_evt){
    std::stringstream s;
    if (_evt.count("event")){
        s << "\nevent: " << _evt.at("event") << "\n";
        _evt.erase("event");
    }
    for (auto &rec: _evt) {
        s << rec.first << ": " << rec.second << "\n";
    }
    s << "\n\n";
    return s.str();
};

std::string comment(const std::string &_cmt){
    return ": "+_cmt+"\n\n";
}

int main(int argc, char** argv) {

    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <sse_port> <unix_socket_path>" << std::endl;
        return 0;
    }

    auto sse_server = interproc::make_listener(interproc::conn_type::tcp, "0.0.0.0:"+std::string(argv[1]));
    sse_server->on_connect = [](std::shared_ptr<interproc::session<interproc::buffer>> _session){
        // TODO: read session ID
        _session->send(interproc::to_buffer(http_headers()));
        _session->send(interproc::to_buffer("retry: 1000\n\n"));
        _session->send(interproc::to_buffer("event: ready\ndata:\n\n"));
    };

    sse_server->start();

    auto dcm_server = dcm::streamsocket::make_receiver(interproc::conn_type::unix, argv[2]);
    dcm_server->on_message = [&sse_server](const dcm::signal &_message) {
        //std::cout << "caught " << _message.header.at("signal") << " signal with data: "<< _message.body.at("data") << std::endl;
        std::unordered_map<std::string, std::string> signal;
        json11::Json::object json;
        for (auto &k: _message.data()){
            json[k.first] = k.second;
        }
        signal["event"] = _message.name();
        signal["data"] = json11::Json(json).dump();
        std::cout << "retransmitting " << signal["event"] << std::endl;
        sse_server->broadcast(interproc::to_buffer(event_string(signal)));
    };
    dcm_server->start();

    dcm_server->wait_until_stopped();
    sse_server->wait_until_stopped();

    return 0;
}
