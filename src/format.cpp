#include <string>
#include <vector>

#include "format.h"

using std::string;


string Format::ElapsedTime(long seconds)
{
    std::string rv = "";
    std::vector <std::string> dhms;             //days, hours, minutes, seconds
    long quot;
    long rem = seconds;
    std::vector <long> sp{86400, 3600, 60, 1};  //seconds per: day, hour, minute, second
    for (int i = 0; i < 4; i++)
    {
        quot = rem / sp[i];
        rem = rem - quot * sp[i];
        //convert quotient to string and add to dhms vector
        dhms.emplace_back(std::to_string(quot));
        //add leading zero if applicable
        if (quot < 10) dhms[i] = "0" + dhms[i];
    }
    rv = dhms[1] + ":" + dhms[2] + ":" + dhms[3];
    return rv;
}