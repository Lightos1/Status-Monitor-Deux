#include <switch.h>
#include <string.h>
#include <time.h>
#include <array>
#include "timeOffsetter.h"

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

extern "C" TimeZone convertTimeLocationNameToTimeZone(TimeLocationName* name) {
    const char* m_name = name->name;
    uint32_t current_hash_tz = hash_tz(m_name);
    
    switch(current_hash_tz) {
        case hash_tz("Africa/Brazzaville"): return TimeZone_AfricaBrazzaville;
        case hash_tz("Africa/Casablanca"): return TimeZone_AfricaCasablanca;
        case hash_tz("Africa/Dakar"): return TimeZone_AfricaDakar;
        case hash_tz("Africa/Harare"): return TimeZone_AfricaHarare;
        case hash_tz("Africa/Nairobi"): return TimeZone_AfricaNairobi;
        case hash_tz("Africa/Windhoek"): return TimeZone_AfricaWindhoek;
        case hash_tz("Africa/Cairo"): return TimeZone_AfricaCairo;
        case hash_tz("America/Anchorage"): return TimeZone_AmericaAnchorage;
        case hash_tz("America/Asuncion"): return TimeZone_AmericaAsuncion;
        case hash_tz("America/Barbados"): return TimeZone_AmericaBarbados;
        case hash_tz("America/Bogota"): return TimeZone_AmericaBogota;
        case hash_tz("America/Buenos_Aires"): return TimeZone_AmericaBuenos_Aires;
        case hash_tz("America/Caracas"): return TimeZone_AmericaCaracas;
        case hash_tz("America/Chicago"): return TimeZone_AmericaChicago;
        case hash_tz("America/Chihuahua"): return TimeZone_AmericaChihuahua;
        case hash_tz("America/Ciudad_Juarez"): return TimeZone_AmericaCiudad_Juarez;
        case hash_tz("America/Costa_Rica"): return TimeZone_AmericaCosta_Rica;
        case hash_tz("America/Denver"): return TimeZone_AmericaDenver;
        case hash_tz("America/Godthab"): return TimeZone_AmericaGodthab;
        case hash_tz("America/Halifax"): return TimeZone_AmericaHalifax;
        case hash_tz("America/Lima"): return TimeZone_AmericaLima;
        case hash_tz("America/Los_Angeles"): return TimeZone_AmericaLos_Angeles;
        case hash_tz("America/Manaus"): return TimeZone_AmericaManaus;
        case hash_tz("America/Mexico_City"): return TimeZone_AmericaMexico_City;
        case hash_tz("America/Montevideo"): return TimeZone_AmericaMontevideo;
        case hash_tz("America/New_York"): return TimeZone_AmericaNew_York;
        case hash_tz("America/Ojinaga"): return TimeZone_AmericaOjinaga;
        case hash_tz("America/Phoenix"): return TimeZone_AmericaPhoenix;
        case hash_tz("America/Recife"): return TimeZone_AmericaRecife;
        case hash_tz("America/Regina"): return TimeZone_AmericaRegina;
        case hash_tz("America/Santiago"): return TimeZone_AmericaSantiago;
        case hash_tz("America/Sao_Paulo"): return TimeZone_AmericaSao_Paulo;
        case hash_tz("America/South_Georgia"): return TimeZone_AmericaSouth_Georgia;
        case hash_tz("America/St_Johns"): return TimeZone_AmericaSt_Johns;
        case hash_tz("America/Tijuana"): return TimeZone_AmericaTijuana;
        case hash_tz("Asia/Almaty"): return TimeZone_AsiaAlmaty;
        case hash_tz("Asia/Amman"): return TimeZone_AsiaAmman;
        case hash_tz("Asia/Baghdad"): return TimeZone_AsiaBaghdad;
        case hash_tz("Asia/Baku"): return TimeZone_AsiaBaku;
        case hash_tz("Asia/Bangkok"): return TimeZone_AsiaBangkok;
        case hash_tz("Asia/Beirut"): return TimeZone_AsiaBeirut;
        case hash_tz("Asia/Calcutta"): return TimeZone_AsiaCalcutta;
        case hash_tz("Asia/Colombo"): return TimeZone_AsiaColombo;
        case hash_tz("Asia/Dhaka"): return TimeZone_AsiaDhaka;
        case hash_tz("Asia/Dubai"): return TimeZone_AsiaDubai;
        case hash_tz("Asia/Irkutsk"): return TimeZone_AsiaIrkutsk;
        case hash_tz("Asia/Jerusalem"): return TimeZone_AsiaJerusalem;
        case hash_tz("Asia/Kabul"): return TimeZone_AsiaKabul;
        case hash_tz("Asia/Karachi"): return TimeZone_AsiaKarachi;
        case hash_tz("ASia/Katmandu"): return TimeZone_AsiaKatmandu;
        case hash_tz("Asia/Krasnoyarsk"): return TimeZone_AsiaKrasnoyarsk;
        case hash_tz("Asia/Kuala_Lumpur"): return TimeZone_AsiaKuala_Lumpur;
        case hash_tz("Asia/Kuwait"): return TimeZone_AsiaKuwait;
        case hash_tz("Asia/Magadan"): return TimeZone_AsiaMagadan;
        case hash_tz("Asia/Oral"): return TimeZone_AsiaOral;
        case hash_tz("Asia/Riyadh"): return TimeZone_AsiaRiyadh;
        case hash_tz("Asia/Seoul"): return TimeZone_AsiaSeoul;
        case hash_tz("Asia/Shanghai"): return TimeZone_AsiaShanghai;
        case hash_tz("Asia/Taipei"): return TimeZone_AsiaTaipei;
        case hash_tz("Asia/Tbilisi"): return TimeZone_AsiaTbilisi;
        case hash_tz("Asia/Tehran"): return TimeZone_AsiaTehran;
        case hash_tz("Asia/Tokyo"): return TimeZone_AsiaTokyo;
        case hash_tz("Asia/Vladivostok"): return TimeZone_AsiaVladivostok;
        case hash_tz("Asia/Yakutsk"): return TimeZone_AsiaYakutsk;
        case hash_tz("Asia/Yekaterinburg"): return TimeZone_AsiaYekaterinburg;
        case hash_tz("Asia/Yerevan"): return TimeZone_AsiaYerevan;
        case hash_tz("Atlantic/Azores"): return TimeZone_AtlanticAzores;
        case hash_tz("Atlantic/Cape_Verde"): return TimeZone_AtlanticCape_Verde;
        case hash_tz("Atlantic/Reykjavik"): return TimeZone_AtlanticReykjavik;
        case hash_tz("Australia/Adelaide"): return TimeZone_AustraliaAdelaide;
        case hash_tz("Australia/Brisbane"): return TimeZone_AustraliaBrisbane;
        case hash_tz("Australia/Darwin"): return TimeZone_AustraliaDarwin;
        case hash_tz("Australia/Hobart"): return TimeZone_AustraliaHobart;
        case hash_tz("Australia/Perth"): return TimeZone_AustraliaPerth;
        case hash_tz("Australia/Sydney"): return TimeZone_AustraliaSydney;
        case hash_tz("Europe/Athens"): return TimeZone_EuropeAthens;
        case hash_tz("Europe/Belgrade"): return TimeZone_EuropeBelgrade;
        case hash_tz("Europe/Berlin"): return TimeZone_EuropeBerlin;
        case hash_tz("Europe/Copenhagen"): return TimeZone_EuropeCopenhagen;
        case hash_tz("Europe/Helsinki"): return TimeZone_EuropeHelsinki;
        case hash_tz("Europe/Istanbul"): return TimeZone_EuropeIstanbul;
        case hash_tz("Europe/London"): return TimeZone_EuropeLondon;
        case hash_tz("Europe/Madrid"): return TimeZone_EuropeMadrid;
        case hash_tz("Europe/Minsk"): return TimeZone_EuropeMinsk;
        case hash_tz("Europe/Moscow"): return TimeZone_EuropeMoscow;
        case hash_tz("Europe/Sarajevo"): return TimeZone_EuropeSarajevo;
        case hash_tz("Pacific/Auckland"): return TimeZone_PacificAuckland;
        case hash_tz("Pacific/Fiji"): return TimeZone_PacificFiji;
        case hash_tz("Pacific/Guam"): return TimeZone_PacificGuam;
        case hash_tz("Pacific/Honolulu"): return TimeZone_PacificHonolulu;
        case hash_tz("Pacific/Majuro"): return TimeZone_PacificMajuro;
        case hash_tz("Pacific/Midway"): return TimeZone_PacificMidway;
        case hash_tz("Pacific/Noumea"): return TimeZone_PacificNoumea;
        case hash_tz("Pacific/Tongatapu"): return TimeZone_PacificTongatapu;
        default: return TimeZone_Invalid;
    }
}

extern "C" time_t getLocalPosixTime(time_t posix_time, TimeZone name) {
    const time_t last_valid_posix_time = 1779460399; // May 22, 2026
    if (posix_time < last_valid_posix_time) return 0;

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
    switch (name) {
        // --- PERMANENT / NO DST ZONES ---
		case TimeZone_AfricaBrazzaville:   return posix_time + hourMinutesToSecond(1, 0);
		case TimeZone_AfricaDakar:         return posix_time + hourMinutesToSecond(0, 0);
		case TimeZone_AfricaHarare:        return posix_time + hourMinutesToSecond(2, 0);
		case TimeZone_AfricaNairobi:       return posix_time + hourMinutesToSecond(3, 0);
		case TimeZone_AfricaWindhoek:      return posix_time + hourMinutesToSecond(2, 0);

		case TimeZone_AmericaAnchorage:    standard_us_offset = -9; is_us_zone = 1; break;
		case TimeZone_AmericaAsuncion:     return posix_time + hourMinutesToSecond(-3, 0);
		case TimeZone_AmericaBarbados:     return posix_time + hourMinutesToSecond(-4, 0);
		case TimeZone_AmericaBogota:       return posix_time + hourMinutesToSecond(-5, 0);
		case TimeZone_AmericaBuenos_Aires: return posix_time + hourMinutesToSecond(-3, 0);
		case TimeZone_AmericaCaracas:      return posix_time + hourMinutesToSecond(-4, 0);
		case TimeZone_AmericaChicago:      standard_us_offset = -6; is_us_zone = 1; break;
		case TimeZone_AmericaChihuahua:    return posix_time + hourMinutesToSecond(-6, 0);
		case TimeZone_AmericaCiudad_Juarez:standard_us_offset = -7; is_us_zone = 1; break;
		case TimeZone_AmericaCosta_Rica:   return posix_time + hourMinutesToSecond(-6, 0);
		case TimeZone_AmericaDenver:       standard_us_offset = -7; is_us_zone = 1; break;
		case TimeZone_AmericaGodthab:      standard_eu_offset = -2; is_eu_zone = 1; break;
		case TimeZone_AmericaHalifax:      standard_us_offset = -4; is_us_zone = 1; break;
		case TimeZone_AmericaLima:         return posix_time + hourMinutesToSecond(-5, 0);
		case TimeZone_AmericaLos_Angeles:  standard_us_offset = -8; is_us_zone = 1; break;
		case TimeZone_AmericaManaus:       return posix_time + hourMinutesToSecond(-4, 0);
		case TimeZone_AmericaMexico_City:  return posix_time + hourMinutesToSecond(-6, 0);
		case TimeZone_AmericaMontevideo:   return posix_time + hourMinutesToSecond(-3, 0);
		case TimeZone_AmericaNew_York:     standard_us_offset = -5; is_us_zone = 1; break;
		case TimeZone_AmericaOjinaga:      standard_us_offset = -6; is_us_zone = 1; break;
		case TimeZone_AmericaPhoenix:      return posix_time + hourMinutesToSecond(-7, 0);
		case TimeZone_AmericaRecife:       return posix_time + hourMinutesToSecond(-3, 0);
		case TimeZone_AmericaRegina:       return posix_time + hourMinutesToSecond(-6, 0);
		case TimeZone_AmericaSao_Paulo:    return posix_time + hourMinutesToSecond(-3, 0);
		case TimeZone_AmericaTijuana:      standard_us_offset = -8; is_us_zone = 1; break;

		case TimeZone_AsiaAlmaty:          return posix_time + hourMinutesToSecond(5, 0);
		case TimeZone_AsiaAmman:           return posix_time + hourMinutesToSecond(3, 0);
		case TimeZone_AsiaBaghdad:         return posix_time + hourMinutesToSecond(3, 0);
		case TimeZone_AsiaBaku:            return posix_time + hourMinutesToSecond(4, 0);
		case TimeZone_AsiaBangkok:         return posix_time + hourMinutesToSecond(7, 0);
		case TimeZone_AsiaCalcutta:        return posix_time + hourMinutesToSecond(5, 30);
		case TimeZone_AsiaColombo:         return posix_time + hourMinutesToSecond(5, 30);
		case TimeZone_AsiaDhaka:           return posix_time + hourMinutesToSecond(6, 0);
		case TimeZone_AsiaDubai:           return posix_time + hourMinutesToSecond(4, 0);
		case TimeZone_AsiaIrkutsk:         return posix_time + hourMinutesToSecond(8, 0);
		case TimeZone_AsiaKabul:           return posix_time + hourMinutesToSecond(4, 30);
		case TimeZone_AsiaKarachi:         return posix_time + hourMinutesToSecond(5, 0);
		case TimeZone_AsiaKatmandu:        return posix_time + hourMinutesToSecond(5, 45);
		case TimeZone_AsiaKrasnoyarsk:     return posix_time + hourMinutesToSecond(7, 0);
		case TimeZone_AsiaKuala_Lumpur:    return posix_time + hourMinutesToSecond(8, 0);
		case TimeZone_AsiaKuwait:          return posix_time + hourMinutesToSecond(3, 0);
		case TimeZone_AsiaMagadan:         return posix_time + hourMinutesToSecond(11, 0);
		case TimeZone_AsiaOral:            return posix_time + hourMinutesToSecond(5, 0);
		case TimeZone_AsiaRiyadh:          return posix_time + hourMinutesToSecond(3, 0);
		case TimeZone_AsiaSeoul:           return posix_time + hourMinutesToSecond(9, 0);
		case TimeZone_AsiaShanghai:        return posix_time + hourMinutesToSecond(8, 0);
		case TimeZone_AsiaTaipei:          return posix_time + hourMinutesToSecond(8, 0);
		case TimeZone_AsiaTbilisi:         return posix_time + hourMinutesToSecond(4, 0);
		case TimeZone_AsiaTehran:          return posix_time + hourMinutesToSecond(3, 30);
		case TimeZone_AsiaTokyo:           return posix_time + hourMinutesToSecond(9, 0);
		case TimeZone_AsiaVladivostok:     return posix_time + hourMinutesToSecond(10, 0);
		case TimeZone_AsiaYakutsk:         return posix_time + hourMinutesToSecond(9, 0);
		case TimeZone_AsiaYekaterinburg:   return posix_time + hourMinutesToSecond(5, 0);
		case TimeZone_AsiaYerevan:         return posix_time + hourMinutesToSecond(4, 0);

		case TimeZone_AtlanticAzores:      standard_eu_offset = -1; is_eu_zone = 1; break;
		case TimeZone_AtlanticCape_Verde:  return posix_time + hourMinutesToSecond(-1, 0);
		case TimeZone_AtlanticReykjavik:   return posix_time + hourMinutesToSecond(0, 0);

		case TimeZone_AustraliaAdelaide:   standard_au_offset_h = 9;  standard_au_offset_m = 30; is_au_zone = 1; break;
		case TimeZone_AustraliaBrisbane:   return posix_time + hourMinutesToSecond(10, 0);
		case TimeZone_AustraliaDarwin:     return posix_time + hourMinutesToSecond(9, 30);
		case TimeZone_AustraliaHobart:     standard_au_offset_h = 10; standard_au_offset_m = 0;  is_au_zone = 1; break;
		case TimeZone_AustraliaPerth:      return posix_time + hourMinutesToSecond(8, 0);
		case TimeZone_AustraliaSydney:     standard_au_offset_h = 10; standard_au_offset_m = 0;  is_au_zone = 1; break;

		case TimeZone_EuropeAthens:        standard_eu_offset =  2; is_eu_zone = 1; break;
		case TimeZone_EuropeBelgrade:      standard_eu_offset =  1; is_eu_zone = 1; break;
		case TimeZone_EuropeBerlin:        standard_eu_offset =  1; is_eu_zone = 1; break;
		case TimeZone_EuropeCopenhagen:    standard_eu_offset =  1; is_eu_zone = 1; break;
		case TimeZone_EuropeHelsinki:      standard_eu_offset =  2; is_eu_zone = 1; break;
		case TimeZone_EuropeIstanbul:      return posix_time + hourMinutesToSecond(3, 0);
		case TimeZone_EuropeLondon:        standard_eu_offset =  0; is_eu_zone = 1; break;
		case TimeZone_EuropeMadrid:        standard_eu_offset =  1; is_eu_zone = 1; break;
		case TimeZone_EuropeMinsk:         return posix_time + hourMinutesToSecond(3, 0);
		case TimeZone_EuropeMoscow:        return posix_time + hourMinutesToSecond(3, 0);
		case TimeZone_EuropeSarajevo:      standard_eu_offset =  1; is_eu_zone = 1; break;

		case TimeZone_PacificAuckland:     standard_au_offset_h = 12; standard_au_offset_m = 0;  is_au_zone = 1; break;
		case TimeZone_PacificFiji:         return posix_time + hourMinutesToSecond(12, 0);
		case TimeZone_PacificGuam:         return posix_time + hourMinutesToSecond(10, 0);
		case TimeZone_PacificHonolulu:     return posix_time + hourMinutesToSecond(-10, 0);
		case TimeZone_PacificMajuro:       return posix_time + hourMinutesToSecond(12, 0);
		case TimeZone_PacificMidway:       return posix_time + hourMinutesToSecond(-11, 0);
		case TimeZone_PacificNoumea:       return posix_time + hourMinutesToSecond(11, 0);
		case TimeZone_PacificTongatapu:    return posix_time + hourMinutesToSecond(13, 0);
        case TimeZone_AmericaSt_Johns: {
            int march_dst_day = 14 - get_day_of_week(year, 3, 1);
            int nov_dst_day = 7 - get_day_of_week(year, 11, 1);
            int is_dst = (month > 3 && month < 11) || 
                        (month == 3 && (day > march_dst_day || (day == march_dst_day && hour >= 7))) || 
                        (month == 11 && (day < nov_dst_day || (day == nov_dst_day && hour < 6)));
            return posix_time + (is_dst ? hourMinutesToSecond(-2, -30) : hourMinutesToSecond(-3, -30));            
        }
        default: return 0;
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

    return 0;
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