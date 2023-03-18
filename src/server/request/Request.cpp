#include "Request.hpp"
#include "../../log/Log.hpp"
#include "../../utils/Utils.hpp"
#include <cstddef>
#include <exception>
#include <iostream>
#include <string>
#include <cstring>  
#include <vector>
#include <map>
#include "../../colours.h"
#include <string.h>
#include <fstream>

using namespace webserv;

using std::cout;
using std::endl;
using std::string;

int Request::find_request_type() {
    std::vector<string> request_types;
    string  type_line = _header_lines[0].substr(0, _header_lines[0].find(" "));

    request_types.push_back("GET");
    request_types.push_back("POST");
    request_types.push_back("DELETE");

    for (std::vector<string>::iterator it = request_types.begin(); it != request_types.end(); it++)
    {
        if (type_line.find(*it) != string::npos)
        {
            _type = *it;
            return (1);
        }
    }
    return (0);
}

void Request::find_request_path() {
    string first_line = _header_lines[0];

    std::vector<size_t> positions;
    size_t pos = first_line.find(" ", 0);
    while(pos != string::npos)
    {
        positions.push_back(pos);
        pos = first_line.find(" ", pos+1);
    }
    if (positions.size() != 2)
        _path = string("");
    else
        _path = first_line.substr(first_line.find("/"), positions[1] - positions[0] - 1); 
}

void Request::find_request_protocol_version() {
    string first_line = _header_lines[0];

    std::vector<size_t> positions;
    size_t pos = first_line.find(" ", 0);
    while(pos != string::npos)
    {
        positions.push_back(pos);
        pos = first_line.find(" ", pos + 1);
    }

	if (positions.size() == 0) //if Request type is the only parameter.
		;
	else if (positions.size() == 1)
		_protocol_version = first_line.substr(positions[0], first_line.length() - positions[0]);
	else
		_protocol_version = first_line.substr(positions[1], first_line.length() - positions[1]);
}

bool isNumber(const string &s)
{
	return s.find_first_not_of("0123456789") == string::npos;
}

void Request::parse_headers()
{
    for (std::vector<string>::iterator line = _header_lines.begin() + 1; line != _header_lines.end(); line++)
    {
        if (*line == "\r\n")
            break;

        string key = line->substr(0, line->find(" ") - 1);
        string value = (*line).substr((*line).find(" ") + 1, (*line).size() - (*line).find(" ") - 1); // -1 is for invisible characters at the end of every line in a http header
        _headers[key] = value;
    }
    // if post and no Content-Length
    // _response_code = 411;
}

void    Request::process_post()
{
    if (_headers.find("Content-Length") == _headers.end() || !isNumber(_headers["Content-Length"]))
    {
		cout << "no content length found" << endl;
        _is_done = true; //this changes to error_code
        return ;
    }
	
    if (std::stoi(_headers["Content-Length"]) >= 1
        && _body.size() == std::stoul(_headers["Content-Length"]))
        _is_done = true;
}

static string find_filename(string line)
{
    string temp = line;
    string data = "";
    std::map<string, string> temp_dict;

    while (temp.size() > 0)
    {
        if (temp.find("; ") == string::npos){
            data = temp;
            temp = temp.erase(0, data.length());
        }
        else{
            data = temp.substr(0, temp.find("; ") + 1);
            temp = temp.erase(0, data.length() + 1);
        }
        if (temp.size() >= 2 && temp[0] == '\r' && temp[1] == '\n')
            break;
        if (data.find("=") != string::npos)
        {
            string key = data.substr(0, data.find("="));
            string value = data.substr(data.find("=") + 1, (data.length() - data.find("=")));
            value.erase(std::remove(value.begin(), value.end(), '"'), value.end());
            value.erase(std::remove(value.begin(), value.end(), ';'), value.end());
            temp_dict[key] = value;
        }
    }
    if (temp_dict.find("filename") != temp_dict.end())
        return (temp_dict.find("filename")->second);
    return ("nofilename");
}

void    Request::process_image()
{
    if (_headers.find("Content-Type") == _headers.end())
        return ;

    if (_headers["Content-Type"].find("multipart/form-data;") == 0)
    {
        string boundary;
        string body = _body;
        std::vector<string> datas;

        boundary = "--" + _headers["Content-Type"].substr(_headers["Content-Type"].find("=") + 1, 38);
        while (body.find(boundary) != string::npos)
        {
            string data = body.substr(boundary.length() + 2, body.find(boundary, 1) - boundary.length() - 2);
            body = body.erase(0, boundary.length() + data.length() + 2);
            datas.push_back(data);
        }

        for (std::vector<string>::iterator data = datas.begin(); data != datas.end(); data++)
        {
            string headers = *data;
            std::map<string, string> temp_dict;

			/* cout << "START: <" << *data << ">" << endl; */
			/* cout << "data length : " << data->length() << endl; */

            while (headers.find("\r\n") != string::npos)
            {
                string line =  (headers.substr(0, headers.find("\r\n")));
                if (line.find(":") != string::npos)
                {
                    string key = (line).substr(0, (line).find(" ") - 1);
                    string value = (line).substr((line).find(" ") + 1, (line).size() - (line).find(" ") - 1);
					/* cout << "Key : [" << key << "]" << endl; */
					/* cout << "value : [" << value << "]" << endl; */
                    temp_dict[key] = value;
                }
                headers = headers.erase(0, headers.find("\r\n") + 2);
                if (headers.size() >= 2 && headers[0] == '\r' && headers[1] == '\n') {
                    temp_dict["body"] = headers.substr(2, headers.length() - 4);
                    break;
                }
            }

			/* if (temp_dict.find("Content-Type") != temp_dict.end()) */
			/* { */
			/* 	cout << "CT : " << "[" << temp_dict.find("Content-Type")->first << "]" << endl; */
			/* 	cout << "CTV: " << "[" << temp_dict.find("Content-Type")->second<< "]"  << endl; */
			/* } */

            if (temp_dict.find("Content-Type") != temp_dict.end() &&
                (temp_dict.find("Content-Type")->second == "image/png" ||
                 temp_dict.find("Content-Type")->second == "image/jpeg"))
            {
				cout << "doing something" << endl;
				cout << "writing length : " << temp_dict["body"].length() << endl;
				/* cout << "SENT :" << temp_dict["body"] << endl; */
                string filename = find_filename(temp_dict["Content-Disposition"]);
                std::ofstream out(filename);
                out << temp_dict["body"];
                out.close();
            }
        }
    }
}

void	Request::read_header(string request_string)
{

	//if no header content yet and first characters are \r\n (ignoring empty lines on first request)
	
	/* cout << "header line size " << _header_lines.size() << endl; */
	/* cout << "request_string = [" << request_string << "]" << endl; */
	string test = request_string;
	utils::replaceAll(test, "\r\n", "\\r\\n\n");
	/* cout << test; */
	/* cout << "request_string length " << request_string.length() << endl; */
	/* cout << "request_string.find " << request_string.find("\r\n") << endl; */

	if (_header_lines.size() == 0 && request_string.find("\r\n") == 0) // can merge in while loop
	{
		/* cout << "Empty \\r\\n" << endl; */
		return ;
	}

	/* for (std::vector<string>::iterator it = _header_lines.begin(); it != _header_lines.end(); it++) */
	/* 	cout << "header line = [" << *it << "]" <<endl; */

    while (request_string.find("\r\n") != request_string.npos)
    {
		/* cout << "loop" << endl; */
		if (_header_lines.size() == 1)
		{
			if (!find_request_type())
			{
				_bad_request = true;
				return ;
			}
			cout << "TYPE : " << type() << endl;
			find_request_path();
			find_request_protocol_version();
		}
        if (request_string.find("\r\n") == 0)
        {
			//set header_done()
			Log(DEBUG, "header done");
			_header_done = true;
            request_string = request_string.substr(request_string.find("\r\n") + 2);
            _body = request_string;
			/* cout << _body << endl; */
			cout << _body.length() << endl;
			/* cout << "header line size " << _header_lines.size() << endl; */
            break;
        }
        string line = request_string.substr(0, request_string.find("\r\n"));
		cout << "line [" << line << "]" << endl;
        _header_lines.push_back(line);
		/* cout << "header line size " << _header_lines.size() << endl; */
        request_string = request_string.substr(request_string.find("\r\n") + 2);
		/* cout << "req string [" << request_string << "]" << endl; */
    }

	/* cout << "after loop" << endl; */

	/* for (std::vector<string>::iterator it = _header_lines.begin(); it != _header_lines.end(); it++) */
	/* 	cout << "header line = [" << *it << "]" <<endl; */

	/*     if (_header_lines.size() < 1 || _header_lines[0].size() <= 2) //might not need */
	/* { */
	/*         _bad_request = true; */
	/*         return ; */
	/* } */

	if (!header_done())
	{
		cout << "HEADER NOT DONE" << endl;
		return;
	}

    parse_headers();
    if (type() == "POST")
	{
		cout << "IN POST" << endl;
        process_post();
	}
    if (type() == "GET" || type() == "DELETE")
        _is_done = true;
}

Request::Request(string request_string, int socket) :
	_body(""), _socket(socket),
	_is_done(false), _header_done(false), _bad_request(false)
{
#ifdef PRINT_MSG
	cout << "Request Assignment Constructor called" << endl;
#endif
	read_header(request_string);
}

Request::Request() :
	_is_done(false), _header_done(false), _bad_request(false)
{
#ifdef PRINT_MSG
	cout << "Request Default Constructor called" << endl;
#endif
}

Request::Request(const Request &to_copy) :
	_headers(to_copy._headers), _header_lines(to_copy._header_lines),
	_body(to_copy._body), _type(to_copy._type), _path(to_copy._path),
	_protocol_version(to_copy._protocol_version), _socket(to_copy._socket),
	_is_done(to_copy._is_done), _header_done(to_copy._header_done),
	_bad_request(to_copy._bad_request)

{
#ifdef PRINT_MSG
	cout << "Request Copy Constructor called" << endl;
#endif
}

Request::~Request()
{
#ifdef PRINT_MSG
	cout << "Request Destructor Constructor called" << endl;
#endif
}

const std::map<string, string> &Request::headers() const { return _headers; }

const string    Request::to_str() const
{
    string ret;

    ret = string(CYAN) + "REQUEST TYPE: " + string(RESET) + string(YELLOW) + type() + string(RESET) + "\n"\
		  + string(CYAN) + "REQUEST PATH: " + string(RESET) + string(YELLOW) + path() + string(RESET) + "\n"\
		  + string(CYAN) + "REQUEST PROTOCOL VERSION: " + string(RESET) + string(YELLOW) + protocol_version() + string(RESET) + "\n";
    for (std::map<string, string>::const_iterator it = headers().begin(); it != headers().end(); it++)
        ret += string(CYAN) + it->first + ": " + string(RESET) + string(YELLOW) + it->second + string(RESET) + "\n";
    return (ret);
}

void  Request::add_body(string buffer)
{
    if (type() != "POST")
        return ;
    if (_headers.find("Content-Length") == _headers.end())
        return ;
    /* if (std::stoul(_headers["Content-Length"]) == _body.size()) */
    /*     _is_done = true; */
    for (string::iterator it = buffer.begin(); it != buffer.end(); it++)
        _body.push_back(*it);
    if (std::stoul(_headers["Content-Length"]) == _body.size())
        _is_done = true;
	cout << _body.length() << endl;
}


std::ostream &operator<<(std::ostream &os, const webserv::Request &request)
{
    os << CYAN << "REQUEST TYPE: " << RESET << YELLOW << request.type() << RESET << std::endl;
    os << CYAN << "REQUEST PATH: " << RESET << YELLOW << request.path() << RESET << std::endl;
    os << CYAN << "REQUEST PROTOCOL VERSION: " << RESET << YELLOW << request.protocol_version() << RESET << std::endl;
    for (std::map<string, string>::const_iterator it = request.headers().begin(); it != request.headers().end(); it++)
        os << CYAN << it->first << ": " << RESET << YELLOW << it->second << RESET << std::endl;
    return (os);
}


bool      Request::done() { return _is_done; }

bool      Request::header_done() { return _header_done; }

bool	  Request::bad_request() { return _bad_request; }

string    &Request::body() { return _body; }

const string &Request::type() const { return _type; }

const string &Request::path() const { return _path; }

const int   &Request::socket() const { return _socket; }


const string &Request::protocol_version() const { return _protocol_version; }
