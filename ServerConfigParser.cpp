#include "ServerConfigParser.hpp"
#include "BaseConfig.hpp"
#include "ServerBaseConfig.hpp"
#include "ServerConfig.hpp"
#include "ServerLocationDirectiveConfig.hpp"
#include "ServerNormalDirectiveConfig.hpp"
#include <cstddef>
#include <map>
#include <string>
#include <sys/_types/_size_t.h>
#include <utility>
#include <vector>

using namespace webserv;

const char *valid_base_directives_array[] = {"server"};

const char *valid_server_normal_directives_array[] = {"listen",
	"server_name", "access_log", "location", "root"};

const char *valid_server_location_directives_array[] = {"fastcig_pass",
	"root", "expires"};

ServerConfigParser::ServerConfigParser(const string &config_str) : _config_str(config_str)
{
#ifdef PRINT_MSG
	cout << "ServerConfigParser string Assignement Constructor Called" << endl;
#endif // !PRINT_MSG
	initialize_valid_directives(valid_base_directives_array,
			&_valid_base_directives,
			sizeof(valid_base_directives_array) / sizeof(valid_base_directives_array[0]));
	initialize_valid_directives(valid_server_normal_directives_array,
			&_valid_server_normal_directives,
			sizeof(valid_server_normal_directives_array) / sizeof(valid_server_normal_directives_array[0]));
	initialize_valid_directives(valid_server_location_directives_array,
			&_valid_server_location_directives,
			sizeof(valid_server_location_directives_array) / sizeof(valid_server_location_directives_array[0]));
}

ServerConfigParser::ServerConfigParser(std::ifstream &config_file)
{
#ifdef PRINT_MSG
	cout << "ServerConfigParser ifstream Assignement Constructor Called" << endl;
#endif // !PRINT_MSG

	//if (!config_file)
	//	throw someshit here
	if (config_file.is_open())
	{
		std::ostringstream	ss;
		ss << config_file.rdbuf();
		_config_str = ss.str();
	}
	initialize_valid_directives(valid_base_directives_array,
			&_valid_base_directives,
			sizeof(valid_base_directives_array) / sizeof(valid_base_directives_array[0]));
	initialize_valid_directives(valid_server_normal_directives_array,
			&_valid_server_normal_directives,
			sizeof(valid_server_normal_directives_array) / sizeof(valid_server_normal_directives_array[0]));
	initialize_valid_directives(valid_server_location_directives_array,
			&_valid_server_location_directives,
			sizeof(valid_server_location_directives_array) / sizeof(valid_server_location_directives_array[0]));
}

ServerConfigParser::~ServerConfigParser()
{
#ifdef PRINT_MSG
	cout << "ServerConfigParser Destructor Called" << endl;
#endif // !PRINT_MSG
}

/*
 * @note : I currently move server to next serverblock in extract_bracket_content
 *	but i might want to seperate the actions. 
 *	(i could just pass in int and move by that amount)
 * */

void ServerConfigParser::parse_config(void)
{
	string			config(_config_str);

	if (config.length() == 0)
		cout << "throw u passed an empty conf fucker" << endl; //throw error

	//cout << "===============" << endl;
	//cout << config;
	//cout << "===============" << endl;

	//Keep trimming till I find a non white space (basically first word)
	//Cut out everything before first word instance
	config = config.substr(config.find_first_not_of("\t\n "));

	while (1)
	{
		size_t	bracket_distance = config.find_first_of("{");
		size_t	colon_distance = config.find_first_of(";");
		size_t	line_length = (bracket_distance < colon_distance) ? bracket_distance : colon_distance;
		size_t	valid_key;
		string	key;

		if (line_length == config.npos)
			break;

		//cout << "Config" << endl;
		//cout << "===============" << endl;
		//cout << config;
		//cout << "===============" << endl;

		key = extract_key(config);
		valid_key = is_valid_base_directive(key);
		if (valid_key == 1) //if keyword is server
		{
			ServerConfig	server_config;

			if (line_length == colon_distance)
			{
				cout << "throw found non bracket after server keyword" << endl;
				exit(1);
			}

			config = config.substr(config.find_first_of("{") + 1);
			//cout << "Config" << endl;
			//cout << "===============" << endl;
			//cout << config;
			//cout << "===============" << endl;
			cout << "here" << endl;
			string server_block = extract_bracket_content(&config, '{', '}');
			server_config = parse_server_block(server_block);
			insert_config(std::make_pair(key, &server_config));
			//cout << server_config << endl;
		}
		//else if (key == "user") //other keyword without block
		//{
		//	cout << "other valid keyword" << endl;
		//}
		else
		{
			cout << "throw not keyword" << endl; //throw someshit
			return;
		}
	}
	cout << *this << endl;
}

const ServerConfigParser::map_type&	ServerConfigParser::get_config() const
{
	return (_config);
}

pair<ServerConfigParser::const_iterator_type, ServerConfigParser::const_iterator_type>
ServerConfigParser::find_values(const string &key) const
{
	return (_config.equal_range(key));
}

void	ServerConfigParser::insert_config(const pair<string, BaseConfig*> &pair_to_insert)
{
	pair<string, BaseConfig*> copy(pair_to_insert.first,
			dynamic_cast<BaseConfig*>(pair_to_insert.second->get_copy()));

	_config.insert(copy);
}

/*
 * @note : i need to revise this.
 * */

string	ServerConfigParser::extract_bracket_content(std::string *config,
		const char &open_bracket, const char &close_bracket) const
{
	string	server_block = *config;
	size_t	final_pos = 0;

	cout << "intro" << endl;
	cout << "=========" << endl;
	cout << server_block << endl;
	cout << "=========" << endl;

	while (1)
	{
		size_t	open_bracket_pos = server_block.find(open_bracket);
		size_t	close_bracket_pos = server_block.find(close_bracket);
		cout << "........................." << endl;
		cout << "open_bracket_pos : " << open_bracket_pos << endl;
		cout << "close_bracket_pos : " << close_bracket_pos << endl;
		cout << "........................." << endl;
		if (close_bracket_pos == server_block.npos)
		{
			cout << "throw no closing bracket" << endl; //throw some error
			exit(1);
		}
		if (open_bracket_pos < close_bracket_pos)
		{
			server_block = server_block.substr(close_bracket_pos + 1);
			final_pos += close_bracket_pos + 1;
			cout << "----------" << endl;
			cout << server_block << endl;
			cout << "----------" << endl;
		}
		else if (open_bracket_pos > close_bracket_pos ||
				open_bracket_pos == string::npos)
		{
			if (close_bracket_pos == string::npos) //if no closing bracket is found.
				cout << "throw no closing bracket found" << endl;
			final_pos += close_bracket_pos;
			break ;
		}
	}
	//cout << "final_pos : " << final_pos << endl;

	//Final server_block
	server_block = config->substr(0, final_pos);

	//Move the config to next block (might move somwhere else?)
	*config = config->substr(final_pos + 1);
	//cout << "server_block in" << endl;
	//cout << "--------------" << endl;
	//cout << server_block;
	//cout << "--------------" << endl;
	//cout << "config in" << endl;
	//cout << "--------------" << endl;
	//cout << *config;
	//cout << "--------------" << endl;

	//Trimming spaces around server_block
	//cout << server_block[server_block.find_last_not_of("\t\n ")] << endl;
	//cout << server_block.find_last_not_of("\t\n ") << endl;
	//cout << server_block.length() << endl;
	server_block = server_block.substr(server_block.find_first_not_of("\t\n "),
			server_block.find_last_not_of("\t\n ") + 1);
	//cout << "--------------" << endl;
	//cout << server_block;
	//cout << "--------------" << endl;
	return (server_block);
}

string	ServerConfigParser::extract_key(const string &line) const
{
	string	key;
	string	skip_spaces;

	//dont think i need this.
	//if (line.find_first_of(("{")) == line.npos)
	//	cout << "throw no opening block in key" << endl; //throw some error
	
	//key = line.substr(0, line.find_first_of("{\t\n "));
	
	try 
	{
		skip_spaces = line.substr(line.find_first_not_of("\t\n "));
		key = skip_spaces.substr(0, skip_spaces.find_first_of("{\t\n "));
	}
	catch (...)
	{
		cout << "throw no key to extract" << endl;
		exit(1);
	}
	//cout << "----------------" << endl;
	//cout << "key = [" << key << "]" << endl;
	//cout << "----------------" << endl;
	
	return (key);
}

string	ServerConfigParser::extract_value(const string &line, int look_for_bracket) const
{
	string	value;
	string	skip_spaces_and_key;

	//Maybe add new line here idk
	//Used to skip the key and extra space idk how to do it faster.
	//skip_spaces_and_key = line.substr(line.find_first_of("\t "), 
	//		line.find_last_not_of("\t\n ") - 1);

	try
	{
		//skip space before key
		skip_spaces_and_key = line.substr(line.find_first_not_of("\t\n "));

		//skip key
		skip_spaces_and_key = skip_spaces_and_key.substr(skip_spaces_and_key.find_first_of("\t\n "));

		//skip spaces before value
		skip_spaces_and_key = skip_spaces_and_key.substr(skip_spaces_and_key.find_first_not_of("\t\n "));
	}
	catch (...)
	{
		cout << "throw some error value not found" << endl;
		exit(1);
	}

	//cout << "----------------" << endl;
	//cout << "skip = [" << skip_spaces_and_key << "]" << endl;
	//cout << "----------------" << endl;

	//Value has to be on same line as key
	if (look_for_bracket == 0)
		value = skip_spaces_and_key.substr(skip_spaces_and_key.find_first_not_of("\t "));
	else
	{
		//Look for bracket
		value = skip_spaces_and_key.substr(0, skip_spaces_and_key.find_first_of("{"));
		//Skip spaces after value
		value = value.substr(0, value.find_last_not_of("\t ") + 1);
	}

	//cout << "----------------" << endl;
	//cout << "value = [" << value << "]" << endl;
	//cout << "----------------" << endl;

	return (value);
}

int				ServerConfigParser::count_values_in_line(const string &line) const
{
	int		values = 0;
	string	line_copy = line;

	line_copy = line_copy.substr(line_copy.find_first_not_of("\t\n "));
	//cout << "[" << line_copy << "]" << endl;
	while (1)
	{
		size_t	len;

		len = line_copy.find_first_of("\t ");
		if (len == line_copy.npos)
			break ;
		line_copy = line_copy.substr(len);
		len = line_copy.find_first_not_of("\t ");
		if (len == line_copy.npos)
			break ;
		line_copy = line_copy.substr(len);
		values++;
	}
	return (values);
}

/*
 * @note '*' : might need a better way to check for ; and {
 * */

ServerConfig	ServerConfigParser::parse_server_block(const std::string &server_block)
{
	string			server_block_copy = server_block;
	ServerConfig	server_config;

	while (1)
	{
		size_t	bracket_distance = server_block_copy.find_first_of("{");
		size_t	colon_distance = server_block_copy.find_first_of(";");
		size_t	line_length = (bracket_distance < colon_distance) ? bracket_distance : colon_distance;
		if (line_length == server_block_copy.npos)
			break;
		string line = server_block_copy.substr(0, line_length);

		//cout << "==================" << endl;
		//cout << "line [" << line << "]" << endl;
		//cout << "==================" << endl;

		string	key = extract_key(line);
		size_t	key_is_valid = is_valid_server_normal_directive(key);
		if (key_is_valid == 1 || key_is_valid == 2) //Server Normal Directive Config
		{
			ServerNormalDirectiveConfig	normal_directive = parse_server_normal_directive(line);

			server_block_copy = server_block_copy.substr(server_block_copy.find_first_of(";") + 1);
			server_config.insert_config(std::make_pair(key, &normal_directive));
		}
		else if (key_is_valid == 3) //Server Location Directive Config
		{
			ServerLocationDirectiveConfig location_directive;

			if (line_length == colon_distance)
			{
				cout << "location block not followed up by bracket" << endl;
				exit(1);
			}

			string path = extract_value(line, 1);
			server_block_copy = server_block_copy.substr(server_block_copy.find_first_of("{") + 1);
			string location_block = extract_bracket_content(&server_block_copy, '{', '}');
			location_directive = parse_server_location_block(location_block);
			server_config.insert_config(std::make_pair(key, &location_directive));
		}
		else
		{
			cout << "throw key is not valid" << endl;
			exit(1);
		}
	}
	return (server_config);
}

ServerNormalDirectiveConfig		ServerConfigParser::parse_server_normal_directive(const string &normal_directive_line)
{
	ServerNormalDirectiveConfig	normal_directive;
	string						value;
	string						normal_directive_copy = normal_directive_line;
	string						key =  extract_key(normal_directive_copy);
	int							is_valid = is_valid_server_normal_directive(key);
	int							value_count = count_values_in_line(normal_directive_line);


	if (is_valid == 2) //Check for Normal Directives with multiple values
	{
		string	temp;
		string	value2;

		if (value_count != 2)
		{
			cout << "throw some error wrong value count for key " << key << endl;
			exit(1);
		}
		temp = extract_value(normal_directive_copy);
		value = extract_key(temp);
		value2 = extract_value(temp);
		normal_directive.set_config(std::make_pair(value, value2));
	}
	else if (is_valid == 1) //Check for Normal Directives with one value
	{
		if (value_count != 1)
		{
			cout << "throw some error wrong value count for key " << key << endl;
			exit(1);
		}
		value = extract_value(normal_directive_copy);
		normal_directive.set_config(value);
	}
	return (normal_directive);
}

ServerLocationDirectiveConfig	ServerConfigParser::parse_server_location_block(const string &location_block)
{
	string							location_block_copy = location_block;
	ServerLocationDirectiveConfig	location_directive;

	while (1)
	{
		size_t	line_length = location_block_copy.find_first_of(";");
		string	line;

		if (line_length == location_block_copy.npos)
			break ;
		line = location_block_copy.substr(0, line_length);
		location_directive.insert_config(std::make_pair(extract_key(line), extract_value(line)));
		location_block_copy = location_block_copy.substr(location_block_copy.find_first_of(";") + 1);
	}
	return (location_directive);
}

int			ServerConfigParser::is_valid_server_normal_directive(const string &key) const
{
	if (_valid_server_normal_directives.find(key) == _valid_server_normal_directives.end())
		return (0);
	else if (key == "server_name" || key == "access_log")
		return (2);
	else if (key == "location")
		return (3);
	return (1);
}

int			ServerConfigParser::is_valid_base_directive(const string &key) const
{
	if (_valid_base_directives.find(key) == _valid_base_directives.end())
		return (0);
	return (1);
}

void		ServerConfigParser::initialize_valid_directives(const char **directives, std::set<string> *set, size_t len)
{
	for (size_t i = 0; i < len; ++i)
		set->insert((directives[i]));
}

std::ostream&	operator<<(std::ostream& os, const ServerConfigParser &config)
{
	ServerConfigParser::map_type	config_map = config.get_config();

	for (ServerConfigParser::iterator_type cit = config_map.begin(); cit != config_map.end(); cit++)
	{
		os << "[Config] key : " BLUE << (*cit).first << RESET << endl;
		if ((*cit).first == "server")
		{
			ServerConfig					sc = dynamic_cast<ServerConfig&>(*(*cit).second);
			ServerConfig::map_type			server_map = sc.get_server_config();
			for (ServerConfig::iterator_type mit = server_map.begin(); mit != server_map.end(); mit++)
			{
				os << "\t[ServerConfig] key : " BLUE << (*mit).first << RESET << endl;
				if ((*mit).first != "location")
				{
					ServerNormalDirectiveConfig	nd = dynamic_cast<ServerNormalDirectiveConfig&>(*(*mit).second);
					os << "\t\t[NormalDirectiveConfig] : ";
					os << nd << endl;
				}
				else
				{
					ServerLocationDirectiveConfig	ld = dynamic_cast<ServerLocationDirectiveConfig&>(*(*mit).second);
					ServerLocationDirectiveConfig::map_type	location_map = ld.get_config();

					os << "\t\t[LocaitonDirectiveConfig] : " << endl;
					for (ServerLocationDirectiveConfig::map_type::iterator lit = location_map.begin(); lit != location_map.end(); lit++)
						os << "\t\t\t<key : " MAGENTA << (*lit).first << RESET ", value : " CYAN << (*lit).second << ">" RESET << endl;
					os << endl;
				}
			}
		}
		os << endl;
	}
	return (os);
}
