#include "user.h"

User::User(const std::string &name, const std::string &mbox)
    : name_(name), mbox_(mbox), addr_(name + "@localhost") {}
