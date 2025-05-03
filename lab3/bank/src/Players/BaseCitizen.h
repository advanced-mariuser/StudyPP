#ifndef BANK_BASECITIZEN_H
#define BANK_BASECITIZEN_H

#include "ICitizen.h"
#include "utils.h"
#include <syncstream>

class BaseCitizen : public ICitizen
{
protected:
    static void SafePrint(const std::string& message)
    {
        std::osyncstream(std::cout) << message;
    }

    static void SafePrintError(const std::string& message)
    {
        std::osyncstream(std::cerr) << message;
    }

public:
};

#endif //BANK_BASECITIZEN_H