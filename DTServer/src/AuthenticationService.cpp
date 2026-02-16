#include "AuthenticationService.h"

namespace DIGITAL_TWIN_SERVER
{
	AuthenticationService::AuthenticationService()
	{
	}

	AuthenticationService::~AuthenticationService()
	{
	}

	bool AuthenticationService::canSubscribe(Principal const& p, std::string_view filter) const
	{
		// devices/<id>/cmd/#
		std::string prefix = "devices/" + p.id + "/cmd/";
		return filter.rfind(prefix, 0) == 0;
	}

	bool AuthenticationService::canPublish(Principal const& p, std::string_view topic) const
	{
		// devices/<id>/telemetry
		return topic == ("devices/" + p.id + "/telemetry");
	}
}
