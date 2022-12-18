#ifndef AUTHORHANDLER_H
#define AUTHORHANDLER_H

#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Timestamp.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/Exception.h"
#include "Poco/ThreadPool.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include <iostream>
#include <iostream>
#include <fstream>

using Poco::DateTimeFormat;
using Poco::DateTimeFormatter;
using Poco::ThreadPool;
using Poco::Timestamp;
using Poco::Net::HTMLForm;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerParams;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::NameValueCollection;
using Poco::Net::ServerSocket;
using Poco::Util::Application;
using Poco::Util::HelpFormatter;
using Poco::Util::Option;
using Poco::Util::OptionCallback;
using Poco::Util::OptionSet;
using Poco::Util::ServerApplication;

#include "../../database/author.h"

class AuthorHandler : public HTTPRequestHandler
{
private:
    bool check_name(const std::string &name, std::string &reason)
    {
        if (name.length() < 3)
        {
            reason = "Name must be at leas 3 signs";
            return false;
        }

        if (name.find(' ') != std::string::npos)
        {
            reason = "Name can't contain spaces";
            return false;
        }

        if (name.find('\t') != std::string::npos)
        {
            reason = "Name can't contain spaces";
            return false;
        }

        return true;
    };

    bool check_email(const std::string &email, std::string &reason)
    {
        if (email.find('@') == std::string::npos)
        {
            reason = "Email must contain @";
            return false;
        }

        if (email.find(' ') != std::string::npos)
        {
            reason = "EMail can't contain spaces";
            return false;
        }

        if (email.find('\t') != std::string::npos)
        {
            reason = "EMail can't contain spaces";
            return false;
        }

        return true;
    };

public:
    AuthorHandler(const std::string &format) : _format(format)
    {
    }

    void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response)
    {
        HTMLForm form(request, request.stream());
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        std::ostream &ostr = response.send();

        if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET)
        {
            if (form.has("login"))
            {
                std::string login = form.get("login");

                // try to read from cache
                std::optional<database::Author> result = database::Author::read_from_cache_by_login(login);
                if (result) {
                    Poco::JSON::Stringifier::stringify(result->toJSON(), ostr);
                    std::cout << "cached taken" << std::endl;
                    return;
                }

                try
                {
                    database::Author result = database::Author::read_by_login(login);
                    result.save_to_cache();  // save to cache for non-cached items
                    Poco::JSON::Stringifier::stringify(result.toJSON(), ostr);
                    std::cout << "non-cached taken" << std::endl;
                    return;
                }
                catch (std::exception &ex)
                {
                    ostr << "{ \"result\": false , \"reason\": \"" << ex.what() << "\" }";
                    return;
                }
                catch (...)
                {
                    ostr << "{ \"result\": false , \"reason\": \"not found\" }";
                    response.send();
                    return;
                }
            }
            else if ([](const std::string &str,const std::string & prefix){
                        return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);}
                        (request.getURI(),"/search"))
            {
                try
                {
                    std::string fn = form.get("first_name");
                    std::string ln = form.get("last_name");
                    auto results = database::Author::search(fn, ln);
                    Poco::JSON::Array arr;
                    for (auto s : results)
                        arr.add(s.toJSON());
                    Poco::JSON::Stringifier::stringify(arr, ostr);
                }
                catch (...)
                {
                    ostr << "{ \"result\": false , \"reason\": \"not found\" }";
                    response.send();
                    return;
                }
                return;
            }
        }
        else if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST)
        {
            if (form.has("first_name"))
                if (form.has("last_name"))
                    if (form.has("email"))
                        if (form.has("login"))
                        {
                            database::Author author;
                            author.first_name() = form.get("first_name");
                            author.last_name() = form.get("last_name");
                            author.email() = form.get("email");
                            author.login() = form.get("login");

                            bool check_result = true;
                            std::string message;
                            std::string reason;

                            if (!check_name(author.get_first_name(), reason))
                            {
                                check_result = false;
                                message += reason;
                                message += "<br>";
                            }

                            if (!check_name(author.get_last_name(), reason))
                            {
                                check_result = false;
                                message += reason;
                                message += "<br>";
                            }

                            if (!check_email(author.get_email(), reason))
                            {
                                check_result = false;
                                message += reason;
                                message += "<br>";
                            }
                            if (check_result)
                            {
                                try
                                {
                                    author.save_to_mysql();
                                    author.save_to_cache();
                                    ostr << "Inserted";
                                    return;
                                }
                                catch (...)
                                {
                                    ostr << "database error";
                                    response.send();
                                    return;
                                }
                            }
                            else
                            {
                                ostr << message;
                                response.send();
                                return;
                            }
                        }
        }
        ostr << "Author request error";
        response.send();
    }

private:
    std::string _format;
};
#endif // !AUTHORHANDLER_H