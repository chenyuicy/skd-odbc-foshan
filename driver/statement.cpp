#include "statement.h"
#include "escaping/escape_sequences.h"

#include <string>
#include <Poco/Base64Encoder.h>
#include <Poco/Exception.h>
#include <Poco/Net/HTTPRequest.h>

Statement::Statement(Connection & conn_) : connection(conn_), metadata_id(conn_.environment.metadata_id) {
    ard.reset(new DescriptorClass);
    apd.reset(new DescriptorClass);
    ird.reset(new DescriptorClass);
    ipd.reset(new DescriptorClass);
}

bool Statement::getScanEscapeSequences() const {
    return scan_escape_sequences;
}

void Statement::setScanEscapeSequences(bool value) {
    scan_escape_sequences = value;
}

SQLUINTEGER Statement::getMetadataId() const {
    return metadata_id;
}

void Statement::setMetadataId(SQLUINTEGER id) {
    metadata_id = id;
}

const std::string Statement::getQuery() const {
    return query;
}

const TypeInfo & Statement::getTypeInfo(const std::string & type_name) const {
    return connection.environment.types_info.at(type_name);
}

bool Statement::isEmpty() const {
    return query.empty();
}

bool Statement::isPrepared() const {
    return prepared;
}

void Statement::sendRequest(IResultMutatorPtr mutator) {
    std::ostringstream user_password_base64;
    Poco::Base64Encoder base64_encoder(user_password_base64);
    base64_encoder << connection.user << ":" << connection.password;
    base64_encoder.close();

    Poco::Net::HTTPRequest request;

    request.setMethod(Poco::Net::HTTPRequest::HTTP_POST);
    request.setVersion(Poco::Net::HTTPRequest::HTTP_1_1);
    request.setKeepAlive(true);
    request.setChunkedTransferEncoding(true);
    request.setCredentials("Basic", user_password_base64.str());
    //request.setURI(
    //    "/?database=" + connection.getDatabase() + "&default_format=ODBCDriver"); /// TODO Ability to transfer settings. TODO escaping
	
	//rox
	request.setURI("/?database=" + connection.getDatabase() + "&default_format=ODBCDriver&result_overflow_mode=throw&max_result_rows=500000000&timeout_overflow_mode=throw&max_execution_time=" + std::to_string(connection.timeout) );
	//request.setURI("/?database=" + connection.getDatabase() + "&default_format=ODBCDriver&result_overflow_mode=throw&max_result_rows=500000000&timeout_overflow_mode=throw&max_execution_time=300" );

    if (in && in->peek() != EOF)
        connection.session->reset();
    // Send request to server with finite count of retries.
    for (int i = 1;; ++i) {
        try {
            connection.session->sendRequest(request) << prepared_query;
            response = std::make_unique<Poco::Net::HTTPResponse>();
            in = &connection.session->receiveResponse(*response);
            break;
        } catch (const Poco::IOException & e) {
            connection.session->reset(); // reset keepalived connection

            LOG("Http request try=" << i << "/" << connection.retry_count << " failed: " << e.what() <<": "<< e.message());
            if (i > connection.retry_count) {
                throw;
            }
        }
    }

    Poco::Net::HTTPResponse::HTTPStatus status = response->getStatus();

    if (status != Poco::Net::HTTPResponse::HTTP_OK) {
        std::stringstream error_message;
        error_message << "HTTP status code: " << status << std::endl << "Received error:" << std::endl << in->rdbuf() << std::endl;
        LOG(error_message.str());
        throw std::runtime_error(error_message.str());
    }

    result.init(this, std::move(mutator));
}

bool Statement::fetchRow() {
    current_row = result.fetch();
    return current_row.isValid();
}

void Statement::prepareQuery(const std::string & q) {
	//LOGQUERY("\r\nbeforeReplace::\r\n"+q);

	query = q;
	//rox

	//std::string replaces[][4] = {
	//	//{ "{fn "                          , "}"             , ""                    ,""         },
	//	//{ "CAST("                         ,"AS FLOAT)"      , " toFloat64("         ,") "       },
	//	//{ "CAST("                         ,"AS INTEGER)"    , " toInt32("           ,") "       },
	//	{ "CAST("                         ,"AS TIMESTAMP)"       , "("               ,")" },
	//	/*{ "CONVERT("                      , ", SQL_BIGINT)" , " toInt64("           ,") "       },
	//	{ "FLOOR("                        , ")"             , " floor("             ,") "       },
	//	{ "SQRT("                         , ")"             , " sqrt("              ,") "       },
	//	{ "POWER("                        , ")"             , " pow("               ,") "       },
	//	{ "{ts "                          , "}"             , " toDateTime("        ,") "       },
	//	{ "{d "                           , "}"             , " toDate("            ,") "       } ,
	//	{ "EXTRACT(YEAR FROM "            , ")"             , " toYear("            ,") "       } ,
	//	{ "EXTRACT(MONTH FROM "           , ")"             , " toMonth("           ,") "       } ,
	//	{ "EXTRACT(DAY FROM "             , ")"             , " toDayOfMonth("      ,") "       } ,
	//	{ "EXTRACT('year' FROM "            , ")"             , " toYear("            ,") " } ,
	//	{ "EXTRACT('month' FROM "           , ")"             , " toMonth("           ,") " } ,
	//	{ "EXTRACT('day' FROM "             , ")"             , " toDayOfMonth("      ,") " }*/
	//};
	//int len = sizeof(replaces) / sizeof(replaces[0]);
	//for (int i = 0; i < sizeof(replaces) / sizeof(replaces[0]); i++) {

	//	while (query.find(replaces[i][0]) != -1) {
	//		int n = query.find(replaces[i][0]);
	//		if (query.find(replaces[i][1], n) == -1)
	//		{
	//			break;
	//		}
	//		query.replace(n, replaces[i][0].length(), replaces[i][2]);
	//		int m = query.find(replaces[i][1], n);
	//		query.replace(m, replaces[i][1].length(), replaces[i][3]);
	//	}
	//}

    if (scan_escape_sequences) {
        prepared_query = replaceEscapeSequences(query);
    } else {
        prepared_query = q;
    }
    prepared = true;

	
	//LOGQUERY("\r\nafterReplace::\r\n" + prepared_query);
}

void Statement::setQuery(const std::string & q) {
    query = q;
    prepared_query = q;
}

void Statement::reset() {
    in = nullptr;
    response.reset();
    connection.session->reset();
    diagnostic_record.reset();
    result = ResultSet();

    ard.reset(new DescriptorClass);
    apd.reset(new DescriptorClass);
    ird.reset(new DescriptorClass);
    ipd.reset(new DescriptorClass);
}
