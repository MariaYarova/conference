#include <string>
#include <iostream>
#include <fstream>

#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Data/MySQL/MySQLException.h>
#include <Poco/Data/SessionFactory.h>

#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>



auto main() -> int
{
    std::string host("127.0.0.1");
    std::cout << "connecting to:" << host << std::endl;
    Poco::Data::MySQL::Connector::registerConnector();
    std::cout << "connector registered" << std::endl;

    std::string connection_str;
    connection_str = "host=";
    connection_str += host;
    connection_str += ";user=stud;db=stud;password=stud";

    std::cout << "connectiong to: " << connection_str << std::endl;
    Poco::Data::Session session(
        Poco::Data::SessionFactory::instance().create(
            Poco::Data::MySQL::Connector::KEY, connection_str));
    std::cout << "session 1 created" << std::endl;

    try
    {
        Poco::Data::Statement drop_t(session);
        drop_t << "DROP TABLE IF EXISTS `Article`;";
        drop_t.execute();
        std::cout << "table Article dropped" << std::endl;

        Poco::Data::Statement create_stmt(session);
        create_stmt << "CREATE TABLE IF NOT EXISTS `Article` ("
                    << "`id` INT NOT NULL AUTO_INCREMENT,"
                    << "`login` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL UNIQUE,"
                    << "`article_number_in_catalog` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NULL,"
                    << "`organization` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
                    << "`field_of_study` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
                    << "`article_dt` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
                    << "`acceptance_fg`  VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
                    << "`presentation_tm` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci,"
                    << "PRIMARY KEY (`id`));";
        create_stmt.execute();
        std::cout << "table Article created" << std::endl;

        Poco::Data::Statement truncate_stmt(session);
        truncate_stmt << "TRUNCATE TABLE `Article`;";
        truncate_stmt.execute();
        std::cout << "table Article TRUNCATED" << std::endl;

        // https://www.onlinedatagenerator.com/
        std::string json;
        std::ifstream is("./article_data.json");
        std::istream_iterator<char> eos;
        std::istream_iterator<char> iit(is);
        while (iit != eos)
            json.push_back(*(iit++));  
        is.close();

        Poco::JSON::Parser parser;
        // check that we read the json
        if (json.empty()) 
        {
            std::cout << "Empty json. Check that the file exists." << std::endl;
            return 1;
        }
        Poco::Dynamic::Var result = parser.parse(json);
        Poco::JSON::Array::Ptr arr = result.extract<Poco::JSON::Array::Ptr>();
        std::cout << "Parced" << std::endl;

        int i{0};
        for (i = 0; i < 109; ++i)
        {
            Poco::JSON::Object::Ptr object = arr->getObject(i);

            std::string article_number_in_catalog = object->getValue<std::string>("article_number_in_catalog");
            std::string organization = object->getValue<std::string>("organization");
            std::string field_of_study = object->getValue<std::string>("field_of_study");
            std::string article_dt = object->getValue<std::string>("article_dt");
            std::string acceptance_fg = object->getValue<std::string>("acceptance_fg");
            std::string presentation_tm = object->getValue<std::string>("presentation_time");
            std::string login = "login_" + std::to_string(i);

            Poco::Data::Statement insert(session);
            insert << "INSERT INTO Article (login, article_number_in_catalog, organization, field_of_study, article_dt, acceptance_fg, presentation_tm) VALUES(?, ?, ?, ?, ?, ?, ?)",
                Poco::Data::Keywords::use(login),
                Poco::Data::Keywords::use(article_number_in_catalog),
                Poco::Data::Keywords::use(organization),
                Poco::Data::Keywords::use(field_of_study),
                Poco::Data::Keywords::use(article_dt),
                Poco::Data::Keywords::use(acceptance_fg),
                Poco::Data::Keywords::use(presentation_tm);

            insert.execute();
        }       
        std::cout << "Inserted " << i << " records." << std::endl;
    }
    catch (Poco::Data::MySQL::ConnectionException &e)
    {
        std::cout << "connection: " << e.what() << std::endl;
    }
    catch (Poco::Data::MySQL::StatementException &e)
    {
        std::cout << "statement: " << e.what() << std::endl;
    }
    return 0;
}