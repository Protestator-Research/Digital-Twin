#pragma once

#include <string>

namespace DIGITAL_TWIN_SERVER
{
	struct Principal
	{
		std::string id;
	};

	class AuthenticationService
	{
	public:
		AuthenticationService();
		~AuthenticationService();

		bool canSubscribe(Principal const& p, std::string_view filter) const;
		bool canPublish(Principal const& p, std::string_view filter) const;

	private:



	};
}