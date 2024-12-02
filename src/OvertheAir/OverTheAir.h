#include <dev_config.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>

class OverTheAir
{
private:
    WiFiClientSecure client;
    WiFiClient client_not_secure;

public:
    OverTheAir(/* args */);
    bool update(String server, int port, String endpoint, bool secure = true);
};
