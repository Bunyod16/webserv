#include "ServerNormalDirectiveConfig.hpp"
#include "colours.h"

using namespace webserv;

using std::cout;
using std::endl;

ServerNormalDirectiveConfig::ServerNormalDirectiveConfig()
{
#ifdef PRINT_MSG
	cout << "ServerNormalDirectiveConfig Default Constructor called" << endl;
#endif
}

ServerNormalDirectiveConfig::~ServerNormalDirectiveConfig()
{
#ifdef PRINT_MSG
	cout << "ServerNormalDirectiveConfig Destructor called" << endl;
#endif
}

ServerNormalDirectiveConfig::ServerNormalDirectiveConfig(const ServerNormalDirectiveConfig &to_copy)
{
	_normal_directive_config = to_copy._normal_directive_config;
}

ServerNormalDirectiveConfig*
ServerNormalDirectiveConfig::get_base() const
{
	return (new ServerNormalDirectiveConfig());
}

ServerNormalDirectiveConfig*
ServerNormalDirectiveConfig::get_copy() const
{
	return (new ServerNormalDirectiveConfig(*this));
}

//const pair<string, pair<string, string> >&
const pair<string, string> &
ServerNormalDirectiveConfig::get_config(void) const
{
	return (_normal_directive_config);
}

//const string&
//ServerNormalDirectiveConfig::get_key(void) const
//{
//	return (_normal_directive_config.first);
//}

const string&
ServerNormalDirectiveConfig::get_value(void) const
{
	//return (_normal_directive_config.second.first);
	return (_normal_directive_config.first);
}

const string&
ServerNormalDirectiveConfig::get_value2(void) const
{
	//return (_normal_directive_config.second.second);
	return (_normal_directive_config.second);
}

//const pair<string, string>&
//ServerNormalDirectiveConfig::get_value_pair(void) const
//{
//	return (_normal_directive_config.second);
//}

void	ServerNormalDirectiveConfig::set_config(const pair<string, string> &pair_to_insert)
{
	_normal_directive_config = pair_to_insert;
}

void	ServerNormalDirectiveConfig::set_config(const string &value1,
		const string &value2)
{
	_normal_directive_config.first = value1;
	_normal_directive_config.second = value2;
}

std::ostream&	operator<<(std::ostream& os,
		const ServerNormalDirectiveConfig &normal_directive)
{
	os << "<value1 : " MAGENTA << normal_directive.get_value() << RESET;
	if (normal_directive.get_value2() == "")
		os << RESET << ">" << endl;
	else
		os << ", value2 : " MAGENTA << normal_directive.get_value2() << RESET << ">" << endl;
	return (os);
}
