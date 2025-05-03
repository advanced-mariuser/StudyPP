#ifndef BANK_ICITIZEN_H
#define BANK_ICITIZEN_H

class ICitizen
{
public:
    virtual void Run() = 0;

    virtual ~ICitizen() = default;
};

#endif //BANK_ICITIZEN_H
