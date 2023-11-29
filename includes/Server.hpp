#ifndef SERVER_HPP
#define SERVER_HPP

#include "FdSet.hpp"
#include <iostream>
#include <string>

class Location;

class Server : public FdSet
{
	private: 
		std::string name;
		std::string ip;
		std::string port;

	public:
		Server();
		virtual ~Server();

		// ㅍㅏ시ㅇ하다가 location ~ {} 만만나면 아래 함수 호출할거임
		// location 클래스에 잘 담아주세용 (new 해서 주세야ㅕ)
		// location 멤버 변수들은 냅다 적어놓고, 없으면 null 채우ㅓ주기
		// 한줄로 쭉 이어서 드립니다.
		Location& parseLocation(std::string &str);
};

#endif
