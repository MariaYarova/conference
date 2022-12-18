#ifndef ARTICLEHANDLER_H
#define ARTICLEHANDLER_H

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

#include "../../database/article.h"

class ArticleHandler : public HTTPRequestHandler
{
public:
    ArticleHandler(const std::string &format) : _format(format)
    {
    }

    void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response)
    {
        HTMLForm form(request, request.stream());
        if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) {
            if (form.has("in_conf")) {
                int in_conf = stoi(form.get("in_conf"));
                try
                {
                    std::vector<database::Article> result;
                    if (in_conf) {
                        result = database::Article::read_all_on_conf();
                    } else {
                        result = database::Article::read_all();
                    }

                    Poco::JSON::Array arr;
                    for (auto s : result)
                        arr.add(s.toJSON());

                    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                    response.setChunkedTransferEncoding(true);
                    response.setContentType("application/json");
                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(arr, ostr);
                    response.send();
                }
                catch (...)
                {
                    response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
                    std::ostream &ostr = response.send();
                    ostr << "{ \"result\": false , \"reason\": \"not found\" }";
                    response.send();
                }
                return;
            }
            else if ([](const std::string &str,const std::string & prefix) {
                        return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
                    } (request.getURI(),"/accept_article"))
            {
                try
                {
                    long id = stoi(form.get("id"));
                    std::string pres_time = database::Article::accept_article(id);

                    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                    response.setChunkedTransferEncoding(true);
                    response.setContentType("application/json");
                    std::ostream &ostr = response.send();
                    ostr << "Set presentation time: " + pres_time + "\n";
                    response.send();
                    return;
                }
                catch (...)
                {
                    response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
                    std::ostream &ostr = response.send();
                    ostr << "{ \"result\": false , \"reason\": \"the article is already accepted\" }";
                    response.send();
                    return;
                }
                return;
            }
        }
        else if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST)
        {
            if (form.has("login") && form.has("article_number_in_catalog") &&
                form.has("organization") && form.has("field_of_study") && form.has("article_dt"))
            {
                database::Article article;
                article.login() = form.get("login");
                article.article_number_in_catalog() = form.get("article_number_in_catalog");
                article.organization() = form.get("organization");
                article.field_of_study() = form.get("field_of_study");
                article.article_dt() = form.get("article_dt");
                article.acceptance_fg() = "0";
                article.presentation_tm() = "";

                try
                {
                    article.save_to_mysql();
                    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                    response.setChunkedTransferEncoding(true);
                    response.setContentType("application/json");
                    std::ostream &ostr = response.send();
                    ostr << "Article added.\n";
                    return;
                }
                catch (...)
                {
                    response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
                    std::ostream &ostr = response.send();
                    ostr << "database error";
                    response.send();
                    return;
                }
                return;
            }
        }
        response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
        std::ostream &ostr = response.send();
        ostr << "Article request error\n";
        response.send();
    }

private:
    std::string _format;
};
#endif // !ARTICLEHANDLER_H