// Autorzy: Mikołaj Duch, Antoni Wiśniewski

#include <iostream>
#include <cstdint>
#include <regex>
#include <unordered_map>
#include <utility>
#include <set>
#include <charconv>

// hours and minutes
using Time = std::pair<uint16_t, uint16_t>; 
using TimeInterval = std::pair<Time, Time>;
// we can keep registration encoded in 64 bits
using Registration = uint64_t;
using RegisteredCars = std::unordered_map<Registration, Time>;
using Tickets = std::set<std::pair<Time, Registration>>;

RegisteredCars registeredCars{};
Tickets tickets{};

constexpr uint16_t timeToMinutes(Time time) {
    return 60 * time.first + time.second;
}

constexpr uint16_t MINIMAL_PARKING_MINUTES = timeToMinutes(Time{0, 10});
constexpr uint16_t MAXIMAL_PARKING_MINUTES = timeToMinutes(Time{11, 59});
constexpr Time OPENING_TIME = Time{8, 0};
constexpr Time CLOSING_TIME = Time{20, 0};
constexpr Time AFTER_CLOSING_TIME = Time{20, 1};

constexpr std::string_view VALID_TIME =
    R"-(((?:0?[89]|1[0-9])\.[0-5][0-9]|20\.00))-";
constexpr std::string_view registration = R"-(([A-Z][A-Z0-9]{2,10}))-";

const std::string INPUT_LINE = "^\\s*" + std::string(registration) + "\\s+" +
                               std::string(VALID_TIME) + "(?:\\s+" +
                               std::string(VALID_TIME) + ")?\\s*$";

Time readTime(std::string_view input) {
    Time result;
    std::from_chars(input.begin(), input.end(), result.first);
    std::from_chars(input.end() - 2, input.end(), result.second);
    return result;
}

bool operator<=(const Time& lhs, const Time& rhs) {
    return timeToMinutes(lhs) <= timeToMinutes(rhs);
}

bool ticketActive(Registration car) {
    if (!registeredCars.contains(car))
        return false;

    return true;
}

uint16_t duration(Time begin, Time end) {
    if (begin <= end)
        return timeToMinutes(end) - timeToMinutes(begin);

    return timeToMinutes(end) - timeToMinutes(OPENING_TIME)
           - timeToMinutes(begin) + timeToMinutes(CLOSING_TIME);
}

uint16_t duration(TimeInterval interval) {
    return duration(interval.first, interval.second);
}

bool checkTicketLength(Time begin, Time end) {
    uint16_t paidTime = duration(begin, end);

    return MINIMAL_PARKING_MINUTES <= paidTime &&
           paidTime <= MAXIMAL_PARKING_MINUTES;
}

void registerTicket(Registration carRegistration, Time begin, Time end) {
    auto ticket = registeredCars.find(carRegistration);

    if (ticket != registeredCars.end()) {
        Time oldTicketEnd = ticket->second;

        // true if new ticket doesn't improve the old one
        if ((oldTicketEnd > end && (oldTicketEnd < begin || begin <= end)) || 
            (oldTicketEnd < begin && begin <= end)) {
            return;
        }

        tickets.erase({oldTicketEnd, carRegistration});
    }

    tickets.insert({end, carRegistration});
    registeredCars[carRegistration] = end;
}

void removeTicketsCont(TimeInterval inter) {
    auto begin = tickets.lower_bound({inter.first, 0});
    auto end = tickets.upper_bound({inter.second, 0});

    std::for_each(begin, end, [&](std::pair<Time, Registration> r) {
        registeredCars.erase(r.second);
    });
    tickets.erase(begin, end);
}

void updateRegister(Time oldTime, Time newTime) {
    if (newTime < oldTime) {
        removeTicketsCont({oldTime, AFTER_CLOSING_TIME});
        removeTicketsCont({OPENING_TIME, newTime});
    } else {
        removeTicketsCont({oldTime, newTime});
    }
}

// Encodes registration as a 64 bit number 
// by treating it as a numbering system with base 37.
// Encoding assigns
// - empty space to 0
// - digits 0-9 to digits 1-10
// - letters A-Z to digits 11-36
Registration registrationFromString(std::string_view s) {
    Registration out = 0;

    for (size_t i = 0; i < 11; i++) {
        if (s.length() > i && s[i] < '9') {
            out += s[i] - '0' + 1;
        } else if (s.length() > i) {
            out += s[i] - 'A' + 11;
        }

        out *= 37;
    }

    return out;
}

int main() {
    std::string line;
    size_t lineId = 0;
    Time prevTime{8, 0};
    std::smatch match;
    std::regex lineRegex(INPUT_LINE, std::regex_constants::optimize | 
                         std::regex_constants::ECMAScript);

    while (std::getline(std::cin, line)) {
        lineId++;

        if (!std::regex_match(line, match, lineRegex)) {
            std::cerr << "ERROR " << lineId << "\n";
            continue;
        }

        Registration registration = registrationFromString(
            std::string_view(match[1].first, match[1].second));
        Time endTime, newTime = readTime(std::string_view(match[2].first, 
                                                          match[2].second));

        // ticket registration detection
        if (match[3].matched) {
            endTime = readTime(std::string_view(
                match[3].first, match[3].second));

            if (!checkTicketLength(newTime, endTime)) {
                std::cerr << "ERROR " << lineId << "\n";
                continue;
            }
        }

        if (prevTime != newTime)
            updateRegister(prevTime, newTime);

        prevTime = newTime;

        // ticket registration detection
        if (match[3].matched) {
            registerTicket(registration, newTime, endTime);
            std::cout << "OK " << lineId << "\n";
        } else {
            if (ticketActive(registration)) {
                std::cout << "YES " << lineId << "\n";
            } else {
                std::cout << "NO " << lineId << "\n";
            }

            continue;
        }
    }
}
