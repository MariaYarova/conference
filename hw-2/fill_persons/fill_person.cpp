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



auto main(int argc, char *argv[]) -> int
{
    if (argc < 2) 
    {
        std::cout << "Enter ip addres as a first argument." << std::endl;
        return 1;
    }

    std::string host(argv[1]);
    std::cout << "connecting to:" << host << std::endl;
    Poco::Data::MySQL::Connector::registerConnector();
    std::cout << "connector registered" << std::endl;

    std::string connection_str;
    connection_str = "host=";
    connection_str += host;
    connection_str += ";user=test;db=sql_test;password=pzjqUkMnc7vfNHET;port=6033";

    std::cout << "connectiong to: " << connection_str << std::endl;
    Poco::Data::Session session(
        Poco::Data::SessionFactory::instance().create(
            Poco::Data::MySQL::Connector::KEY, connection_str));
    std::cout << "session created" << std::endl;
    try
    {
        // shard 0
        Poco::Data::Statement create_stmt_0(session);
        create_stmt_0 << "CREATE TABLE IF NOT EXISTS `Author` ("
                    << "`login` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL UNIQUE,"
                    << "`first_name` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
                    << "`last_name` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
                    << "`email` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NULL,"
                    << "PRIMARY KEY (`login`))-- sharding:0;";
        create_stmt_0.execute();
        std::cout << "table for shard 0 created" << std::endl;

        Poco::Data::Statement truncate_stmt_0(session);
        truncate_stmt_0 << "TRUNCATE TABLE `Author`-- sharding:0;";
        truncate_stmt_0.execute();

        // shard 1
        Poco::Data::Statement create_stmt(session);
        create_stmt << "CREATE TABLE IF NOT EXISTS `Author` ("
                    << "`login` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL UNIQUE,"
                    << "`first_name` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
                    << "`last_name` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
                    << "`email` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NULL,"
                    << "PRIMARY KEY (`login`))-- sharding:1;";
        create_stmt.execute();
        std::cout << "table for shard 1 created" << std::endl;

        Poco::Data::Statement truncate_stmt_1(session);
        truncate_stmt_1 << "TRUNCATE TABLE `Author`-- sharding:1;";
        truncate_stmt_1.execute();

        // https://www.onlinedatagenerator.com/
        std::string json;
        std::ifstream is("./person_data.json");
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

        for (int i = 0; i < 100; ++i)
        {
            Poco::JSON::Object::Ptr object = arr->getObject(i);

            std::string first_name = object->getValue<std::string>("first_name");
            std::string last_name = object->getValue<std::string>("last_name");
            std::string email = object->getValue<std::string>("email");
            std::string login = "login_" + std::to_string(i);

            Poco::Data::Statement insert(session);

            size_t shard_number = std::hash<std::string>{}(login) % 2;
            insert << "INSERT INTO Author (login,first_name,last_name,email) VALUES(?, ?, ?, ?)-- sharding:" + std::to_string(shard_number),
                Poco::Data::Keywords::use(login),
                Poco::Data::Keywords::use(first_name),
                Poco::Data::Keywords::use(last_name),
                Poco::Data::Keywords::use(email);

            insert.execute();
        }       
    }
    catch (Poco::Data::MySQL::ConnectionException &e)
    {
        std::cout << "connection:" << e.what() << std::endl;
    }
    catch (Poco::Data::MySQL::StatementException &e)
    {
        std::cout << "statement:" << e.what() << std::endl;
    }
    return 0;
}