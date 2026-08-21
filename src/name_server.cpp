/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "name_server.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#endif

#include <sstream>
#include <stdexcept>
#include <cstring>
#include <vector>

using namespace std;

namespace __node_rocketmq__ {

static string ResolveHost(const string& host)
{
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo* result = NULL;
    int error = getaddrinfo(host.c_str(), NULL, &hints, &result);
    if(error != 0 || result == NULL)
    {
        if(result != NULL) freeaddrinfo(result);
        throw runtime_error("unable to resolve name server host: " + host);
    }

    char address[INET_ADDRSTRLEN];
    struct sockaddr_in* socket_address = reinterpret_cast<struct sockaddr_in*>(result->ai_addr);
    const char* converted = inet_ntop(AF_INET, &socket_address->sin_addr, address, sizeof(address));
    freeaddrinfo(result);
    if(converted == NULL)
    {
        throw runtime_error("unable to resolve name server host: " + host);
    }
    return address;
}

std::string ResolveNameServerAddress(const std::string& addresses)
{
    stringstream input(addresses);
    string address;
    vector<string> resolved;
    while(getline(input, address, ';'))
    {
        size_t separator = address.find(':');
        if(separator == string::npos || separator == 0 || separator == address.length() - 1)
        {
            throw runtime_error("nameServer must use host:port format");
        }
        string host = address.substr(0, separator);
        string port = address.substr(separator + 1);
        resolved.push_back(ResolveHost(host) + ":" + port);
    }

    stringstream output;
    for(size_t i = 0; i < resolved.size(); i++)
    {
        if(i > 0) output << ";";
        output << resolved[i];
    }
    return output.str();
}

}
