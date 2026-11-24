#include "StoreLogic.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>

namespace DigitalTwin::Store {
	namespace {
		int64_t floorDiv(int64_t a, int64_t b)
		{
			int64_t q = a / b;
			if ((a % b != 0) && ((a < 0) != (b < 0)))
				--q;
			return q;
		}

		// Civil calendar <-> days since 1970-01-01 (proleptic Gregorian), after Howard Hinnant's algorithms.
		void civilFromDays(int64_t z, int& year, int& month, int& day)
		{
			z += 719468;
			const int64_t era = (z >= 0 ? z : z - 146096) / 146097;
			const int64_t doe = z - era * 146097;
			const int64_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
			const int64_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
			const int64_t mp = (5 * doy + 2) / 153;
			day = static_cast<int>(doy - (153 * mp + 2) / 5 + 1);
			month = static_cast<int>(mp < 10 ? mp + 3 : mp - 9);
			year = static_cast<int>(yoe + era * 400 + (month <= 2 ? 1 : 0));
		}

		int64_t daysFromCivil(int year, int month, int day)
		{
			const int64_t y = year - (month <= 2 ? 1 : 0);
			const int64_t era = (y >= 0 ? y : y - 399) / 400;
			const int64_t yoe = y - era * 400;
			const int64_t doy = (153 * (month > 2 ? month - 3 : month + 9) + 2) / 5 + day - 1;
			const int64_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
			return era * 146097 + doe - 719468;
		}

		int hexValue(char c)
		{
			if (c >= '0' && c <= '9')
				return c - '0';
			if (c >= 'a' && c <= 'f')
				return c - 'a' + 10;
			if (c >= 'A' && c <= 'F')
				return c - 'A' + 10;
			return -1;
		}

		std::string percentDecode(const std::string& text)
		{
			std::string out;
			for (size_t i = 0; i < text.size(); ++i)
			{
				if (text[i] == '%' && i + 2 < text.size() && hexValue(text[i + 1]) >= 0 && hexValue(text[i + 2]) >= 0)
				{
					out.push_back(static_cast<char>(hexValue(text[i + 1]) * 16 + hexValue(text[i + 2])));
					i += 2;
				}
				else
					out.push_back(text[i]);
			}
			return out;
		}

		void replaceAll(std::string& text, const std::string& what, const std::string& with)
		{
			if (what.empty())
				return;
			size_t position = 0;
			while ((position = text.find(what, position)) != std::string::npos)
			{
				text.replace(position, what.size(), with);
				position += with.size();
			}
		}

		std::string lower(std::string text)
		{
			std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
			return text;
		}

		std::vector<std::string> secretsOf(const std::string& url)
		{
			std::vector<std::string> secrets;
			const auto add = [&secrets](const std::string& secret) {
				if (!secret.empty() && std::find(secrets.begin(), secrets.end(), secret) == secrets.end())
					secrets.push_back(secret);
			};

			// postgresql://user:password@host/db. Where the user info ends is ambiguous: libpq stops the authority at
			// the first '/', but a password may contain '/', '?' or '#' unencoded, and a '@' may follow in the query
			// (?user=a@b). So every reading is redacted: the user info up to each '@' of the URL, the first one
			// before the first '/' being the one libpq uses. Over-redacting does no harm, leaving a password part does.
			const auto scheme = url.find("://");
			if (scheme != std::string::npos)
			{
				const auto userInfoStart = scheme + 3;
				const auto addUserInfo = [&](const std::string& userInfo) {
					const auto colon = userInfo.find(':');
					if (colon == std::string::npos)
						return;
					const auto password = userInfo.substr(colon + 1);
					add(password);
					add(percentDecode(password));
					// An error message may quote the part of the URL that it could not parse.
					std::string piece;
					for (const char c : password + "/")
					{
						if (c == '/' || c == '?' || c == '#')
						{
							if (piece.size() >= 3)
								add(piece);
							piece.clear();
						}
						else
							piece.push_back(c);
					}
				};
				// Every '@' may end the user info: the first one before the first '/' (what libpq reads), the last one
				// (a password with '/'), and those in between.
				for (auto at = url.find('@', userInfoStart); at != std::string::npos; at = url.find('@', at + 1))
					addUserInfo(url.substr(userInfoStart, at - userInfoStart));
			}

			// password=secret (key/value form or URI parameter)
			const auto lowered = lower(url);
			size_t position = 0;
			while ((position = lowered.find("password=", position)) != std::string::npos)
			{
				position += 9;
				std::string value;
				if (position < url.size() && url[position] == '\'')
				{
					++position;
					while (position < url.size() && url[position] != '\'')
					{
						if (url[position] == '\\' && position + 1 < url.size())
							++position;
						value.push_back(url[position++]);
					}
				}
				else
				{
					while (position < url.size() && !std::isspace(static_cast<unsigned char>(url[position])) && url[position] != '&')
						value.push_back(url[position++]);
				}
				add(value);
				add(percentDecode(value));
			}
			return secrets;
		}
	}

	Micros toMicros(std::chrono::system_clock::time_point time)
	{
		return std::chrono::duration_cast<std::chrono::microseconds>(time.time_since_epoch()).count();
	}

	std::chrono::system_clock::time_point toTimePoint(Micros micros)
	{
		return std::chrono::system_clock::time_point(
			std::chrono::duration_cast<std::chrono::system_clock::duration>(std::chrono::microseconds(micros)));
	}

	YearMonth monthOf(Micros time)
	{
		int year = 1970;
		int month = 1;
		int day = 1;
		civilFromDays(floorDiv(time, MICROS_PER_DAY), year, month, day);
		return YearMonth{year, month};
	}

	YearMonth addMonths(YearMonth month, int months)
	{
		const int64_t index = static_cast<int64_t>(month.Year) * 12 + (month.Month - 1) + months;
		const int64_t year = floorDiv(index, 12);
		return YearMonth{static_cast<int>(year), static_cast<int>(index - year * 12 + 1)};
	}

	Micros startOf(YearMonth month)
	{
		return daysFromCivil(month.Year, month.Month, 1) * MICROS_PER_DAY;
	}

	Micros endOf(YearMonth month)
	{
		return startOf(addMonths(month, 1));
	}

	std::string partitionName(YearMonth month)
	{
		char buffer[40];
		std::snprintf(buffer, sizeof(buffer), "measurement_y%04dm%02d", month.Year, month.Month);
		return buffer;
	}

	std::optional<YearMonth> parsePartitionName(std::string_view name)
	{
		constexpr std::string_view prefix = "measurement_y";
		if (name.size() != prefix.size() + 4 + 1 + 2 || name.substr(0, prefix.size()) != prefix || name[prefix.size() + 4] != 'm')
			return std::nullopt;
		int year = 0;
		int month = 0;
		for (size_t i = 0; i < 4; ++i)
		{
			const char c = name[prefix.size() + i];
			if (c < '0' || c > '9')
				return std::nullopt;
			year = year * 10 + (c - '0');
		}
		for (size_t i = 0; i < 2; ++i)
		{
			const char c = name[prefix.size() + 5 + i];
			if (c < '0' || c > '9')
				return std::nullopt;
			month = month * 10 + (c - '0');
		}
		if (month < 1 || month > 12)
			return std::nullopt;
		return YearMonth{year, month};
	}

	std::string createPartitionSql(YearMonth month)
	{
		return "CREATE TABLE IF NOT EXISTS " + partitionName(month) + " PARTITION OF measurement FOR VALUES FROM ('" +
			formatTimestamp(startOf(month)) + "') TO ('" + formatTimestamp(endOf(month)) + "')";
	}

	std::string dropPartitionSql(YearMonth month)
	{
		return "DROP TABLE IF EXISTS " + partitionName(month);
	}

	std::optional<Micros> retentionCutoff(Micros now, uint32_t retentionDays)
	{
		if (retentionDays == 0)
			return std::nullopt;
		return now - static_cast<Micros>(retentionDays) * MICROS_PER_DAY;
	}

	bool partitionExpired(YearMonth month, Micros cutoff)
	{
		return endOf(month) <= cutoff;
	}

	std::string formatTimestamp(Micros time)
	{
		const int64_t days = floorDiv(time, MICROS_PER_DAY);
		const int64_t rest = time - days * MICROS_PER_DAY;
		int year = 1970;
		int month = 1;
		int day = 1;
		civilFromDays(days, year, month, day);
		const int hour = static_cast<int>(rest / (3600 * MICROS_PER_SECOND));
		const int minute = static_cast<int>(rest / (60 * MICROS_PER_SECOND) % 60);
		const int second = static_cast<int>(rest / MICROS_PER_SECOND % 60);
		const int micro = static_cast<int>(rest % MICROS_PER_SECOND);
		char buffer[64];
		std::snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d.%06d+00", year, month, day, hour, minute, second, micro);
		return buffer;
	}

	Micros MonotonicClock::clamp(Micros time)
	{
		if (HasLast && time < Last)
			return Last;
		Last = time;
		HasLast = true;
		return time;
	}

	void MonotonicClock::seed(Micros time)
	{
		if (!HasLast || time > Last)
		{
			Last = time;
			HasLast = true;
		}
	}

	std::optional<Micros> MonotonicClock::last() const
	{
		if (!HasLast)
			return std::nullopt;
		return Last;
	}

	Micros effectiveNow(Micros now, const MonotonicClock& clock)
	{
		const auto last = clock.last();
		return last && *last > now ? *last : now;
	}

	std::optional<CheckpointReason> decideCheckpoint(const CheckpointTracker& tracker, Micros now, Micros intervalMicros)
	{
		if (tracker.Gap)
			return now >= tracker.GapRetryNotBefore ? std::optional(CheckpointReason::AfterDataLoss) : std::nullopt;
		if (!tracker.HasCheckpoint)
			return CheckpointReason::Interval;
		if (monthOf(now) != monthOf(tracker.LastCheckpointAt))
			return CheckpointReason::NewMonth;
		if (intervalMicros > 0 && tracker.ChangedSinceCheckpoint && now - tracker.LastCheckpointAt >= intervalMicros)
			return CheckpointReason::Interval;
		return std::nullopt;
	}

	ChangeNoAllocator::ChangeNoAllocator(int64_t blockSize, std::function<int64_t()> fetchBlockStart) :
		BlockSize(blockSize > 0 ? blockSize : 1), Fetch(std::move(fetchBlockStart))
	{
	}

	int64_t ChangeNoAllocator::next()
	{
		if (Current >= End)
		{
			const int64_t start = Fetch();   // may throw, nothing has changed then
			Current = start;
			End = start + BlockSize;
		}
		return Current++;
	}

	ValueColumns columnsOf(const Model::VariableValue& value)
	{
		ValueColumns columns;
		if (value.num.has_value())
			columns.Num = std::isfinite(*value.num) ? *value.num : 0.0;
		columns.Text = value.text;
		if (columns.Text)
			columns.Text->erase(std::remove(columns.Text->begin(), columns.Text->end(), '\0'), columns.Text->end());
		columns.Json = value.json;
		return columns;
	}

	Model::VariableValue valueFromColumns(Model::ValueType type, std::optional<double> num, std::optional<std::string> text,
	                                      std::optional<std::string> json)
	{
		Model::VariableValue value;
		value.type = type;
		value.num = num;
		value.text = std::move(text);
		value.json = std::move(json);
		return value;
	}

	std::vector<std::string> splitSqlStatements(std::string_view sql)
	{
		std::vector<std::string> statements;
		std::string current;
		const auto flush = [&]() {
			const auto begin = current.find_first_not_of(" \t\r\n");
			if (begin != std::string::npos)
			{
				const auto end = current.find_last_not_of(" \t\r\n");
				statements.push_back(current.substr(begin, end - begin + 1));
			}
			current.clear();
		};

		size_t i = 0;
		while (i < sql.size())
		{
			const char c = sql[i];
			if (c == '-' && i + 1 < sql.size() && sql[i + 1] == '-')
			{
				while (i < sql.size() && sql[i] != '\n')
					++i;
				continue;
			}
			if (c == '/' && i + 1 < sql.size() && sql[i + 1] == '*')
			{
				i += 2;
				while (i + 1 < sql.size() && !(sql[i] == '*' && sql[i + 1] == '/'))
					++i;
				i += 2;
				current.push_back(' ');
				continue;
			}
			if (c == '\'' || c == '"')
			{
				current.push_back(c);
				++i;
				while (i < sql.size())
				{
					current.push_back(sql[i]);
					if (sql[i] == c)
					{
						if (i + 1 < sql.size() && sql[i + 1] == c)   // doubled quote
						{
							current.push_back(sql[i + 1]);
							i += 2;
							continue;
						}
						++i;
						break;
					}
					++i;
				}
				continue;
			}
			if (c == ';')
			{
				flush();
				++i;
				continue;
			}
			current.push_back(c);
			++i;
		}
		flush();
		return statements;
	}

	std::string redactSecrets(std::string message, const std::string& databaseUrl)
	{
		if (databaseUrl.empty())
			return message;
		replaceAll(message, databaseUrl, "<database-url>");
		// Longest first, so that a secret that contains another one is removed completely.
		auto secrets = secretsOf(databaseUrl);
		std::sort(secrets.begin(), secrets.end(), [](const auto& a, const auto& b) { return a.size() > b.size(); });
		for (const auto& secret : secrets)
			replaceAll(message, secret, "***");
		return message;
	}

	bool looksLikeUuid(std::string_view text)
	{
		if (text.size() != 36)
			return false;
		for (size_t i = 0; i < text.size(); ++i)
		{
			if (i == 8 || i == 13 || i == 18 || i == 23)
			{
				if (text[i] != '-')
					return false;
			}
			else if (hexValue(text[i]) < 0)
				return false;
		}
		return true;
	}

	bool isValidSchemaName(std::string_view name)
	{
		if (name.empty() || name.size() > 63)
			return false;
		for (size_t i = 0; i < name.size(); ++i)
		{
			const unsigned char c = static_cast<unsigned char>(name[i]);
			const bool letter = std::isalpha(c) != 0 || c == '_';
			const bool digit = std::isdigit(c) != 0;
			if (!(letter || (digit && i > 0)))
				return false;
		}
		return true;
	}

	ErrorClass classifySqlState(std::string_view sqlState)
	{
		if (sqlState.size() < 2)
			return ErrorClass::Transient;
		const auto sqlClass = sqlState.substr(0, 2);
		// 08 connection exception, 40 transaction rollback (deadlock, serialization), 53 insufficient resources
		// (disk full, out of memory, too many connections), 55 object not in prerequisite state (lock not available),
		// 57 operator intervention (statement timeout, admin shutdown), 58 system error.
		for (const char* transient : {"08", "40", "53", "55", "57", "58"})
			if (sqlClass == transient)
				return ErrorClass::Transient;
		return ErrorClass::Data;
	}
}
