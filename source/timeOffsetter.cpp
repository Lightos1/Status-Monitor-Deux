//Simple and fast timezone clock offset calculator.
//Supports all timezones possible to set in Nintendo Switch.
//It doesn't support anything before 22.05.2026

#include <switch.h>
#include <string.h>
#include <time.h>
#include <array>

static inline int64_t hourMinutesToSecond(int64_t hour, int64_t minutes) {
    return (hour * 3600) + (minutes * 60);    
}

// Helper to find the day of the week (0=Sunday, 6=Saturday)
static int get_day_of_week(int year, int month, int day) {
    static const int t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
    if (month < 3) {
        year -= 1;
    }
    return (year + year/4 - year/100 + year/400 + t[month-1] + day) % 7;
}

// Compile-time FNV-1a Hash Function
constexpr uint32_t hash_tz(const char* str) {
    uint32_t hash = 2166136261u;
    for (int i = 0; str[i] != '\0'; ++i) {
        hash ^= static_cast<uint32_t>(str[i]);
        hash *= 16777619u;
    }
    return hash;
}

extern "C" time_t getLocalPosixTimeSafe(time_t posix_time, TimeLocationName* name) {
    const time_t last_valid_posix_time = 1779460399; // May 22, 2026
    if (posix_time < last_valid_posix_time) return 0;

    const char* m_name = name->name;

    // Hash the runtime string exactly ONCE
    uint32_t current_tz_hash = hash_tz(m_name);

    time_t raw_time = (time_t)posix_time;
    struct tm utc;
    gmtime_r(&raw_time, &utc);

    int year = utc.tm_year + 1900;
    int month = utc.tm_mon + 1;
    int day = utc.tm_mday;
    int hour = utc.tm_hour;

    // DST State Trackers
    int standard_us_offset = 0, is_us_zone = 0;
    int standard_eu_offset = 0, is_eu_zone = 0;
    int standard_au_offset_h = 0, standard_au_offset_m = 0, is_au_zone = 0;

        // O(1) jump table via integer switch
    switch (current_tz_hash) {
        // --- PERMANENT / NO DST ZONES ---
        case hash_tz("Africa/Brazzaville"):   return posix_time + hourMinutesToSecond(1, 0);
        case hash_tz("Africa/Dakar"):         return posix_time + hourMinutesToSecond(0, 0);
        case hash_tz("Africa/Harare"):        return posix_time + hourMinutesToSecond(2, 0);
        case hash_tz("Africa/Nairobi"):       return posix_time + hourMinutesToSecond(3, 0);
        case hash_tz("Africa/Windhoek"):      return posix_time + hourMinutesToSecond(2, 0);

        case hash_tz("America/Anchorage"):     standard_us_offset = -9; is_us_zone = 1; break;
        case hash_tz("America/Asuncion"):     return posix_time + hourMinutesToSecond(-3, 0);
        case hash_tz("America/Barbados"):     return posix_time + hourMinutesToSecond(-4, 0);
        case hash_tz("America/Bogota"):       return posix_time + hourMinutesToSecond(-5, 0);
        case hash_tz("America/Buenos_Aires"): return posix_time + hourMinutesToSecond(-3, 0);
        case hash_tz("America/Caracas"):      return posix_time + hourMinutesToSecond(-4, 0);
        case hash_tz("America/Chicago"):       standard_us_offset = -6; is_us_zone = 1; break;
        case hash_tz("America/Chihuahua"):    return posix_time + hourMinutesToSecond(-6, 0);
        case hash_tz("America/Ciudad_Juarez"): standard_us_offset = -7; is_us_zone = 1; break;
        case hash_tz("America/Costa_Rica"):   return posix_time + hourMinutesToSecond(-6, 0);
        case hash_tz("America/Denver"):        standard_us_offset = -7; is_us_zone = 1; break;
        case hash_tz("America/Godthab"):   standard_eu_offset = -2; is_eu_zone = 1; break;
        case hash_tz("America/Halifax"):       standard_us_offset = -4; is_us_zone = 1; break;
        case hash_tz("America/Lima"):         return posix_time + hourMinutesToSecond(-5, 0);
        case hash_tz("America/Los_Angeles"):   standard_us_offset = -8; is_us_zone = 1; break;
        case hash_tz("America/Manaus"):       return posix_time + hourMinutesToSecond(-4, 0);
        case hash_tz("America/Mexico_City"):  return posix_time + hourMinutesToSecond(-6, 0);
        case hash_tz("America/Montevideo"):   return posix_time + hourMinutesToSecond(-3, 0);
        case hash_tz("America/New_York"):      standard_us_offset = -5; is_us_zone = 1; break;
        case hash_tz("America/Ojinaga"):       standard_us_offset = -6; is_us_zone = 1; break;
        case hash_tz("America/Phoenix"):      return posix_time + hourMinutesToSecond(-7, 0);
        case hash_tz("America/Recife"):       return posix_time + hourMinutesToSecond(-3, 0);
        case hash_tz("America/Regina"):       return posix_time + hourMinutesToSecond(-6, 0);
        case hash_tz("America/Sao_Paulo"):    return posix_time + hourMinutesToSecond(-3, 0);
        case hash_tz("America/Tijuana"):       standard_us_offset = -8; is_us_zone = 1; break;

        case hash_tz("Asia/Almaty"):          return posix_time + hourMinutesToSecond(5, 0);
        case hash_tz("Asia/Amman"):           return posix_time + hourMinutesToSecond(3, 0);
        case hash_tz("Asia/Baghdad"):         return posix_time + hourMinutesToSecond(3, 0);
        case hash_tz("Asia/Baku"):            return posix_time + hourMinutesToSecond(4, 0);
        case hash_tz("Asia/Bangkok"):         return posix_time + hourMinutesToSecond(7, 0);
        case hash_tz("Asia/Calcutta"):        return posix_time + hourMinutesToSecond(5, 30);
        case hash_tz("Asia/Colombo"):         return posix_time + hourMinutesToSecond(5, 30);
        case hash_tz("Asia/Dhaka"):           return posix_time + hourMinutesToSecond(6, 0);
        case hash_tz("Asia/Dubai"):           return posix_time + hourMinutesToSecond(4, 0);
        case hash_tz("Asia/Irkutsk"):         return posix_time + hourMinutesToSecond(8, 0);
        case hash_tz("Asia/Kabul"):           return posix_time + hourMinutesToSecond(4, 30);
        case hash_tz("Asia/Karachi"):         return posix_time + hourMinutesToSecond(5, 0);
        case hash_tz("Asia/Katmandu"):        return posix_time + hourMinutesToSecond(5, 45);
        case hash_tz("Asia/Krasnoyarsk"):     return posix_time + hourMinutesToSecond(7, 0);
        case hash_tz("Asia/Kuala_Lumpur"):    return posix_time + hourMinutesToSecond(8, 0);
        case hash_tz("Asia/Kuwait"):          return posix_time + hourMinutesToSecond(3, 0);
        case hash_tz("Asia/Magadan"):         return posix_time + hourMinutesToSecond(11, 0);
        case hash_tz("Asia/Oral"):            return posix_time + hourMinutesToSecond(5, 0);
        case hash_tz("Asia/Riyadh"):          return posix_time + hourMinutesToSecond(3, 0);
        case hash_tz("Asia/Seoul"):           return posix_time + hourMinutesToSecond(9, 0);
        case hash_tz("Asia/Shanghai"):        return posix_time + hourMinutesToSecond(8, 0);
        case hash_tz("Asia/Taipei"):          return posix_time + hourMinutesToSecond(8, 0);
        case hash_tz("Asia/Tbilisi"):         return posix_time + hourMinutesToSecond(4, 0);
        case hash_tz("Asia/Tehran"):          return posix_time + hourMinutesToSecond(3, 30);
        case hash_tz("Asia/Tokyo"):           return posix_time + hourMinutesToSecond(9, 0);
        case hash_tz("Asia/Vladivostok"):     return posix_time + hourMinutesToSecond(10, 0);
        case hash_tz("Asia/Yakutsk"):         return posix_time + hourMinutesToSecond(9, 0);
        case hash_tz("Asia/Yekaterinburg"):   return posix_time + hourMinutesToSecond(5, 0);
        case hash_tz("Asia/Yerevan"):         return posix_time + hourMinutesToSecond(4, 0);

        case hash_tz("Atlantic/Azores"):   standard_eu_offset = -1; is_eu_zone = 1; break;
        case hash_tz("Atlantic/Cape_Verde"):  return posix_time + hourMinutesToSecond(-1, 0);
        case hash_tz("Atlantic/Reykjavik"):   return posix_time + hourMinutesToSecond(0, 0);

        case hash_tz("Australia/Adelaide"): standard_au_offset_h = 9;  standard_au_offset_m = 30; is_au_zone = 1; break;
        case hash_tz("Australia/Brisbane"):   return posix_time + hourMinutesToSecond(10, 0);
        case hash_tz("Australia/Darwin"):     return posix_time + hourMinutesToSecond(9, 30);
        case hash_tz("Australia/Hobart"):   standard_au_offset_h = 10; standard_au_offset_m = 0;  is_au_zone = 1; break;
        case hash_tz("Australia/Perth"):      return posix_time + hourMinutesToSecond(8, 0);
        case hash_tz("Australia/Sydney"):   standard_au_offset_h = 10; standard_au_offset_m = 0;  is_au_zone = 1; break;

        case hash_tz("Europe/Athens"):     standard_eu_offset =  2; is_eu_zone = 1; break;
        case hash_tz("Europe/Belgrade"):   standard_eu_offset =  1; is_eu_zone = 1; break;
        case hash_tz("Europe/Berlin"):     standard_eu_offset =  1; is_eu_zone = 1; break;
        case hash_tz("Europe/Copenhagen"): standard_eu_offset =  1; is_eu_zone = 1; break;
        case hash_tz("Europe/Helsinki"):   standard_eu_offset =  2; is_eu_zone = 1; break;
        case hash_tz("Europe/Istanbul"):      return posix_time + hourMinutesToSecond(3, 0);
        case hash_tz("Europe/London"):     standard_eu_offset =  0; is_eu_zone = 1; break;
        case hash_tz("Europe/Madrid"):     standard_eu_offset =  1; is_eu_zone = 1; break;
        case hash_tz("Europe/Minsk"):         return posix_time + hourMinutesToSecond(3, 0);
        case hash_tz("Europe/Moscow"):        return posix_time + hourMinutesToSecond(3, 0);
        case hash_tz("Europe/Sarajevo"):   standard_eu_offset =  1; is_eu_zone = 1; break;

        case hash_tz("Pacific/Auckland"):   standard_au_offset_h = 12; standard_au_offset_m = 0;  is_au_zone = 1; break;
        case hash_tz("Pacific/Fiji"):         return posix_time + hourMinutesToSecond(12, 0);
        case hash_tz("Pacific/Guam"):         return posix_time + hourMinutesToSecond(10, 0);
        case hash_tz("Pacific/Honolulu"):     return posix_time + hourMinutesToSecond(-10, 0);
        case hash_tz("Pacific/Majuro"):       return posix_time + hourMinutesToSecond(12, 0);
        case hash_tz("Pacific/Midway"):       return posix_time + hourMinutesToSecond(-11, 0);
        case hash_tz("Pacific/Noumea"):       return posix_time + hourMinutesToSecond(11, 0);
        case hash_tz("Pacific/Tongatapu"):    return posix_time + hourMinutesToSecond(13, 0);
    }

    // Process dynamic zones based on flags set in the switch statement
    if (is_us_zone) {
        int march_dst_day = 14 - get_day_of_week(year, 3, 1);
        int nov_dst_day = 7 - get_day_of_week(year, 11, 1);
        int is_dst = 0;
        if (month > 3 && month < 11) is_dst = 1;
        else if (month == 3 && (day > march_dst_day || (day == march_dst_day && hour >= 7))) is_dst = 1;
        else if (month == 11 && (day < nov_dst_day || (day == nov_dst_day && hour < 6))) is_dst = 1;
        return posix_time + hourMinutesToSecond(standard_us_offset + (is_dst ? 1 : 0), 0);
    }

    if (is_eu_zone) {
        int march_last_sun = 31 - get_day_of_week(year, 3, 31);
        int oct_last_sun = 31 - get_day_of_week(year, 10, 31);
        int is_dst = 0;
        if (month > 3 && month < 10) is_dst = 1;
        else if (month == 3 && (day > march_last_sun || (day == march_last_sun && hour >= 1))) is_dst = 1;
        else if (month == 10 && (day < oct_last_sun || (day == oct_last_sun && hour < 1))) is_dst = 1;
        return posix_time + hourMinutesToSecond(standard_eu_offset + (is_dst ? 1 : 0), 0);
    }

    if (is_au_zone) {
        int apr_first_sun = 7 - get_day_of_week(year, 4, 1);
        int oct_first_sun = 7 - get_day_of_week(year, 10, 1);
        int is_dst = 0;
        if (month < 4 || month > 10) is_dst = 1;
        else if (month == 4 && (day < apr_first_sun || (day == apr_first_sun && hour < 15))) is_dst = 1;
        else if (month == 10 && (day > oct_first_sun || (day == oct_first_sun && hour >= 16))) is_dst = 1;
        return posix_time + hourMinutesToSecond(standard_au_offset_h + (is_dst ? 1 : 0), standard_au_offset_m);
    }

    // Special case for St. Johns, as it wasn't grouped easily into the base integer flags
    if (current_tz_hash == hash_tz("America/St_Johns")) {
        int march_dst_day = 14 - get_day_of_week(year, 3, 1);
        int nov_dst_day = 7 - get_day_of_week(year, 11, 1);
        int is_dst = (month > 3 && month < 11) || 
                     (month == 3 && (day > march_dst_day || (day == march_dst_day && hour >= 7))) || 
                     (month == 11 && (day < nov_dst_day || (day == nov_dst_day && hour < 6)));
        return posix_time + (is_dst ? hourMinutesToSecond(-2, -30) : hourMinutesToSecond(-3, -30));
    }

    return 0; // Fallback
}

//It checks only to first unique byte, currently you can save 1400 bytes at -O3 with it.
extern "C" time_t getLocalPosixTimeUnsafe(time_t posix_time, TimeLocationName* name) {
    const time_t last_valid_posix_time = 1779460399; // May 22, 2026
    if (posix_time < last_valid_posix_time) return 0;

    const char* m_name = name->name;

    time_t raw_time = (time_t)posix_time;
    struct tm utc;
    gmtime_r(&raw_time, &utc);

    int year = utc.tm_year + 1900;
    int month = utc.tm_mon + 1;
    int day = utc.tm_mday;
    int hour = utc.tm_hour;

    // DST State Trackers
    int standard_us_offset = 0, is_us_zone = 0;
    int standard_eu_offset = 0, is_eu_zone = 0;
    int standard_au_offset_h = 0, standard_au_offset_m = 0, is_au_zone = 0;

    switch(m_name[0]) {
        case 'A': {
            switch(m_name[1]) {
                case 'f': { //Africa
                    if (m_name[7] == 'B') return posix_time + hourMinutesToSecond(1, 0);
                    if (m_name[7] == 'D') return posix_time;
                    if (m_name[7] == 'H') return posix_time + hourMinutesToSecond(2, 0);
                    if (m_name[7] == 'N') return posix_time + hourMinutesToSecond(3, 0);
                    if (m_name[7] == 'W') return posix_time + hourMinutesToSecond(1, 0);
                    break;
                }
                case 'm': { //America
                    if (m_name[8] == 'A' && m_name[9] == 'n') {standard_us_offset = -9; is_us_zone = 1; break;}
                    if (m_name[8] == 'A' && m_name[9] == 's') return posix_time + hourMinutesToSecond(-3, 0);
                    if (m_name[8] == 'B' && m_name[9] == 'a') return posix_time + hourMinutesToSecond(-4, 0);
                    if (m_name[8] == 'B' && m_name[9] == 'o') return posix_time + hourMinutesToSecond(-5, 0);
                    if (m_name[8] == 'B' && m_name[9] == 'u') return posix_time + hourMinutesToSecond(-3, 0);
                    if (m_name[8] == 'C' && m_name[9] == 'a') return posix_time + hourMinutesToSecond(-4, 0);
                    if (m_name[8] == 'C' && m_name[9] == 'h' && m_name[10] == 'i' && m_name[11] == 'c') {standard_us_offset = -6; is_us_zone = 1; break;}
                    if (m_name[8] == 'C' && m_name[9] == 'h' && m_name[10] == 'i' && m_name[11] == 'h') return posix_time + hourMinutesToSecond(-6, 0);
                    if (m_name[8] == 'C' && m_name[9] == 'i') {standard_us_offset = -7; is_us_zone = 1; break;}
                    if (m_name[8] == 'C' && m_name[9] == 'o') return posix_time + hourMinutesToSecond(-6, 0);
                    if (m_name[8] == 'D') {standard_us_offset = -7; is_us_zone = 1; break;}
                    if (m_name[8] == 'G') {standard_eu_offset = -2; is_eu_zone = 1; break;}
                    if (m_name[8] == 'H') {standard_us_offset = -4; is_us_zone = 1; break;}
                    if (m_name[8] == 'L' && m_name[9] == 'i') return posix_time + hourMinutesToSecond(-5, 0);
                    if (m_name[8] == 'L' && m_name[9] == 'o') {standard_us_offset = -8; is_us_zone = 1; break;}
                    if (m_name[8] == 'M' && m_name[9] == 'a') return posix_time + hourMinutesToSecond(-4, 0);
                    if (m_name[8] == 'M' && m_name[9] == 'e') return posix_time + hourMinutesToSecond(-6, 0);
                    if (m_name[8] == 'M' && m_name[9] == 'o') return posix_time + hourMinutesToSecond(-3, 0);
                    if (m_name[8] == 'N') {standard_us_offset = -5; is_us_zone = 1; break;}
                    if (m_name[8] == 'O') {standard_us_offset = -6; is_us_zone = 1; break;}
                    if (m_name[8] == 'P') return posix_time + hourMinutesToSecond(-7, 0);
                    if (m_name[8] == 'R' && m_name[9] == 'e' && m_name[10] == 'c') return posix_time + hourMinutesToSecond(-3, 0);
                    if (m_name[8] == 'R' && m_name[9] == 'e' && m_name[10] == 'g') return posix_time + hourMinutesToSecond(-6, 0);
                    if (m_name[8] == 'S' && m_name[8] == 'a') return posix_time + hourMinutesToSecond(-3, 0);
                    if (m_name[8] == 'S' && m_name[8] == 't') {
                        int march_dst_day = 14 - get_day_of_week(year, 3, 1);
                        int nov_dst_day = 7 - get_day_of_week(year, 11, 1);
                        int is_dst = (month > 3 && month < 11) || 
                                    (month == 3 && (day > march_dst_day || (day == march_dst_day && hour >= 7))) || 
                                    (month == 11 && (day < nov_dst_day || (day == nov_dst_day && hour < 6)));
                        return posix_time + (is_dst ? hourMinutesToSecond(-2, -30) : hourMinutesToSecond(-3, -30));
                    }
                    if (m_name[8] == 'T') {standard_us_offset = -8; is_us_zone = 1; break;}
                    break;
                }
                case 's': { //Asia
                    if (m_name[5] == 'A' && m_name[6] == 'l') return posix_time + hourMinutesToSecond(5, 0);
                    if (m_name[5] == 'A' && m_name[6] == 'n') return posix_time + hourMinutesToSecond(3, 0);
                    if (m_name[5] == 'B' && m_name[6] == 'a' && m_name[7] == 'g') return posix_time + hourMinutesToSecond(3, 0);
                    if (m_name[5] == 'B' && m_name[6] == 'a' && m_name[7] == 'k') return posix_time + hourMinutesToSecond(4, 0);
                    if (m_name[5] == 'B' && m_name[6] == 'a' && m_name[7] == 'n') return posix_time + hourMinutesToSecond(7, 0);
                    if (m_name[5] == 'C') return posix_time + hourMinutesToSecond(5, 30);
                    if (m_name[5] == 'D' && m_name[6] == 'h') return posix_time + hourMinutesToSecond(6, 0);
                    if (m_name[5] == 'D' && m_name[6] == 'u') return posix_time + hourMinutesToSecond(4, 0);
                    if (m_name[5] == 'I') return posix_time + hourMinutesToSecond(8, 0);
                    if (m_name[5] == 'K' && m_name[6] == 'a' && m_name[7] == 'b') return posix_time + hourMinutesToSecond(4, 30);
                    if (m_name[5] == 'K' && m_name[6] == 'a' && m_name[7] == 'r') return posix_time + hourMinutesToSecond(5, 0);
                    if (m_name[5] == 'K' && m_name[6] == 'a' && m_name[7] == 't') return posix_time + hourMinutesToSecond(5, 45);
                    if (m_name[5] == 'K' && m_name[6] == 'r') return posix_time + hourMinutesToSecond(7, 0);
                    if (m_name[5] == 'K' && m_name[6] == 'u' && m_name[7] == 'a') return posix_time + hourMinutesToSecond(8, 0);
                    if (m_name[5] == 'K' && m_name[6] == 'u' && m_name[7] == 'w') return posix_time + hourMinutesToSecond(3, 0);
                    if (m_name[5] == 'M') return posix_time + hourMinutesToSecond(11, 0);
                    if (m_name[5] == 'O') return posix_time + hourMinutesToSecond(5, 0);
                    if (m_name[5] == 'R') return posix_time + hourMinutesToSecond(3, 0);
                    if (m_name[5] == 'S' && m_name[6] == 'e') return posix_time + hourMinutesToSecond(9, 0);
                    if (m_name[5] == 'S' && m_name[6] == 'h') return posix_time + hourMinutesToSecond(8, 0);
                    if (m_name[5] == 'T' && m_name[6] == 'a') return posix_time + hourMinutesToSecond(8, 0);
                    if (m_name[5] == 'T' && m_name[6] == 'b') return posix_time + hourMinutesToSecond(4, 0);
                    if (m_name[5] == 'T' && m_name[6] == 'e') return posix_time + hourMinutesToSecond(3, 30);
                    if (m_name[5] == 'T' && m_name[6] == 'o') return posix_time + hourMinutesToSecond(9, 0);
                    if (m_name[5] == 'V') return posix_time + hourMinutesToSecond(10, 0);
                    if (m_name[5] == 'Y' && m_name[6] == 'a') return posix_time + hourMinutesToSecond(9, 0);
                    if (m_name[5] == 'Y' && m_name[6] == 'e' && m_name[7] == 'k') return posix_time + hourMinutesToSecond(5, 0);
                    if (m_name[5] == 'Y' && m_name[6] == 'e' && m_name[7] == 'r') return posix_time + hourMinutesToSecond(4, 0);
                    break;
                }
                case 't': { //Atlantic
                    if (m_name[9] == 'A') {standard_eu_offset = -1; is_eu_zone = 1; break;}
                    if (m_name[9] == 'C') return posix_time + hourMinutesToSecond(-1, 0);
                    if (m_name[9] == 'R') return posix_time;
                    break; 
                }
                case 'u': { //Australia
                    if (m_name[10] == 'A') {standard_au_offset_h = 9;  standard_au_offset_m = 30; is_au_zone = 1; break;}
                    if (m_name[10] == 'B') return posix_time + hourMinutesToSecond(10, 0);
                    if (m_name[10] == 'D') return posix_time + hourMinutesToSecond(9, 30);
                    if (m_name[10] == 'H' || m_name[10] == 'S') {standard_au_offset_h = 10; standard_au_offset_m = 0;  is_au_zone = 1; break;}
                    if (m_name[10] == 'P') return posix_time + hourMinutesToSecond(8, 0);
                    break;
                }
            }
            break;
        }
        case 'E': { //Europe
            switch(m_name[7]) {
                case 'A': 
                case 'H': standard_eu_offset = 2; is_eu_zone = 1; break;
                case 'B':
                case 'C': 
                case 'M':
                    if (m_name[8] == 'a') {standard_eu_offset =  1; is_eu_zone = 1; break;}
                    if (m_name[8] == 'i' || m_name[8] == 'o') return posix_time + hourMinutesToSecond(3, 0);
                    break;
                case 'S': standard_eu_offset = 1; is_eu_zone = 1; break;
                case 'I': 
                case 'L': standard_eu_offset = 0; is_eu_zone = 1; break;
            }
            break;
        }
        case 'P': { //Pacific
            switch(m_name[8]) {
                case 'A': standard_au_offset_h = 12; standard_au_offset_m = 0;  is_au_zone = 1; break;
                case 'F': return posix_time + hourMinutesToSecond(12, 0);
                case 'G': return posix_time + hourMinutesToSecond(10, 0);
                case 'H': return posix_time + hourMinutesToSecond(-10, 0);
                case 'M':
                    if (m_name[9] == 'a') return posix_time + hourMinutesToSecond(12, 0);
                    if (m_name[9] == 'i') return posix_time + hourMinutesToSecond(-11, 0);
                    break;
                case 'N': return posix_time + hourMinutesToSecond(11, 0);
                case 'T': return posix_time + hourMinutesToSecond(13, 0);
            }
            break;
        }
    }

    // Process dynamic zones based on flags set in the switch statement
    if (is_us_zone) {
        int march_dst_day = 14 - get_day_of_week(year, 3, 1);
        int nov_dst_day = 7 - get_day_of_week(year, 11, 1);
        int is_dst = 0;
        if (month > 3 && month < 11) is_dst = 1;
        else if (month == 3 && (day > march_dst_day || (day == march_dst_day && hour >= 7))) is_dst = 1;
        else if (month == 11 && (day < nov_dst_day || (day == nov_dst_day && hour < 6))) is_dst = 1;
        return posix_time + hourMinutesToSecond(standard_us_offset + (is_dst ? 1 : 0), 0);
    }

    if (is_eu_zone) {
        int march_last_sun = 31 - get_day_of_week(year, 3, 31);
        int oct_last_sun = 31 - get_day_of_week(year, 10, 31);
        int is_dst = 0;
        if (month > 3 && month < 10) is_dst = 1;
        else if (month == 3 && (day > march_last_sun || (day == march_last_sun && hour >= 1))) is_dst = 1;
        else if (month == 10 && (day < oct_last_sun || (day == oct_last_sun && hour < 1))) is_dst = 1;
        return posix_time + hourMinutesToSecond(standard_eu_offset + (is_dst ? 1 : 0), 0);
    }

    if (is_au_zone) {
        int apr_first_sun = 7 - get_day_of_week(year, 4, 1);
        int oct_first_sun = 7 - get_day_of_week(year, 10, 1);
        int is_dst = 0;
        if (month < 4 || month > 10) is_dst = 1;
        else if (month == 4 && (day < apr_first_sun || (day == apr_first_sun && hour < 15))) is_dst = 1;
        else if (month == 10 && (day > oct_first_sun || (day == oct_first_sun && hour >= 16))) is_dst = 1;
        return posix_time + hourMinutesToSecond(standard_au_offset_h + (is_dst ? 1 : 0), standard_au_offset_m);
    }

    return 0; // Fallback
}

constexpr uint32_t hashes[] = {
    hash_tz("Africa/Brazzaville"),
    hash_tz("Africa/Casablanca"),
    hash_tz("Africa/Dakar"),
    hash_tz("Africa/Harare"),
    hash_tz("Africa/Nairobi"),
    hash_tz("Africa/Windhoek"),
    hash_tz("Afrika/Cairo"),
    hash_tz("America/Anchorage"),
    hash_tz("America/Asuncion"),
    hash_tz("America/Barbados"),
    hash_tz("America/Bogota"),
    hash_tz("America/Buenos_Aires"),
    hash_tz("America/Caracas"),
    hash_tz("America/Chicago"),
    hash_tz("America/Chihuahua"),
    hash_tz("America/Ciudad_Juarez"),
    hash_tz("America/Costa_Rica"),
    hash_tz("America/Denver"),
    hash_tz("America/Godthab"),
    hash_tz("America/Halifax"),
    hash_tz("America/Lima"),
    hash_tz("America/Los_Angeles"),
    hash_tz("America/Manaus"),
    hash_tz("America/Mexico_City"),
    hash_tz("America/Montevideo"),
    hash_tz("America/New_York"),
    hash_tz("America/Ojinaga"),
    hash_tz("America/Phoenix"),
    hash_tz("America/Recife"),
    hash_tz("America/Regina"),
    hash_tz("America/Santiago"),
    hash_tz("America/Sao_Paulo"),
    hash_tz("America/South_Georgia"),
    hash_tz("America/St_Johns"),
    hash_tz("America/Tijuana"),
    hash_tz("Asia/Almaty"),
    hash_tz("Asia/Amman"),
    hash_tz("Asia/Baghdad"),
    hash_tz("Asia/Baku"),
    hash_tz("Asia/Bangkok"),
    hash_tz("Asia/Beirut"),
    hash_tz("Asia/Calcutta"),
    hash_tz("Asia/Colombo"),
    hash_tz("Asia/Dhaka"),
    hash_tz("Asia/Dubai"),
    hash_tz("Asia/Irkutsk"),
    hash_tz("Asia/Jerusalem"),
    hash_tz("Asia/Kabul"),
    hash_tz("Asia/Karachi"),
    hash_tz("ASia/Katmandu"),
    hash_tz("Asia/Krasnoyarsk"),
    hash_tz("Asia/Kuala_Lumpur"),
    hash_tz("Asia/Kuwait"),
    hash_tz("Asia/Magadan"),
    hash_tz("Asia/Oral"),
    hash_tz("Asia/Riyadh"),
    hash_tz("Asia/Seoul"),
    hash_tz("Asia/Shanghai"),
    hash_tz("Asia/Taipei"),
    hash_tz("Asia/Tbilisi"),
    hash_tz("Asia/Tehran"),
    hash_tz("Asia/Tokyo"),
    hash_tz("Asia/Vladivostok"),
    hash_tz("Asia/Yakutsk"),
    hash_tz("Asia/Yekaterinburg"),
    hash_tz("Asia/Yerevan"),
    hash_tz("Atlantic/Azores"),
    hash_tz("Atlantic/Cape_Verde"),
    hash_tz("Atlantic/Reykjavik"),
    hash_tz("Australia/Adelaide"),
    hash_tz("Australia/Brisbane"),
    hash_tz("Australia/Darwin"),
    hash_tz("Australia/Hobart"),
    hash_tz("Australia/Perth"),
    hash_tz("Australia/Sydney"),
    hash_tz("Europe/Athens"),
    hash_tz("Europe/Belgrade"),
    hash_tz("Europe/Berlin"),
    hash_tz("Europe/Copenhagen"),
    hash_tz("Europe/Helsinki"),
    hash_tz("Europe/Istanbul"),
    hash_tz("Europe/London"),
    hash_tz("Europe/Madrid"),
    hash_tz("Europe/Minsk"),
    hash_tz("Europe/Moscow"),
    hash_tz("Europe/Sarajevo"),
    hash_tz("Pacific/Auckland"),
    hash_tz("Pacific/Fiji"),
    hash_tz("Pacific/Guam"),
    hash_tz("Pacific/Honolulu"),
    hash_tz("Pacific/Majuro"),
    hash_tz("Pacific/Midway"),
    hash_tz("Pacific/Noumea"),
    hash_tz("Pacific/Tongatapu")
};

template <typename T> constexpr bool has_duplicates(const T *array, std::size_t size)
{
    for (std::size_t i = 1; i < size; i++)
        for (std::size_t j = 0; j < i; j++)
            if (array[i] == array[j]) {
                return true;
            }
    return false;
}

static_assert(!has_duplicates(hashes, std::size(hashes)), "Detected repeated hash!");
